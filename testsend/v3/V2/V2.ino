char buffer[10];  // Buffer to store incoming messages
bool isSending = false;
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
  static int index = 0;  // Keep track of position across function calls
  while (Serial.available() > 0) {
    char receivedChar = Serial.read();

    // Handle line endings (\n or \r\n)
    if (receivedChar == '\n' || receivedChar == '\r') {
      if (index > 0) {  // Only process complete messages
        buffer[index] = '\0'; // Null-terminate
        Serial.print("Received: ");
        Serial.println(buffer);

        if (strcmp(buffer, "end") == 0) {
          index = 0;  // Reset index for next message
          return true;
        }
      }
      index = 0; // Reset buffer for the next message
    } 
    else if (index < sizeof(buffer) - 1) {  // Prevent buffer overflow
      buffer[index++] = receivedChar;
    }
  }
  return false;
}

void processState() {
  Serial.println("Processing...");
  delay(processingDelay);
  Serial.println("Processing complete.");
}
