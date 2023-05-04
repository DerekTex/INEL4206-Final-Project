#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <driver/gpio.h>
#include <freeRTOS/FreeRTOS.h>
#include <Ultrasonic.h>

/// @brief  Constantes definidas para el proyecto

#define LED_BUILTIN 2
#define WIFI_NAME "Gabriel's Galaxy S23"
#define WIFI_PASSWORD "ddfe1234"
#define WIFI_TIMEOUT 10000
#define trigPin 13 
#define echoPin 14 
#define MAX_DISTANCE 700 

/// @brief  Variables definidas para NodeRed

const char *mqttServer = "esp32proj.space";
const char *TempC = "TempC";
const char *TempF = "TempF";
const char *People = "People";
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

/// @brief  Variables Globales

int calibration = 0;
int counter = 0;
int people = 0;
int soundVelocity = 340;
bool recalibrated = false;
bool countup = true;
float timeOut = MAX_DISTANCE * 60; 
float recalibratedVoltage = 0;
float temperatureQuotient = 0;
float recalibratingVals[5];
float prevDistance = 0;
float highestTemp(float values[5]);
float lowestTemp(float values[5]);
float recalibrate(float values[5]);
void connectToWifi();

/// @brief Funcion que toma un arreglo de valores de temperatura de tipo float que calcula el promedio de la temperatura mediante interpolacion 
/// y esto lo recalibra.
/// @param values 
/// @return temperatureQuotient

float recalibrate(float values[])
{
  float lowestTemperature = lowestTemp(values);
  float highestTemperature = highestTemp(values);
  float averageTemperature = (lowestTemperature + highestTemperature) / 2;

  float interTemp = lowestTemperature + ((5 / 2) - 1) * (highestTemperature - lowestTemperature) / (5 - 1);//Interpolation to find the temperature of the thermistor
  float temperatureQuotient = interTemp / averageTemperature;

  return temperatureQuotient;
}

/// @brief Funcion que toma un arreglo de valores de temperatura de tipo float que calcula el valor menor de la temperatura
/// @param values 
/// @return lowestTemperature

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

/// @brief Funcion que toma un arreglo de valores de temperatura de tipo float que calcula el valor mayor de la temperatura
/// @param values
/// @return highestTemperature

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

/// @brief Funcion que calcula la distancia mediante ondas
/// @return distance

float getSonar() {
  unsigned long pingTime;
  float distance;
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  pingTime = pulseIn(echoPin, HIGH, timeOut); 
  distance = (float)pingTime * soundVelocity / 2 / 10000; 
  return distance;
}

/// @brief Funcion que conecta el ESP32 a la internet
/// @return void

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

/// @brief Funcion que conecta el ESP32 al mqtt broker de NodeRed
/// @return void

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

/// @brief Funcion inicia el codigo del ESP32
/// @return void

void setup()
{
  Serial.begin(115200);
  connectToWifi();
  connectToBroker();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(trigPin,OUTPUT)
  pinMode(echoPin,INPUT); 
}

/// @brief Funcion que corre el codigo del ESP32
/// @return void

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
  
  float distance = getSonar();
  if (distance < prevDistance) {
  if(countup == true) {
    people++;
    if (people >= 5) {
      countup = false;
    }
  } else if (countup == false) {
    people--;
    if (people == 0) {
      countup = true;
    }
  }
} else if (distance == prevDistance) {
  people = people;
}
  prevDistance = distance;

  Serial.printf("People in the room: %d \tCalibration: %d  \tTemperature : %.2fC    %.2fF\n", people, calibration, tempC, tempF);
  delay(1000);

  if (!client.connected())
  {
    connectToBroker();
  }
  client.publish(TempC, String(tempC).c_str());
  client.publish(TempF, String(tempF).c_str());
  client.publish(People,String(people).c_str());
  client.loop();

  delay(100);
}