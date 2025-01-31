
#if defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ADS1118.h>
    // ESP 12-F SPI Definition
  #define PIN_SPI_SS   (9)
  #define PIN_SPI_MOSI (13)
  #define PIN_SPI_MISO (10)
  #define PIN_SPI_SCK  (14)

  // static const uint8_t SS    = PIN_SPI_SS;
  // static const uint8_t MOSI  = PIN_SPI_MOSI;
  // static const uint8_t MISO  = PIN_SPI_MISO;
  // static const uint8_t SCK   = PIN_SPI_SCK;
    ADS1118 ads1118(PIN_SPI_SS);
#endif
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <ESPUI.h>
#include <map>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;


//Creating an ADS1118 object (object's name is ads1118)


const char * ssid = "Exellent_outdoor";
const char * APId = "ESP-WaterControl";
const char * password = "Exellenz";
const char * hostname = "ESP-WaterControl";
uint16_t valve0;
uint16_t valve1;
uint16_t valve2;
uint16_t valve3;
uint16_t status;
uint16_t LOG;
uint16_t graphId;
uint16_t gauge1;
int pin = 0;
long timers[] = {0,0,0,0,0};
int rain_stop[] = {80,80,80,80,80};
int rain_start[] = {30, 30,30,30,30};
long max_raintime[] = {1*60*1000, 2*60*1000, 3*60*1000, 4*60*1000};
int measuredVal = 0;
int soilMoisturePercent = 50;
int previousSoilMoisturePercent = 50;
int selectedZone = 0;
std::map<std::string, int> zone_assoc { {"Zone 1", 0}, {"Zone 2", 1}, {"Zone 3", 2}, {"Zone 4", 3} };
std::map<int,uint16_t> valvesUI;
#if defined(ESP32)
  int sensors[] = {35,34,39,36};
  int relayPin[] = {0,15,2,4};
  int buttonPin[] = {11,10,9,13};
  int airVal = 3478;
  int waterVal = 772;
#else
  int sensors[] = {ads1118.AIN_0,ads1118.AIN_1,ads1118.AIN_2,ads1118.AIN_3};
  int relayPin[] = {D0,D5,D6,D7};
  int airVal = 300;
  int waterVal = 1326;

#endif
  



int label_to_int(const char * value)
{
 return zone_assoc[value];
}

void logfunction(String text) {
  Serial.println(text);
  ESPUI.updateControlValue(LOG, text);
}

void measureSoil(int zone) {
  measuredVal = analogRead(sensors[zone]);
  soilMoisturePercent = map(measuredVal, airVal, waterVal, 0, 100);
  if (soilMoisturePercent > 100) {
    logfunction("Error Zone"+ String(zone)+":  Moisture too high: "+ String(measuredVal)+"! Please calibrate." );
  } else if (soilMoisturePercent < 0) {
    logfunction("Error Zone "+ String(zone)+": Moisture too low: "+ String(measuredVal)+"! Please calibrate.");
  } else if (soilMoisturePercent <= 100 &&soilMoisturePercent >= 0) {
    Serial.print("Feuchtigkeit Zone "+ String(zone)+": ");
    Serial.print(soilMoisturePercent);
    Serial.println(" %");
    ESPUI.addGraphPoint(graphId, soilMoisturePercent);
    ESPUI.updateGauge(gauge1, soilMoisturePercent, -1);
    if (soilMoisturePercent != previousSoilMoisturePercent) {
      previousSoilMoisturePercent = soilMoisturePercent;
    }

  }
  // timers[0] =0;
}

void rawVals(Control * sender, int value) {

  measuredVal = analogRead(sensors[selectedZone]);

  logfunction("Current value: "+ String(measuredVal));
  Serial.print("Current Air value: ");
  Serial.println(airVal);
  Serial.print("Current Water value: ");
  Serial.println(waterVal);
  

}
void debug_analog(){
for (int i = 0; i < 4; i++) {
  measuredVal = analogRead(sensors[i]);

  logfunction("analog redout channel: "+ String(i) +"  val:" + String(measuredVal));
  // logfunction(String(ads1118.getTemperature(),6)+" C");
  }
  delay(1000);
}


void measureAir(Control * sender, int type) {
  switch (type) {
    case B_DOWN:
      measuredVal = analogRead(sensors[selectedZone]);
      airVal = measuredVal;
      logfunction("NEW AIR value: " + String(airVal));
      
      break;
  }
}

