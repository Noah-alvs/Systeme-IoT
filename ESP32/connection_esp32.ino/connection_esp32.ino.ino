/*
  Cette include permet d'utiliser les fonctions et les classes définies dans cette lib 
  pour gérer la connexion wifi sur le microcontroleur
*/
#include <WiFi.h>
#include <PubSubClient.h>

//Cette chaine de caractère représente le nom du réseau wifi auquel l'esp32 doit se connecter
const char* ssid = "Iphone de Safa";
const char* password = "test_95400"; 
// const char* ssid = "Box 4G SFR";
// const char* password = "Sami7893"; 
// const char* ssid = "A35 de Tarek";
// const char* password = "nd34dtxjzy6tgwi"; 



const char* BROKER_IP   = "172.20.10.10";  //ip du pc ou de la RPI
const int   BROKER_PORT = 1883;
const char* TOPIC_LED   = "esp32/led";

//#define LED_PIN 2

WiFiClient   wifiClient; //Client wifi
PubSubClient mqttClient(wifiClient); // Création de l'objet client pour gérer la connexion MQTT

// Variables blink
bool  blink_actif    = false; // Mode blink activé ou non
int   blink_interval = 500;
bool  blink_etat     = false; // Etat actuelle de la Led, allumé ou éteinte
long  blink_last     = 0;

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

  if(msg == "led_on"){
    blink_actif = false;  // stopper le blink si actif
    digitalWrite(LED_BUILTIN, HIGH); //Mettre la LED à l'état haut. 
    Serial.println("[LED] ON");
  } else if (msg == "led_off"){
    blink_actif = false;  // stopper le blink si actif
    digitalWrite(LED_BUILTIN, LOW); //Mettre la LED à l'état bas. 
    Serial.println("[LED] OFF");
  }else if (msg.startsWith("led_blink:")) {
    blink_interval = msg.substring(10).toInt(); // extraire le nombre X apres "led_blink:X, correspondnant à l'intervalle"
    blink_actif = true;
    blink_last = millis();
    Serial.print("[LED] BLINK intervalle = ");
    Serial.print(blink_interval);
    Serial.println(" ms");
  }
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
      mqttClient.subscribe(TOPIC_LED);
      Serial.print("[MQTT] Subscribe sur : ");
      Serial.println(TOPIC_LED);
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


void setup_led(){
  //Configurer le pin de la LED comme sortie 
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);// La broche sera mise à une tension de 0V => état eteint pour une led
}

void setup() {
  // Initialiser le moniteur série pour afficher l'état de la connexion.
  Serial.begin(115200);
  delay(500);  

  setup_led();
  setup_wifi();
  setup_mqtt();
}


void loop() {
  if (!mqttClient.connected()) setup_mqtt(); // reconnexion si coupure
  mqttClient.loop(); //Déclenche on_message si mesage en attente

  // Gestion du blink en autonomie — non bloquant avec millis()
  if (blink_actif && (millis() - blink_last >= (unsigned long)blink_interval)) {
    blink_etat = !blink_etat;
    digitalWrite(LED_BUILTIN, blink_etat ? HIGH : LOW);
    blink_last = millis();
  }
}
