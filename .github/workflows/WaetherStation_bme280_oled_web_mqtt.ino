//WaetherStation_bme280_oled_web_mqtt
// NodeMCU v.3 ESP8266 0,96 OLED
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include <ESP8266Wifi.h>


Adafruit_BME280 bme;  // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI
float temperature, humidity, pressure;

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
unsigned long delayTime;
const char* ssid = "XXXXX";          // Enter SSID here
const char* password = "xxxxxx";  //Enter Password here
const char* mqtt_server = "192.xxx.xxx.xxx";
const int mqtt_port = 1883;
const char* mqtt_user = "xxx";
const char* mqtt_password = "xxx";

// SENSOR
const int sending_period = 300;
const bool retain_flag = false;


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
uint32_t tmr1;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

}
void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
        Serial.println("connected");
        // Subscribe
        client.subscribe("esp32/output");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 300 seconds");
        // Wait 5 seconds before retrying
        delay(1000);
      }
    }
  }
  
  void sendData() {
    float temperature = bme.readTemperature();
    float pressure = bme.readPressure() / 133.3224;
    float humidity = bme.readHumidity();

    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" mm");
    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");

    
  }
  ESP8266WebServer server(80);

  void setup() {
    Serial.begin(115200);
    Serial.println(F("BME280 test"));

    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
                                                // init done
    display.display();
    delay(100);
    display.clearDisplay();
    display.display();
    display.setTextSize(2);
    display.cp437(true);
    display.setTextColor(WHITE);
    display.setRotation(2);
    
    delay(1000);

    bool status;
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin(0x76);
    if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1)
        ;
    }

    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();

    Serial.println("Connecting to ");
    Serial.println(ssid);

    //Connect to your local wi-fi network
    WiFi.begin(ssid, password);
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    //check wi-fi is connected to wi-fi network
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", handle_OnConnect);
    server.onNotFound(handle_NotFound);

    server.begin();
    Serial.println("HTTP server started");
   
  }
  void loop() {

    display.setCursor(0, 0);
    display.clearDisplay();
    
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");
    display.print("T:");
    display.print(round(bme.readTemperature()), 0);
    display.println("*C");


    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    display.print("H:");
    display.print(round(bme.readHumidity()), 0);
    display.println("%   ");

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 133.3224);
    Serial.println(" mmHg");
    display.print("P:");
    display.print(round(bme.readPressure()/ 133.3224), 0);
    display.println("mm");


    Serial.println();
    display.display();
    delay(5000);
    server.handleClient();

    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    sendData();
      unsigned long now = millis();
      if (now - lastMsg > 300000) {
        lastMsg = now;

        // Temperature in Celsius
        temperature = bme.readTemperature();
        // Uncomment the next line to set temperature in Fahrenheit
        // (and comment the previous temperature line)
        //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit

        // Convert the value to a char array
        char tempString[8];
        dtostrf(temperature, 1, 2, tempString);
        Serial.print("Temperature: ");
        Serial.println(tempString);
        client.publish("esp32/temperature", tempString);

        humidity = bme.readHumidity();

        // Convert the value to a char array
        char humString[8];
        dtostrf(humidity, 1, 2, humString);
        Serial.print("Humidity: ");
        Serial.println(humString);
        client.publish("esp32/humidity", humString);

        pressure = bme.readPressure() / 133.3224;

        char presString[8];
        dtostrf(pressure, 1, 2, presString);
        Serial.print("Pressure: ");
        Serial.println(presString);
        client.publish("esp32/pressure", presString);


      }
    
  }
void handle_OnConnect() {
      temperature = bme.readTemperature();
      humidity = bme.readHumidity();
      pressure = bme.readPressure() / 133.3224;
      server.send(200, "text/html", SendHTML(temperature, humidity, pressure));
  }

void handle_NotFound() {
      server.send(404, "text/plain", "Not found");
}

String SendHTML(float temperature, float humidity, float pressure) {
      String ptr = "<!DOCTYPE html>";
      ptr += "<html>";
      ptr += "<head>";
      ptr += "<title>ESP8266 Weather Station</title>";
      ptr += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
      ptr += "<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
      ptr += "<style>";
      ptr += "html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}";
      ptr += "body{margin: 0px;} ";
      ptr += "h1 {margin: 50px auto 30px;} ";
      ptr += ".side-by-side{display: table-cell;vertical-align: middle;position: relative;}";
      ptr += ".text{font-weight: 600;font-size: 19px;width: 200px;}";
      ptr += ".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}";
      ptr += ".temperature .reading{color: #F29C1F;}";
      ptr += ".humidity .reading{color: #3B97D3;}";
      ptr += ".pressure .reading{color: #26B99A;}";
      ptr += ".altitude .reading{color: #955BA5;}";
      ptr += ".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}";
      ptr += ".data{padding: 10px;}";
      ptr += ".container{display: table;margin: 0 auto;}";
      ptr += ".icon{width:65px}";
      ptr += "</style>";
      ptr += "</head>";
      ptr += "<body>";
      ptr += "<h1>ESP8266 Weather Station</h1>";
      ptr += "<h3>Dacha Inc.</h3>";
      ptr += "<div class='container'>";
      ptr += "<div class='data temperature'>";
      ptr += "<div class='side-by-side icon'>";
      ptr += "<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
      ptr += "C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
      ptr += "c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
      ptr += "c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
      ptr += "s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
      ptr += "</div>";
      ptr += "<div class='side-by-side text'>Temperature</div>";
      ptr += "<div class='side-by-side reading'>";
      ptr += (int)temperature;
      ptr += "<span class='superscript'>&deg;C</span></div>";
      ptr += "</div>";
      ptr += "<div class='data humidity'>";
      ptr += "<div class='side-by-side icon'>";
      ptr += "<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
      ptr += "C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
      ptr += "c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
      ptr += "C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
      ptr += "</div>";
      ptr += "<div class='side-by-side text'>Humidity</div>";
      ptr += "<div class='side-by-side reading'>";
      ptr += (int)humidity;
      ptr += "<span class='superscript'>%</span></div>";
      ptr += "</div>";
      ptr += "<div class='data pressure'>";
      ptr += "<div class='side-by-side icon'>";
      ptr += "<svg enable-background='new 0 0 40.542 40.541'height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541'width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424";
      ptr += "c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424";
      ptr += "c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25";
      ptr += "c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414";
      ptr += "c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804";
      ptr += "c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178";
      ptr += "C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814";
      ptr += "c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05";
      ptr += "C23.196,19.843,23.464,20.973,23.084,22.037z'fill=#26B999 /></g></svg>";
      ptr += "</div>";
      ptr += "<div class='side-by-side text'>Pressure</div>";
      ptr += "<div class='side-by-side reading'>";
      ptr += (int)pressure;
      ptr += "<span class='superscript'>hPa</span></div>";
      ptr += "</div>";
      ptr += "</div>";
      ptr += "</body>";
      ptr += "</html>";
      return ptr;
}
