#include <Arduino.h>

// The dialling pulse
const unsigned char pinPulse = 2;

// The dialling-in-progress pin
const unsigned char pinDialling = 8;

// Updated by the ISR (Interrupt Service Routine)
volatile int pulseCnt = 0;

// ------------------------------------------------------------------
// INTERRUPT     INTERRUPT     INTERRUPT     INTERRUPT     INTERRUPT
// ------------------------------------------------------------------
void isr() {

	static unsigned long lastMillis = millis();
	if (millis() > lastMillis + 50) {
		// Are we dialling?
		if (digitalRead(pinDialling) == LOW) {
			pulseCnt++;
		}
	}
	lastMillis = millis();
}

// ------------------------------------------------------------------
// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
// ------------------------------------------------------------------
void setup() {
	// Just whilst we debug, view output on serial monitor
	Serial.begin(9600);

	// Rotary pulses are INPUTs
	pinMode(pinPulse, INPUT_PULLUP);
	pinMode(pinDialling, INPUT_PULLUP);

	// Attach the routine to service the interrupts
	attachInterrupt(digitalPinToInterrupt(pinPulse), isr, RISING);

	// Ready to go!
	Serial.println("Ready.");
}

// ------------------------------------------------------------------
// MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP
// ------------------------------------------------------------------
void loop() {

	// Keep track of when we were last here
	static unsigned long lastMillis = millis();

	// The final number string after all dialling completed
	static String finalNumber = "";

	// If dialling has finished (maybe just for this digit)
	if (digitalRead(pinDialling) == HIGH) {

		// Has dialling completed totally?
		if (millis() > lastMillis + 2000) {

			// What was the number we dialled?
			if (finalNumber != "") {
				switch (finalNumber.toInt()){
					case 997:
					Serial.print("Dzwonię na policję.... ");
					break;
					case 28082011:
					Serial.print("Sto lat Zuziu!!.... ");
					break;
					default:
					Serial.print("Nieznany numer, próbuj jeszcze raz.");
					break;
          
				}
				Serial.print("You entered ");
				Serial.println(finalNumber);
				finalNumber = "";
			}
			lastMillis = millis();
		}
		// Looks like we might still be dialling numbers
		else {
			if (pulseCnt > 0) {
				finalNumber += pulseCnt < 10 ? String(pulseCnt) : "0";
				pulseCnt = 0;
				lastMillis = millis();
			}
		}
	}
	else {
		// We're in the middle of a dialling operation
		lastMillis = millis();
	}

	delay(100);
}