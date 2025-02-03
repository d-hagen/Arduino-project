char buffer[10];        // Buffer to store incoming messages
bool isSending = true;  
unsigned long delaySend = 1000;  
unsigned long delayReceive = 500; 

void setup() {
  Serial.begin(9600);
  delay(5000)
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
    delay(1000); // Short delay between sending numbers
  }
  Serial.println("end");
}

bool receiveMessage() {
  int index = 0;
  while (Serial.available() > 0) {
    char receivedChar = Serial.read();
    if (receivedChar == '\n') {
      buffer[index] = '\0'; // Null-terminate the string
      break;
    } else if (index < sizeof(buffer) - 1) {
      buffer[index++] = receivedChar;
    }
  }

  if (index > 0) {
    buffer[index] = '\0'; // Ensure the buffer is null-terminated
    Serial.print("Received: ");
    Serial.println(buffer);

    if (strcmp(buffer, "end") == 0) {
      return true; // Trigger state change when "end" is received
    }
  }
  return false;
}
