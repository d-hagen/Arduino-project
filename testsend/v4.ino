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

// --------------------  SEND  --------------------
void sendSequence() {
  Serial.println("start");
  delay(1000);
  for (int i = 1; i <= 5; i++) {
    Serial.println(i);
    delay(1000);
  }
  Serial.println("end");
}

// --------------------  RECEIVE  -----------------
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

// --------------------  PROCESS  -----------------
char* handleWriteN(char* token) {
  // Current token is "writeN", so move to the next token for N
  token = strtok(NULL, " ");
  if (token == NULL) {
    Serial.println("No integer found after 'writeN'.");
    return token; // token is NULL
  }

  int n = atoi(token);
  if (n <= 0) {
    Serial.println("Invalid or zero N after 'writeN'.");
    return strtok(NULL, " ");
  }

  // Print the next N tokens
  for (int i = 0; i < n; i++) {
    token = strtok(NULL, " ");
    if (token == NULL) {
      Serial.println("Not enough tokens to print.");
      return token;
    }
    Serial.print("Next message [");
    Serial.print(i + 1);
    Serial.print("]: ");
    Serial.println(token);
  }

  // Return the next token after these N
  return strtok(NULL, " ");
}

void processState() {
  Serial.println("Processing...");
  Serial.print("All received messages: ");
  Serial.println(receivedBuffer);

  // Make a copy so we can tokenize safely
  char tempBuffer[200];
  strncpy(tempBuffer, receivedBuffer, sizeof(tempBuffer));
  tempBuffer[sizeof(tempBuffer) - 1] = '\0';

  // Tokenize
  char* token = strtok(tempBuffer, " ");
  while (token != NULL) {
    // Check if it is "writeN"
    if (strcmp(token, "writeN") == 0) {
      token = handleWriteN(token);
    } else {
      // Move on
      token = strtok(NULL, " ");
    }
  }

  // Reset for next time
  receivedBuffer[0] = '\0';
  receivedIndex = 0;

  Serial.println("Processing complete.");
}
