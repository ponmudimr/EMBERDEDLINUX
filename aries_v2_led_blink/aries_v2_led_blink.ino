/*
 * Sketch: aries_v2_led_blink.ino
 * Target Board: C-DAC ARIES v2.0 (THEJAS32 SoC, VEGA ET1031 RISC-V)
 * FQBN: vega:riscv:aries_v2
 * 
 * Description:
 * DIP Switch Controlled LED Modes with Serial Monitor Output:
 * - SW1 ON, SW2 OFF : Blinks LED1, LED2, & LED3 sequentially one after another (500 ms).
 * - SW2 ON, SW1 OFF : All LEDs blink TOGETHER with 2 second (2000 ms) interval.
 * - BOTH OFF        : LED 1 blinks with 1000 ms interval, CHANGING COLOR after each blink 
 *                     (Red -> Green -> Blue -> Cyan -> Purple -> Yellow -> White).
 * - BOTH ON         : Only LED 1 blinks RED (500 ms).
 */

// DIP Switch Pins
const int DIP_SW1   = 16; // GPIO 16
const int DIP_SW2   = 17; // GPIO 17

// Onboard LED Pins
const int LD1_GREEN = 22; // GPIO 22
const int LD1_BLUE  = 23; // GPIO 23
const int LD1_RED   = 24; // GPIO 24

const int LD2_PIN   = 21; // GPIO 21
const int LD3_PIN   = 20; // GPIO 20

// Color names array for serial logging
const char* colorNames[] = {
  "RED", "GREEN", "BLUE", "CYAN", "PURPLE", "YELLOW", "WHITE"
};

// Color index for LED 1 when both switches are OFF
int colorIndex = 0;

// Turn OFF all LEDs
void turnOffAll() {
  digitalWrite(LD1_GREEN, HIGH);
  digitalWrite(LD1_BLUE,  HIGH);
  digitalWrite(LD1_RED,   HIGH);
  digitalWrite(LD2_PIN, LOW);
  digitalWrite(LD3_PIN, LOW);
}

// Turn ON all LEDs
void turnOnAll() {
  digitalWrite(LD1_GREEN, LOW);
  digitalWrite(LD1_BLUE,  LOW);
  digitalWrite(LD1_RED,   LOW);
  digitalWrite(LD2_PIN, HIGH);
  digitalWrite(LD3_PIN, HIGH);
}

// Set LED 1 color by index
void setLED1Color(int index) {
  digitalWrite(LD1_GREEN, HIGH);
  digitalWrite(LD1_BLUE,  HIGH);
  digitalWrite(LD1_RED,   HIGH);

  switch (index % 7) {
    case 0: // RED
      digitalWrite(LD1_RED, LOW);
      break;
    case 1: // GREEN
      digitalWrite(LD1_GREEN, LOW);
      break;
    case 2: // BLUE
      digitalWrite(LD1_BLUE, LOW);
      break;
    case 3: // CYAN
      digitalWrite(LD1_GREEN, LOW);
      digitalWrite(LD1_BLUE,  LOW);
      break;
    case 4: // PURPLE
      digitalWrite(LD1_RED,  LOW);
      digitalWrite(LD1_BLUE, LOW);
      break;
    case 5: // YELLOW
      digitalWrite(LD1_RED,   LOW);
      digitalWrite(LD1_GREEN, LOW);
      break;
    case 6: // WHITE
      digitalWrite(LD1_RED,   LOW);
      digitalWrite(LD1_GREEN, LOW);
      digitalWrite(LD1_BLUE,  LOW);
      break;
  }
}

// Responsive delay that returns true if DIP switch state changes
bool delayCheck(unsigned long ms, bool sw1State, bool sw2State) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    bool currentSW1 = (digitalRead(DIP_SW1) == HIGH);
    bool currentSW2 = (digitalRead(DIP_SW2) == HIGH);
    if (currentSW1 != sw1State || currentSW2 != sw2State) {
      return true; // Switch state changed during delay
    }
    delay(20);
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(DIP_SW1, INPUT);
  pinMode(DIP_SW2, INPUT);

  pinMode(LD1_GREEN, OUTPUT);
  pinMode(LD1_BLUE,  OUTPUT);
  pinMode(LD1_RED,   OUTPUT);
  pinMode(LD2_PIN,   OUTPUT);
  pinMode(LD3_PIN,   OUTPUT);

  turnOffAll();

  Serial.println("==================================================");
  Serial.println("   ARIES v2.0 DIP Switch Controlled LED System    ");
  Serial.println("==================================================");
  Serial.println("Baud Rate: 115200");
  Serial.println("Monitoring DIP Switches & LED States...");
  Serial.println("--------------------------------------------------");
}

void loop() {
  bool sw1 = (digitalRead(DIP_SW1) == HIGH);
  bool sw2 = (digitalRead(DIP_SW2) == HIGH);

  // Mode 1: DIP SW1 ONLY is ON
  if (sw1 && !sw2) {
    Serial.println("[MODE 1] SW1=ON, SW2=OFF -> Sequential Blink (LED 1 -> LED 2 -> LED 3)");
    
    // Step A: LED 1 (Red) ON
    turnOffAll();
    digitalWrite(LD1_RED, LOW);
    Serial.println("  -> LED 1 (RED) ON");
    if (delayCheck(500, sw1, sw2)) return;

    // Step B: LED 2 ON
    turnOffAll();
    digitalWrite(LD2_PIN, HIGH);
    Serial.println("  -> LED 2 ON");
    if (delayCheck(500, sw1, sw2)) return;

    // Step C: LED 3 ON
    turnOffAll();
    digitalWrite(LD3_PIN, HIGH);
    Serial.println("  -> LED 3 ON");
    if (delayCheck(500, sw1, sw2)) return;
  }
  // Mode 2: DIP SW2 ONLY is ON
  else if (!sw1 && sw2) {
    Serial.println("[MODE 2] SW1=OFF, SW2=ON -> All LEDs Blink Together (2 sec interval)");

    turnOnAll();
    Serial.println("  -> All LEDs ON");
    if (delayCheck(2000, sw1, sw2)) return;

    turnOffAll();
    Serial.println("  -> All LEDs OFF");
    if (delayCheck(2000, sw1, sw2)) return;
  }
  // Mode 3: BOTH OFF -> LED 1 blinks with 1000 ms interval & changes color after each blink
  else if (!sw1 && !sw2) {
    Serial.print("[MODE 3] BOTH OFF -> LED 1 Color Change Blink: ");
    Serial.println(colorNames[colorIndex % 7]);

    turnOffAll();
    setLED1Color(colorIndex);
    Serial.print("  -> LED 1 ON (Color: ");
    Serial.print(colorNames[colorIndex % 7]);
    Serial.println(")");
    if (delayCheck(1000, sw1, sw2)) return;

    turnOffAll();
    Serial.println("  -> LED 1 OFF");
    if (delayCheck(1000, sw1, sw2)) return;

    colorIndex = (colorIndex + 1) % 7;
  }
  // Mode 4: BOTH ON
  else {
    Serial.println("[MODE 4] BOTH ON -> LED 1 RED Blink (500 ms)");

    turnOffAll();
    digitalWrite(LD1_RED, LOW);
    Serial.println("  -> LED 1 (RED) ON");
    if (delayCheck(500, sw1, sw2)) return;

    turnOffAll();
    Serial.println("  -> LED 1 OFF");
    if (delayCheck(500, sw1, sw2)) return;
  }
}
