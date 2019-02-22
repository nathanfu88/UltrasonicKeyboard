#include "pitches.h"

const byte ECHO_PIN = 2;     // HC-SR04 Echo pin (D2)
const byte TRIG_PIN = 3;     // HC-SR04 Trigger pin (D3)
const byte BUZZER_PIN = 8;   // Piezo speaker pin (D8)
const byte MUTE_PIN = 6;     // Push button mute pin (D6)

// Distance (in cm) the keyboard should start from the sensor
// HC-SR04's suggested minimum distance is 2 cm
const float DIST_OFFSET = 2.0;

// Estimated value to easily get note index from distance
const float AVG_NOTE_DIST = 2.75;
// Number of notes to limit playing sounds past a certain distance
const byte NUM_NOTES = 24;
const int NOTES[] = {
  0, NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4,
  NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
  NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5,
  NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
};

// Delay (in ms) before reading the sensor again
const int POLL_DELAY = 800;
// Required to use a push button as a toggle switch
byte prevButtonState = LOW;

bool muted = false;

void setup() {
  // Define HC-SR04 I/O
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);

  // Define mute pushbutton input
  pinMode(MUTE_PIN, INPUT);

  // Initialize on-board LED for mute signaling
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Check for input of mute pushbutton
  byte newButtonState = digitalRead(MUTE_PIN);
  if (newButtonState == HIGH && prevButtonState == LOW) {
    muted = !muted;
  }

  prevButtonState = newButtonState;

  // Only send ultrasonic pulses if we are not muted
  if (!muted) {
    digitalWrite(LED_BUILTIN, LOW);
    
    /**
     * Send a ultrasonic pulse
     * 
     * HC-SR04 uses a HIGH pulse for 10+ usec
     * Emit a LOW signal of 5 usec first to handle noise for a "clean" pulse
     */
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    // "Close" the pulse with a LOW
    digitalWrite(TRIG_PIN, LOW);
  
   /**
    * Read signal from sensor
    * 
    * Reading from the ECHO_PIN will be the time (in usec) of reflection
    * Keep in mind that the duration is "time in flight", so the distance
    * of the object from the sensor is actually `time in flight / 2`
    */
    pinMode(ECHO_PIN, INPUT);
    long duration = pulseIn(ECHO_PIN, HIGH) / 2;
   
    /**
     * Convert the time into distance
     * 
     * Speed of sound at room temp (~68F) is ~343 m/s
     * 343*10^-6 m/us => 343*10^-4 cm/us
     * To get distance (in cm) from time (in us): `duration` * 343 * 10^-4
     */
    float cm = (float)duration * 0.0343;
  
    /**
     * Play note
     * 
     * Frequencies stored in NOTES[]. Each key from the printout maps to
     * a note. Each note has a span of distance that varies. To make it
     * easier to determine the note, I use a rough estimation by dividing
     * the distance by the distance between notes on the printout.
     * 
     * The quotient is the number note to play from C4 (since C4 is the
     * first note in the printout).
     * 
     * If the distance is outside the printout's keyboard range, do not
     * play any sound
     */
    byte noteIndex = round((cm - DIST_OFFSET) / AVG_NOTE_DIST) - 1;
    if (noteIndex > NUM_NOTES) {
      noteIndex = 0;
    }
    
    if (noteIndex == 0) {
      noTone(BUZZER_PIN);
    }
    else {
      tone(BUZZER_PIN, NOTES[noteIndex]);
    }
  
    // Add some delay to prevent flooding
    delay(POLL_DELAY);
  }
  else {
    // We are muted, so do not send or attempt to read ultrasonic pulse
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
