// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2018.2.2 (win64) Build 2348494 Mon Oct  1 18:25:44 MDT 2018
// Date        : Tue Dec 10 10:56:08 2019
// Host        : LAPTOP-OEOHUQ1P running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix -prefix
//               decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix_ system_edge_detect_0_0_stub.v
// Design      : system_edge_detect_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg400-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* x_core_info = "edge_detect,Vivado 2018.2.2" *)
module decalper_eb_ot_sdeen_pot_pi_dehcac_xnilix(stream_in_TVALID, stream_in_TREADY, 
  stream_in_TDATA, stream_in_TKEEP, stream_in_TSTRB, stream_in_TUSER, stream_in_TLAST, 
  stream_in_TID, stream_in_TDEST, stream_out_TVALID, stream_out_TREADY, stream_out_TDATA, 
  stream_out_TKEEP, stream_out_TSTRB, stream_out_TUSER, stream_out_TLAST, stream_out_TID, 
  stream_out_TDEST, ap_clk, ap_rst_n)
/* synthesis syn_black_box black_box_pad_pin="stream_in_TVALID,stream_in_TREADY,stream_in_TDATA[23:0],stream_in_TKEEP[2:0],stream_in_TSTRB[2:0],stream_in_TUSER[0:0],stream_in_TLAST[0:0],stream_in_TID[0:0],stream_in_TDEST[0:0],stream_out_TVALID,stream_out_TREADY,stream_out_TDATA[23:0],stream_out_TKEEP[2:0],stream_out_TSTRB[2:0],stream_out_TUSER[0:0],stream_out_TLAST[0:0],stream_out_TID[0:0],stream_out_TDEST[0:0],ap_clk,ap_rst_n" */;
  input stream_in_TVALID;
  output stream_in_TREADY;
  input [23:0]stream_in_TDATA;
  input [2:0]stream_in_TKEEP;
  input [2:0]stream_in_TSTRB;
  input [0:0]stream_in_TUSER;
  input [0:0]stream_in_TLAST;
  input [0:0]stream_in_TID;
  input [0:0]stream_in_TDEST;
  output stream_out_TVALID;
  input stream_out_TREADY;
  output [23:0]stream_out_TDATA;
  output [2:0]stream_out_TKEEP;
  output [2:0]stream_out_TSTRB;
  output [0:0]stream_out_TUSER;
  output [0:0]stream_out_TLAST;
  output [0:0]stream_out_TID;
  output [0:0]stream_out_TDEST;
  input ap_clk;
  input ap_rst_n;
endmodule
