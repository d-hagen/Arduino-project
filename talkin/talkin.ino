const int signalPin = 2;          // Pin to receive the signal
const int outputPin = 1;          // Pin to send the signal (Talking)
const int tolerance = 1;          // Tolerance in milliseconds (+/- 1 ms)
const int digitLength = 500;        // Expected duration of a digit in ms
const int maxBits = (17 * 10);    // Maximum number of binary digits to store

int inBuffer[maxBits];            // Array to store binary digits (BUFFER)
int outBuffer[100];               // Array for output buffer
int bitIndex = 0;                 // Current index in the binary list
char state[] = "Talking";    // Current system state
int runs = 0;
int totalruns= 0;

void setup() {
  pinMode(signalPin, INPUT);      // Configure the signal pin as input
  pinMode(outputPin, OUTPUT);     // Configure the output pin as output
  Serial.begin(9600);             // Start serial communication for debugging
  generateOutBuffer();            // Generate the output buffer

  delay(5000); 
}

void loop() {
  if (strcmp(state, "Listening") == 0) {
    listen();
  }

  if (strcmp(state, "Interpreting") == 0) {
    interpret();
  }

  if (strcmp(state, "Talking") == 0) {
    talk();
  }
}

void listen() {
  // Clear the inBuffer and reset bitIndex
  for (int i = 0; i < maxBits; i++) {
    inBuffer[i] = 0; // Reset each element of the buffer to 0
  }
  bitIndex = 0; // Reset the bit index to 0

  unsigned long signalStart = 0;     // Start time of a HIGH or LOW signal
  bool isHigh = false;               // Current signal state
  int bitCount = 0;                  // Count of bits read

  Serial.println("Waiting for initial HIGH signal...");

  // Wait for the first HIGH signal
  while (digitalRead(signalPin) == LOW) {
    // Do nothing until the signal goes HIGH
  }

  // Record the start time of the first HIGH signal
  signalStart = millis();
  isHigh = true;

  // Wait for the HIGH signal to finish
  while (digitalRead(signalPin) == HIGH) {
    // Wait for the signal to transition to LOW
  }

  // Calculate the duration of the initial HIGH signal
  unsigned long highDuration = millis() - signalStart;

  // Ensure the initial HIGH signal is long enough
  if (highDuration < (0.9 * digitLength)) {
    Serial.println("Initial HIGH signal too short. Aborting...");
    return; // Exit if the initial HIGH signal is too short
  }

  Serial.println("Initial HIGH signal detected and valid. Listening...");
  addBinaryDigit(1); // Count the initial HIGH signal as the first bit
  bitCount++;

  while (true) {

    if (bitCount == 100){
      strcpy(state, "Interpreting");
      Serial.println("State changed to 'Interpreting'.");
    }
    int signal = digitalRead(signalPin); // Read the signal on the pin
    unsigned long currentTime = millis();

    if (signal == HIGH && !isHigh) {
      // Transition from LOW to HIGH
      isHigh = true;
      signalStart = currentTime;
    } else if (signal == LOW && isHigh) {
      // Transition from HIGH to LOW
      isHigh = false;
      int duration = currentTime - signalStart;
      int bits = round((float)duration / digitLength); // Calculate bits based on duration

      for (int i = 0; i < bits; i++) {
        addBinaryDigit(1);
        bitCount++;
        if (bitCount % 18 == 0) {
          // Check the next two bits after every 18 bits
          if (checkTwoOnes()) {
            // Read 16 more bits and stop
            for (int j = 0; j < 16; j++) {
              waitForNextBit();
            }
            strcpy(state, "Interpreting");
            Serial.println("State changed to 'Interpreting'.");
            return;
          }
        }
      }
    } else if (signal == LOW && !isHigh) {
      // Continuous LOW signal
      int duration = currentTime - signalStart;
      if (duration >= digitLength) {
        int bits = round((float)duration / digitLength); // Calculate bits based on duration

        for (int i = 0; i < bits; i++) {
          addBinaryDigit(0);
          bitCount++;
          if (bitCount % 18 == 0) {
            // Check the next two bits after every 18 bits
            if (checkTwoOnes()) {
              // Read 16 more bits and stop
              for (int j = 0; j < 16; j++) {
                waitForNextBit();
              }
              strcpy(state, "Interpreting");
              Serial.println("State changed to 'Interpreting'.");
              return;
            }
          }
        }
        signalStart = currentTime; // Reset signalStart for the next LOW period
      }
    }
  }
}



