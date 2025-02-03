const int signalPin = 2;  // Pin to receive the signal
const int outputPin = 1;  // Pin to send the signal
const int digitLength = 50;  // Duration of each bit in ms
const int startSignalLength = 200;  // Duration of the start signal in ms
const int maxBits = 100;  // Maximum bits in the message

int outBuffer[maxBits];  // Data to send
int inBuffer[maxBits];   // Data received

void setup() {
  pinMode(signalPin, INPUT);
  pinMode(outputPin, OUTPUT);
  Serial.begin(9600);

  // Generate a random binary message
  randomSeed(analogRead(0));
  for (int i = 0; i < maxBits; i++) {
    outBuffer[i] = random(0, 2);
  }

  Serial.println("Generated message:");
  for (int i = 0; i < maxBits; i++) {
    Serial.print(outBuffer[i]);
  }
  Serial.println();
}

void loop() {
  sendSignal();
  delay(100);  // Delay to avoid simultaneous talking
  receiveSignal();
  compareSignals();
  delay(500);  // Pause before the next communication
}

void sendSignal() {
  Serial.println("Sending start signal...");
  digitalWrite(outputPin, HIGH);
  delay(startSignalLength / 2);
  digitalWrite(outputPin, LOW);
  delay(startSignalLength / 2);

  Serial.println("Sending message...");
  for (int i = 0; i < maxBits; i++) {
    digitalWrite(outputPin, outBuffer[i]);
    delay(digitLength);
  }
  digitalWrite(outputPin, LOW);  // Ensure the line is LOW after sending
  Serial.println("Message sent.");
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

void compareSignals() {
  Serial.println("Comparing signals...");
  bool match = true;
  for (int i = 0; i < maxBits; i++) {
    if (outBuffer[i] != inBuffer[i]) {
      match = false;
      Serial.print("Mismatch at bit ");
      Serial.println(i);
    }
  }

  if (match) {
    Serial.println("Signals match!");
  } else {
    Serial.println("Signals do not match.");
  }
}
