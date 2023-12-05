#include <NimBLEDevice.h>
#include <DHT.h>
#include <DHT_U.h>

//Board used: Wemos Lolin32 lite

#define NODE 1 //  1 2 3 4 5 
#define BATTERYPIN 36 
#define LIGHTSENSORPIN 34 //pin 2 in node 5, pin 34 in node 1 to 4
#define DHTPIN 4 //pin 23 in node 5, pin 4 in node 1 to 4
#define DHTTYPE DHT11

uint32_t delayMS;
DHT_Unified dht(DHTPIN, DHTTYPE);

// Measurement functions constructor declaration
void measureTempEnv();
void measureHumEnv();
void measureLightingLevel();
void measureBatteryLevel();

// Nodes UUID's for services and characteristics
// Node 1
NimBLEUUID UUID128_SVC_SENSOR("a8d3f02c-7a24-4e5b-9d33-620e97277591");
NimBLEUUID UUID128_CHR_TEMPERATURE("725b1e3d-579f-4616-94a2-21e74622d8a0");
NimBLEUUID UUID128_CHR_HUMIDITY("e12a3f4b-8a69-40d1-acec-24f6e5c31514");
NimBLEUUID UUID128_CHR_LIGHT("d7f6b0e9-10b8-4960-a692-7d9d291b7ec1");
NimBLEUUID UUID128_CHR_BATTERY("c2b81cf1-0589-49ac-9430-56738f9708a1"); 

// Node 2
/*NimBLEUUID UUID128_SVC_SENSOR("dbd5cf43-6de7-4ce5-9f92-6a0486c2dbee");
NimBLEUUID UUID128_CHR_TEMPERATURE("a67e845d-f15c-4c4e-938e-501f2458e7d3");
NimBLEUUID UUID128_CHR_HUMIDITY("842f6d2a-ecb2-4a47-8e2f-109d832f2d13");
NimBLEUUID UUID128_CHR_LIGHT("f83e1478-2c15-4727-bad4-31f51f89421c");
NimBLEUUID UUID128_CHR_BATTERY("19446a58-3a67-4f21-8694-ec6f289c18b5");*/

//Node 3
/*NimBLEUUID UUID128_SVC_SENSOR("63d852ea-b7a2-4902-8a8f-69c7d82f1b0a");
NimBLEUUID UUID128_CHR_TEMPERATURE("1f56c15a-1be4-45e1-92ce-5ae6c423fd18");
NimBLEUUID UUID128_CHR_HUMIDITY("ebc49f6c-49c7-467f-a3e2-6f1b3c6b2d74");
NimBLEUUID UUID128_CHR_LIGHT("46dca8a9-7636-4e9c-8de7-3ca6b6bf0b5b");
NimBLEUUID UUID128_CHR_BATTERY("2d6b14ed-4a49-4cbf-bc65-4b26aa44dbee"); */

//Node 4
/*NimBLEUUID UUID128_SVC_SENSOR("0f6a982e-95cc-4324-8ba7-3d6c01b1b2a9");
NimBLEUUID UUID128_CHR_TEMPERATURE("bdff6e73-74f5-4a9d-a137-96843e7e8a4f");
NimBLEUUID UUID128_CHR_HUMIDITY("5c0b4959-8e92-4b19-96f3-22b127f9e9f5");
NimBLEUUID UUID128_CHR_LIGHT("89053c9f-dc91-4b3a-9c57-27a5e1c06288");
NimBLEUUID UUID128_CHR_BATTERY("cb3b8262-ffcd-4f2f-9f9a-9c6b786ad836");  */

//Node 5
/*NimBLEUUID UUID128_SVC_SENSOR("43a219b3-c3df-457b-8af7-09e93ce54b87");
NimBLEUUID UUID128_CHR_TEMPERATURE("57edf3be-6c95-47a6-9df9-93a8b77b1d5c");
NimBLEUUID UUID128_CHR_HUMIDITY("3e546e4b-4ee6-40ea-8119-03a9de53ad20");
NimBLEUUID UUID128_CHR_LIGHT("bda3d652-1ff5-44b2-9fe5-3833cf6dd38d");
NimBLEUUID UUID128_CHR_BATTERY("0eb92cd9-2ce6-47e1-8e63-9e8b20a3de5a"); */ 


