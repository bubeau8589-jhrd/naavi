#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

/* =========================
   SERVO SETUP
   ========================= */
Servo panServo;
Servo tiltServo;

int pan_servo = 4;
int tilt_servo = 5;

/* =========================
   ULTRASONIC SETUP
   ========================= */
// Ultrasonic 1: Dry & Wet
#define TRIG_1 6
#define ECHO_1 7

// Ultrasonic 2: Medical & E-Waste
#define TRIG_2 8
#define ECHO_2 9

/* =========================
   NEOPIXEL SETUP
   ========================= */
#define LED_PIN 3
#define LED_COUNT 78

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

/* =========================
   LCD I2C SETUP
   ========================= */
// LCD with I2C address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* =========================
   FUNCTIONS
   ========================= */

/* ---- LCD Display Functions ---- */
void showDefaultMessage() {
  lcd.clear();
  lcd.setCursor(2, 0); // Center "AI Vision" on row 0 (16-8=8, 8/2=4, but "AI
                       // Vision"=9 chars, so (16-9)/2=3.5≈2)
  lcd.print("AI Vision");
  lcd.setCursor(1, 1); // Center "Smart Dustbin" on row 1 (16-13=3, 3/2=1.5≈1)
  lcd.print("Smart Dustbin");
}

void showWasteCategory(String category) {
  lcd.clear();
  lcd.setCursor(0, 0);

  if (category == "dry") {
    lcd.setCursor(4, 0); // Center "DRY WASTE"
    lcd.print("DRY WASTE");
  } else if (category == "wet") {
    lcd.setCursor(4, 0); // Center "WET WASTE"
    lcd.print("WET WASTE");
  } else if (category == "medical") {
    lcd.setCursor(2, 0); // Center "MEDICAL WASTE"
    lcd.print("MEDICAL WASTE");
  } else if (category == "e-waste") {
    lcd.setCursor(3, 0); // Center "E-WASTE"
    lcd.print("E-WASTE");
  }
}

void pull_back() {
  tiltServo.write(90);
  delay(1000);
  panServo.write(80);
  delay(1000);

  // Restore default LCD message after returning to home position
  showDefaultMessage();
}

void setAllColor(int r, int g, int b) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

/* ---- Stable Ultrasonic Reading ---- */
long getDistanceCM(int trigPin, int echoPin) {
  long total = 0;

  for (int i = 0; i < 3; i++) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);
    if (duration == 0)
      return 0;

    total += (duration * 0.034 / 2);
    delay(50);
  }
  return total / 3;
}

/* =========================
   SETUP
   ========================= */
void setup() {
  Serial.begin(115200);

  // Removed IR Sensor Pin Setup

  pinMode(TRIG_1, OUTPUT);
  pinMode(ECHO_1, INPUT);
  pinMode(TRIG_2, OUTPUT);
  pinMode(ECHO_2, INPUT);

  panServo.attach(pan_servo);
  tiltServo.attach(tilt_servo);

  strip.begin();
  strip.setBrightness(80);
  strip.show();

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  showDefaultMessage();

  pull_back();
  Serial.println("System Ready");
}

/* =========================
   LOOP
   ========================= */
void loop() {
  // Directly listen for serial commands without IR trigger
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.length() > 0) {

      /* ===== DRY ===== */
      if (command == "dry") {
        showWasteCategory("dry");
        setAllColor(0, 0, 255);
        panServo.write(45);
        delay(1200);
        delay(300);

        long dist = getDistanceCM(TRIG_1, ECHO_1);

        if (dist > 10) {
          tiltServo.write(30);
          delay(1000);
        } else {
          Serial.println("dry_full");
        }

        pull_back();
        setAllColor(0, 0, 0);
      }

      /* ===== WET ===== */
      else if (command == "wet") {
        showWasteCategory("wet");
        setAllColor(0, 255, 0);
        panServo.write(140);
        delay(1200);
        delay(300);

        long dist = getDistanceCM(TRIG_1, ECHO_1);

        if (dist > 10) {
          tiltServo.write(30);
          delay(1000);
        } else {
          Serial.println("wet_full");
        }

        pull_back();
        setAllColor(0, 0, 0);
      }

      /* ===== MEDICAL ===== */
      else if (command == "medical" || command == "med") {
        showWasteCategory("medical");
        setAllColor(255, 0, 0);
        panServo.write(45);
        delay(1200);
        delay(300);

        long dist = getDistanceCM(TRIG_2, ECHO_2);

        if (dist > 10) {
          tiltServo.write(140);
          delay(1000);
        } else {
          Serial.println("medical_full");
        }

        pull_back();
        setAllColor(0, 0, 0);
      }

      /* ===== E-WASTE ===== */
      else if (command == "e-waste") {
        showWasteCategory("e-waste");
        setAllColor(255, 255, 0);
        panServo.write(140);
        delay(1200);
        delay(300);

        long dist = getDistanceCM(TRIG_2, ECHO_2);

        if (dist > 10) {
          tiltServo.write(140);
          delay(1000);
        } else {
          Serial.println("ewaste_full");
        }

        pull_back();
        setAllColor(0, 0, 0);
      }
    }
  }
  // Short delay to prevent CPU hogging
  delay(50);
}
