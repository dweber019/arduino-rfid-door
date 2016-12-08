int LEDblau   = 11;
int LEDrot    = 12;
int LEDgruen  = 13;

int alarm = 8;

#include <Stepper.h>
int SPMU = 32;
int doorCloseDelay = 2500;
Stepper myStepper(SPMU, 39, 38, 37, 36);

#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 53
#define RST_PIN 49
MFRC522 mfrc522(SS_PIN, RST_PIN);

#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// RFID cards
long badgeWhite  = 1383100;
long badgeBlue   = 513100;
long badgeCar    = 2025120;
long badgeSbb    = 2216770;
long badgeFit    = 809320;
long allowedBadges[] = {badgeWhite, badgeCar, badgeSbb};
int  allowedBadgesCount = 3;

int retry = 2;

int doorknob = 9;

void setup() {

  // Setup RGB LED
  pinMode(LEDblau, OUTPUT);
  pinMode(LEDgruen, OUTPUT);
  pinMode(LEDrot, OUTPUT);

  // Setup serial output
  Serial.begin(9600);

  // Setup stepper
  myStepper.setSpeed(1000);

  // Setup RFID
  SPI.begin();
  mfrc522.PCD_Init();

  // Setup LCD
  lcd.begin(16, 2);

  // Setup alarm
  pinMode(alarm, OUTPUT);

  // Setup inner door nop
  pinMode(doorknob, INPUT);
}

void loop() {
  // Ready
  instructionReady();

  // handle doorknob
  if (readDoorknob()) {
    openCloseDoor();
  }

  // handle rfid access
  if (isRfidPresent()) {
    // rfid present
    if (badgeAllowed(getRfidNumber())) {
      // rfid allowed
      instructionSuccess();
      openCloseDoor();
    } else {
      // rfid not allowed
      instructionFail();
    }
  }
}

boolean readDoorknob() {
  Serial.println("DOOR: doorknob");
  return digitalRead(doorknob);
}

void instructionReady() {
  Serial.println("LCD: ready");
  ledOrange();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready. Please");
  lcd.setCursor(0, 1);
  lcd.print("show your badge");
}

void instructionFail() {
  Serial.println("LCD: failed");
  ledRed();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Not authorized");
  lcd.setCursor(0, 1);

  if (retry == 0) {
    // Trigger alarm
    lcd.print("Alarm!!!!");
    makeAlarm();
    resetAlarmCount();
  } else {
    // Show retries
    lcd.print("retries left: ");
    lcd.setCursor(14, 1);
    lcd.print(retry);
    retry = retry - 1;
    delay(2100);
  }
}

void makeAlarm() {
  Serial.println("ALARM: alarm");
  digitalWrite(alarm, HIGH);
  delay(300);
  digitalWrite(alarm, LOW);
  delay(300);
  digitalWrite(alarm, HIGH);
  delay(300);
  digitalWrite(alarm, LOW);
  delay(300);
  digitalWrite(alarm, HIGH);
  delay(300);
  digitalWrite(alarm, LOW);
  delay(300);
  digitalWrite(alarm, HIGH);
  delay(300);
  digitalWrite(alarm, LOW);
}

void resetAlarmCount() {
  retry = 2;
}

void instructionSuccess() {
  Serial.println("LCD: success");
  ledGreen();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome badge");
  lcd.setCursor(0, 1);
  lcd.print(getRfidNumber());
  resetAlarmCount();
  delay(1000);
}

boolean badgeAllowed(long badgeId) {
  for (int count = 0; count < allowedBadgesCount; count++) {
    if (badgeId == allowedBadges[count]) {
      Serial.print("PERMISSION: badge allowed ");
      Serial.println(badgeId);
      return true;
    }
  }
  Serial.print("PERMISSION: badge not allowed ");
  Serial.println(badgeId);
  return false;
}

boolean isRfidPresent() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  Serial.println("RFID: badge detected");
  return true;
}

long getRfidNumber() {
  long code=0;
  
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    code=((code+mfrc522.uid.uidByte[i])*10);
  }
  Serial.print("RFID: badge number ");
  Serial.println(code);
  return code;
}

void openCloseDoor() {
  openDoor();
  delay(doorCloseDelay);
  closeDoor();
}

void openDoor() {
  Serial.println("DOOR: open");
  instructionOpenDoor();
  myStepper.step(2048);
}

void instructionOpenDoor() {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("opening door...");
}

void closeDoor() {
  Serial.println("DOOR: close");
  instructionCloseDoor();
  myStepper.step(-2048);
}

void instructionCloseDoor() {
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("closing door...");
}

void ledGreen() {
  Serial.println("LED: Green");
  analogWrite(LEDrot, 0);
  analogWrite(LEDgruen, 255);
  analogWrite(LEDblau, 0);
}

void ledRed() {
  Serial.println("LED: Red");
  analogWrite(LEDrot, 255);
  analogWrite(LEDgruen, 0);
  analogWrite(LEDblau, 0);
}

void ledOrange() {
  Serial.println("LED: Orange");
  analogWrite(LEDrot, 255);
  analogWrite(LEDgruen, 5);
  analogWrite(LEDblau, 0);
}
