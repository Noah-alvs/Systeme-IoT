
// Cette include permet d'utiliser les fonctions et les classes définies dans cette lib 
// pour gérer la connexion wifi sur le microcontroleur
#include <WiFi.h>
#include <PubSubClient.h>

// Includes permettant de manipuler l'Oled
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// Configuration Reseau 
const char* ssid = "Iphone de Safa";//Cette chaine de caractère représente le nom du réseau wifi auquel l'esp32 doit se connecter
const char* password = "test_95400"; 
// const char* ssid = "Box 4G SFR";
// const char* password = "Sami7893"; 
// const char* ssid = "A35 de Tarek";
// const char* password = "nd34dtxjzy6tgwi"; 
const char* BROKER_IP   = "172.20.10.10";  //ip du pc ou de la RPI
const int   BROKER_PORT = 1883;


// TOPICS
const char* TOPIC_LED    = "esp32/led";
const char* TOPIC_BUZZER = "esp32/buzzer";
const char* TOPIC_OLED   = "esp32/oled";

// PINs
#define BUZZER_PIN 17
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     16

// Variables blink LED
bool  blink_actif    = false; // Mode blink activé ou non
int   blink_interval = 500;
bool  blink_etat     = false; // Etat actuelle de la Led, allumé ou éteinte
long  blink_last     = 0;

// Variables beep buzzer
bool buz_beep_actif = false;
int  buz_beep_count = 0;
bool buz_beep_etat  = false;
long buz_beep_last  = 0;
#define BUZ_BEEP_INTERVAL 200
#define BUZZER_CHANNEL  0     // canal PWM (0 à 15 disponibles sur ESP32)
#define BUZZER_FREQ     1000  // frequence en Hz
#define BUZZER_RES      8     // resolution 8 bits (valeurs 0-255)

// Declaration de l'objet display (OLED)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClient   wifiClient; //Client wifi
PubSubClient mqttClient(wifiClient); // Création de l'objet client pour gérer la connexion MQTT


//Gestion LED
void handle_led(String msg) {
  if(msg == "led_on"){
    blink_actif = false;  // stopper le blink si actif
    blink_etat  = true; //Indique que la LED est allumé
    digitalWrite(LED_BUILTIN, HIGH); //Mettre la LED à l'état haut. 
    Serial.println("[LED] ON");
  } else if (msg == "led_off"){
    blink_actif = false;  // stopper le blink si actif
     blink_etat  = false; //Indique que la LED est eteint
    digitalWrite(LED_BUILTIN, LOW); //Mettre la LED à l'état bas. 
    Serial.println("[LED] OFF");
  }else if (msg.startsWith("led_blink:")) {
    blink_interval = msg.substring(10).toInt(); // extraire le nombre X apres "led_blink:X, correspondnant à l'intervalle"
    blink_actif = true;
    blink_etat = false; 
    digitalWrite(LED_BUILTIN, LOW); //Commencer le cycle à partir de la Led éteinte
    blink_last = millis();
    Serial.print("[LED] BLINK intervalle = ");
    Serial.print(blink_interval);
    Serial.println(" ms");
  }
}

//Gestion Buzzer
void handle_buzzer(String msg) {
  if (msg == "buzzer_on") {
    buz_beep_actif = false; //Stopper le buzzer lorsqu'il bipe à répétition
    buz_beep_etat  = true; //Indiquer que le buzzer est allumé
    ledcWrite(BUZZER_CHANNEL, 128); // Allume le buzzer
    Serial.println("[BUZ] ON");

  } else if (msg == "buzzer_off") {
    buz_beep_actif = false; //Stopper le buzzer lorsqu'il bipe à répétition
    buz_beep_etat  = false; //Indiquer que le buzzer est éteint
    ledcWrite(BUZZER_CHANNEL, 0); //Eteint le buzzer
    Serial.println("[BUZ] OFF");

  } else if (msg.startsWith("buzzer_beep:")) {
    int n = msg.substring(12).toInt();
    buz_beep_count = n * 2;  // chaque bip = 1 HIGH + 1 LOW 
    buz_beep_actif = true;
    buz_beep_etat  = false;
    ledcWrite(BUZZER_CHANNEL, 0); //Commence le cycle à l'état éteint
    buz_beep_last  = millis();
    Serial.print("[BUZ] BEEP x");
    Serial.println(n);
  }
}


//Gestion OLED
void handle_oled(String msg) {
  if (msg.startsWith("oled:")) {
    String texte = msg.substring(5); //extraction du texte à afficher apres "oled:"
    Serial.print("[OLED] Afficher : ");
    Serial.println(texte);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(texte);
    display.display();

  } else if (msg == "oled_clear") {
    Serial.println("[OLED] Clear");
    display.clearDisplay();
    display.display();
  }
}



