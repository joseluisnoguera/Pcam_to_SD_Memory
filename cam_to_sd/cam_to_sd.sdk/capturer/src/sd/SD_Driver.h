/*
 * SDDriver.h
 *
 *  Created on: 2 ago. 2019
 *      Author: Jose Noguera
 */


#ifndef SRC_SD_SDDRIVER_H_
#define SRC_SD_SDDRIVER_H_

#include "ff.h"
#include "xdevcfg.h"
#include "xil_printf.h"

#include <string>

#define CONFIG_DATA_AMOUNT 4

class SD_Driver {

private:
	u32 *config_buffer; //File counter, resolution, white balance and gamma correction
	FATFS fatfs;
	const std::string CONFIG_FILE_NAME = "config.txt";
	const std::string FILE_EXT = ".bin";
	const std::string BASE_FILE_NAME = "cap";

	int SD_Init();
	void get_error_msg(FRESULT rc);
	void set_counter();
	void update_counter();

public:
	SD_Driver();
	virtual ~SD_Driver() { }
	int SD_Transfer_read(char *FileName,u32 DestinationAddress,u32 ByteLength);
	int SD_Transfer_write(u32 SourceAddress,u32 ByteLength);
	int SD_Transfer_write(char *FileName, u32 SourceAddress,u32 ByteLength);
	int* get_config();

};

SD_Driver::SD_Driver(){	SD_Init(); }

int SD_Driver::SD_Init()
{
    FRESULT rc;

    rc = f_mount(&fatfs,"",0);
    if(rc)
    {
        get_error_msg(rc);
        return XST_FAILURE;
    }
    config_buffer = new u32[CONFIG_DATA_AMOUNT];
    set_counter();
    return XST_SUCCESS;
}

void SD_Driver::set_counter() {
	char *FileName = (char*) CONFIG_FILE_NAME.c_str();
	if (SD_Transfer_read(FileName, (u32)config_buffer, (u32)1) == XST_FAILURE){
		xil_printf("Creando archivo de configuración ... \n");
		for (int i = 0; i < CONFIG_DATA_AMOUNT; i++)
			config_buffer[i] = 0; // Por defecto todo 0
		SD_Transfer_write(FileName, (u32)config_buffer, (u32)CONFIG_DATA_AMOUNT);
		xil_printf("%d",CONFIG_DATA_AMOUNT);
	}
}

void SD_Driver::get_error_msg (FRESULT rc)
{
	int i = rc;
	std::string msg ="ERROR: ";
	switch(i)
	{
		case 1:
			msg += "A hard error occurred in the low level disk I/O layer";
			break;
		case 2:
			msg += "Assertion failed";
			break;
		case 3:
			msg += "The physical drive cannot work";
			break;
		case 4:
			msg += "Could not find the file";
			break;
		case 5:
			msg += "Could not find the path";
			break;
		case 6:
			msg += "The path name format is invalid";
			break;
		case 7:
			msg += "Access denied due to prohibited access or directory full";
			break;
		case 8:
			msg += "Access denied due to prohibited access";
			break;
		case 9:
			msg += "The file/directory object is invalid";
			break;
		case 10:
			msg += "The physical drive is write protected";
			break;
		case 11:
			msg += "The logical drive number is invalid";
			break;
		case 12:
			msg += "The volume has no work area";
			break;
		case 13:
			msg += "There is no valid FAT volume";
			break;
		case 14:
			msg += "The f_mkfs() aborted due to any problem";
			break;
		case 15:
			msg += "Could not get a grant to access the volume within defined period";
			break;
		case 16:
			msg += "The operation is rejected according to the file sharing policy";
			break;
		case 17:
			msg += "LFN working buffer could not be allocated";
			break;
		case 18:
			msg += "Number of open files > FF_FS_LOCK";
			break;
		case 19:
			msg += "Given parameter is invalid";
			break;
	}
	msg += "\n";
	char * cstr = new char [msg.length()+1];
	std::strcpy (cstr, msg.c_str());
	xil_printf(cstr);
}

int SD_Driver::SD_Transfer_read(char *FileName, u32 DestinationAddress,u32 ByteLength)
{
    FIL fil;
    FRESULT rc;
    UINT br;

    rc = f_open(&fil,FileName,FA_READ);
    if(rc)
    {
    	get_error_msg(rc);
        return XST_FAILURE;
    }
    rc = f_lseek(&fil, 0);
    if(rc)
    {
    	get_error_msg(rc);
        return XST_FAILURE;
    }
    rc = f_read(&fil, (void*)DestinationAddress,ByteLength,&br);
    if(rc)
    {
    	get_error_msg(rc);
        return XST_FAILURE;
    }
    rc = f_close(&fil);
    if(rc)
    {
    	get_error_msg(rc);
        return XST_FAILURE;
    }
    return XST_SUCCESS;
}

int SD_Driver::SD_Transfer_write(char *FileName, u32 SourceAddress,u32 ByteLength)
{
    FIL fil;
    FRESULT rc;
    UINT bw;

    rc = f_open(&fil,FileName,FA_CREATE_ALWAYS | FA_WRITE);
    if(rc)
    {
    	get_error_msg(rc);
        return XST_FAILURE;
    }
    rc = f_lseek(&fil, 0);
    if(rc)
    {
    	get_error_msg(rc);
        return XST_FAILURE;
    }
    rc = f_write(&fil,(void*) SourceAddress,ByteLength,&bw);
    if(rc)
    {
    	get_error_msg(rc);
        return XST_FAILURE;
    }
    rc = f_close(&fil);
    if(rc){
    	get_error_msg(rc);
        return XST_FAILURE;
    }
    return XST_SUCCESS;
}

int SD_Driver::SD_Transfer_write(u32 SourceAddress,u32 ByteLength)
{
	std::string file_name = BASE_FILE_NAME + std::to_string(config_buffer[0]) + FILE_EXT;
	config_buffer[0]++;
    char *FileName = (char*) file_name.c_str();
    update_counter();
    return SD_Transfer_write(FileName, SourceAddress, ByteLength);
}

void SD_Driver::update_counter(){
	char *FileName = (char*) CONFIG_FILE_NAME.c_str();
	SD_Transfer_write(FileName, (u32)config_buffer, (u32)CONFIG_DATA_AMOUNT);
}

int* SD_Driver::get_config() {
	int *tmp;
	tmp = new int[CONFIG_DATA_AMOUNT];
	for (int i = 1; i < CONFIG_DATA_AMOUNT; i++)
		tmp[i-1] = config_buffer[i];
	return tmp;
}

#endif /* SRC_SD_SDDRIVER_H_ */
