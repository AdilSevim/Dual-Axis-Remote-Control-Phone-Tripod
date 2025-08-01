/**
 * Two-axis IR Stepper Controller (Single-file, Learn + Run)
 * - Adafruit Motor Shield V2 + 2x stepper + IR receiver
 * - Arrow keys => smooth, exact 90° turns (microstepping + trapezoid profile)
 * - Built-in IR "Learn Mode" with EEPROM persistence
 * 
 * Author: (c) 2024 Adil Sevim 
 */

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <IRremote.hpp>
#include <EEPROM.h>

// ================== USER CONFIG ==================
static const int   IR_PIN           = 2;   // IR receiver signal pin
static const int   LEARN_PIN        = 4;   // Pull LOW at boot to enter Learn Mode
static const int   STEPS_PER_REV    = 200; // 1.8° stepper -> 200 full steps
static const int   MICROSTEPS       = 16;  // MotorShield V2 supports up to 16 microsteps

// Motion profile (tune to taste)
static const unsigned int START_DELAY_US = 1500; // slowest microstep delay (µs)
static const unsigned int MIN_DELAY_US   = 400;  // fastest microstep delay (µs)
static const int          ACCEL_SEGMENT  = 200;  // microsteps for accel & decel

// ================== INTERNALS ==================
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60);
Adafruit_StepperMotor *motorX; // LEFT/RIGHT  -> PORT 1
Adafruit_StepperMotor *motorY; // UP/DOWN     -> PORT 2

// Quarter turn in microsteps
static const int QUARTER_TURN_MS = (int)((STEPS_PER_REV * (90.0 / 360.0)) * MICROSTEPS);

// EEPROM layout
struct IRMap {
  uint32_t magic;     // signature
  uint32_t up;
  uint32_t down;
  uint32_t left;
  uint32_t right;
};
static const uint32_t IR_MAGIC = 0xA51DBEEF; // any distinctive value
static const int      EE_ADDR  = 0;          // address to store the struct

IRMap irCodes = {0};

// ============ HELPERS ============

// Safe integer linear interpolation for delays (avoids map() divide-by-zero)
static inline unsigned int rampDelay(unsigned int index, unsigned int span) {
  if (span <= 1) return MIN_DELAY_US;
  long delta = (long)MIN_DELAY_US - (long)START_DELAY_US;
  return (unsigned int)(START_DELAY_US + (delta * (long)index) / (long)(span - 1));
}

// Microstep move with trapezoidal profile (blocking)
void smoothMove(Adafruit_StepperMotor *m, int microsteps, uint8_t dir) {
  if (microsteps <= 0) return;

  int accel = ACCEL_SEGMENT;
  if (microsteps < 2 * ACCEL_SEGMENT) {
    accel = microsteps / 2;
    if (accel < 1) accel = 1;
  }

  for (int i = 0; i < microsteps; ++i) {
    unsigned int delayUs;
    if (i < accel) {
      delayUs = rampDelay(i, accel);                 // accelerating
    } else if (i >= microsteps - accel) {
      int j = (microsteps - 1) - i;
      delayUs = rampDelay(j, accel);                 // decelerating
    } else {
      delayUs = MIN_DELAY_US;                        // cruise
    }
    m->onestep(dir, MICROSTEP);
    delayMicroseconds(delayUs);
  }
}

// Convenience wrappers for 90° moves
void turnLeft90()  { smoothMove(motorX, QUARTER_TURN_MS, BACKWARD); }
void turnRight90() { smoothMove(motorX, QUARTER_TURN_MS, FORWARD);  }
void turnUp90()    { smoothMove(motorY, QUARTER_TURN_MS, FORWARD);  }
void turnDown90()  { smoothMove(motorY, QUARTER_TURN_MS, BACKWARD); }

// EEPROM I/O
bool loadIRCodes(IRMap &dst) {
  EEPROM.get(EE_ADDR, dst);
  return (dst.magic == IR_MAGIC);
}
void saveIRCodes(const IRMap &src) {
  EEPROM.put(EE_ADDR, src);
}

// Wait for a non-repeat IR frame and return raw 32-bit code
uint32_t waitIR(const char *prompt) {
  Serial.print(F(">> Press ")); Serial.print(prompt); Serial.println(F(" key..."));
  while (true) {
    if (IrReceiver.decode()) {
      auto &d = IrReceiver.decodedIRData;
      if (!(d.flags & IRDATA_FLAGS_IS_REPEAT)) {
        uint32_t raw = d.decodedRawData;
        Serial.print(F("   Captured RAW=0x")); Serial.println(raw, HEX);
        IrReceiver.resume();
        delay(150); // small debounce
        return raw;
      }
      IrReceiver.resume();
    }
  }
}

// Learn Mode: capture UP/DOWN/LEFT/RIGHT and store to EEPROM
void learnMode() {
  Serial.println(F("\n=== IR Learn Mode ==="));
  Serial.println(F("We'll record: UP, DOWN, LEFT, RIGHT."));
  Serial.println(F("Aim your remote at the receiver and follow the prompts.\n"));

  irCodes.up    = waitIR("UP");
  irCodes.down  = waitIR("DOWN");
  irCodes.left  = waitIR("LEFT");
  irCodes.right = waitIR("RIGHT");
  irCodes.magic = IR_MAGIC;

  saveIRCodes(irCodes);
  Serial.println(F("\nSaved to EEPROM. Restarting in RUN mode...\n"));
  delay(800);
}

// Handle one decoded press in RUN mode
void handleIRPress(uint32_t raw) {
  if      (raw == irCodes.left)  turnLeft90();
  else if (raw == irCodes.right) turnRight90();
  else if (raw == irCodes.up)    turnUp90();
  else if (raw == irCodes.down)  turnDown90();
  else {
    // Unknown code; uncomment to debug
    // Serial.print(F("Unknown IR: 0x")); Serial.println(raw, HEX);
  }
}

// ================== SETUP/LOOP ==================
void setup() {
  Serial.begin(115200);
  pinMode(LEARN_PIN, INPUT_PULLUP);

  // Motors
  if (!AFMS.begin()) {
    Serial.println(F("ERROR: Motor Shield not found. Check wiring/I2C address."));
    while (true) {}
  }
  motorX = AFMS.getStepper(STEPS_PER_REV, 1); // PORT 1
  motorY = AFMS.getStepper(STEPS_PER_REV, 2); // PORT 2
  motorX->setSpeed(60);
  motorY->setSpeed(60);

  // IR
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  // Load or learn codes
  bool haveCodes = loadIRCodes(irCodes);
  if (!haveCodes || digitalRead(LEARN_PIN) == LOW) {
    learnMode();
  }

  Serial.println(F("=== RUN Mode ==="));
  Serial.println(F("Arrow keys => exact 90° smooth turns."));
  Serial.println(F("Hold LEARN_PIN (D4) LOW at boot or send 'L' to relearn.\n"));
}

void loop() {
  // Serial command to re-enter Learn Mode
  if (Serial.available()) {
    int c = Serial.read();
    if (c == 'L' || c == 'l') {
      learnMode();
      Serial.println(F("Back to RUN Mode."));
    }
  }

  // IR handling
  if (IrReceiver.decode()) {
    auto &data = IrReceiver.decodedIRData;
    if (!(data.flags & IRDATA_FLAGS_IS_REPEAT)) {
      handleIRPress(data.decodedRawData);
    }
    IrReceiver.resume();
  }
}
