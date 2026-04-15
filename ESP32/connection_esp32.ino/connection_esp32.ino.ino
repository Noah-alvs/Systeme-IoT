
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


// TOPICS broker -> esp32
const char* TOPIC_LED    = "esp32/led";
const char* TOPIC_BUZZER = "esp32/buzzer";
const char* TOPIC_OLED   = "esp32/oled";

//TOPICS esp32 -> broker
const char* TOPIC_LUMIERE  = "esp32/lumiere";
const char* TOPIC_BOUTON = "esp32/bouton";

// PINs
#define BUZZER_PIN 17
#define PHOTORESISTANCE_PIN 36
#define BP_PIN 23




// Variables LED
bool  blink_actif    = false; // Mode blink activé ou non
int   blink_interval = 500;
bool  blink_etat     = false; // Etat actuelle de la Led, allumé ou éteinte
long  blink_last     = 0;
bool led_auto_actif = false; // Quand actif, la frequence de blink varie en fonction de la photoresistance

// Variables beep buzzer
bool buz_beep_actif = false;
int  buz_beep_count = 0;
bool buz_beep_etat  = false;
long buz_beep_last  = 0;
#define BUZ_BEEP_INTERVAL 200
#define BUZZER_CHANNEL  0     // canal PWM (0 à 15 disponibles sur ESP32)
#define BUZZER_FREQ     1000  // frequence en Hz
#define BUZZER_RES      8     // resolution 8 bits (valeurs 0-255)

//define et variable OLED
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     16
String oled_texte_site = ""; //Stocke le dernier texte envoye depuis le site web. step_oled_lum l'affiche en ligne 2 a chaque rafraichissement.

// Declaration de l'objet display (OLED)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Variable Bouton Poussoir
bool bp_etat_prec = HIGH; 


WiFiClient   wifiClient; //Client wifi
PubSubClient mqttClient(wifiClient); // Création de l'objet client pour gérer la connexion MQTT


