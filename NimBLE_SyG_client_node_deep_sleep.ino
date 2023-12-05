#include <NimBLEDevice.h>

//Board used: Heltec WiFi LoRa32(V2)

NimBLEScan* pBLEScan;
const int maxConnections = 5;

#define NODE 6

NimBLEClient* pClients[maxConnections] = {nullptr};
bool isConnected[maxConnections] = {false};
bool notificationEnabled[maxConnections] = {false};

// Array of service UUIDs for all sensor nodes
NimBLEUUID serviceUUIDs[] = {
  NimBLEUUID("a8d3f02c-7a24-4e5b-9d33-620e97277591"), // Node 1 service UUID
  NimBLEUUID("dbd5cf43-6de7-4ce5-9f92-6a0486c2dbee"), // Node 2 service UUID
  NimBLEUUID("63d852ea-b7a2-4902-8a8f-69c7d82f1b0a"), // Node 3 service UUID
  NimBLEUUID("0f6a982e-95cc-4324-8ba7-3d6c01b1b2a9"), // Node 4 service UUID
  NimBLEUUID("43a219b3-c3df-457b-8af7-09e93ce54b87"), // Node 5 service UUID
};

// Arrays of characteristics UUIDs for all sensor nodes
NimBLEUUID temperatureCharUUIDs[] = {
  NimBLEUUID("725b1e3d-579f-4616-94a2-21e74622d8a0"), // Node 1 temperature UUID
  NimBLEUUID("a67e845d-f15c-4c4e-938e-501f2458e7d3"), // Node 2 temperature UUID
  NimBLEUUID("1f56c15a-1be4-45e1-92ce-5ae6c423fd18"), // Node 3 temperature UUID
  NimBLEUUID("bdff6e73-74f5-4a9d-a137-96843e7e8a4f"), // Node 4 temperature UUID
  NimBLEUUID("57edf3be-6c95-47a6-9df9-93a8b77b1d5c"), // Node 5 temperature UUID
};

NimBLEUUID humidityCharUUIDs[] = {
  NimBLEUUID("e12a3f4b-8a69-40d1-acec-24f6e5c31514"), // Node 1 humidity UUID
  NimBLEUUID("842f6d2a-ecb2-4a47-8e2f-109d832f2d13"), // Node 2 humidity UUID
  NimBLEUUID("ebc49f6c-49c7-467f-a3e2-6f1b3c6b2d74"), // Node 3 humidity UUID
  NimBLEUUID("5c0b4959-8e92-4b19-96f3-22b127f9e9f5"), // Node 4 humidity UUID
  NimBLEUUID("3e546e4b-4ee6-40ea-8119-03a9de53ad20"), // Node 5 humidity UUID
};

NimBLEUUID lightCharUUIDs[] = {
  NimBLEUUID("d7f6b0e9-10b8-4960-a692-7d9d291b7ec1"), // Node 1 light UUID
  NimBLEUUID("f83e1478-2c15-4727-bad4-31f51f89421c"), // Node 2 light UUID
  NimBLEUUID("46dca8a9-7636-4e9c-8de7-3ca6b6bf0b5b"), // Node 3 light UUID
  NimBLEUUID("89053c9f-dc91-4b3a-9c57-27a5e1c06288"), // Node 4 light UUID
  NimBLEUUID("bda3d652-1ff5-44b2-9fe5-3833cf6dd38d"), // Node 5 light UUID
};

NimBLEUUID batteryCharUUIDs[] = {
  NimBLEUUID("c2b81cf1-0589-49ac-9430-56738f9708a1"),  // Node 1 battery UUID
  NimBLEUUID("19446a58-3a67-4f21-8694-ec6f289c18b5"),  // Node 2 battery UUID
  NimBLEUUID("2d6b14ed-4a49-4cbf-bc65-4b26aa44dbee"),  // Node 3 battery UUID
  NimBLEUUID("cb3b8262-ffcd-4f2f-9f9a-9c6b786ad836"),  // Node 4 battery UUID
  NimBLEUUID("0eb92cd9-2ce6-47e1-8e63-9e8b20a3de5a"),  // Node 5 battery UUID
};


void setup() {
    Serial.begin(115200);
    NimBLEDevice::init("GatewayNode");
}

