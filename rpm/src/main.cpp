#include <Arduino.h>

#include "sensesp_app.h"
#include "sensesp_app_builder.h"
#include "signalk/signalk_put_request.h"

#include "camera.h"

// #define SSID "ottophone"
// #define WIFI_PASSWD "ottopilot"

void initCamera();
SKPutRequest<StaticJsonDocument<1024>> *request;
uint32_t chipId = 0;
uint16_t  camera_server_port;

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

  SensESPAppBuilder builder;

  sensesp_app = builder.set_standard_sensors(WIFI_SIGNAL)
              ->set_hostname(host_name)
#ifdef SSID              
              ->set_wifi(SSID, WIFI_PASSWD)
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
      request = new SKPutRequest<StaticJsonDocument<1024>>("camera.info");
      DynamicJsonDocument doc(1024);
      doc["ip"] = WiFi.localIP().toString();
      doc["id"] = chipId;
      doc["camera_port"] = camera_server_port;
      doc["stream_port"] = camera_server_port + 1;
      request->set_input(doc);
    }
  });


  // Start the SensESP application running
  sensesp_app->enable();
});
