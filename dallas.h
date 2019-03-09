
#include <DallasTemperature.h>
#include <OneWire.h>


// temperatuur meting met Dallas
#define ONE_WIRE_BUS 14  //  gpio14 op de NodeMCU 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

boolean init_dallas() {

  
  //init temperature sensors
  sensors.begin();  //Dallas temperatuur sensor
 //show(  "Temp sensors: ", "start");
  //lees de temperatuur sensor uit
  sensors.requestTemperatures();// Dallas temperatuur sensor
 // Tbinnen_val =   sensors.getTempCByIndex(0);
 // show(  "Temp binnen: ", String(Tbinnen_val));
//  Tbuit_val =     sensors.getTempCByIndex(1);
 // show(  "Temp buiten: ", String(Tbuit_val));

}

boolean read_dallas(float *t) {
    //lees de temperatuur sensor uit
    sensors.requestTemperatures();// Dallas temperatuur sensor
    t[0] =   sensors.getTempCByIndex(0);
    t[1]   = sensors.getTempCByIndex(1);
}
