

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define MAX_WAIT_FOR_TIMER 3
unsigned long waitFor(int timer, unsigned long period) {
    static unsigned long last_period[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches
    unsigned long current = micros() / period;             // numéro de période
    unsigned long delta   = current - last_period[timer];  // gère le wrap-around
    if (delta) last_period[timer] = current;               // mise à jour si déclenchement
    return delta;                                          // nombre de periode depuis le dernier appel
}


enum {EMPTY, FULL};

typedef struct {
  int state;
  int val;
}mailbox_t;

mailbox_t mb1 = {.state = EMPTY};
mailbox_t mb2 = {.state = EMPTY};


//--------déclaration de la tache compteur sur OLED
typedef struct {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                   // periode d'affichage
} ctx_oled_t; 

void init_oled(ctx_oled_t * ctx, int timer, unsigned long period);
void step_oled(ctx_oled_t * ctx, mailbox_t * mb);

//--------déclaration de la tache lum
typedef struct {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                   // periode d'affichage
} ctx_lum_t; 


void init_lum(ctx_lum_t * ctx, int timer, unsigned long period);
void step_lum(ctx_lum_t * ctx, mailbox_t * mb1, mailbox_t * mb2);

//--------- déclaration de la tache Led
typedef struct {
  int timer;                                              // n° du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;                                               // etat interne de la led
} ctx_led_t; 

void init_led(ctx_led_t * ctx, int timer, unsigned long period, byte pin);
void step_led(ctx_led_t * ctx, mailbox_t * mb);


void init_led(ctx_led_t * ctx, int timer, unsigned long period, byte pin) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->pin = pin;
  ctx->etat = 0;
  pinMode(pin,OUTPUT);
  digitalWrite(pin, ctx->etat);
}

void step_led(ctx_led_t * ctx, mailbox_t * mb_lum, mailbox_t * mb_isr){
  if(mb_lum->state == FULL)
  {
    ctx->period = map(mb_lum->val, 0, 100, 50000, 2000000);
    mb_lum->state = EMPTY;
  }
  if (!waitFor(ctx->timer, ctx->period)) return;
  digitalWrite(ctx->pin,ctx->etat);                       // ecriture
  ctx->etat = 1 - ctx->etat;                              // changement d'état
}


//--------- définition de la tache oled 
void init_oled(ctx_oled_t * ctx, int timer, unsigned long period){
  Wire.begin(4, 15); // pins SDA , SCL
  Serial.begin(9600);
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  ctx->timer = timer;
  ctx->period = period;
   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}

void step_oled(ctx_oled_t * ctx, mailbox_t * mb) {
  if (mb->state != FULL) return; // attend que la mailbox soit pleine
  
  if (!waitFor(ctx->timer, ctx->period)) return; 
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(mb->val);
  display.display();
  mb->state = EMPTY;
}


//--------- définition de la tache lum
void init_lum(ctx_lum_t * ctx, int timer, unsigned long period){
  ctx->timer = timer;
  ctx->period = period;
}

void step_lum(ctx_lum_t * ctx, mailbox_t * mb1, mailbox_t * mb2) {
   if (!waitFor(ctx->timer, ctx->period)) return; 
   int v = analogRead(36);
   int pct = map(v,0, 4095, 100, 0);

   if(mb1->state == EMPTY){
      mb1->val = pct;
      mb1->state = FULL;
   }

   if(mb2->state == EMPTY){
      mb2->val = pct;
      mb2->state = FULL;
   }
}



//--------- déclaration des contextes de tâches


ctx_oled_t Oled1;
ctx_lum_t Lum1;
ctx_led_t Led1;

void setup() {
  init_led(&Led1, 0, 2000000, LED_BUILTIN);               // Led est exécutée toutes les 100ms 
  init_lum(&Lum1, 1, 1000000/2);
  init_oled(&Oled1, 2, 1000000/2);  

}

void loop() {
  step_led(&Led1, &mb2);
  step_lum(&Lum1,&mb1, &mb2);
  step_oled(&Oled1, &mb1);
}
