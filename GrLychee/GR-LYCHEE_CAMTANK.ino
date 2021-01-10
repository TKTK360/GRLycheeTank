#include <Arduino.h>
#include <Camera.h>
#include <SD.h>
#include <HTTPServer.h>
#include <mbed_rpc.h>
#include <SdUsbConnect.h>
#include <ESP32Interface.h>

//Zumo
#include <ZumoMotors.h>

//Camera Image Size
#define IMAGE_HW 320
#define IMAGE_VW 240

//Wifi Infomation
#define WIFI_SSID "xxxxxxx"
#define WIFI_PW "xxxxxxx"

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

//Wifi
ESP32Interface wifi;

// Camera 
Camera camera(IMAGE_HW, IMAGE_VW);

SdUsbConnect storage("storage"); 
static char result_str[] = "success";

// Zumo
#define LED_PIN 13
ZumoMotors _motors;
int  _speed = 0;

// Program
enum ZUMO_MODE_TYPE
{
  Unknown,
  Auto,
  Foward,
  Back,
  Left,
  Right,
  Stop,
};

ZUMO_MODE_TYPE _zumo_mode = ZUMO_MODE_TYPE::Unknown;

static Thread httpTask(osPriorityAboveNormal, (1024 * 4));

////////////////////////////////////////////////////////
//snapshot_req
static int snapshot_req(const char* rootPath, const char* path, const char ** pp_data) 
{
    if (strcmp(rootPath, "/camera") == 0) 
    {
        Serial.println("snap shot");

        size_t size = camera.createJpeg();
        *pp_data = (const char*)camera.getJpegAdr();
        return size;

    }
    else if (strcmp(rootPath, "/auto") == 0) 
    {
        _speed = atoi(path+1);
        Serial.println("auto");
        Serial.println(_speed);
        _zumo_mode = ZUMO_MODE_TYPE::Auto;
        *pp_data = (const char *)result_str;
        return strlen(result_str);
    } 
    else if (strcmp(rootPath, "/foward") == 0) 
    {
        _speed = atoi(path+1);
        Serial.print("foward ");
        Serial.println(_speed);
        _zumo_mode = ZUMO_MODE_TYPE::Foward;
        *pp_data = (const char *)result_str;
        return strlen(result_str);
    } 
    else if (strcmp(rootPath, "/back") == 0) 
    {
        _speed = atoi(path+1);
        Serial.print("back ");
        Serial.println(_speed);

        _zumo_mode = ZUMO_MODE_TYPE::Back;
        *pp_data = (const char *)result_str;
        return strlen(result_str);
    } 
    else if (strcmp(rootPath, "/right") == 0) 
    {
        _speed = atoi(path+1);
        Serial.print("right ");
        Serial.println(_speed);
      
        _zumo_mode = ZUMO_MODE_TYPE::Right;
        *pp_data = (const char *)result_str;
        return strlen(result_str);
    } 
    else if (strcmp(rootPath, "/left") == 0) 
    {
        _speed = atoi(path+1);
        Serial.print("left ");
        Serial.println(_speed);
      
        _zumo_mode = ZUMO_MODE_TYPE::Left;
        *pp_data = (const char *)result_str;
        return strlen(result_str);
    } 
    else if (strcmp(rootPath, "/stop") == 0) 
    {
        Serial.println("stop");
        _speed = 0;
        
        _zumo_mode = ZUMO_MODE_TYPE::Stop;
        *pp_data = (const char *)result_str;
        return strlen(result_str);
    } 
    else
    {
        Serial.println("unknown");
    }

    return 0;
}


////////////////////////////////////////////////////////
//http_task
void http_task(void) 
{
  // wifi
  Serial.print("Connecting Wi-Fi..");

  if (wifi.connect(WIFI_SSID, WIFI_PW, NSAPI_SECURITY_WPA_WPA2) == 0)
  {
    Serial.println("success");
  }
  else
  {
    Serial.println("fail");
  }
 
  Serial.print("MAC Address is ");
  Serial.println(wifi.get_mac_address());
  Serial.print("IP Address is ");
  Serial.println(wifi.get_ip_address());
  Serial.print("NetMask is ");
  Serial.println(wifi.get_netmask());
  Serial.print("Gateway Address is ");
  Serial.println(wifi.get_gateway());
  Serial.println("Network Setup OK\r\n");
 
  //req
  SnapshotHandler::attach_req(&snapshot_req);
  
  HTTPServerAddHandler<SnapshotHandler>("/camera"); //Camera
  HTTPServerAddHandler<SnapshotHandler>("/auto");
  HTTPServerAddHandler<SnapshotHandler>("/foward");
  HTTPServerAddHandler<SnapshotHandler>("/back");
  HTTPServerAddHandler<SnapshotHandler>("/right");
  HTTPServerAddHandler<SnapshotHandler>("/left");
  HTTPServerAddHandler<SnapshotHandler>("/stop");

  Serial.println("Handler");

  FSHandler::mount("/storage", "/");
  HTTPServerAddHandler<FSHandler>("/");
  HTTPServerAddHandler<RPCHandler>("/rpc");

  Serial.println("Server Start");

  HTTPServerStart(&wifi, 80);  
}

