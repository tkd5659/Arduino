#include <ESP8266WiFi.h>
#include <DHT.h>
#include <FastLED.h>


#define PIN_DHT D2
#define PIN_SOIL_MOISTURE A0
#define PIN_WATER_LEVEL D6
#define PIN_WATER_PUMP_ON D7
#define PIN_WATER_PUMP_OFF D8
#define NUM_LEDS 34//불의 갯수
#define LEDPIN D1//입력핀
String ledstate="off";

CRGB led[NUM_LEDS];
const char* ssid = "hanul302";
const char* password = "hanul302";

WiFiServer server(80);
WiFiClient client;
DHT DHTsensor(PIN_DHT, DHT11);

void setup() {
    DHTsensor.begin();
    pinMode(PIN_WATER_PUMP_ON, OUTPUT);
    pinMode(PIN_WATER_PUMP_OFF, OUTPUT);
    
    FastLED.addLeds<WS2812B,LEDPIN,GRB>(led,NUM_LEDS);
    FastLED.setBrightness(50);
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();
   
    Serial.println("Server started");
}

void loop() {
    
    client = server.available();
    if (!client) return;

    Serial.println("New client");
    client.setTimeout(500);

    String request = client.readStringUntil('\r');
    

    while (client.available()) {
        client.read();
    }
    Serial.println("Request: ");
    Serial.println(request);

    if (request.indexOf("led/on") > -1) {
         for(int i=0;i<NUM_LEDS;i++){
        led[i] = CRGB(255,0,255);
        }
      FastLED.show();
      ledstate="on";
    } else if (request.indexOf("led/off") > -1) {
        for(int i=0;i<NUM_LEDS;i++){
        led[i] = CRGB(0,0,0);
        }
      FastLED.show();
      ledstate="off";
    }

    if(ledstate.equals("on")){
      for(int i=0;i<NUM_LEDS;i++){
        led[i] = CRGB(255,0,255);
        }
      FastLED.show();
    }else{
      for(int i=0;i<NUM_LEDS;i++){
        led[i] = CRGB(0,0,0);
        }
      FastLED.show();
    }

    Serial.println(ledstate);
    if (request.indexOf("/waterpump/on") != -1) {
        handleWaterPumpRequest();
        
    }

    float humidity = DHTsensor.readHumidity();
    float temp = DHTsensor.readTemperature();
    int soilMoisture = analogRead(PIN_SOIL_MOISTURE);
    int waterLevelRaw = analogRead(PIN_WATER_LEVEL);
    

    // 변환된 수위 값 (0 ~ 100 범위로 정규화)
    int waterLevel = map(waterLevelRaw, 0, 1023, 0, 100);
    int soilLevel = map(soilMoisture, 0, 1023, 0, 100);
    

    Serial.print("Humidity : ");
    Serial.print(humidity);
    Serial.print(" Temperature : ");
    Serial.print(temp);
    Serial.println(" ºC");
    Serial.print("Soil Moisture: ");
    Serial.println(soilMoisture);
    Serial.print("Soil Level (%): ");
    Serial.println(soilLevel);
    Serial.print("Water Level (Raw): ");
    Serial.println(waterLevelRaw);
    Serial.print("Water Level (%): ");
    Serial.println(waterLevel);

    // Create a JSON string with the sensor data
    String jsonData = "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(humidity) + ",\"soil_moisture\":" + String(soilMoisture) + ",\"water_level\":" + String(waterLevel) + "}";

    // Send the JSON string to the client
    client.print("HTTP/1.1 200 OK\r\n");
    client.print("Content-Type: application/json\r\n\r\n");
    client.print(jsonData);

     Serial.println("Client disconnected");

    //delay(5000);
}

void handleWaterPumpRequest() {
    // 워터펌프를 켜는 코드
    digitalWrite(PIN_WATER_PUMP_ON, HIGH);
    digitalWrite(PIN_WATER_PUMP_OFF, LOW);
    delay(3000);  // 3초 동안 대기

    // 워터펌프를 끄는 코드
    digitalWrite(PIN_WATER_PUMP_ON, LOW);
    digitalWrite(PIN_WATER_PUMP_OFF, LOW);
}