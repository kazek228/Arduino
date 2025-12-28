const int counterPin = 8;    // Przycisk do zliczania
const int controlPin = 2;    // Przycisk sterujący (START/STOP)

int counter = 0;             
int lastCounterState = HIGH; 
bool isSessionActive = false; // Flaga określająca, czy trwa zliczanie

void setup() {
  pinMode(counterPin, INPUT_PULLUP);
  pinMode(controlPin, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println("Trzymaj przycisk na pinie 9, aby zaczac zliczac...");
}

void loop() {
  // Odczytujemy stan przycisku sterującego (pin 9)
  int controlState = digitalRead(controlPin);

  // --- LOGIKA STARTU SESJI ---
  if (controlState == LOW && !isSessionActive) {
    isSessionActive = true;
    counter = 0; // Resetujemy licznik na początku nowej sesji
  }

  // --- LOGIKA ZLICZANIA (tylko gdy sesja aktywna) ---
  if (isSessionActive) {
    int currentCounterState = digitalRead(counterPin);
    
    // Wykrywanie zbocza opadającego (z HIGH na LOW)
    if (lastCounterState == HIGH && currentCounterState == LOW) {
      counter++;
      delay(50); // Debouncing
    }
    lastCounterState = currentCounterState;
  }

  // --- LOGIKA KONCA SESJI I WYSWIETLANIA WYNIKU ---
  if (controlState == HIGH && isSessionActive) {
    isSessionActive = false;
    counter--;
    if(counter == 10) counter = 0;
    if(counter >=0 && counter<=10){
        Serial.print(counter);
    }
    
    counter = 0; // Zerujemy licznik po wyświetleniu wyniku
    lastCounterState = HIGH; // Resetujemy stan pomocniczy
  }
}