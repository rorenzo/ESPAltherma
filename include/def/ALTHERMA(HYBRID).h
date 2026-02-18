#include "labeldef.h"
//  This file is a definition file for ESPAtherma
//  uncomment each value you want to query for your installation.


LabelDef labelDefs[] = {
  // ---- Outdoor / base ----
  {0x20,0,105,2,1,"Outdoor air temp."},

  // ---- Stato generale ----
  {0x10,0,217,1,-1,"Operation Mode"},
  {0x10,1,307,1,-1,"Thermostat ON/OFF"},
  {0x10,1,304,1,-1,"Defrost Operation"},
  {0x10,4,203,1,-1,"Error type"},
  {0x10,5,204,1,-1,"Error Code"},

  // ---- Compressore (proxy migliore) ----
  {0x30,0,152,1,-1,"INV frequency (rps)"},

  // (consigliato) separatore per stabilità
  {0x00,0,995,1,-1,"NextDataGrid"},

  // ---- Hydro / acqua ----
  {0x61,2,105,2,1,"Outlet Water Heat Exch. Temp. (R1T)"},
  {0x61,8,105,2,1,"Inlet water temp.(R4T)"},
  {0x60,9,105,2,1,"LW setpoint (main)"},

  // ---- Idraulica ----
  {0x62,9,105,2,-1,"Flow sensor (l/min)"},
  {0x62,11,105,1,2,"Water pressure"},
  {0x62,12,152,1,-1,"Water pump signal (0:max-100:stop)"},
  {0x60,12,301,1,-1,"Water pump operation"},
  {0x62,2,303,1,-1,"Space heating Operation ON/OFF"},

  // ---- ACS ----
  {0x61,10,105,2,1,"DHW tank temp. (R5T)"},
  {0x60,7,105,2,1,"DHW setpoint"},
  {0x60,12,306,1,-1,"3way valve(On:DHW_Off:Space)"},

  // ---- Hybrid ----
  {0x64,2,316,1,-1,"Hybrid Op. Mode"},
  {0x64,2,303,1,-1,"Boiler Operation Demand"},
  {0x64,2,302,1,-1,"Boiler DHW Demand"},
  {0x64,5,105,2,1,"Hybrid Heating Target Temp."},
  {0x64,7,105,2,1,"Boiler Heating Target Temp."},


};
