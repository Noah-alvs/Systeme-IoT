// --------------------------------------------------------------------------------------------------------------------
// Multi-tâches cooperatives : solution basique mais efficace :-)
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// unsigned int waitFor(timer, period) 
// Timer pour taches périodiques 
// configuration :
//  - MAX_WAIT_FOR_TIMER : nombre maximum de timers utilisés
// arguments :
//  - timer  : numéro de timer entre 0 et MAX_WAIT_FOR_TIMER-1
//  - period : période souhaitée
// retour :
//  - nombre de périodes écoulées depuis le dernier appel
// --------------------------------------------------------------------------------------------------------------------
#define MAX_WAIT_FOR_TIMER 2
unsigned long waitFor(int timer, unsigned long period) {
    static unsigned long last_period[MAX_WAIT_FOR_TIMER];  // il y a autant de timers que de tâches
    unsigned long current = micros() / period;             // numéro de période
    unsigned long delta   = current - last_period[timer];  // gère le wrap-around
    if (delta) last_period[timer] = current;               // mise à jour si déclenchement
    return delta;                                          // nombre de periode depuis le dernier appel
}

//--------- déclaration de la tache Led

typedef struct {
  int timer;                                              // n° du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int pin;                                                // numéro de la broche sur laquelle est la LED
  int etat;                                               // etat interne de la led
} ctx_led_t; 

void init_led(ctx_led_t * ctx, int timer, unsigned long period, byte pin);
void step_led(ctx_led_t * ctx);

//--------- déclaration de la tache Mess

typedef struct {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                   // periode d'affichage
  char mess[20];
} ctx_mess_t; 

void init_mess(ctx_mess_t * ctx, int timer, unsigned long period, const char * mess);
void step_mess(ctx_mess_t *ctx);

//--------- définition de la tache Led

void init_led(ctx_led_t * ctx, int timer, unsigned long period, byte pin) {
  ctx->timer = timer;
  ctx->period = period;
  ctx->pin = pin;
  ctx->etat = 0;
  pinMode(pin,OUTPUT);
  digitalWrite(pin, ctx->etat);
}

void step_led(ctx_led_t * ctx) {
  if (!waitFor(ctx->timer, ctx->period)) return;          // sort s'il y a moins d'une période écoulée
  digitalWrite(ctx->pin,ctx->etat);                       // ecriture
  ctx->etat = 1 - ctx->etat;                              // changement d'état
}

//--------- definition de la tache Mess

void init_mess(ctx_mess_t * ctx, int timer, unsigned long period, const char * mess) {
  ctx->timer = timer;
  ctx->period = period;
  strcpy(ctx->mess, mess);
  Serial.begin(9600);                                     // initialisation du débit de la liaison série
}

void step_mess(ctx_mess_t *ctx) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
  Serial.println(ctx->mess);                              // affichage du message
}

//--------- déclaration des contextes de tâches

ctx_led_t Led1;
ctx_mess_t Mess1;

//--------- Setup et Loop

void setup() {
  init_led(&Led1, 0, 100000, LED_BUILTIN);               // Led est exécutée toutes les 100ms 
  init_mess(&Mess1, 1, 1000000, "bonjour");              // Mess est exécutée toutes les secondes 
}

void loop() {
  step_led(&Led1);                                        
  step_mess(&Mess1); 
}    