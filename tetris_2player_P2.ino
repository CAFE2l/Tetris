// ============================================
// TETRIS ARDUINO — 2 JOGADORES
// ESTE ARQUIVO: JOGADOR 2 (P2)
// Joystick Shield + Arduino UNO
// Baud Rate: 9600
//
// MAPEAMENTO DOS BOTÕES:
//   BTN_LFT (pino 5)  → Mover peça para ESQUERDA
//   BTN_RGT (pino 3)  → Mover peça para DIREITA
//   BTN_DWN (pino 4)  → Descer peça rápido (soft drop)
//   BTN_UP  (pino 2)  → GIRAR peça
//   BTN_B   (pino 9)  → Hard Drop (queda instantânea)
//   BTN_SEL (pino 6)  → Pausar / Continuar
//
// Joystick analógico DESABILITADO.
// ============================================

#define BTN_UP  2
#define BTN_DWN 4
#define BTN_LFT 5
#define BTN_RGT 3
#define BTN_SEL 6
#define BTN_STR 7   // reservado
#define BTN_A   8   // reservado
#define BTN_B   9   // Hard Drop

// Timings
const unsigned long DEBOUNCE_MS    = 100;
const unsigned long HOLD_DELAY_MS  = 280;  // espera antes de repetir ao segurar
const unsigned long HOLD_REPEAT_MS = 90;   // intervalo de repeticao

// Struct para botoes com hold-repeat
struct DirBtn {
  int           pin;
  const char*   cmd;
  bool          lastState;
  bool          holding;
  unsigned long pressTime;
  unsigned long lastRepeat;
};

DirBtn dirBtns[] = {
  { BTN_LFT, "LEFT",  HIGH, false, 0, 0 },
  { BTN_RGT, "RIGHT", HIGH, false, 0, 0 },
  { BTN_DWN, "DOWN",  HIGH, false, 0, 0 },
};
const int DIR_COUNT = 3;

// Botoes fire-once (borda de descida)
bool upLast  = HIGH;
bool bLast   = HIGH;
bool selLast = HIGH;

// ─────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(BTN_UP,  INPUT_PULLUP);
  pinMode(BTN_DWN, INPUT_PULLUP);
  pinMode(BTN_LFT, INPUT_PULLUP);
  pinMode(BTN_RGT, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(BTN_STR, INPUT_PULLUP);
  pinMode(BTN_A,   INPUT_PULLUP);
  pinMode(BTN_B,   INPUT_PULLUP);

  delay(1500);
  Serial.println("ARDUINO_P2_READY");
}

void sendCmd(const char* cmd) {
  Serial.print("P2:");
  Serial.println(cmd);
}

// ─────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // BTN_UP → GIRAR (fire-once na borda de descida)
  bool upNow = digitalRead(BTN_UP);
  if (upNow == LOW && upLast == HIGH) {
    sendCmd("BTN_A");
  }
  upLast = upNow;

  // BTN_B → HARD DROP (fire-once)
  bool bNow = digitalRead(BTN_B);
  if (bNow == LOW && bLast == HIGH) {
    sendCmd("BTN_B");
  }
  bLast = bNow;

  // BTN_SEL → PAUSAR (fire-once)
  bool selNow = digitalRead(BTN_SEL);
  if (selNow == LOW && selLast == HIGH) {
    sendCmd("SELECT");
  }
  selLast = selNow;

  // LEFT / RIGHT / DOWN com hold-repeat
  for (int i = 0; i < DIR_COUNT; i++) {
    DirBtn& b = dirBtns[i];
    bool cur = digitalRead(b.pin);

    if (cur == LOW) {
      if (b.lastState == HIGH) {
        // primeiro toque: envia imediatamente
        sendCmd(b.cmd);
        b.holding    = true;
        b.pressTime  = now;
        b.lastRepeat = now;
      } else if (b.holding &&
                 (now - b.pressTime  >= HOLD_DELAY_MS) &&
                 (now - b.lastRepeat >= HOLD_REPEAT_MS)) {
        // segurado: repete
        sendCmd(b.cmd);
        b.lastRepeat = now;
      }
    } else {
      b.holding = false;
    }

    b.lastState = cur;
  }

  delay(8);
}