void measureWater(Control * sender, int type) {
  switch (type) {
    case B_DOWN:
      measuredVal = analogRead(sensors[selectedZone]);
      waterVal = measuredVal;
      logfunction("NEW WATER value: " + String(waterVal));
      break;
  }
}

void clearGraph(Control * sender, int type) {
  logfunction("Graph should be clean now");
  ESPUI.clearGraph(graphId);

}

void slider1_max(Control * sender, int type) {
  
  rain_stop[0] = sender->value.toInt();
  logfunction("Beregnungs Zone " +String(sender->id) + " obere grenze : "+ rain_stop[0] );
}
void slider1_min(Control * sender, int type) {
  rain_start[0] = sender->value.toInt();
  logfunction("Beregnungs Zone " +String(sender->id) + " untere grenze : "+ rain_start[0] );
}
void slider2_max(Control * sender, int type) {
  rain_stop[1] = sender->value.toInt();
  logfunction("Beregnungs Zone " +String(sender->id) + " obere grenze : "+ rain_stop[1] );
}
void slider2_min(Control * sender, int type) {
  rain_start[1] = sender->value.toInt();
  logfunction("Beregnungs Zone " +String(sender->id) + " untere grenze : "+ rain_start[1] );
}
void slider3_max(Control * sender, int type) {
  rain_stop[2] = sender->value.toInt();
  logfunction("Beregnungs Zone " +String(sender->id) + " obere grenze : "+ rain_stop[2] );
}
void slider3_min(Control * sender, int type) {
  rain_start[2] = sender->value.toInt();
  logfunction("Beregnungs Zone " +String(sender->id) + " untere grenze : "+ rain_start[2] );
}
void slider4_max(Control * sender, int type) {
  rain_stop[3] = sender->value.toInt();
  logfunction("Beregnungs Zone " +String(sender->id) + " obere grenze : "+ rain_stop[3] );
}
void slider4_min(Control * sender, int type) {
  rain_start[3] = sender->value.toInt();
  logfunction("Beregnungs Zone " +String(sender->id) + " untere grenze : "+ rain_start[3] );
}
void max_raintime_function(Control * sender, int type) {
  max_raintime[(sender->id-27)] = sender->value.toInt()*60*1000;
  logfunction("raintime Zone " +String(sender->id-32) +" gesetzt: "+ max_raintime[(sender->id-27)] );
}


void ValveFunction(Control * sender, int type) {
  int valveNr = label_to_int(sender->label);
   pin = relayPin[valveNr];
   switch (type)
    {
    case S_ACTIVE:
        logfunction("Valve " + String(valveNr+1) + " is ON");
        digitalWrite(pin, HIGH);
        timers[valveNr+1] = millis();
        
        break;

    case S_INACTIVE:
        logfunction("Valve " + String(valveNr+1) + " is OFF");
        digitalWrite(pin, LOW);
        timers[valveNr+1] = 0;
    }
 }


void ValveClose(int valvenr){
    	timers[valvenr+1] =0;
      logfunction("Valve should close: "+String(valvenr));
      pin = relayPin[valvenr];
      ESPUI.updateSwitcher(valvesUI[valvenr],false);
      digitalWrite(pin, LOW);
      
}
void selectZone(Control* sender, int value)
{
   logfunction("Select: ID: "+ (sender->id));
    logfunction("Value: " + (sender->value));
    selectedZone = label_to_int(sender->value.c_str())-1;
}
void tabcallback(Control* sender, int value)
{
   
   
}




void debug(){
    for (int a=0; a<= 4; a++) {
    logfunction("Timer"+String(a)+": "+ String(timers[a]));
    }
    for (int b=0; b<= 3; b++) {
    logfunction("Pin"+String(b)+": "+ String(digitalRead(relayPin[b])));
    }
}

void timer_checkFunction(int zone){
  if ((millis() - timers[zone+1] > max_raintime[zone]) && (digitalRead(relayPin[zone]) == HIGH)) {
  ValveClose(zone);
  timers[zone+1] = 0;
  }

}

void  timer_function(void){
  
  if (millis() - timers[0] > 15000) {
  for (size_t i = 0; i <= 3; i++)
  {
       measureSoil(i);
      
      
  }
  debug();
  timers[0] = millis();
  }
  for (size_t x = 0; x <= 3; x++)
  {
    timer_checkFunction(x);
  }
  
  
}

void IRAM_ATTR button_press(){
  for (int i=0; i< 4; i++) {
    if (digitalRead(buttonPin[i]) == LOW) {
      ValveClose(i);
    }
  }
  

}


     
// TODO Start on MOISTURE AND STOP ON MOISTURE

