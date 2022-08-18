#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Arduino.h"
#include<string.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include "config.h"
#define MAX_LENGTH 100
#define NUM_STRINGS 10
 
//define alguns pinos do esp32 que serao conectados aos modulos e componentes
#define rele 2
#define SS_PIN 14
#define RST_PIN 27
#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int i = 0;
int state = 0;
char st[20];
String temp, aux, conteudo;
String topic_mote = String(MOTE_ID) + "/";

char ids[NUM_STRINGS][MAX_LENGTH] = { {"39 9B 70 63"}};

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

// Add more topics that want your ESP to be subscribed to
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  
  // ESP subscribed to test topic
  uint16_t packetIdSub = mqttClient.subscribe(MOTE_ID, 0);
  Serial.println("Subscribing at QoS 0");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

// You can modify this function to handle what happens when you receive a certain message in a specific topic
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  String messageTemp;
  aux = " ";
  for (int i = 0; i < len; i++) {
    //Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  // Check if the MQTT message was received on topic test
  if (strcmp(topic, MOTE_ID) == 0) {
    Serial.println("TRUE");
  }
  for (int i = 0; i < NUM_STRINGS; ++i) 
  {
    if (messageTemp == String(topic_mote) + String(ids[i]))
    {
        state = 1;
        String stats =  "Porta Aberta \n\n";
        
        aux = stats;
        uint16_t packetIdPub1 = mqttClient.publish(MOTE_ID, 1, true, aux.c_str()); 
    }
  }  
  messageTemp = "\0"; 
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
void remove_spaces(char* s) {
    char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}
void setup() 
{
  Serial.begin(115200);   // Inicia a serial
// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  display.clearDisplay();
  display.setTextSize(1);             
  display.setTextColor(WHITE);  

  display.setCursor(0,10);             
  display.println("Aproxime o seu cartao do leitor...");

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
  
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522

  Serial.println("Aproxime o seu cartao do leitor...");
  Serial.println();
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  
  WiFi.onEvent(WiFiEvent);
  connectToWifi();
  
  pinMode(rele, OUTPUT);
  digitalWrite(rele, LOW);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}
 
void loop() 
{
  if (state)
  {
    digitalWrite(rele, HIGH);              //abrimos a tranca por 5 segundos
    state = 0;
  }
  if (digitalRead(rele))
  {
    display.clearDisplay();
    display.display();
    display.setTextSize(1);             
    display.setTextColor(WHITE);  
    
    display.setCursor(0,10);             
    display.println("Acesso Autorizado!");
    
    // Show the display buffer on the screen. You MUST call display() after
    // drawing commands to make them visible on screen!
    display.display();
    for(byte s = 5; s > 0; s--){             //vai informando ao usuario quantos segundos faltao para a tranca ser fechada
      delay(1000);
    }

    digitalWrite(rele, LOW);               // fecha a tranca
    display.clearDisplay();
    display.display();
    display.setTextSize(1);             
    display.setTextColor(WHITE);  
    
    display.setCursor(0,10);             
    display.println("Aproxime o seu cartao do leitor...");
    
    // Show the display buffer on the screen. You MUST call display() after
    // drawing commands to make them visible on screen!
    display.display();
  }
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
     return;                 // se nao tiver um cartao para ser lido recomeça o void loop
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;                  //se nao conseguir ler o cartao recomeça o void loop tambem
  }

  conteudo = "";      // cria uma string

  Serial.print("id da tag :"); //imprime na serial o id do cartao

  for (byte i = 0; i < mfrc522.uid.size; i++){       // faz uma verificacao dos bits da memoria do cartao
     //ambos comandos abaixo vão concatenar as informacoes do cartao...
     //porem os 2 primeiros irao mostrar na serial e os 2 ultimos guardarao os valores na string de conteudo para fazer as verificacoes
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  conteudo.toUpperCase();                      // deixa as letras da string todas maiusculas
  for (int i = 0; i < 1; ++i) 
  {
    if (conteudo.substring(1) == ids[i])
    { // verifica se o ID do cartao lido tem o mesmo ID do cartao que queremos liberar o acesso 
      digitalWrite(rele, HIGH);              //abrimos a tranca por 5 segundos
      aux = conteudo.substring(1);
      uint16_t packetIdPub1 = mqttClient.publish(MOTE_ID, 1, true, aux.c_str());
    }
  }
}

 
