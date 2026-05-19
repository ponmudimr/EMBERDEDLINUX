#include <BLEDevice.h>
#include <BLEScan.h>

BLEScan* pBLEScan;

String registeredDevices[] = {
  "AA:BB:CC:11:22:33",
  "11:22:33:44:55:66"
};

int totalDevices = 2;

void setup() {

  Serial.begin(115200);

  BLEDevice::init("");

  pBLEScan = BLEDevice::getScan();

  pBLEScan->setActiveScan(true);

  Serial.println("BLE Attendance System Started");
}

void loop() {

  BLEScanResults foundDevices = pBLEScan->start(5);

  Serial.println("-------------------");

  for (int i = 0; i < foundDevices.getCount(); i++) {

    BLEAdvertisedDevice device = foundDevices.getDevice(i);

    String mac = device.getAddress().toString().c_str();

    Serial.print("Found Device: ");
    Serial.println(mac);

    for (int j = 0; j < totalDevices; j++) {

      if (mac.equalsIgnoreCase(registeredDevices[j])) {

        Serial.print("Attendance Marked For: ");
        Serial.println(mac);
      }
    }
  }

  Serial.println("-------------------");

  delay(5000);
}
