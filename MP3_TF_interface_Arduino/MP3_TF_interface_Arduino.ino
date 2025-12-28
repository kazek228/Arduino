#include <SoftwareSerial.h>

// Konfiguracja pinów
const int counterPin = 8;    // Przycisk zliczający
const int controlPin = 2;    // Przycisk sesji (START/STOP)
SoftwareSerial dySerial(10, 11); // RX, TX
// Zmienne stanów
int counter = 0;             
int lastCounterState = HIGH; 
bool isSessionActive = false; 

void setup() {
  pinMode(counterPin, INPUT_PULLUP);
  pinMode(controlPin, INPUT_PULLUP);
  
  Serial.begin(9600);      // Monitor szeregowy
  dySerial.begin(9600);    // Komunikacja z DY-SV5W
  
  Serial.println("System gotowy. Tryb UART dla DY-SV5W.");
}

void loop() {
  int controlState = digitalRead(controlPin);

  // 1. ROZPOCZĘCIE SESJI (Wciśnięcie Pin 2)
  if (controlState == LOW && !isSessionActive) {
    isSessionActive = true;
    counter = 0; 
    Serial.println(">>> Sesja START. Licz klikniecia...");
  }

  // 2. ZLICZANIE (Tylko gdy trzymasz Pin 2)
  if (isSessionActive) {
    int currentCounterState = digitalRead(counterPin);
    
    if (lastCounterState == HIGH && currentCounterState == LOW) {
      counter++;
      Serial.print("Klikniecie: ");
      Serial.println(counter);
      delay(50); // Debouncing
    }
    lastCounterState = currentCounterState;
  }

  // 3. KONIEC SESJI I ODTWARZANIE (Puszczenie Pin 2)
  if (controlState == HIGH && isSessionActive) {
    isSessionActive = false;
    
    Serial.print("Koniec sesji. Wynik: ");
    Serial.println(counter);

    if (counter > 0) {
      playTrack(counter); // Odtwórz plik o numerze odpowiadającym licznikowi
    } else {
      Serial.println("Brak klikniec - nic nie odtwarzam.");
    }

    // Przygotowanie do nastepnej sesji
    lastCounterState = HIGH; 
  }
}

/**
 * Funkcja wysyłająca komendę odtwarzania do DY-SV5W
 * Format: AA 07 02 HighByte LowByte Checksum
 */
void playTrack(int trackNumber) {
  byte highByte = highByte(trackNumber);
  byte lowByte = lowByte(trackNumber);
  
  // Budowanie ramki danych
  byte command[6];
  command[0] = 0xAA; // Nagłówek
  command[1] = 0x07; // Instrukcja: Odtwarzaj konkretny numer
  command[2] = 0x02; // Długość danych (2 bajty numeru utworu)
  command[3] = highByte;
  command[4] = lowByte;
  
  // Obliczanie sumy kontrolnej (ostatni bajt)
  byte checksum = 0;
  for (int i = 0; i < 5; i++) {
    checksum += command[i];
  }
  command[5] = checksum;

  // Wysyłanie do modułu
  dySerial.write(command, 6);
  
  Serial.print("Wyslano komende PLAY dla utworu nr: ");
  Serial.println(trackNumber);
}