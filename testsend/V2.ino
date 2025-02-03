char buffer[10];
bool isSending = true;
bool isProcessing = false;
unsigned long delaySend = 2000;
unsigned long delayReceive = 1000;
unsigned long processingDelay = 3000;

void setup() {
  Serial.begin(9600);
  delay(5000);

}

void loop() {
  if (isSending) {
    sendSequence();
    delay(delayReceive);
    isSending = false;
    isProcessing = false;
  } else if (isProcessing) {
    processState();
    isProcessing = false;
    isSending = true;
  } else {
    if (receiveMessage()) {
      delay(delaySend);
      isProcessing = true;
    }
  }
}

void sendSequence() {
  Serial.println("start");
  delay(1000);
  for (int i = 1; i <= 5; i++) {
    Serial.println(i);
    delay(1000);
  }
  Serial.println("end");
}

bool receiveMessage() {
  int index = 0;
  while (Serial.available() > 0) {
    char receivedChar = Serial.read();
    if (receivedChar == '\n') {
      buffer[index] = '\0';
      break;
    } else if (index < sizeof(buffer) - 1) {
      buffer[index++] = receivedChar;
    }
  }

  if (index > 0) {
    buffer[index] = '\0';
    Serial.print("Received: ");
    Serial.println(buffer);

    if (strcmp(buffer, "end") == 0) {
      return true;
    }
  }
  return false;
}

void processState() {
  Serial.println("Processing...");
  delay(processingDelay);
  Serial.println("Processing complete.");
}
