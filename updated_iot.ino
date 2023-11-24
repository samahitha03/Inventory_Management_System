#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "";
const char* password = "";
const char* thingSpeakApiKey = "";
const char* thingSpeakChannelId = "";
const char* pushingBoxHost = "api.pushingbox.com";
const char* pushingBoxDevId = "";

#define RST_PIN D3
#define SS_PIN D4

MFRC522 mfrc522(SS_PIN, RST_PIN);

#define MAX_RFID_TAGS 10
#define RFID_TAG_LENGTH 8

struct Product {
  char rfidTag[RFID_TAG_LENGTH + 1];
  String name;
  int quantity;
  float price;
  unsigned long lastUpdateTime;
  String alertMessage;
};

Product inventory[MAX_RFID_TAGS];
int inventoryCount = 0;

WiFiClient client;

const unsigned long updateThreshold = 3000;

void addProduct(const char *rfidTag, String name, int quantity, float price, String alertMessage);
Product *findProductByRFID(const char *rfidTag);
void sendToThingSpeak(const char *rfidTag, int quantity, float price);
void updateInventory();
void sendPushingBoxNotification();
void connectToWiFi();

void setup() {
  Serial.begin(9600);
  Serial.println("Setup started");
  delay(10);
  connectToWiFi();

  SPI.begin();  // Initialize SPI
  mfrc522.PCD_Init();  // Initialize RFID module
  Serial.println("RFID Tag Reader initialized.");

  if (!mfrc522.PCD_PerformSelfTest()) {
    Serial.println("MFRC522 initialization failed. Check your wiring!");
    while (1);
  }

  Serial.println("MFRC522 initialized.");

  // Initialize products with different alert messages
  addProduct("835E8B15", "iPhone 15", 15, 90000, "Alert: iPhone 15 quantity is below 10!");
  addProduct("03935317", "Samsung S23", 15, 75000, "Alert: Samsung S23 quantity is below 10!");
  addProduct("A355D617", "Oneplus 11", 15, 60000, "Alert: Oneplus 11 quantity is below 10!");

  Serial.println("RFID Tags in Inventory:");
  for (int i = 0; i < inventoryCount; i++) {
    Serial.println(inventory[i].rfidTag);
  }
}

void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected to Wi-Fi");
}

void loop() {
  updateInventory(); // Check for updates based on time

  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.println("Card detected!");

      char rfidData[RFID_TAG_LENGTH * 2 + 1];
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        sprintf(rfidData + 2 * i, "%02X", mfrc522.uid.uidByte[i]);
      }

      Serial.println("RFID Data: " + String(rfidData));

      Serial.println("RFID Tags in Inventory:");
      for (int i = 0; i < inventoryCount; i++) {
        Serial.println(inventory[i].rfidTag);
      }

      Product *product = findProductByRFID(rfidData);
      if (product != NULL) {
        Serial.println("Product found: " + product->name);
        Serial.println("Quantity: " + String(product->quantity));
        Serial.print("Price: ₹");
        Serial.println(String(product->price, 2));

        // Decrement quantity
        product->quantity--;

        // Check if quantity is below 10 and give an alert
        if (product->quantity < 10) {
          Serial.println(product->alertMessage);
          sendPushingBoxNotification();
          // Add any additional actions for low quantity alerts here
        }

        // Now, measure distance and update ThingSpeak
        unsigned long currentTime = millis();
        if (currentTime - product->lastUpdateTime > updateThreshold) {
          sendToThingSpeak(rfidData, product->quantity, product->price);
          product->lastUpdateTime = currentTime; // Update last update time
        }
      } else {
        Serial.println("Product not found in inventory.");
      }

      mfrc522.PICC_HaltA();
    } else {
      Serial.println("Error reading card. Make sure the card is placed correctly.");
    }
  }

  delay(1000);
}

void addProduct(const char *rfidTag, String name, int quantity, float price, String alertMessage) {
  if (inventoryCount < MAX_RFID_TAGS) {
    strncpy(inventory[inventoryCount].rfidTag, rfidTag, RFID_TAG_LENGTH);
    inventory[inventoryCount].rfidTag[RFID_TAG_LENGTH] = '\0';
    inventory[inventoryCount].name = name;
    inventory[inventoryCount].quantity = quantity;
    inventory[inventoryCount].price = price;
    inventory[inventoryCount].lastUpdateTime = 0; // Initialize last update time
    inventory[inventoryCount].alertMessage = alertMessage;
    inventoryCount++;
  }
}

Product *findProductByRFID(const char *rfidTag) {
  for (int i = 0; i < inventoryCount; i++) {
    if (strcmp(inventory[i].rfidTag, rfidTag) == 0) {
      return &inventory[i];
    }
  }
  return NULL;
}

void sendToThingSpeak(const char *rfidTag, int quantity, float price) {
  String url = "/update";
  String apiKeyParam = "api_key=";
  apiKeyParam += thingSpeakApiKey;
  String quantityParam = "&field1=";
  quantityParam += String(quantity);
  String priceParam = "&field2=";
  priceParam += String(price);
  String rfidParam = "&field3=";
  rfidParam += String(rfidTag);

  String postData = apiKeyParam + quantityParam + priceParam + rfidParam;

  HTTPClient http;
  http.begin(client, "http://api.thingspeak.com" + url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(postData);
  if (httpCode > 0) {
    Serial.println("ThingSpeak update successful");
  } else {
    Serial.println("ThingSpeak update failed");
  }

  http.end();
}

void sendPushingBoxNotification() {
  Serial.print("Connecting to ");
  Serial.println(pushingBoxHost);

  // Use WiFiClient class to create TCP connections
  WiFiClient pbClient;
  const int httpPort = 80;
  if (!pbClient.connect(pushingBoxHost, httpPort)) {
    Serial.println("connection to PushingBox failed");
    return;
  }

  // We now create a URI for the request
  String url = "/pushingbox";
  url += "?devid=";
  url += pushingBoxDevId;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  pbClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + pushingBoxHost + "\r\n" +
                 "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (pbClient.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> PushingBox Client Timeout !");
      pbClient.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (pbClient.available()) {
    String line = pbClient.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("PushingBox notification sent");
  pbClient.stop();
  delay(5000);
}

void updateInventory() {
  // Empty function for now
}
