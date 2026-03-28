const int tapPin = 2;              // HW-500 signal pin -> D2
const int expectedKnocks = 3;      // total knocks in the pattern
const int debounceMs = 200;        // ignore repeated triggers from one tap
const int patternTimeoutMs = 2500; // reset if the sequence takes too long

unsigned long knockTimes[expectedKnocks];
int knockCount = 0;
unsigned long lastKnockMs = 0;

// Example target pattern:
// knock -> short gap -> knock -> long gap -> knock
const int targetGap1 = 400;
const int targetGap2 = 900;
const int tolerance = 180;

bool matchesPattern(unsigned long gap1, unsigned long gap2) {
  bool firstOk = abs((long)gap1 - targetGap1) <= tolerance;
  bool secondOk = abs((long)gap2 - targetGap2) <= tolerance;
  return firstOk && secondOk;
}

void resetPattern() {
  knockCount = 0;
}

void setup() {
  pinMode(tapPin, INPUT);
  Serial.begin(9600);
  Serial.println("HW-500 knock lock ready");
}

void loop() {
  unsigned long now = millis();

  // reset if too much time passes during a sequence
  if (knockCount > 0 && (now - lastKnockMs > patternTimeoutMs)) {
    Serial.println("Pattern timeout. Resetting.");
    resetPattern();
  }

  int tapState = digitalRead(tapPin);

  // Many HW-500 modules trigger LOW when tapped.
  // If this does not work, change LOW to HIGH.
  if (tapState == LOW) {
    if (now - lastKnockMs > debounceMs) {
      Serial.print("Knock detected: ");
      Serial.println(knockCount + 1);

      if (knockCount < expectedKnocks) {
        knockTimes[knockCount] = now;
        knockCount++;
        lastKnockMs = now;
      }

      if (knockCount == expectedKnocks) {
        unsigned long gap1 = knockTimes[1] - knockTimes[0];
        unsigned long gap2 = knockTimes[2] - knockTimes[1];

        Serial.print("Gap1: ");
        Serial.print(gap1);
        Serial.print(" ms, Gap2: ");
        Serial.print(gap2);
        Serial.println(" ms");

        if (matchesPattern(gap1, gap2)) {
          Serial.println("ACCESS GRANTED");
        } else {
          Serial.println("ACCESS DENIED");
        }

        resetPattern();
      }
    }
  }
}