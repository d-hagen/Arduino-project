char buffer[10];
char receivedBuffer[200];
int receivedIndex = 0;
bool isSending = true;
bool isProcessing = false;
unsigned long delaySend = 2000;
unsigned long delayReceive = 1000;


void setup() {
  Serial.begin(9600);
  delay(5000);

  receivedBuffer[0] = '\0';
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

    int bufferLength = strlen(buffer);
    if (receivedIndex + bufferLength < sizeof(receivedBuffer) - 1) {
      strcat(receivedBuffer, buffer);
      strcat(receivedBuffer, " ");
      receivedIndex += bufferLength + 1;
    } else {
      Serial.println("Buffer overflow, cannot store more messages.");
    }

    if (strcmp(buffer, "end") == 0) {
      return true;
    }
  }
  return false;
}

void processState() {
  Serial.println("Processing...");
  Serial.print("All received messages: ");
  Serial.println(receivedBuffer);
  receivedBuffer[0] = '\0';
  receivedIndex = 0;
  Serial.println("Processing complete.");
}
