#include "BluetoothSerial.h" //Header File for Serial Bluetooth, will be added by default into Arduino

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#include "ArduinoNvs.h"
#include <WiFi.h>
bool res;
BluetoothSerial ESP_BT; //Object for Bluetooth
int incoming;
String message = "";
String nvs_ssid_key = "ssid";
String nvs_wifi_pswd_key = "pswd";
boolean is_wifi_cred_exists;
int bluetooth_received_parameter = 0;
boolean bluetooth_prompt_shown = false;
boolean continue_input = false;
int LED_BUILTIN = 2;
int pushButton = 19;
/////methods for BLUETOOTH CLassic
void eraseWifiCredentials();
boolean hasWifiCredentials();
void setSSIDFromBluetooth();


void setup() {
  Serial.begin(115200); //Start Serial monitor in 9600
  Serial.println("Bluetooth Device is Ready to Pair");

  pinMode (LED_BUILTIN, OUTPUT);//Specify that LED pin is output
  pinMode(pushButton, INPUT_PULLUP);
  NVS.begin("storage");
  is_wifi_cred_exists = hasWifiCredentials();
  if (!is_wifi_cred_exists) {
    // setSSIDFromPCSerial();
    setSSIDFromBluetooth();
  }
  String WIFI_SSID = NVS.getString(nvs_ssid_key);
  String  WIFI_PASSWORD = NVS.getString(nvs_wifi_pswd_key);
  Serial.println("connecting to ssid " + WIFI_SSID);
  Serial.println("using password " + WIFI_PASSWORD);
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
  byte wait_counter = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
    wait_counter++;
    if (wait_counter > 100) {
      eraseWifiCredentials();
      ESP.restart();
    }
  }
  // Wifi turned on
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  int buttonState = digitalRead(pushButton);
  String inp = String(buttonState);
  if (buttonState == LOW) {
    Serial.println("Resetting the device");
    eraseWifiCredentials();
    digitalWrite(LED_BUILTIN, LOW);
    ESP.restart();
  }

  if (Serial.available() > 0) {
    byte r = Serial.read();
    if (char(r) == '0') {
      Serial.println("Resetting the device");
      eraseWifiCredentials();
      digitalWrite(LED_BUILTIN, LOW);
      ESP.restart();
    }
  }
  delay(500);
}







void eraseWifiCredentials() {
  NVS.erase(nvs_ssid_key);
  NVS.erase(nvs_wifi_pswd_key);
}

boolean hasWifiCredentials() {
  String stored_SSID = NVS.getString(nvs_ssid_key);
  String  stored_password = NVS.getString(nvs_wifi_pswd_key);
  return stored_SSID != NULL && stored_password != NULL;
}



void setSSIDFromBluetooth() {
  ESP_BT.begin("ESP32_LED_Control"); //Name of your Bluetooth Signal
  while (true) {
    if (ESP_BT.connected()) {
      ESP_BT.println("Chose your Wifi Name");
      ESP_BT.println();
      String ssid_list[] = {"ssid0", "ssid1", "ssid2", "ssid3"};

      byte size_array = sizeof(ssid_list) / sizeof(ssid_list[0]);
      //Serial.println(size_array);

      for (int i = 0; i < size_array; i++) {
        ESP_BT.println(ssid_list[i]);
      }
      continue_input = true;
    }
    delay(50);
    if (continue_input) {
      while (bluetooth_received_parameter < 3) {
        if (bluetooth_received_parameter == 2) {
          ESP.restart();
          break;
        }
        if (ESP_BT.available()) {

          byte r = ESP_BT.read();
          if (char(r) != '\n') {
            if (r != 13 && r != 10 && char(r) != '\0') {
              message += char(r);
            }
          }
          else {
            if (bluetooth_received_parameter == 0) {
              res = NVS.setString(nvs_ssid_key, message);
              if (res) {
                ESP_BT.println("success");
                res = false;
              }
              ESP_BT.println("Enter your password");
            }

            if (bluetooth_received_parameter == 1 ) {
              res = NVS.setString(nvs_wifi_pswd_key, message);
              if (res) {
                ESP_BT.println("success");
                res = false;
              }
            }
            Serial.println(message);
            message = "";
            bluetooth_received_parameter++;
          }
        }
        delay(20);
      }
    }
  }
}

void setSSIDFromPCSerial() {
  bluetooth_received_parameter = 0;
  Serial.println("Enter your SSID");
  while (true) {
    if (Serial.available() > 0) {
      continue_input = true;
    }
    delay(50);
    if (continue_input) {
      while (bluetooth_received_parameter < 3) {
        if (bluetooth_received_parameter == 2) {
          ESP.restart();
          break;
        }
        if (Serial.available() > 0) {
          byte r = Serial.read();
          if (char(r) != '\n') {
            if (r != 13 && r != 10 && char(r) != '\0') {
              message += char(r);
            }
          }
          else {
            if (bluetooth_received_parameter == 0) {
              res = NVS.setString(nvs_ssid_key, message);
              if (res) {
                Serial.println("success");
                res = false;
              }
              Serial.println("Enter your password");
            }

            if (bluetooth_received_parameter == 1 ) {
              res = NVS.setString(nvs_wifi_pswd_key, message);
              if (res) {
                Serial.println("success");
                res = false;
              }
            }
            Serial.println(message);
            message = "";
            bluetooth_received_parameter++;
          }
        }
        delay(20);
      }
    }
  }
}