// TODO STORE SETTINGS AS JSON AND LOAD SETTINGS ON BOOT
  void saveValues(){
  
  }
  void loadValues(){
  }



void setup(void) {
  
  Serial.begin(115200);
  // SETUP Pin Modes 
  
  for (int i=0; i< 4; i++) {
  pinMode(relayPin[i], OUTPUT);
  pinMode(sensors[i], INPUT_PULLUP);
  pinMode(buttonPin[i], INPUT_PULLUP);
  attachInterrupt(buttonPin[i],button_press, FALLING);
  }
  

  #if defined(ESP32)
  WiFi.setHostname(hostname);
  #else
    WiFi.hostname(hostname);
  #endif
  

  // try to connect to existing network
  WiFi.begin(ssid, password);
  Serial.print("\n\nTry to connect to existing network");

  {
    uint8_t timeout = 20;

    // Wait for connection, 10s timeout
    do {
      delay(500);
      Serial.print(".");
      timeout--;
    } while (timeout &&WiFi.status() != WL_CONNECTED);

    // not connected -> create hotspot
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("\n\nCreating hotspot");

      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      WiFi.softAP(APId);

      timeout = 5;

      do {
        delay(500);
        Serial.print(".");
        timeout--;
      } while (timeout);
    }
  }

  dnsServer.start(DNS_PORT, "*", apIP);

  Serial.println("\n\nWiFi parameters:");
  Serial.print("Mode: ");
  Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
  Serial.print("IP address: ");
  Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
  // Start ADC config 
  // ads1118.begin();