// topic : le nom du topic sur lequel le message est arrivé.
// payload : le contenu du message sous forme de tablezu d'octets (bytes)
// length : la taille en octets du payload.
void on_message(char* topic, byte* payload, unsigned int length) {
  
  String msg = "";  //Construction d'une string à partir du tableau d'octets bruts.
  for (int i = 0; i < length; i++){
    msg += (char)payload[i];
  }

  Serial.print("[MQTT] Message recu sur ");
  Serial.print(topic);
  Serial.print(" : ");
  Serial.println(msg);

  // Appel à la fonction handler selon le topic
    if (String(topic) == TOPIC_LED)    handle_led(msg);
    else if (String(topic) == TOPIC_BUZZER) handle_buzzer(msg);
    else if (String(topic) == TOPIC_OLED)   handle_oled(msg);
}

void setup_led(){
  //Configurer le pin de la LED comme sortie 
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);// La broche sera mise à une tension de 0V => état eteint pour une led
}

void setup_buzzer(){
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQ, BUZZER_RES); // configurer le canal PWM
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL); // lier le canal au pin
  ledcWrite(BUZZER_CHANNEL, 0); // silence au demarrage
}

void setup_oled() {
  //Setup repris du fichier exemple d'AdaFruit
  Wire.begin(4, 15);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[OLED] Erreur : SSD1306 allocation failed");
    return;
  }

  display.display();
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("IOC - ESP32 pret");
  display.display();
  Serial.println("[OLED] Initialise.");
}

//Connexion Wifi
void setup_wifi() {
   //Initialise la connexion wifi de l'esp32 
  WiFi.begin(ssid, password);

  //Attente de la connexion
  Serial.print("Connexion à ");
  Serial.print(ssid);
  Serial.println("...");
  while(WiFi.status() != WL_CONNECTED){
    //delay(1000);
    Serial.print(".");
  }

  //Affichage de l'adresse ip une fois connecté
  Serial.println("");
  Serial.println("Connexion réussie !");
  Serial.println("Adresse IP : ");
  Serial.println(WiFi.localIP()); //Envoie l'adresse ip local de l'esp32 au moniteur série de l'IDE arduino
}

void setup_mqtt(){
  //Configurer le serveur MQTT en définissant l'adresse du broker et le port
  mqttClient.setServer(BROKER_IP, BROKER_PORT);
  //Definir la fonction de rappel qui sera appelée lorsque des messages mqtt sont reçus
  mqttClient.setCallback(on_message);

  //Boucle qui attend que le client se connecte au serveur MQTT
  while (!mqttClient.connected()) {
    Serial.print("[MQTT] Connexion au broker ");
    Serial.print(BROKER_IP);
    Serial.print("...");

    //Essaye de connecter le client au borker MQTT
    if (mqttClient.connect("ESP32_client")) {
      Serial.println(" OK");

      //S'abonner aux topics
      mqttClient.subscribe(TOPIC_LED);
      mqttClient.subscribe(TOPIC_BUZZER);
      mqttClient.subscribe(TOPIC_OLED);
      Serial.print("[MQTT] Subscribe sur : ");
      Serial.println(TOPIC_LED);
      Serial.println(TOPIC_BUZZER);
      Serial.println(TOPIC_OLED);
    } 
    // Echec de connexion
    else {
      Serial.print(" Echec rc=");
      Serial.print(mqttClient.state());
      Serial.println(" - nouvelle tentative dans 2s");
      //delay(2000);
    }
  }
}

void setup() {
  // Initialiser le moniteur série pour afficher l'état de la connexion.
  Serial.begin(115200);
  delay(500);  

  setup_led();
  setup_buzzer();
  setup_oled();
  setup_wifi();
  setup_mqtt();
}

// Gestion du blink non bloquant 
void loop_led(){
  if (blink_actif && (millis() - blink_last >= (unsigned long)blink_interval)) {
    blink_etat = !blink_etat;
    digitalWrite(LED_BUILTIN, blink_etat ? HIGH : LOW);
    blink_last = millis();
  }
}

//gestion du beep non bloquant
void loop_buzzer(){
  if (buz_beep_actif && (millis() - buz_beep_last >= BUZ_BEEP_INTERVAL)) {
    buz_beep_etat = !buz_beep_etat;
    ledcWrite(BUZZER_CHANNEL, buz_beep_etat ? 128 : 0);  // 128 = son, 0 = silence
    buz_beep_last = millis();
    buz_beep_count--;
    if (buz_beep_count <= 0) {
        buz_beep_actif = false;
        ledcWrite(BUZZER_CHANNEL, 0);
    }
  }
}

void loop() {
  if (!mqttClient.connected()) setup_mqtt(); // reconnexion si coupure
  mqttClient.loop(); //Déclenche on_message si mesage en attente

  loop_led();
  loop_buzzer();
}
