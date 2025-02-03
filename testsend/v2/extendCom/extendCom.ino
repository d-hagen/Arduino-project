char buffer[10];  // Buffer to store incoming messages
bool isSending = true;  
unsigned long delaySend = 1000;  
unsigned long delayReceive = 500; 

void setup() {
  Serial.begin(9600);
  delay(5000);
}

void loop() {
  if (isSending) {
    sendSequence();
    delay(delayReceive);
    isSending = false;
  } else {
    if (receiveMessage()) {
      delay(delaySend);
      isSending = true;
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
  static int index = 0;  // Static index to track buffer position
  while (Serial.available() > 0) {
    char receivedChar = Serial.read();

    // Handle line ending correctly
    if (receivedChar == '\n' || receivedChar == '\r') {
      if (index > 0) {  // Ensure we don't process empty messages
        buffer[index] = '\0'; // Null-terminate
        Serial.print("Received: ");
        Serial.println(buffer);

        if (strcmp(buffer, "end") == 0) {
          index = 0;  // Reset index for next message
          return true; // Valid message received
        }
      }
      index = 0; // Reset buffer for next message
    } 
    else if (index < sizeof(buffer) - 1) {  // Prevent buffer overflow
      buffer[index++] = receivedChar;
    }
  }
  return false;
}
