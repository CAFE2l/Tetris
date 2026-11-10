// TETRIS ARDUINO — JOGADOR 1 (P1)
// Arduino Joystick Shield + Arduino UNO | 9600 baud
//
// Joystick Shield:
//   A / D2 → cima; B / D3 → direita; C / D4 → baixo; D / D5 → esquerda
//   E / D6 → abrir habilidades (exceto no Zen); especial 360° no X1.
//   F / D7 → ativar habilidade selecionada (ou habilidade Zen); guardar no X1.
//   K / D8 → guardar/trocar peça em 1P; abrir inventário em 2P.
//   No multiplayer clássico, E / D6 e F / D7 guardam/trocam a peça.
//   A0 / A1 → X / Y analógicos para carregar o giro 360° no X1.
//   D9    → queda instantânea (entrada auxiliar, se conectada)
//
// Serial: P1:E, P1:F e P1:K identificam os botões físicos sem ambiguidade.
// Os botões usam INPUT_PULLUP e são ativos em LOW.

#define BTN_UP   2
#define BTN_RGT  3
#define BTN_DWN  4
#define BTN_LFT  5
#define BTN_E    6
#define BTN_F    7
#define BTN_K    8
#define BTN_HARD 9

const unsigned long DEBOUNCE_MS = 35;
const unsigned long HOLD_DELAY_MS = 200;
const unsigned long HOLD_REPEAT_MS = 80;
const unsigned long JOYSTICK_REPORT_MS = 40;
unsigned long lastJoystickReport = 0;

struct Button {
  uint8_t pin;
  const char* command;
  bool repeat;
  bool stableState;
  bool reading;
  unsigned long changedAt;
  unsigned long pressedAt;
  unsigned long lastRepeat;
};

Button buttons[] = {
  { BTN_UP, "UP", false, HIGH, HIGH, 0, 0, 0 },
  { BTN_RGT, "RIGHT", true, HIGH, HIGH, 0, 0, 0 },
  { BTN_DWN, "DOWN", true, HIGH, HIGH, 0, 0, 0 },
  { BTN_LFT, "LEFT", true, HIGH, HIGH, 0, 0, 0 },
  { BTN_E, "E", false, HIGH, HIGH, 0, 0, 0 },
  { BTN_F, "F", false, HIGH, HIGH, 0, 0, 0 },
  { BTN_K, "K", false, HIGH, HIGH, 0, 0, 0 },
  { BTN_HARD, "BTN_B", false, HIGH, HIGH, 0, 0, 0 },
};
const uint8_t BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

void sendCommand(const char* command) {
  Serial.print("P1:");
  Serial.println(command);
}

void sendJoystickPosition() {
  Serial.print("P1:JOY:");
  Serial.print(analogRead(A0));
  Serial.print(',');
  Serial.println(analogRead(A1));
}

void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  delay(300);
  Serial.println("ARDUINO_P1_READY");
}

void loop() {
  const unsigned long now = millis();

  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    Button& button = buttons[i];
    const bool current = digitalRead(button.pin);

    if (current != button.reading) {
      button.reading = current;
      button.changedAt = now;
    }

    if (button.reading != button.stableState &&
        now - button.changedAt >= DEBOUNCE_MS) {
      button.stableState = button.reading;
      if (button.stableState == LOW) {
        sendCommand(button.command);
        button.pressedAt = now;
        button.lastRepeat = now;
      }
    } else if (button.repeat &&
               button.stableState == LOW &&
               now - button.pressedAt >= HOLD_DELAY_MS &&
               now - button.lastRepeat >= HOLD_REPEAT_MS) {
      sendCommand(button.command);
      button.lastRepeat = now;
    }
  }

  if (now - lastJoystickReport >= JOYSTICK_REPORT_MS) {
    lastJoystickReport = now;
    sendJoystickPosition();
  }

  delay(2);
}
