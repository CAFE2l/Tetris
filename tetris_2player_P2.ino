// TETRIS POP! — CONTROLE DO JOGADOR 2 (P2)
// Arduino Joystick Shield + Arduino UNO | 9600 baud
//
// Joystick Shield:
//   A / D2 → cima; B / D3 → direita; C / D4 → baixo; D / D5 → esquerda
//   E / D6 → evento E
//   F / D7 → evento F
//   K / D8 → ação BTN_A existente
//   D9    → queda instantânea (entrada auxiliar, se conectada)
//
// Protocolo: P2:LEFT, P2:LEFT_HOLD, P2:E, P2:F, P2:K e P2:PING.
// Os botões usam INPUT_PULLUP e são ativos em LOW.

#define PLAYER_ID "P2"

#define BTN_UP   2
#define BTN_RGT  3
#define BTN_DWN  4
#define BTN_LFT  5
#define BTN_E    6
#define BTN_F    7
#define BTN_K    8
#define BTN_HARD 9

const unsigned long DEBOUNCE_MS = 35;
const unsigned long HOLD_DELAY_MS = 220;
const unsigned long HOLD_REPEAT_MS = 75;
const unsigned long PING_MS = 2000;

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

unsigned long lastPing = 0;

void sendCommand(const char* command, bool hold = false) {
  Serial.print(PLAYER_ID);
  Serial.print(':');
  Serial.print(command);
  if (hold) Serial.print("_HOLD");
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }
  delay(300);
  Serial.println("ARDUINO_P2_READY");
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
      sendCommand(button.command, true);
      button.lastRepeat = now;
    }
  }

  if (now - lastPing >= PING_MS) {
    lastPing = now;
    sendCommand("PING");
  }
  delay(2);
}
