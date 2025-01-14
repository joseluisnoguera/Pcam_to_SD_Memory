#include "xparameters.h"

#include "platform/platform.h"
#include "ov5640/OV5640.h"
#include "ov5640/ScuGicInterruptController.h"
#include "ov5640/PS_GPIO.h"
#include "ov5640/AXI_VDMA.h"
#include "ov5640/PS_IIC.h"
#include "sd/SD_Driver.h"

#include "MIPI_D_PHY_RX.h"
#include "MIPI_CSI_2_RX.h"

#include <xgpiops.h>
#include "xtime_l.h"

/**
 * TODO Agregar creaci�n de buffers din�micos para guardar las im�genes que se van capturando previo su guardado en la SD
 * El driver (cambiar el nombre) de la SD puede generar buffers din�micamente
 * Va a necesitar que el guardado en la SD no sea bloqueante �c�mo?
 */

#define IRPT_CTL_DEVID 		XPAR_PS7_SCUGIC_0_DEVICE_ID
#define GPIO_DEVID			XPAR_PS7_GPIO_0_DEVICE_ID
#define GPIO_DEVIDLED		XPAR_XGPIOPS_0_DEVICE_ID
#define GPIO_IRPT_ID		XPAR_PS7_GPIO_0_INTR
#define CAM_I2C_DEVID		XPAR_PS7_I2C_0_DEVICE_ID
#define CAM_I2C_IRPT_ID		XPAR_PS7_I2C_0_INTR
#define VDMA_DEVID			XPAR_AXIVDMA_0_DEVICE_ID
#define VDMA_MM2S_IRPT_ID	XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR
#define VDMA_S2MM_IRPT_ID	XPAR_FABRIC_AXI_VDMA_0_S2MM_INTROUT_INTR
#define CAM_I2C_SCLK_RATE	100000

#define DDR_BASE_ADDR		XPAR_DDR_MEM_BASEADDR
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x0A000000)

#define GAMMA_BASE_ADDR     XPAR_AXI_GAMMACORRECTION_0_BASEADDR
#define SOME XPAR_AXI_GPIO_0_BASEADDR

#define LEDPIN 7
#define BUTTONPIN 50 //13

using namespace digilent;

struct Measurements {
	u32 width;
	u32 height;
};


const int BUFF_SZ_1080p = 6220800; //1920x1080x3
const int BUFF_SZ_720p = 2764800; //1280x720x3

Resolution default_resolution;
int default_gamma_correction;
OV5640_cfg::awb_t default_white_balance;

void pipeline_mode_change(AXI_VDMA<ScuGicInterruptController>& vdma_driver, OV5640& cam, VideoOutput& vid, Resolution res, OV5640_cfg::mode_t mode)
{
	vdma_driver.resetWrite();
	//Bring up input pipeline back-to-front
	{
		MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
		MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
		cam.reset();
	}

	{
		vdma_driver.configureWrite(timing[static_cast<int>(res)].h_active, timing[static_cast<int>(res)].v_active);
		Xil_Out32(GAMMA_BASE_ADDR, default_gamma_correction); // Set Gamma correction factor from 1 to 5 (higher is more white)
		cam.init();
	}

	{
		vdma_driver.enableWrite();
		MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, CR_ENABLE_MASK);
		MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, CR_ENABLE_MASK);
		cam.set_mode(mode);
		cam.set_awb(default_white_balance);
		//cam.set_awb(OV5640_cfg::awb_t::AWB_SIMPLE);
		//cam.set_awb(OV5640_cfg::awb_t::AWB_ADVANCED);
	}

	//Bring up output pipeline back-to-front
	{
		vid.reset();
		vdma_driver.resetRead();
	}

	{
		vid.configure(res);
		vdma_driver.configureRead(timing[static_cast<int>(res)].h_active, timing[static_cast<int>(res)].v_active);
	}

	{
		vid.enable();
		vdma_driver.enableRead();
	}
}

