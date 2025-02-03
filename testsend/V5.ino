#include <Arduino.h>

char buffer[10];
char receivedBuffer[200];
int receivedIndex = 0;
bool isSending = true;
bool isProcessing = false;
unsigned long delaySend = 2000;
unsigned long delayReceive = 1000;

// Adjust as needed
static const int MAX_TOKENS = 20;    // maximum number of tokens
static const int MAX_TOKEN_LEN = 20; // maximum length of each token

// Storage for tokenized input
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
  } 
  else if (isProcessing) {
    processState();
    isProcessing = false;
    isSending = true;
  } 
  else {
    // Attempt to receive
    if (receiveMessage()) {
      // Once "end" is read, process
      delay(delaySend);
      isProcessing = true;
    }
  }
}

// ------------------------------------------------------------
// 1) Sending
// ------------------------------------------------------------
void sendSequence() {
  Serial.println("start");
  delay(500);
  for (int i = 1; i <= 5; i++) {
    Serial.println(i);
    delay(500);
  }
  Serial.println("end");
}

// ------------------------------------------------------------
// 2) Receiving
// ------------------------------------------------------------
bool receiveMessage() {
  int index = 0;
  while (Serial.available() > 0) {
    char receivedChar = Serial.read();
    // Stop reading at newline
    if (receivedChar == '\n') {
      buffer[index] = '\0';
      break;
    } 
    // Accumulate in buffer
    else if (index < (int)sizeof(buffer) - 1) {
      buffer[index++] = receivedChar;
    }
  }

  // If we got something in buffer
  if (index > 0) {
    buffer[index] = '\0'; // null-terminate
    Serial.print("Received: ");
    Serial.println(buffer);

    int bufferLength = strlen(buffer);
    // Append to the big "receivedBuffer"
    if (receivedIndex + bufferLength < (int)sizeof(receivedBuffer) - 1) {
      strcat(receivedBuffer, buffer);
      strcat(receivedBuffer, " "); // add space as a delimiter
      receivedIndex += bufferLength + 1;
    } 
    else {
      Serial.println("Buffer overflow, cannot store more messages.");
    }

    // If the token was "end", signal to process
    if (strcmp(buffer, "end") == 0) {
      return true;
    }
  }
  return false;
}

// ------------------------------------------------------------
// 3) Processing
// ------------------------------------------------------------

/**
 * Splits 'str' on spaces into up to 'maxTokens' tokens.
 * Each token is stored in tokens[i], up to MAX_TOKEN_LEN.
 * Returns the number of tokens found.
 */
int tokenize(const char* str, char tokens[][MAX_TOKEN_LEN], int maxTokens) {
  int count = 0;
  char tempBuffer[200];
  strncpy(tempBuffer, str, sizeof(tempBuffer));
  tempBuffer[sizeof(tempBuffer) - 1] = '\0';

  // Get first token
  char* t = strtok(tempBuffer, " ");
  
  while (t != NULL && count < maxTokens) {
    // Copy token into tokens[count]
    strncpy(tokens[count], t, MAX_TOKEN_LEN - 1);
    tokens[count][MAX_TOKEN_LEN - 1] = '\0';
    count++;
    t = strtok(NULL, " ");
  }
  return count;
}

/**
 * Returns true if 's' consists of only an optional minus sign
 * followed by digits (i.e. a valid integer string).
 */
bool isInteger(const char* s) {
  if (*s == '\0') return false;

  // optional minus
  if (*s == '-') {
    s++;
    if (*s == '\0') return false; // string was just "-"
  }

  // now all must be digits
  while (*s) {
    if (*s < '0' || *s > '9') {
      return false;
    }
    s++;
  }
  return true;
}

/**
 * Transform the token list by handling any "add" commands.
 * When we see "add", we attempt to consume the next two tokens,
 * combine them into a single token, and replace the "add" + 2 tokens
 * with the single result.
 *
 * Example:
 *   tokens: [ "writeN", "2", "add", "3", "4", "Hello" ]
 *   transforms to: [ "writeN", "2", "7", "Hello" ]
 *   (because "3" + "4" => 7).
 *
 * If the next two tokens are not both integers, we concatenate them
 * as strings. e.g. "hello" + "world" => "helloworld"
 */
void transformAddCommands(char tokens[][MAX_TOKEN_LEN], int &count) {
  int i = 0;
  while (i < count) {
    if (strcmp(tokens[i], "add") == 0) {
      // We need two more tokens
      if (i + 2 < count) {
        char combined[MAX_TOKEN_LEN];
        combined[0] = '\0';

        bool bothInts = (isInteger(tokens[i + 1]) && isInteger(tokens[i + 2]));
        if (bothInts) {
          // Sum them
          int val1 = atoi(tokens[i + 1]);
          int val2 = atoi(tokens[i + 2]);
          int sum  = val1 + val2;
          snprintf(combined, sizeof(combined), "%d", sum);
        }
        else {
          // Concatenate them
          snprintf(combined, sizeof(combined), "%s%s", tokens[i + 1], tokens[i + 2]);
        }

        // Now replace "add" with the combined token
        strncpy(tokens[i], combined, MAX_TOKEN_LEN);
        tokens[i][MAX_TOKEN_LEN - 1] = '\0';

        // Shift everything else left by 2
        for (int j = i + 1; j + 2 < count; j++) {
          strncpy(tokens[j], tokens[j + 2], MAX_TOKEN_LEN);
          tokens[j][MAX_TOKEN_LEN - 1] = '\0';
        }
        // We removed two tokens
        count -= 2;
      }
      else {
        Serial.println("Warning: 'add' found but not enough tokens to consume.");
      }
    }
    // Move to next token
    i++;
  }
}

/**
 * Handles the "writeN" command and any other top-level commands.
 * After transformAddCommands(), an "add" sequence has been replaced
 * by exactly one token, so "writeN" will see it as one.
 */
void handleCommands(char tokens[][MAX_TOKEN_LEN], int count) {
  int i = 0;
  while (i < count) {
    if (strcmp(tokens[i], "writeN") == 0) {
      i++; // move past "writeN"
      if (i < count) {
        // Next token should be N
        int n = atoi(tokens[i]);
        i++;
        if (n <= 0) {
          Serial.println("Invalid N for writeN");
          continue; // move on
        }
        // Print next n tokens (if available)
        for (int printed = 0; printed < n && i < count; printed++) {
          Serial.print("Next message [");
          Serial.print(printed + 1);
          Serial.print("]: ");
          Serial.println(tokens[i]);
          i++;
        }
      }
    }
    else {
      // Unrecognized command => skip
      i++;
    }
  }
}

void processState() {
  Serial.println("Processing...");
  Serial.print("All received messages: ");
  Serial.println(receivedBuffer);

  // 1) Break the entire receivedBuffer into tokens
  tokenCount = tokenize(receivedBuffer, tokens, MAX_TOKENS);

  // 2) Replace any "add" command + next 2 tokens with a single token
  transformAddCommands(tokens, tokenCount);

  // 3) Handle commands (like "writeN") on the resulting token list
  handleCommands(tokens, tokenCount);

  // Reset for next time
  receivedBuffer[0] = '\0';
  receivedIndex = 0;
  Serial.println("Processing complete.");
}
