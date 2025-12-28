/*
 * Rotary Telephone Dial Decoder for Arduino
 *
 * This sketch reads pulses from a rotary telephone dial and decodes the dialed digits.
 *
 * Rotary dials work by:
 * 1. When you rotate the dial, it opens a pulse contact
 * 2. As the dial returns, it generates pulses (1 pulse = digit 1, 10 pulses = digit 0)
 * 3. Each pulse is ~60ms long with ~40ms between pulses
 *
 * 3-WIRE PHONE SETUP:
 * - Wire 1: Pulse contact -> Pin 2
 * - Wire 2: Common/Ground -> GND
 * - Wire 3: Ready contact -> Pin 3 (or leave unconnected)
 *
 * DIAGNOSTIC MODE: Set to true to help identify wires
 *
 * Author: Arduino Dial Project
 * Date: December 2025
 */


// ===== CONFIGURATION =====
const bool DIAGNOSTIC_MODE = false;  // Set to true to see raw pin states
const bool USE_READY_PIN = false;    // Set to true only if ready contact is connected and working


// Pin definitions
const int PULSE_PIN = 2;   // Pulse contact pin (interrupt capable)
const int READY_PIN = 3;   // Ready/off-normal contact pin (optional)
const int LED_PIN = 13;    // Built-in LED for feedback


// Timing constants (in milliseconds)
const unsigned long PULSE_MIN_WIDTH = 20;     // Minimum pulse width to be valid
const unsigned long DIGIT_TIMEOUT = 100;      // Time between last pulse and digit completion
const unsigned long DIAL_TIMEOUT = 2000;      // Time before clearing the dialed number


// Variables
volatile int pulseCount = 0;              // Count of pulses received
volatile unsigned long lastPulseTime = 0; // Time of last pulse
volatile unsigned long pulseStartTime = 0; // Time when pulse started
String dialedNumber = "";                 // Complete dialed number
bool dialingInProgress = false;           // Flag to track dialing state
unsigned long lastDigitTime = 0;          // Time when last digit was completed
int lastPulseState = HIGH;                // Previous state of pulse pin
int lastReadyState = HIGH;                // Previous state of ready pin


void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("========================================");
  Serial.println("Rotary Phone Decoder Started");
  Serial.println("========================================");
 
  if (DIAGNOSTIC_MODE) {
    Serial.println("** DIAGNOSTIC MODE ENABLED **");
    Serial.println("Watch pin states to identify your wires:");
    Serial.println("- Pulse pin should change rapidly when dial returns");
    Serial.println("- Ready pin changes when you rotate the dial");
    Serial.println();
  }
 
  Serial.print("Configuration: ");
  Serial.print("PULSE_PIN="); Serial.print(PULSE_PIN);
  Serial.print(", READY_PIN="); Serial.print(READY_PIN);
  Serial.print(" ("); Serial.print(USE_READY_PIN ? "ENABLED" : "DISABLED"); Serial.println(")");
  Serial.println("Waiting for dial...");
  Serial.println();
 
  // Configure pins
  pinMode(PULSE_PIN, INPUT_PULLUP);
  pinMode(READY_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
 
  // Attach interrupt to pulse pin (triggers on falling edge)
  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), pulseInterrupt, FALLING);
 
  // Initial LED state
  digitalWrite(LED_PIN, LOW);
}


void loop() {
  // Diagnostic mode - print raw pin states
  if (DIAGNOSTIC_MODE) {
    int pulseState = digitalRead(PULSE_PIN);
    int readyState = digitalRead(READY_PIN);
   
    if (pulseState != lastPulseState || readyState != lastReadyState) {
      Serial.print("PULSE_PIN="); Serial.print(pulseState ? "HIGH" : "LOW ");
      Serial.print("  READY_PIN="); Serial.println(readyState ? "HIGH" : "LOW ");
      lastPulseState = pulseState;
      lastReadyState = readyState;
    }
    delay(10);
    return;
  }
 
  // Normal operation
  // Check if ready pin indicates dialing (only if enabled)
  if (USE_READY_PIN) {
    bool dialIsRotating = (digitalRead(READY_PIN) == LOW);
   
    if (dialIsRotating && !dialingInProgress) {
      dialingInProgress = true;
      pulseCount = 0;
      digitalWrite(LED_PIN, HIGH);  // Turn on LED when dialing
      Serial.println("[Dial rotating...]");
    }
  }
 
  // Check if we've received all pulses for current digit
  if (pulseCount > 0 && (millis() - lastPulseTime) > DIGIT_TIMEOUT) {
    processDigit();
  }
 
  // Check if complete number should be processed
  if (dialedNumber.length() > 0 && (millis() - lastDigitTime) > DIAL_TIMEOUT) {
    processDialedNumber();
  }
}


// Interrupt service routine - called on each pulse
void pulseInterrupt() {
  unsigned long now = millis();
  unsigned long timeSinceLastPulse = now - lastPulseTime;
 
  // Debounce: ignore pulses that come too quickly (likely noise)
  if (timeSinceLastPulse > PULSE_MIN_WIDTH) {
    pulseCount++;
    lastPulseTime = now;
   
    // Visual feedback on each pulse
    digitalWrite(LED_PIN, HIGH);
  }
}


// Process a completed digit
void processDigit() {
  int digit = pulseCount;
 
  // Validate pulse count (should be 1-10)
  if (digit < 1 || digit > 10) {
    Serial.print("Invalid pulse count: ");
    Serial.print(digit);
    Serial.println(" (expected 1-10, ignoring)");
    pulseCount = 0;
    dialingInProgress = false;
    digitalWrite(LED_PIN, LOW);
    return;
  }
 
  // Convert 10 pulses to 0
  if (digit == 10) {
    digit = 0;
  }
 
  // Add digit to dialed number
  dialedNumber += String(digit);
  lastDigitTime = millis();
 
  // Print feedback
  Serial.print(">>> Digit dialed: ");
  Serial.print(digit);
  Serial.print(" (");
  Serial.print(pulseCount);
  Serial.println(" pulses)");
  Serial.print(">>> Number so far: ");
  Serial.println(dialedNumber);
  Serial.println();
 
  // Flash LED to indicate digit registered
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(50);
    digitalWrite(LED_PIN, HIGH);
    delay(50);
  }
 
  // Reset for next digit
  pulseCount = 0;
  dialingInProgress = false;
  digitalWrite(LED_PIN, LOW);
}


// Process the complete dialed number
void processDialedNumber() {
  Serial.println("==================");
  Serial.print("Complete number: ");
  Serial.println(dialedNumber);
  Serial.println("==================");
 
  // Here you can add your custom logic:
  // - Make phone calls via GSM module
  // - Control smart home devices
  // - Play MP3 files based on number
  // - Send data to computer/server
  // - Trigger specific actions for specific numbers
 
  // Example: Check for specific numbers
  if (dialedNumber == "911") {
    Serial.println("Emergency number dialed!");
    emergencyAction();
  } else if (dialedNumber == "411") {
    Serial.println("Information number dialed!");
    informationAction();
  } else {
    Serial.println("Regular number dialed.");
    regularAction();
  }
 
  // Clear the dialed number
  dialedNumber = "";
}


// Custom action functions - customize these for your project
void emergencyAction() {
  // Add your emergency action code here
  // Example: activate an alarm, send notification, etc.
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
}


void informationAction() {
  // Add your information action code here
  // Example: play a voice message, display info, etc.
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
}


void regularAction() {
  // Add your regular action code here
  // Example: process normal phone calls, log number, etc.
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}