void loop() {
    if (checkConnectionStatus()) {
        scanAndConnect();
    }
}

bool checkConnectionStatus() {
    bool anyDisconnected = false;

    for (int node = 0; node < maxConnections; node++) {
        if (pClients[node] && !pClients[node]->isConnected()) {
            anyDisconnected = true;
            if (isConnected[node]) {
                isConnected[node] = false;
                Serial.print("Disconnected from server node ");
                Serial.println(node + 1);
            }
        }
    }
    return anyDisconnected;
}

void scanAndConnect() {
    pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    
    BLEScanResults foundDevices = pBLEScan->start(5);

    for (int i = 0; i < foundDevices.getCount(); i++) {
        NimBLEAdvertisedDevice device = foundDevices.getDevice(i);
        NimBLEAddress address = device.getAddress();

        for (int node = 0; node < maxConnections; node++) {
            if (device.haveServiceUUID() && device.isAdvertisingService(serviceUUIDs[node])) {
                if (!isConnected[node]) {
                    pClients[node] = NimBLEDevice::createClient();
                    if (pClients[node]->connect(address, false)) {
                        isConnected[node] = true;
                        enableNotifications(node);
                        Serial.print("Connected to the server node ");
                        Serial.println(node + 1);
                    }
                }
            }
        }
    }
    pBLEScan->clearResults();
}

void printRSSI(int node) {
    if (pClients[node] && isConnected[node]) {
        int rssi = pClients[node]->getRssi();
        Serial.print("N");
        Serial.print(node + 1);
        Serial.print("R");
        Serial.println(rssi);
    }
}

void enableNotifications(int node) {
    NimBLERemoteService* pRemoteService = pClients[node]->getService(serviceUUIDs[node]);
    if (pRemoteService) {
        NimBLEUUID temperatureCharUUID = temperatureCharUUIDs[node];
        NimBLERemoteCharacteristic* pTemperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharUUID);
        NimBLEUUID humidityCharUUID = humidityCharUUIDs[node];
        NimBLERemoteCharacteristic* pHumidityCharacteristic = pRemoteService->getCharacteristic(humidityCharUUID);
        NimBLEUUID lightCharUUID = lightCharUUIDs[node];
        NimBLERemoteCharacteristic* pLightCharacteristic = pRemoteService->getCharacteristic(lightCharUUID);
        NimBLEUUID batteryCharUUID = batteryCharUUIDs[node];
        NimBLERemoteCharacteristic* pBatteryCharacteristic = pRemoteService->getCharacteristic(batteryCharUUID);

        if (pTemperatureCharacteristic && pHumidityCharacteristic && pLightCharacteristic && pBatteryCharacteristic) {
            pTemperatureCharacteristic->registerForNotify(notifyCallback);
            pHumidityCharacteristic->registerForNotify(notifyCallback);
            pLightCharacteristic->registerForNotify(notifyCallback);
            pBatteryCharacteristic->registerForNotify(notifyCallback);
            notificationEnabled[node] = true;
        }
    }
}

void notifyCallback(NimBLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (isNotify) {
        NimBLEUUID charUUID = pCharacteristic->getUUID();
        int node = -1;

        for (int i = 0; i < maxConnections; i++) {
            if (charUUID.equals(temperatureCharUUIDs[i])) {
                node = i;
                Serial.print("S");
                Serial.print(((node)*3) + 1);
                Serial.print("R");
                break;
            } else if (charUUID.equals(humidityCharUUIDs[i])) {
                node = i;
                Serial.print("S");
                Serial.print(((node)*3) + 2);
                Serial.print("R");
                break;
            } else if (charUUID.equals(lightCharUUIDs[i])) {
                node = i;
                Serial.print("S");
                Serial.print(((node)*3) + 3);
                Serial.print("R");
                break;
            } else if (charUUID.equals(batteryCharUUIDs[i])) {
                node = i;
                printRSSI(node);
                pClients[node]->disconnect();
                isConnected[node] = false;
                Serial.print("N");
                Serial.print(node + 1);
                Serial.print("B");
                break;
            }
        }

        if (node != -1) {
            String data = "";
            for (size_t i = 0; i < length; i++) {
                data += (char)pData[i];
            }
            Serial.println(data);
        }
    }
}
