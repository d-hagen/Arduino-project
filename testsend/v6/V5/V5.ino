#include <Arduino.h>

char buffer[10];  // Small buffer for one incoming message
char receivedBuffer[200]; // Stores accumulated messages
int receivedIndex = 0;   // Position tracker for receivedBuffer

bool isSending = false;
bool isProcessing = false;
unsigned long delaySend = 2000;
unsigned long delayReceive = 1000;

// Maximum tokens for processing
static const int MAX_TOKENS = 20;
static const int MAX_TOKEN_LEN = 20;
char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
int tokenCount = 0;

void setup() {
  Serial.begin(9600);
  delay(3000);
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
  delay(500);

  // Send command parts with a delay between each, similar to numbers
  Serial.println("writeN");
  delay(500);
  Serial.println("3");
  delay(500);
  Serial.println("add");
  delay(500);
  Serial.println("6");
  delay(500);
  Serial.println("7");
  delay(500);
  Serial.println("Test");
  delay(500);
  Serial.println("Message");
  delay(500);
  Serial.println("Hello");
  delay(500);
  Serial.println("World");
  delay(500);

  Serial.println("end");
}


bool receiveMessage() {
  static int index = 0;  // Persistent index for buffer filling

  while (Serial.available() > 0) {
    char receivedChar = Serial.read();

    // Handle end of message
    if (receivedChar == '\n' || receivedChar == '\r') {
      if (index > 0) {  // Only process non-empty messages
        buffer[index] = '\0';  // Null-terminate string
        Serial.print("Received: ");
        Serial.println(buffer);

        int bufferLength = strlen(buffer);
        // Append to receivedBuffer if space allows
        if (receivedIndex + bufferLength < (int)sizeof(receivedBuffer) - 1) {
          strcat(receivedBuffer, buffer);
          strcat(receivedBuffer, " ");  // Use space as delimiter
          receivedIndex += bufferLength + 1;
        } else {
          Serial.println("Buffer overflow, cannot store more messages.");
        }

        // Reset index for next message
        index = 0;

        // If "end" was received, signal processing
        if (strcmp(buffer, "end") == 0) {
          return true;
        }
      }
    } 
    else if (index < (int)sizeof(buffer) - 1) {  // Prevent buffer overflow
      buffer[index++] = receivedChar;
    }
  }
  return false;
}

void processState() {
  Serial.println("Processing...");
  Serial.print("All received messages: ");
  Serial.println(receivedBuffer);

  // Tokenize received messages
  tokenCount = tokenize(receivedBuffer, tokens, MAX_TOKENS);

  // Transform any "add" commands
  transformAddCommands(tokens, tokenCount);

  // Handle commands (like "writeN")
  handleCommands(tokens, tokenCount);

  // Reset buffers after processing
  receivedBuffer[0] = '\0';
  receivedIndex = 0;
  Serial.println("Processing complete.");
}

// ------------------------------------------------------------
// Tokenization and Command Processing
// ------------------------------------------------------------
int tokenize(const char* str, char tokens[][MAX_TOKEN_LEN], int maxTokens) {
  int count = 0;
  char tempBuffer[200];
  strncpy(tempBuffer, str, sizeof(tempBuffer));
  tempBuffer[sizeof(tempBuffer) - 1] = '\0';

  char* t = strtok(tempBuffer, " ");
  while (t != NULL && count < maxTokens) {
    strncpy(tokens[count], t, MAX_TOKEN_LEN - 1);
    tokens[count][MAX_TOKEN_LEN - 1] = '\0';
    count++;
    t = strtok(NULL, " ");
  }
  return count;
}

bool isInteger(const char* s) {
  if (*s == '\0') return false;
  if (*s == '-') s++;
  while (*s) {
    if (*s < '0' || *s > '9') return false;
    s++;
  }
  return true;
}

void transformAddCommands(char tokens[][MAX_TOKEN_LEN], int &count) {
  int i = 0;
  while (i < count) {
    if (strcmp(tokens[i], "add") == 0 && i + 2 < count) {
      char combined[MAX_TOKEN_LEN];
      combined[0] = '\0';

      if (isInteger(tokens[i + 1]) && isInteger(tokens[i + 2])) {
        int sum = atoi(tokens[i + 1]) + atoi(tokens[i + 2]);
        snprintf(combined, sizeof(combined), "%d", sum);
      } else {
        snprintf(combined, sizeof(combined), "%s%s", tokens[i + 1], tokens[i + 2]);
      }

      strncpy(tokens[i], combined, MAX_TOKEN_LEN);
      tokens[i][MAX_TOKEN_LEN - 1] = '\0';

      for (int j = i + 1; j + 2 < count; j++) {
        strncpy(tokens[j], tokens[j + 2], MAX_TOKEN_LEN);
        tokens[j][MAX_TOKEN_LEN - 1] = '\0';
      }
      count -= 2;
    }
    i++;
  }
}

void handleCommands(char tokens[][MAX_TOKEN_LEN], int count) {
  int i = 0;
  while (i < count) {
    if (strcmp(tokens[i], "writeN") == 0 && i + 1 < count) {
      int n = atoi(tokens[++i]);
      if (n <= 0) {
        Serial.println("Invalid N for writeN");
        continue;
      }
      for (int printed = 0; printed < n && i + 1 < count; printed++) {
        Serial.print("Next message [");
        Serial.print(printed + 1);
        Serial.print("]: ");
        Serial.println(tokens[++i]);
      }
    }
    i++;
  }
}
