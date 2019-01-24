int pinButton = 8;
int LED = 2;
int stateLED = LOW;
int stateButton;
int previous = LOW;
long t_ime = 0;
long debounce = 200;

void setup() {
  pinMode(pinButton, INPUT);
  pinMode(LED, OUTPUT);
}

void loop() {
  stateButton = digitalRead(pinButton);  
  if(stateButton == HIGH && previous == LOW && millis() - t_ime > debounce) {
    if(stateLED == HIGH){
      stateLED = LOW; 
    } else {
       stateLED = HIGH; 
    }
    t_ime = millis();
  }
  digitalWrite(LED, stateLED);
  previous = stateButton;
}
