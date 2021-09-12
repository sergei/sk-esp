#include <Arduino.h>

#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "signalk/signalk_put_request.h"

#include "camera.h"

#define SSID "OTTOPI"
#define WIFI_PASSWD "ottopilot"

#define SK_ADDR "192.168.4.1"
#define SK_PORT 80

void initCamera();
uint32_t chipId = 0;
uint16_t  camera_server_port;

SKPutRequest<StaticJsonDocument<1024>> request("camera.info");
StaticJsonDocument<1024> doc;

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([]() {

// Some initialization boilerplate when in debug mode...
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  for(int i=0; i<17; i=i+8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  char host_name[64];
  sprintf(host_name,"sk-cam-%08X", chipId);

  SystemStatusLed *pSystemLed = new SystemStatusLed(33);
  SensESPAppBuilder builder;

  sensesp_app = builder.set_standard_sensors(FREE_MEMORY)
              ->set_hostname(host_name)
              ->set_system_status_led(pSystemLed)
#ifdef SSID              
              ->set_wifi(SSID, WIFI_PASSWD)
#endif
#ifdef SK_ADDR              
              ->set_sk_server(SK_ADDR, SK_PORT)
#endif
              ->get_app(); 

  app.onDelay(1000, []() {
  // init camera 
    SkCamera camera;
    camera_server_port = 3333;
    camera.init(camera_server_port);
  });

  app.onRepeat(10000, []() {
    if ( sensesp_app->get_ws_client()->is_connected() ){
      doc["ip"] = WiFi.localIP().toString();
      doc["id"] = chipId;
      doc["rssi"] = WiFi.RSSI();
      doc["uptime"] = millis() / 1000;
      doc["camera_port"] = camera_server_port;
      doc["stream_port"] = camera_server_port + 1;
      request.set_input(doc);
    }
  });


  // Start the SensESP application running
  sensesp_app->enable();
});
