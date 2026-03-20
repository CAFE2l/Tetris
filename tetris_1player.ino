// ============================================
// TETRIS ARDUINO — JOGADOR 1 (P1)
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
//   BTN_A   (pino 8)  → (reservado)
//   BTN_STR (pino 7)  → (reservado)
//
// Joystick analógico DESABILITADO.
// ============================================

// Definição dos pinos dos botões
#define BTN_UP   2
#define BTN_DWN  4
#define BTN_LFT  5
#define BTN_RGT  3
#define BTN_SEL  6
#define BTN_STR  7
#define BTN_A    8
#define BTN_B    9

// Timings para debounce e repetição
const unsigned long DEBOUNCE_MS    = 50;    // Debounce para todos os botões
const unsigned long HOLD_DELAY_MS  = 200;   // Espera antes de repetir ao segurar
const unsigned long HOLD_REPEAT_MS = 80;    // Intervalo de repetição

// Estrutura para botões direcionais com hold-repeat
struct DirectionalButton {
  int           pin;
  const char*   command;
  bool          lastState;
  bool          holding;
  unsigned long pressTime;
  unsigned long lastRepeat;
};

// Configuração dos botões direcionais
DirectionalButton dirButtons[] = {
  { BTN_LFT, "LEFT",  HIGH, false, 0, 0 },
  { BTN_RGT, "RIGHT", HIGH, false, 0, 0 },
  { BTN_DWN, "DOWN",  HIGH, false, 0, 0 },
};
const int DIR_COUNT = 3;

// Estados dos botões de ação única
bool upLast   = HIGH;
bool bLast    = HIGH;
bool aLast    = HIGH;
bool selLast  = HIGH;

// ─────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  
  // Configurar todos os pinos como INPUT_PULLUP
  pinMode(BTN_UP,  INPUT_PULLUP);
  pinMode(BTN_DWN, INPUT_PULLUP);
  pinMode(BTN_LFT, INPUT_PULLUP);
  pinMode(BTN_RGT, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(BTN_STR, INPUT_PULLUP);
  pinMode(BTN_A,   INPUT_PULLUP);
  pinMode(BTN_B,   INPUT_PULLUP);
  
  // Aguardar estabilização
  delay(1500);
  
  // Sinalizar que o Arduino está pronto
  Serial.println("ARDUINO_P1_READY");
  Serial.flush();
}

// Função para enviar comando formatado
void sendCommand(const char* cmd) {
  Serial.print("P1:");
  Serial.println(cmd);
  Serial.flush();
}

// ─────────────────────────────────────────────
void loop() {
  unsigned long now = millis();
  
  // ── BTN_UP → GIRAR (fire-once) ──
  bool upNow = digitalRead(BTN_UP);
  if (upNow == LOW && upLast == HIGH) {
    sendCommand("UP");
    delay(DEBOUNCE_MS);  // Pequeno delay para debounce
  }
  upLast = upNow;
  
  // ── BTN_B → HARD DROP (fire-once) ──
  bool bNow = digitalRead(BTN_B);
  if (bNow == LOW && bLast == HIGH) {
    sendCommand("BTN_B");
    delay(DEBOUNCE_MS);
  }
  bLast = bNow;
  
  // ── BTN_A → (reservado para futuras funções) ──
  bool aNow = digitalRead(BTN_A);
  if (aNow == LOW && aLast == HIGH) {
    sendCommand("BTN_A");
    delay(DEBOUNCE_MS);
  }
  aLast = aNow;
  
  // ── BTN_SEL → PAUSAR (fire-once) ──
  bool selNow = digitalRead(BTN_SEL);
  if (selNow == LOW && selLast == HIGH) {
    sendCommand("SELECT");
    delay(DEBOUNCE_MS);
  }
  selLast = selNow;
  
  // ── Botões direcionais LEFT, RIGHT, DOWN com hold-repeat ──
  for (int i = 0; i < DIR_COUNT; i++) {
    DirectionalButton& btn = dirButtons[i];
    bool currentState = digitalRead(btn.pin);
    
    if (currentState == LOW) {  // Botão pressionado
      if (btn.lastState == HIGH) {
        // Primeiro toque: envia comando imediatamente
        sendCommand(btn.command);
        btn.holding = true;
        btn.pressTime = now;
        btn.lastRepeat = now;
      } 
      else if (btn.holding && 
               (now - btn.pressTime >= HOLD_DELAY_MS) && 
               (now - btn.lastRepeat >= HOLD_REPEAT_MS)) {
        // Botão segurado: repete comando no intervalo definido
        sendCommand(btn.command);
        btn.lastRepeat = now;
      }
    } 
    else {  // Botão solto
      btn.holding = false;
    }
    
    btn.lastState = currentState;
  }
  
  // Pequeno delay para evitar envio excessivo de dados
  delay(8);
}