int main()
{
	init_platform();

	// Variables para captura
	int b_sz = 0;
	SD_Driver sd_store;

	int* config = sd_store.get_config();
	if (config[0] == 0)
		default_resolution = Resolution::R1280_720_60_PP;
	else
		default_resolution = Resolution::R1920_1080_60_PP;

	if (config[1] == 0)
		default_white_balance = OV5640_cfg::awb_t::AWB_DISABLED;
	else if (config[1] == 1)
		default_white_balance = OV5640_cfg::awb_t::AWB_SIMPLE;
	else
		default_white_balance = OV5640_cfg::awb_t::AWB_ADVANCED;

	if (config[2] <= 5 && config[2] >= 1)
		default_gamma_correction = config[2];
	else
		default_gamma_correction = 1; //No gamma correction


	XGpioPs input, output;
	XGpioPs_Config *ConfigPtr;
	ConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	int Status = XGpioPs_CfgInitialize(&output, ConfigPtr, ConfigPtr ->BaseAddr);

	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	Status = XGpioPs_CfgInitialize(&input, ConfigPtr, ConfigPtr ->BaseAddr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	//Inicializaci�n
	XGpioPs_SetDirectionPin(&output, LEDPIN, 1);
	XGpioPs_SetOutputEnablePin(&output, LEDPIN, 1);
	XGpioPs_SetDirectionPin(&input, BUTTONPIN, 0x0);
	XGpioPs_WritePin(&output, LEDPIN, 0x0);

	//Captura y salida por hdmi
	ScuGicInterruptController irpt_ctl(IRPT_CTL_DEVID);
	PS_GPIO<ScuGicInterruptController> gpio_driver(GPIO_DEVID, irpt_ctl, GPIO_IRPT_ID);
	PS_IIC<ScuGicInterruptController> iic_driver(CAM_I2C_DEVID, irpt_ctl, CAM_I2C_IRPT_ID, 100000);
	OV5640 cam(iic_driver, gpio_driver);

	AXI_VDMA<ScuGicInterruptController> vdma_driver(VDMA_DEVID, MEM_BASE_ADDR, irpt_ctl, VDMA_MM2S_IRPT_ID, VDMA_S2MM_IRPT_ID);
	VideoOutput vid(XPAR_VTC_0_DEVICE_ID, XPAR_VIDEO_DYNCLK_DEVICE_ID);

	xil_printf("\ninit\n");

	if (default_resolution == Resolution::R1920_1080_60_PP){
		b_sz = BUFF_SZ_1080p;
		pipeline_mode_change(vdma_driver, cam, vid, default_resolution, OV5640_cfg::mode_t::MODE_1080P_1920_1080_30fps);
	}
	else if (default_resolution == Resolution::R1280_720_60_PP){
		b_sz = BUFF_SZ_720p;
		pipeline_mode_change(vdma_driver, cam, vid, default_resolution, OV5640_cfg::mode_t::MODE_720P_1280_720_60fps);
	}

	u8* buffer = new u8[b_sz];

	xil_printf("Video init done.\r\n");

//	XTime tStart, tEnd;

	int button_value;
	XGpioPs_WritePin(&output, LEDPIN, 0);

	while (1) {
		button_value = XGpioPs_ReadPin(&input,BUTTONPIN);
		if (button_value == 1){
			xil_printf("Capturando ... ");
			XGpioPs_WritePin(&output, LEDPIN, 1);

//			XTime_GetTime(&tStart);
			vdma_driver.copyDataFrame(buffer, b_sz);
			xil_printf("copiado a memoria ... ");
//			XTime_GetTime(&tEnd);
//			printf("Copy t=%15.5lf sec\n",(long double)((tEnd - tStart) *2)/(long double)XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ);

//			XTime_GetTime(&tStart);
			sd_store.SD_Transfer_write((u32) buffer, (u32)b_sz);
//			XTime_GetTime(&tEnd);
//			printf("SD Transfer t=%15.5lf sec\n",(long double)((tEnd - tStart) *2)/(long double)XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ);

			XGpioPs_WritePin(&output, LEDPIN, 0);
			xil_printf("captura finalizada \n\n");
		}
	}

	cleanup_platform();
	return 0;
}