void interpret() {
  Serial.println("Interpreting");

  // Print the last 18 bits of the inBuffer
  Serial.print("Last 18 bits of inBuffer: ");
  for (int i = maxBits - 18; i < maxBits; i++) {
    Serial.print(inBuffer[i]);
  }
  Serial.println();

  // Compare inBuffer and outBuffer
  bool buffersMatch = true;
  for (int i = 0; i < 100; i++) {
    if (inBuffer[i] != outBuffer[i]) {
      buffersMatch = false;

      // Identify the chunk containing the difference
      int chunkStart = (i / 18) * 18; // Calculate the start of the 18-bit chunk
      int chunkEnd = chunkStart + 17; // Calculate the end of the 18-bit chunk

      // Print the chunk from inBuffer
      Serial.print("Chunk from inBuffer (bits ");
      Serial.print(chunkStart);
      Serial.print(" to ");
      Serial.print(chunkEnd);
      Serial.print("): ");
      for (int j = chunkStart; j <= chunkEnd; j++) {
        Serial.print(inBuffer[j]);
      }
      Serial.println();

      // Print the chunk from outBuffer
      Serial.print("Chunk from outBuffer (bits ");
      Serial.print(chunkStart);
      Serial.print(" to ");
      Serial.print(chunkEnd);
      Serial.print("): ");
      for (int j = chunkStart; j <= chunkEnd; j++) {
        Serial.print(outBuffer[j]);
      }
      Serial.println();

      break; // Exit the loop after printing the first detected difference
    }
  }

  if (buffersMatch) {
    Serial.println("inBuffer matches outBuffer!");
    runs++;
    totalruns++;
    Serial.println("Successfull runs:");
    Serial.println(runs);
    Serial.println("Total runs:");
    Serial.println(totalruns);
  } else {
    totalruns++;
    Serial.println("inBuffer does NOT match outBuffer!");
    Serial.println("Successfull runs:");
    Serial.println(runs);
    Serial.println("Total runs:");
    Serial.println(totalruns);
  }

  // Change state to "Talking"
  strcpy(state, "Talking");
  Serial.println("State changed to 'Talking'.");
}

void talk() {
  Serial.println("Talking...");

  // Send outBuffer as HIGH and LOW signals
  for (int i = 0; i < (maxBits); i++) {
    if (outBuffer[i] == 1) {
      digitalWrite(outputPin, HIGH);
    } else {
      digitalWrite(outputPin, LOW);
    }
    delay(digitLength); // Maintain each signal for the duration of one digit
  }

  Serial.println("Message sent.");

  strcpy(state, "Listening");
  Serial.println("State changed to 'Listening'.");
}

void addBinaryDigit(int bit) {
  if (bitIndex < maxBits) {
    inBuffer[bitIndex++] = bit;
  } else {
    Serial.println("Buffer is full!");
  }
}

void shiftAndAddToLocalBuffer(int buffer[], int bit, int size) {
  // Shift the buffer to the left and add the new bit at the end
  for (int i = 0; i < size - 1; i++) {
    buffer[i] = buffer[i + 1];
  }
  buffer[size - 1] = bit;
}

void generateOutBuffer() {
  randomSeed(analogRead(0)); // Seed the random number generator
  int previousBit = 0;

  // Calculate the number of bits to generate, excluding the final sequence
  int bitsToGenerate = 100 - 18;

  outBuffer[0] = 1;

  for (int i = 1; i < bitsToGenerate; i++) {
    int bit = random(0, 2); // Generate a random 0 or 1

    // Check if the current bit is the 18th or 19th in a sequence
    if ((i ) % 18 == 0 || (i ) % 19 == 0) {
      if (i > 0 && previousBit == 1) {
        bit = 0; // Ensure that two consecutive 1s don't appear at 18th/19th bit positions
      }
    }

    outBuffer[i] = bit;
    previousBit = bit; // Update the previous bit
  }

  // Append the ending sequence 11 0000 0000 0000 0000
  int endSequence[] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (int i = 0; i < 18; i++) {
    outBuffer[bitsToGenerate + i] = endSequence[i];
  }

  // Debug: Print the generated outBuffer
  Serial.println("Generated outBuffer:");
  for (int i = 0; i < 100; i++) {
    Serial.print(outBuffer[i]);
  }
  Serial.println();
}

// Helper Function to Check the Next Two Bits
bool checkTwoOnes() {
  if (bitIndex >= 2) {
    return inBuffer[bitIndex - 2] == 1 && inBuffer[bitIndex - 1] == 1;
  }
  return false;
}

// Helper Function to Wait for the Next Bit
void waitForNextBit() {
  while (digitalRead(signalPin) == HIGH); // Wait for LOW
  while (digitalRead(signalPin) == LOW);  // Wait for HIGH
  unsigned long startTime = millis();

  while (digitalRead(signalPin) == HIGH); // Wait for end of HIGH
  unsigned long duration = millis() - startTime;

  int bits = round((float)duration / digitLength);
  for (int i = 0; i < bits; i++) {
    addBinaryDigit(1);
  }
}