/* Changing the sampling rate. RATE_8SPS, RATE_16SPS, RATE_32SPS, RATE_64SPS, RATE_128SPS, RATE_250SPS, RATE_475SPS, RATE_860SPS*/
    // ads1118.setSamplingRate(ads1118.RATE_8SPS);

    /* Changing the input selected. Differential inputs: DIFF_0_1, DIFF_0_3, DIFF_1_3, DIFF_2_3. Single ended input: AIN_0, AIN_1, AIN_2, AIN_3*/
    // ads1118.setInputSelected(ads1118.AIN_0);

  uint16_t tab0 = ESPUI.addControl(ControlType::Tab, "Control", "Control", None, None, &tabcallback);
  uint16_t tab1 = ESPUI.addControl(ControlType::Tab, "LiveData", "Data", None, None, &tabcallback);
  uint16_t tab2 = ESPUI.addControl(ControlType::Tab, "Calibration", "Calibration", None, None, &tabcallback);
  // uint16_t tab3 = ESPUI.addControl(ControlType::Tab, "Configuration", "Setttings");
  uint16_t tab4 = ESPUI.addControl(ControlType::Tab, "Configuration Z1", "ZONE 1", None, None, &tabcallback);
  uint16_t tab5 = ESPUI.addControl(ControlType::Tab, "Configuration Z2", "ZONE 2", None, None, &tabcallback);
  uint16_t tab6 = ESPUI.addControl(ControlType::Tab, "Configuration Z3", "ZONE 3", None, None, &tabcallback);
  uint16_t tab7 = ESPUI.addControl(ControlType::Tab, "Configuration Z4", "ZONE 4", None, None, &tabcallback);
  // auto select_cal = ESPUI.addControl( ControlType::Select, "Zone to Calibrate", "Zone1", ControlColor::Alizarin, tab2, &selectZone);
  // shown above all tabs
  status = ESPUI.addControl(ControlType::Label, "Status:", "All fine", ControlColor::Turquoise);
  LOG = ESPUI.addControl(ControlType::Label, "LOG:", "-", ControlColor::Turquoise);

  // tab 0
  // static String Valves[] {"Zone 1", "Zone 2", "Zone 3", "Zone 4"};
  // for(auto const& v : Valves) {
  //   ESPUI.addControl(ControlType::Switcher, v.c_str(), "Open", ControlColor::Wetasphalt, tab0, &ValveFunction);
  // }
  valvesUI[0] =   ESPUI.addControl(ControlType::Switcher, "Zone 1", "0", ControlColor::Wetasphalt, tab0, &ValveFunction);
  valvesUI[1] =   ESPUI.addControl(ControlType::Switcher, "Zone 2", "0", ControlColor::Wetasphalt, tab0, &ValveFunction);
  valvesUI[2] =   ESPUI.addControl(ControlType::Switcher, "Zone 3", "0", ControlColor::Wetasphalt, tab0, &ValveFunction);
  valvesUI[3] =   ESPUI.addControl(ControlType::Switcher, "Zone 4", "0", ControlColor::Wetasphalt, tab0, &ValveFunction);

  // tab 1
  graphId = ESPUI.addControl(ControlType::Graph, "Sensor1", "Humidity", ControlColor::Wetasphalt, tab1);
  gauge1 = ESPUI.addControl(ControlType::Gauge, "Sensor1", "Humidity:", ControlColor::Carrot, tab1);
  ESPUI.addControl(ControlType::Button, "Clear Graph", "Clear", ControlColor::Peterriver, tab1, &clearGraph);

  
  // tab 2
  static String optionValues[] {"Zone 1", "Zone 2", "Zone 3", "Zone 4"};
  auto zoneSelect = ESPUI.addControl(Select, "Zone Selector", "Z-Selector", Wetasphalt, tab2, selectZone);
	for(auto const& v : optionValues) {
  	ESPUI.addControl(Option, v.c_str(),v, None, zoneSelect);
  }

  ESPUI.addControl(ControlType::Button, "Calibrate ( in Air )", "Press", ControlColor::Peterriver, tab2, &measureAir);
  ESPUI.addControl(ControlType::Button, "Calibrate ( in Water )", "Press", ControlColor::Alizarin, tab2, &measureWater);
  ESPUI.addControl(ControlType::Button, "Print Raw Value", "Press", ControlColor::Emerald, tab2, &rawVals);


 
 
 
  uint16_t zone1_min = ESPUI.addControl(ControlType::Slider, "Beregnung starten bei %", "30", ControlColor::Alizarin, tab4, &slider1_min);
  uint16_t zone1_max = ESPUI.addControl(ControlType::Slider, "Beregnung Stoppen bei %", "80", ControlColor::Peterriver, tab4, &slider1_max);

  uint16_t zone2_min = ESPUI.addControl(ControlType::Slider, "Beregnung starten bei %", "30", ControlColor::Alizarin, tab5, &slider2_min);
  uint16_t zone2_max = ESPUI.addControl(ControlType::Slider, "Beregnung Stoppen bei %", "80", ControlColor::Peterriver, tab5, &slider2_max);
  
  uint16_t zone3_min = ESPUI.addControl(ControlType::Slider, "Beregnung starten bei %", "30", ControlColor::Alizarin, tab6, &slider3_min);
  uint16_t zone3_max = ESPUI.addControl(ControlType::Slider, "Beregnung Stoppen bei %", "80", ControlColor::Peterriver, tab6, &slider3_max);
  
  uint16_t zone4_min = ESPUI.addControl(ControlType::Slider, "Beregnung starten bei %", "30", ControlColor::Alizarin, tab7, &slider4_min);
  uint16_t zone4_max = ESPUI.addControl(ControlType::Slider, "Beregnung Stoppen bei %", "80", ControlColor::Peterriver, tab7, &slider4_max);


  ESPUI.addControl(ControlType::Number, "Maximale Beregnungszeit in min ZONE1:", "1", ControlColor::Alizarin, tab4, &max_raintime_function);
  ESPUI.addControl(ControlType::Number, "Maximale Beregnungszeit in min ZONE2:", "2", ControlColor::Alizarin, tab5, &max_raintime_function); 
  ESPUI.addControl(ControlType::Number, "Maximale Beregnungszeit in min ZONE3:", "3", ControlColor::Alizarin, tab6, &max_raintime_function);
  ESPUI.addControl(ControlType::Number, "Maximale Beregnungszeit in min ZONE4:", "4", ControlColor::Alizarin, tab7, &max_raintime_function);
  /*
    * .begin loads and serves all files from PROGMEM directly.
    * If you want to serve the files from SPIFFS use ESPUI.beginSPIFFS
    * (.prepareFileSystem has to be run in an empty sketch before)
    */

  // Enable this option if you want sliders to be continuous (update during move) and not discrete (update on stop)
  // ESPUI.sliderContinuous = true;

  /*
    * Optionally you can use HTTP BasicAuth. Keep in mind that this is NOT a
    * SECURE way of limiting access.
    * Anyone who is able to sniff traffic will be able to intercept your password
    * since it is transmitted in cleartext. Just add a string as username and
    * password, for example begin("ESPUI Control", "username", "password")
    */
  ESPUI.jsonInitialDocumentSize = 10000;
  ESPUI.setVerbosity(Verbosity::Verbose);
  ESPUI.begin("DFR25\n WaterControl");
  ArduinoOTA.begin();
 
}

void loop(void) {
  ArduinoOTA.handle();
  dnsServer.processNextRequest();
  timer_function();
  // debug_analog();
}
