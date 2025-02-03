const int signalPin = 2;  // Pin to receive the signal
const int outputPin = 1;  // Pin to send the signal
const int digitLength = 500;  // Duration of each bit in ms
const int startSignalLength = 2000;  // Duration of the start signal in ms
const int maxBits = 100;  // Maximum bits in the message

int inBuffer[maxBits];  // Data received

void setup() {
  pinMode(signalPin, INPUT);
  pinMode(outputPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  receiveSignal();
  delay(100);  // Delay to avoid simultaneous talking
  echoSignal();
}

void receiveSignal() {
  Serial.println("Waiting for start signal...");
  while (digitalRead(signalPin) == LOW) {
    // Wait for the start signal
  }

  unsigned long startTime = millis();
  while (millis() - startTime < startSignalLength) {
    // Wait for the duration of the start signal
  }

  Serial.println("Start signal detected. Receiving message...");
  int bitIndex = 0;
  unsigned long signalStart = millis();

  while (bitIndex < maxBits) {
    int signal = digitalRead(signalPin);
    unsigned long currentTime = millis();
    if (currentTime - signalStart >= digitLength) {
      inBuffer[bitIndex++] = signal;
      signalStart = currentTime;
    }
  }

  Serial.println("Message received:");
  for (int i = 0; i < maxBits; i++) {
    Serial.print(inBuffer[i]);
  }
  Serial.println();
}

void echoSignal() {
  Serial.println("Echoing start signal...");
  digitalWrite(outputPin, HIGH);
  delay(startSignalLength);
  digitalWrite(outputPin, LOW);
  delay(digitLength);

  Serial.println("Echoing message...");
  for (int i = 0; i < maxBits; i++) {
    digitalWrite(outputPin, inBuffer[i]);
    delay(digitLength);
  }
  digitalWrite(outputPin, LOW);  // Ensure the line is LOW after sending
  Serial.println("Message echoed.");
}
