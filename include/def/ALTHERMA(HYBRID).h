#include "labeldef.h"
//  This file is a definition file for ESPAtherma
//  uncomment each value you want to query for your installation.


LabelDef labelDefs[] = {
  // ---- Outdoor / climate ----
  {0x20,0,105,2,1,"Outdoor air temp."},
  {0x61,14,105,2,1,"Outdoor Ambient or Ext Sensor"},

  // ---- General status ----
  {0x10,0,217,1,-1,"Operation Mode"},
  {0x60,2,315,1,-1,"I/U operation mode"},
  {0x10,1,307,1,-1,"Thermostat 2 ON/OFF"},
  {0x60,2,303,1,-1,"Thermostat 1 ON/OFF"},
  {0x62,2,303,1,-1,"Space heating Operation ON/OFF"},
  {0x10,1,304,1,-1,"Defrost Operation"},
  {0x10,4,203,1,-1,"Error type"},
  {0x10,5,204,1,-1,"Error Code"},

  // ---- Performance / estimated power ----
  {0x30,0,152,1,-1,"INV frequency (rps)"},
  {0x21,0,105,2,-1,"INV primary current (A)"},
  {0x21,4,101,2,-1,"Voltage (V)"},
  {0x62,9,105,2,-1,"Flow sensor (l/min)"},
  {0x61,2,105,2,1,"Outlet Water Heat Exch. Temp. (R1T)"},
  {0x61,8,105,2,1,"Inlet water temp.(R4T)"},
  {0x60,9,105,2,1,"LW setpoint (main)"},
  {0x62,5,105,2,1,"RT setpoint"},
  {0x61,12,105,2,1,"RT Temp."},

  // ---- Hydraulic / backup heaters ----
  {0x62,11,105,1,2,"Water pressure"},
  {0x62,12,152,1,-1,"Water pump signal (0:max-100:stop)"},
  {0x60,12,301,1,-1,"Water pump operation"},
  {0x60,11,307,1,-1,"Water flow switch"},
  {0x60,12,304,1,-1,"BUH Step1"},
  {0x60,12,303,1,-1,"BUH Step2"},
  {0x60,12,305,1,-1,"BSH"},
  {0x60,12,302,1,-1,"BPH"},

  // ---- Hybrid / boiler switching ----
  {0x64,2,316,1,-1,"Hybrid Op. Mode"},
  {0x64,2,303,1,-1,"Boiler Operation Demand"},
  {0x64,3,105,2,-1,"BE_COP"},
  {0x64,5,105,2,1,"Hybrid Heating Target Temp."},
  {0x64,7,105,2,1,"Boiler Heating Target Temp."},


};
