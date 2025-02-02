#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Informations WiFi
const char* ssid = "############";
const char* password = "mmmm1111";

// Informations RabbitMQ (via MQTT)
const char* mqtt_server = "192.168.1.167";
const int mqtt_port = 1883;
const char* mqtt_user = "guest";
const char* mqtt_password = "guest";

WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 3600, 60000);  // GMT+1 avec mise à jour toutes les 60 sec

#define trigPin 16  // Broche Trigger commune
#define echoBreakfast 5  // Capteur pour le petit-déjeuner
#define echoLunch 4      // Capteur pour le déjeuner
#define echoDinner 0     // Capteur pour le dîner

#define ledPin 2     // LED d'alerte
#define buzzerPin 14 // Buzzer

int mesurerDistance(int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration / 58.2; // Conversion en cm
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoBreakfast, INPUT);
  pinMode(echoLunch, INPUT);
  pinMode(echoDinner, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Connexion WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté !");
  
  // Connexion MQTT
  client.setServer(mqtt_server, mqtt_port);
  reconnectMQTT();

  // Initialisation du client NTP
  timeClient.begin();
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connexion MQTT...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("Connecté !");
    } else {
      Serial.print("Échec (erreur MQTT) : ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void envoyerNotification(const char* pillboxId, const char* repas) {
  StaticJsonDocument<128> notifJson;
  notifJson["pillboxId"] = pillboxId;
  
  String message = "" + String(repas) + " medication was missed!";
  notifJson["message"] = message;

  char notifBuffer[128];
  serializeJson(notifJson, notifBuffer);

  // Publier la notification sur RabbitMQ
  client.publish("pillbox/notifications", notifBuffer);
  Serial.println("Notification envoyée : " + message);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Lire l'heure actuelle
  timeClient.update();
  int currentHour = timeClient.getHours();

  // Formatage de la date
  char dateBuffer[20];
  snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", timeClient.getYear() + 1900, timeClient.getMonth(), timeClient.getDay());
  String formattedDate = String(dateBuffer);

  // Mesurer les distances des trois capteurs
  int distanceBreakfast = mesurerDistance(echoBreakfast);
  int distanceLunch = mesurerDistance(echoLunch);
  int distanceDinner = mesurerDistance(echoDinner);

  Serial.print("Heure actuelle: ");
  Serial.println(currentHour);
  Serial.print("Distance Breakfast: "); Serial.println(distanceBreakfast);
  Serial.print("Distance Lunch: "); Serial.println(distanceLunch);
  Serial.print("Distance Dinner: "); Serial.println(distanceDinner);

  // Initialisation des statuts
  String statusBreakfast = "not_taken";
  String statusLunch = "not_taken";
  String statusDinner = "not_taken";

  // Déterminer l'état de prise en fonction de l'heure et de la distance
  if (currentHour >= 8 && currentHour < 9) {  // Matin (08h-09h)
    Serial.println("Période : Matin");
    statusBreakfast = (distanceBreakfast > 7) ? "taken" : "missed";
  } else if (currentHour >= 12 && currentHour < 13) {  // Midi (12h-13h)
    Serial.println("Période : Midi");
    statusBreakfast = (distanceBreakfast > 7) ? "taken" : "missed";
    statusLunch = (distanceLunch > 7) ? "taken" : "missed";
  } else if (currentHour >= 19 && currentHour < 21) {  // Soir (19h-21h)
    Serial.println("Période : Soir");
    statusBreakfast = (distanceBreakfast > 7) ? "taken" : "missed";
    statusLunch = (distanceLunch > 7) ? "taken" : "missed";
    statusDinner = (distanceDinner > 7) ? "taken" : "missed";
  } else {
    Serial.println("Pas d'heure de prise de médicament");
  }

  // Créer un JSON conforme à ton API
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["pillboxId"] = "pillbox2";
  jsonDoc["date"] = formattedDate;

  JsonObject intakeStatus = jsonDoc.createNestedObject("intakeStatus");
  intakeStatus["breakfast"] = statusBreakfast;
  intakeStatus["lunch"] = statusLunch;
  intakeStatus["dinner"] = statusDinner;

  char jsonBuffer[256];
  serializeJson(jsonDoc, jsonBuffer);

  // Publier le message MQTT à RabbitMQ
  client.publish("pillbox/medication_status", jsonBuffer);
  Serial.println("Données envoyées à RabbitMQ !");
  
  // Vérifier si un médicament est manqué et envoyer une notification
  if (statusBreakfast == "missed") envoyerNotification("pillbox2", "breakfast");
  if (statusLunch == "missed") envoyerNotification("pillbox2", "lunch");
  if (statusDinner == "missed") envoyerNotification("pillbox2", "dinner");

  delay(5000);
}