//Tank foward
void fowardFunction(int spd)
{
    // run left motor forward
    // run right motor forward
    for (int speed = 0; speed <= spd; speed++)
    {
      _motors.setLeftSpeed(-speed);
      _motors.setRightSpeed(-speed);
      delay(2);
    }

    for (int speed = spd; speed >= 0; speed--)
    {
      _motors.setLeftSpeed(-speed);
      _motors.setRightSpeed(-speed);
      delay(2);
    }
}

//Tank back
void backFunction(int spd)
{
    // run left motor backward
    // run right motor backward
    for (int speed = 0; speed <= spd; speed++)
    {
      _motors.setLeftSpeed(speed);
      _motors.setRightSpeed(speed);
      delay(2);
    }

    for (int speed = spd; speed >= 0; speed--)
    {
      _motors.setLeftSpeed(speed);
      _motors.setRightSpeed(speed);
      delay(2);
    }
}

//Tank right turn
void rightFunction(int spd)
{
    // run left motor forward
    // run right motor backward      
    for (int speed = 0; speed <= spd; speed++)
    {
      _motors.setLeftSpeed(-speed);
      _motors.setRightSpeed(speed);
      delay(2);
    }

    for (int speed = spd; speed >= 0; speed--)
    {
      _motors.setLeftSpeed(-speed);
      _motors.setRightSpeed(speed);
      delay(2);
    }
}

//Tank left turn
void leftFunction(int spd)
{
    // run left motor backward
    // run right motor forward
    for (int speed = 0; speed <= spd; speed++)
    {
      _motors.setLeftSpeed(speed);
      _motors.setRightSpeed(-speed);
      delay(2);
    }

    for (int speed = spd; speed >= 0; speed--)
    {
      _motors.setLeftSpeed(speed);
      _motors.setRightSpeed(-speed);
      delay(2);
    }
}

//Tank stop
void stopFunction()
{
    _motors.setLeftSpeed(0);
    _motors.setRightSpeed(0);
}


////////////////////////////////////////////////////////
//setup
void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Starts.");

  // SD & USB
  Serial.print("Finding strage..");
  storage.wait_connect();
  Serial.println("done");

  // Zumo
  pinMode(LED_PIN, OUTPUT);
  _motors.flipLeftMotor(true);
  _motors.flipRightMotor(true);

  // camera
  camera.begin();

  // http
  httpTask.start(&http_task);

  Serial.println("start");
}


////////////////////////////////////////////////////////
//loop
void loop() 
{
  // Test Input
  while (Serial.available() > 0) 
  {
    char mode = Serial.read();
    Serial.print("modeNo=");
    Serial.println(mode);

    int modeNo = (mode - '0');
    _zumo_mode = (ZUMO_MODE_TYPE)modeNo;
  }

  if(_zumo_mode == ZUMO_MODE_TYPE::Unknown)
  {
    return;
  }

  // Control
  switch(_zumo_mode)  
  {
    case ZUMO_MODE_TYPE::Auto:
      digitalWrite(LED_PIN, HIGH);
      // front 
      fowardFunction(_speed);
      // back
      backFunction(_speed);
      
      // left
      leftFunction(225);
      // front
      fowardFunction(_speed);
      
      // left
      leftFunction(225);
      // front
      fowardFunction(_speed);

      // left
      leftFunction(225);
      // front
      fowardFunction(_speed);

      // left
      leftFunction(225);
      // front
      fowardFunction(_speed);
      digitalWrite(LED_PIN, LOW);
      break;


    case ZUMO_MODE_TYPE::Stop:
      stopFunction();
      break;

    case ZUMO_MODE_TYPE::Back:
      digitalWrite(LED_PIN, HIGH);
      backFunction(_speed);
      digitalWrite(LED_PIN, LOW);
      break;

    case ZUMO_MODE_TYPE::Foward:
      digitalWrite(LED_PIN, HIGH);
      fowardFunction(_speed);
      digitalWrite(LED_PIN, LOW);
      break;


    case ZUMO_MODE_TYPE::Right:
      digitalWrite(LED_PIN, HIGH);
      rightFunction(_speed);
      digitalWrite(LED_PIN, LOW);
      break;

    case ZUMO_MODE_TYPE::Left:
      digitalWrite(LED_PIN, HIGH);
      leftFunction(_speed);
      digitalWrite(LED_PIN, LOW);
      break;
  }

  _zumo_mode = ZUMO_MODE_TYPE::Unknown;
}
