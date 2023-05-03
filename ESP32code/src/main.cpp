#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <driver/gpio.h>
#include <freeRTOS/FreeRTOS.h>

#define GPIO_NUMBER_36
#define GPIO_NUMBER_32
#define LED_BUILTIN 2
#define WIFI_NAME "Gabriel's Galaxy S23"
#define WIFI_PASSWORD "ddfe1234"
#define WIFI_TIMEOUT 10000

const char *mqttServer = "esp32proj.space";
const char *TempC = "TempC";
const char *TempF = "TempF";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

int calibration = 0;
bool recalibrated = false;
float recalibratedVoltage = 0;
float temperatureQuotient = 0;
float recalibratingVals[5];
float highestTemp(float values[5]);
float lowestTemp(float values[5]);
float recalibrate(float values[5]);
void connectToWifi();

void connectToWifi()
{

  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  unsigned long startAttempTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempTime < WIFI_TIMEOUT)
  {
    Serial.print(".");
    delay(1000);                    
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Failed to connect to WiFi");
  }
  else
  {
    Serial.println("Connected");
    digitalWrite(LED_BUILTIN, HIGH); // Enciende el LED cuando se conecta al WiFi
  }
}

void connectToBroker()
{
  client.setServer(mqttServer, mqttPort);

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");

    if (client.connect("mqtt broker"))
    {
      Serial.println("Connected");
    }
    else
    {
      Serial.print("Failed to connect ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  connectToWifi();
  connectToBroker();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  float adcValue = analogRead(GPIO_NUM_36);
  float voltage = (float)adcValue / 4095.0 * 3.3;                
  float Rt = 10 * voltage / (3.3 - voltage);                      
  double tempK = 1 / (1 / (273.15 + 25) + log(Rt / 10) / 3950.0); 
  double tempC = tempK - 273.15;                                         
  double tempF = tempC * 9 / 5 + 32;                                   
  double button = analogRead(GPIO_NUM_32);

  if (temperatureQuotient != 0)
  {
    tempC = tempC * temperatureQuotient;
  }

  if (button == 0 && calibration < 5 && recalibrated == false)
  {
    Serial.println("Recalibrating...");
    recalibratingVals[calibration] = tempC;
    calibration++;
  }

  else if (button != 0 && calibration >= 5)           
  {
    temperatureQuotient = recalibrate(recalibratingVals);
    recalibrated = false;
    calibration = 0;

    for (int i = 0; i < 5; i++)
    {
      recalibratingVals[i] = 0;
    }
    delay(500);
  }
  Serial.printf("Calibration: %d  \tTemperature : %.2fC    %.2fF\n", calibration, tempC, tempF);
  delay(1000);

  if (!client.connected())
  {
    connectToBroker();
  }
  client.publish(TempC, String(tempC).c_str());
  client.publish(TempF, String(tempF).c_str());
  client.loop();
}

float recalibrate(float values[])
{
  float lowestTemperature = lowestTemp(values);
  float highestTemperature = highestTemp(values);
  float averageTemperature = (lowestTemperature + highestTemperature) / 2;

  float interTemp = lowestTemperature + ((5 / 2) - 1) * (highestTemperature - lowestTemperature) / (5 - 1);//Interpolation to find the temperature of the thermistor
  float temperatureQuotient = interTemp / averageTemperature;

  return temperatureQuotient;
}

float lowestTemp(float values[])
{
  float lowestTemperature = values[0];
  for (unsigned int i = 0; i < 5; i++)
  {
    if (values[i] < lowestTemperature)
    {
      lowestTemperature = values[i];
    }
  }
  return lowestTemperature;
}

float highestTemp(float values[])
{
  float highestTemperature = values[0];
  for (unsigned int i = 0; i < 5; i++)
  {
    if (values[i] > highestTemperature)
    {
      highestTemperature = values[i];
    }
  }
  return highestTemperature;
}