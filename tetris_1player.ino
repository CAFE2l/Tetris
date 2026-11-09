// ============================================================
//  TETRIS POP! — CONTROLE DO JOGADOR 1 (P1)
//  Arduino UNO + Joystick Shield | Baud rate: 9600
//
//  BOTÕES (pino → comando enviado ao jogo):
//    BTN_UP  (2) → UP      girar peça  | no menu: VOLTAR
//    BTN_RGT (3) → RIGHT   mover →     | no menu: próximo cartão
//    BTN_DWN (4) → DOWN    descer      | no menu: CLICAR / CONFIRMAR
//    BTN_LFT (5) → LEFT    mover ←     | no menu: cartão anterior
//    BTN_SEL (6) → SELECT  pausar      | no menu: voltar
//    BTN_STR (7) → (reservado, não envia nada)
//    BTN_A   (8) → BTN_A   girar peça  | no menu: CONFIRMAR
//    BTN_B   (9) → BTN_B   queda instantânea | no menu: voltar
//
//  PROTOCOLO (uma linha por evento):
//    P1:LEFT        → botão acabou de ser apertado
//    P1:LEFT_HOLD   → botão segurado (só LEFT, RIGHT e DOWN repetem)
//    P1:PING        → sinal de vida a cada 2 s (o site usa para
//                     reconhecer que este controle é o P1)
//
//  O joystick analógico fica desativado.
// ============================================================

#define PLAYER_ID "P1"

#define BTN_UP   2
#define BTN_RGT  3
#define BTN_DWN  4
#define BTN_LFT  5
#define BTN_SEL  6
#define BTN_STR  7
#define BTN_A    8
#define BTN_B    9

const unsigned long DEBOUNCE_MS    = 25;    // ignora trepidação do botão
const unsigned long HOLD_DELAY_MS  = 220;   // espera antes de começar a repetir
const unsigned long HOLD_REPEAT_MS = 75;    // intervalo entre repetições
const unsigned long PING_MS        = 2000;  // sinal de vida

struct Btn {
  uint8_t       pin;
  const char*   cmd;
  bool          repeat;     // repete ao segurar?
  bool          down;       // está pressionado?
  unsigned long lastLow;    // última vez que leu LOW
  unsigned long pressedAt;
  unsigned long lastRep;
};

Btn btns[] = {
  { BTN_UP,  "UP",     false, false, 0, 0, 0 },
  { BTN_RGT, "RIGHT",  true,  false, 0, 0, 0 },
  { BTN_DWN, "DOWN",   true,  false, 0, 0, 0 },
  { BTN_LFT, "LEFT",   true,  false, 0, 0, 0 },
  { BTN_SEL, "SELECT", false, false, 0, 0, 0 },
  { BTN_A,   "BTN_A",  false, false, 0, 0, 0 },
  { BTN_B,   "BTN_B",  false, false, 0, 0, 0 },
};
const int BTN_COUNT = sizeof(btns) / sizeof(btns[0]);

unsigned long lastPing = 0;

void sendCmd(const char* cmd, bool hold) {
  Serial.print(PLAYER_ID);
  Serial.print(':');
  Serial.print(cmd);
  if (hold) Serial.print("_HOLD");
  Serial.println();
}

void setup() {
  Serial.begin(9600);

  pinMode(BTN_UP,  INPUT_PULLUP);
  pinMode(BTN_RGT, INPUT_PULLUP);
  pinMode(BTN_DWN, INPUT_PULLUP);
  pinMode(BTN_LFT, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(BTN_STR, INPUT_PULLUP);
  pinMode(BTN_A,   INPUT_PULLUP);
  pinMode(BTN_B,   INPUT_PULLUP);

  delay(500);
  Serial.println("ARDUINO_P1_READY");
}

void loop() {
  unsigned long now = millis();

  for (int i = 0; i < BTN_COUNT; i++) {
    Btn& b = btns[i];

    if (digitalRead(b.pin) == LOW) {
      b.lastLow = now;

      if (!b.down) {
        // Aperto agora: envia na hora (sem atraso)
        b.down      = true;
        b.pressedAt = now;
        b.lastRep   = now;
        sendCmd(b.cmd, false);
      }
      else if (b.repeat &&
               now - b.pressedAt >= HOLD_DELAY_MS &&
               now - b.lastRep   >= HOLD_REPEAT_MS) {
        // Segurado: repete marcando como HOLD
        b.lastRep = now;
        sendCmd(b.cmd, true);
      }
    }
    else if (b.down && now - b.lastLow >= DEBOUNCE_MS) {
      // Só considera "solto" depois de ficar solto por DEBOUNCE_MS
      b.down = false;
    }
  }

  if (now - lastPing >= PING_MS) {
    lastPing = now;
    sendCmd("PING", false);
  }
}