NimBLEServer *pServer;
NimBLEService *pService;
NimBLECharacteristic *pTempCharacteristic;
NimBLECharacteristic *pHumidityCharacteristic;
NimBLECharacteristic *pLightCharacteristic;
NimBLECharacteristic *pBatteryCharacteristic;

TaskHandle_t measurementTask;

const long interval = 1 * 60 * 1000; // 1 minutes in milliseconds timer interval

// Define the deep sleep parameters
#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for microseconds to seconds
#define TIME_TO_SLEEP 25*60  // 25 minutes in seconds 1*60

void measurementTaskFunction(void *pvParameters) {
  for (;;) {
    // Measure parameters
    measureTempEnv();
    measureHumEnv();
    measureLightingLevel();
    measureBatteryLevel();

    // Delay for 1 minute
    vTaskDelay(pdMS_TO_TICKS(interval));
  }
}

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer) {
    // When a client connects, the measurement functions are called
    Serial.println("Client connected");

    //Measure parameters
    measureTempEnv();
    measureHumEnv();
    measureLightingLevel();
    measureBatteryLevel();
    
  }
  
  void onDisconnect(BLEServer* pServer) {
    // When a client disconnects, the node enters deep sleep to reset
    Serial.println("Client disconnected. Going into deep sleep.");
    esp_deep_sleep_start();
    
  }
};

void setup() {
  //Serial.println("Starting NimBLE Server");
  pinMode(LIGHTSENSORPIN, INPUT);
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;

  Serial.begin(115200);

  NimBLEDevice::init("SensorNode");

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  pService = pServer->createService(UUID128_SVC_SENSOR);

  pTempCharacteristic = pService->createCharacteristic(
      UUID128_CHR_TEMPERATURE, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  pHumidityCharacteristic = pService->createCharacteristic(
      UUID128_CHR_HUMIDITY, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  pLightCharacteristic = pService->createCharacteristic(
      UUID128_CHR_LIGHT, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  
  pBatteryCharacteristic = pService->createCharacteristic(
      UUID128_CHR_BATTERY, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
  );
  
  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(UUID128_SVC_SENSOR);

  pAdvertising->setScanResponse(true);
  pAdvertising->start();  

  // Configure the deep sleep timer
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  
  // Create the task that calls your functions
  xTaskCreatePinnedToCore(
      measurementTaskFunction,     /* Function to implement the task */
      "measureTask",   /* Name of the task */
      10000,           /* Stack size in words */
      NULL,            /* Task input parameter */
      1,               /* Priority of the task */
      &measurementTask,          /* Task handle. */
      0);              /* Core 0 or 1 */
}

void loop() {
  //Nothing to do here
}

void measureTempEnv() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    pTempCharacteristic->setValue("null");
  } else {
    pTempCharacteristic->setValue(String(event.temperature, 2));
  }
  pTempCharacteristic->notify();
  
}

void measureHumEnv() {
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    pHumidityCharacteristic->setValue("null");
  } else {
    pHumidityCharacteristic->setValue(String(event.relative_humidity, 2));
  } 
  pHumidityCharacteristic->notify();
}

void measureLightingLevel() {
  // Light Reading - TEMT6000
  analogReadResolution(10);
  int reading = analogRead(LIGHTSENSORPIN);
  
  if (isnan(reading)) {
    pLightCharacteristic->setValue("null");
  } else {
    float volts =  reading * 5 / 1024.0; 
   
    float amps = volts / 10000.0; 
    float microamps = amps * 1000000; 
    float lux = microamps * 2.0; 
    
    pLightCharacteristic->setValue(String(lux, 2));
  }
  pLightCharacteristic->notify();
}

void measureBatteryLevel() {
  // Read the raw voltage from the LiPo battery
  int rawValue = analogRead(BATTERYPIN); 
  
  if (isnan(rawValue)) {
    pBatteryCharacteristic->setValue("null");
  } else {
    // Convert the raw value to voltage (0-3.3V)
    float voltage = (3.3 / 4095.0) * rawValue;
  
    // Assuming a LiPo battery voltage range of 3.0V to 3.7V (5v in node 5)
    float batteryLevel = map(voltage, 3.0, 3.7, 0, 100);
    
    pBatteryCharacteristic->setValue(String(float(100)+batteryLevel, 2));
  }
  pBatteryCharacteristic->notify();
}

