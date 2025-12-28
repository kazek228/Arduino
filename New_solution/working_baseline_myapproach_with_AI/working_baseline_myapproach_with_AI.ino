#include "SoftwareSerial.h"

SoftwareSerial dySerial(10, 11); // RX, TX

const int counterPin = 8;    
const int controlPin = 2;    

int counter = 0;             
int lastCounterState = HIGH; 
bool isSessionActive = false; 

// Zmienne do budowania długiego ciągu cyfr
String digitSequence = "";        // Ciąg tekstowy przechowujący cyfry
unsigned long lastSessionEnd = 0; 
bool waitingForNextDigit = false; 
const unsigned long timeout = 2000; 

void setup() {
  pinMode(counterPin, INPUT_PULLUP);
  pinMode(controlPin, INPUT_PULLUP);
  Serial.begin(9600);
  dySerial.begin(9600);  
  delay(1000); 
  Serial.println("System gotowy. Wpisuj kolejne cyfry...");
}

void loop() {
  int controlState = digitalRead(controlPin);

  // 1. START SESJI WPISYWANIA KOLEJNEJ CYFRY
  if (controlState == LOW && !isSessionActive) {
    isSessionActive = true;
    waitingForNextDigit = false; 
    counter = 0; 
  }

  // 2. ZLICZANIE IMPULSÓW
  if (isSessionActive) {
    int currentCounterState = digitalRead(counterPin);
    if (lastCounterState == HIGH && currentCounterState == LOW) {
      counter++;
      delay(50); 
    }
    lastCounterState = currentCounterState;
  }

  // 3. ZAKOŃCZENIE WPISYWANIA CYFRY - DODANIE DO CIĄGU
  if (controlState == HIGH && isSessionActive) {
    isSessionActive = false;
    
    // Twoja korekta (zlicza HIGH na LOW)
    counter--;
    if(counter == 10) counter = 0;

    // Przyjmuj tylko cyfry 0-9
    if (counter >= 0 && counter <= 9) {
      digitSequence += String(counter); // Dodaj cyfrę na koniec ciągu (tekstowo)
      
      Serial.print("Dodano cyfre: ");
      Serial.print(counter);
      Serial.print(" | Aktualny ciag: ");
      Serial.println(digitSequence);
      
      lastSessionEnd = millis(); 
      waitingForNextDigit = true;
    }
    
    lastCounterState = HIGH; 
  }

  // 4. TIMEOUT - KONIEC BUDOWANIA CIĄGU I ODTWARZANIE
  if (waitingForNextDigit && (millis() - lastSessionEnd >= timeout)) {
    waitingForNextDigit = false;

    if (digitSequence.length() > 0) {
      // Zamiana ciągu tekstowego na liczbę typu long
      // Uwaga: Funkcja toInt() obsługuje liczby do ok. 2 miliardów (long)
      long trackToPlay = digitSequence.toInt(); 

      Serial.print("--- KONIEC CZASU. Finalna liczba: ");
      Serial.println(trackToPlay);

      if (trackToPlay > 0) {
        switch(trackToPlay){
          case 2823451: 
            playTrack(12);
            Serial.print("Bajka");
            break;
        }
        //playTrack(trackToPlay);
      }
    }

    digitSequence = ""; // Reset ciągu dla nowej liczby
  }
}

void playTrack(long trackNumber) {
  // Rozbicie liczby long na bajty dla komendy UART
  byte highB = (trackNumber >> 8) & 0xFF;
  byte lowB = trackNumber & 0xFF;
  
  byte command[6];
  command[0] = 0xAA; 
  command[1] = 0x07; 
  command[2] = 0x02; 
  command[3] = highB;
  command[4] = lowB;
  
  byte checksum = 0;
  for (int i = 0; i < 5; i++) {
    checksum += command[i];
  }
  command[5] = checksum;

  dySerial.write(command, 6);
}