#include "SoftwareSerial.h"

SoftwareSerial dySerial(10, 11); // RX, TX

const int counterPin = 8;    
const int controlPin = 2;    

int counter = 0;             
int lastCounterState = HIGH; 
bool isSessionActive = false; 

void setup() {
  pinMode(counterPin, INPUT_PULLUP);
  pinMode(controlPin, INPUT_PULLUP);
  Serial.begin(9600);
  dySerial.begin(9600);    // Komunikacja z DY-SV5W


  delay(1000); // Daj czas modułowi na zastartowanie

}

void loop() {
  int controlState = digitalRead(controlPin);

  if (controlState == LOW && !isSessionActive) {
    isSessionActive = true;
    counter = 0; // Resetujemy licznik na początku nowej sesji
  }

  if (isSessionActive) {
    int currentCounterState = digitalRead(counterPin);
    
    if (lastCounterState == HIGH && currentCounterState == LOW) {
      counter++;
      delay(50); 
    }
    lastCounterState = currentCounterState;
  }

  if (controlState == HIGH && isSessionActive) {
    isSessionActive = false;
    counter--;
    if(counter == 10) counter = 0;
    if(counter >=0 && counter<=10){
      switch(counter){

      case 0: 
      Serial.print("counter = 0");
      break;

      case 1:
      Serial.print("Counter = 1");
      playTrack(counter);
      break;

      case 2:
      Serial.print("Counter = 2");
      playTrack(counter);
      break;

      case 3:
      Serial.print("Counter = 3");
      playTrack(counter);
      break;

      case 4:
      Serial.print("Counter = 4");
      playTrack(counter);
      break;

      case 5:
      Serial.print("Counter = 5");
      playTrack(counter);
      break;

      case 6:
      Serial.print("Counter = 6");
      playTrack(counter);
      break;

      case 7:
      Serial.print("Counter = 7");
      playTrack(counter);
      break;

      case 8:
      Serial.print("Counter = 8");
      playTrack(counter);
      break;

      case 9:
      Serial.print("Counter = 9");
      playTrack(counter);
      break;

      default:
      Serial.print("Wrong number");

  }
    }
  
  

  
    counter = 0; 
    lastCounterState = HIGH; 
  }
}

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