// WaitFor : timer non bloquant Divise le temps en periodes et retourne 1
// quand une nouvelle periode est ecoulee.
#define MAX_WAIT_FOR_TIMER 4
unsigned long waitFor(int timer, unsigned long period) {
  static unsigned long last_period[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches
  unsigned long current = micros() / period;             // numéro de période
  unsigned long delta   = current - last_period[timer];  // gère le wrap-around
  if (delta) last_period[timer] = current;               // mise à jour si déclenchement
  return delta;                                          // nombre de periode depuis le dernier appel
}



// Mailbox : boite aux lettres entre deux taches
// Producteur ecrit si EMPTY, consommateur lit si FULL
enum { EMPTY, FULL };

typedef struct {
  int state;  // EMPTY ou FULL
  int val;    // valeur deposee par le producteur
} mailbox_t;


typedef struct { int timer; unsigned long period; } ctx_lum_t;
typedef struct { int timer; unsigned long period; } ctx_mqtt_t;
typedef struct { int timer; unsigned long period; } ctx_oled_t;

mailbox_t mb_lum_oled = { .state = EMPTY };
mailbox_t mb_lum_mqtt = { .state = EMPTY };
mailbox_t mb_lum_led  = { .state = EMPTY };

// Prototypes
void init_lum(ctx_lum_t* ctx, int timer, unsigned long period);
void step_lum(ctx_lum_t* ctx, mailbox_t* mb_oled, mailbox_t* mb_mqtt, mailbox_t* mb_led);
void init_mqtt_lum(ctx_mqtt_t* ctx, int timer, unsigned long period);
void step_mqtt_lum(ctx_mqtt_t* ctx, mailbox_t* mb);
void init_oled_lum(ctx_oled_t* ctx, int timer, unsigned long period);
void step_oled_lum(ctx_oled_t* ctx, mailbox_t* mb);



void init_lum(ctx_lum_t* ctx, int timer, unsigned long period) {
  ctx->timer  = timer;
  ctx->period = period;
}

void step_lum(ctx_lum_t* ctx, mailbox_t* mb_oled, mailbox_t* mb_mqtt, mailbox_t* mb_led) {
    if (!waitFor(ctx->timer, ctx->period)) return;

    int v   = analogRead(PHOTORESISTANCE_PIN);
    int pct = map(v, 0, 4095, 100, 0);  // conversion en pourcentage : 0=sombre, 100=lumineux

    // Deposer dans chaque mailbox si vide
    if (mb_oled->state == EMPTY) { mb_oled->val = pct; mb_oled->state = FULL; }
    if (mb_mqtt->state == EMPTY) { mb_mqtt->val = pct; mb_mqtt->state = FULL; }
    if (mb_led->state  == EMPTY) { mb_led->val  = pct; mb_led->state  = FULL; }
}





// Tache mqtt_lum — publie la luminosite vers le broker
void init_mqtt_lum(ctx_mqtt_t* ctx, int timer, unsigned long period) {
  ctx->timer  = timer;
  ctx->period = period;
}

void step_mqtt_lum(ctx_mqtt_t* ctx, mailbox_t* mb) {
  if (mb->state != FULL) return;       // rien a publier
  if (!waitFor(ctx->timer, ctx->period)) return;

  mqttClient.publish(TOPIC_LUMIERE, String(mb->val).c_str());
  Serial.print("[CAPTEUR] Luminosite publiee : ");
  Serial.print(mb->val);
  Serial.println("%");

  mb->state = EMPTY;
}





// Tache oled_lum — rafraichit l'ecran OLED :
//   Ligne 1  : luminosite en % 
//   Ligne 2  : dernier texte recu du site web
void init_oled_lum(ctx_oled_t* ctx, int timer, unsigned long period) {
  ctx->timer  = timer;
  ctx->period = period;
}

void step_oled_lum(ctx_oled_t* ctx, mailbox_t* mb) {
  if (mb->state != FULL) return;
  if (!waitFor(ctx->timer, ctx->period)) return;

  display.clearDisplay();

  // Ligne 1 — luminosite 
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("Lum: ");
  display.print(mb->val);
  display.println("%");

  // Ligne 2 — texte recu du site
  display.setCursor(0, 12);
  display.println(oled_texte_site);

  display.display();
  mb->state = EMPTY;
}

//Contexte des taches
ctx_lum_t  Lum1;
ctx_mqtt_t Mqtt1;
ctx_oled_t Oled1;


//Gestion de la LED
void handle_led(String msg) {
  if (msg == "led_on") {
    led_auto_actif = false;   // desactiver le mode auto
    blink_actif    = false; // stopper le blink si actif
    blink_etat     = true; //Indique que la LED est allumé
    digitalWrite(LED_BUILTIN, HIGH); //Mettre la LED à l'état haut. 
    Serial.println("[LED] ON");

  } else if (msg == "led_off") {
    led_auto_actif = false;   // desactiver le mode auto
    blink_actif    = false; // stopper le blink si actif
    blink_etat     = false; //Indique que la LED est eteint
    digitalWrite(LED_BUILTIN, LOW); //Mettre la LED à l'état bas. 
    Serial.println("[LED] OFF");

  } else if (msg.startsWith("led_blink:")) { // extraire le nombre X apres "led_blink:X, correspondnant à l'intervalle"
    led_auto_actif = false;   // desactiver le mode auto
    blink_interval = msg.substring(10).toInt();
    blink_actif    = true;  // stopper le blink si actif
    blink_etat     = false; //Indique que la LED est eteint
    digitalWrite(LED_BUILTIN, LOW); //Commencer le cycle à partir de la Led éteinte
    blink_last = millis();
    Serial.print("[LED] BLINK ");
    Serial.print(blink_interval);
    Serial.println("ms");

  } else if (msg == "led_auto") {
    // Mode auto : la frequence de blink varie avec la luminosite
    led_auto_actif = true;
    blink_actif    = true;   // le blink demarre, loop_led le gere
    Serial.println("[LED] AUTO - variation selon luminosite");
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


// Gestion OLED
// Stock le texte dans oled_text_site qui sera affiché par step_oled_lum
void handle_oled(String msg) {
  if (msg.startsWith("oled:")) {
    oled_texte_site = msg.substring(5);  // stocker le texte
    Serial.print("[OLED] Texte recu : ");
    Serial.println(oled_texte_site);

  } else if (msg == "oled_clear") {
    oled_texte_site = "";  // effacer le texte stocké
    Serial.println("[OLED] Texte efface");
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

void setup_bp() {
  pinMode(BP_PIN, INPUT_PULLUP);  // pullup interne
  // Publier l'etat initial au demarrage
  bool etat = digitalRead(BP_PIN);
  mqttClient.publish(TOPIC_BOUTON, etat == LOW ? "appuye" : "relache");
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
    delay(1000);
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
      delay(2000);
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
  setup_bp();
  setup_wifi();
  setup_mqtt();

  init_lum(&Lum1,      1, 1000000/2);   // lecture toutes les 0.5s
  init_mqtt_lum(&Mqtt1, 2, 1000000/2);  // publication toutes les 0.5s
  init_oled_lum(&Oled1, 3, 1000000/2);   // affichage toutes les 0.5s
}

// Gestion du blink non bloquant 
void loop_led(){
  // Mode auto : adapter l'intervalle de blink a la luminosite
  if (led_auto_actif && mb_lum_led.state == FULL) {
    blink_interval   = map(mb_lum_led.val, 0, 100, 2000, 50);
    blink_actif      = true;
    mb_lum_led.state = EMPTY;
    Serial.print("[LED AUTO] intervalle = ");
    Serial.println(blink_interval);
  }

  // Mode blink
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

void loop_bp() {
  bool etat = digitalRead(BP_PIN);
  if (etat != bp_etat_prec) {  // publier seulement si changement d'etat
    bp_etat_prec = etat;
    mqttClient.publish(TOPIC_BOUTON, etat == LOW ? "appuye" : "relache");
    Serial.print("[BP] ");
    Serial.println(etat == LOW ? "appuye" : "relache");
  }
}


void loop() {
  if (!mqttClient.connected()) setup_mqtt(); // reconnexion si coupure
  mqttClient.loop(); //Déclenche on_message si mesage en attente

  loop_led();
  loop_buzzer();
  loop_bp();

  step_lum(&Lum1, &mb_lum_oled, &mb_lum_mqtt, &mb_lum_led);
  step_oled_lum(&Oled1, &mb_lum_oled);
  step_mqtt_lum(&Mqtt1, &mb_lum_mqtt);
}
