const int signalPin = 2;          // Pin to receive the signal
const int outputPin = 1;          // Pin to send the signal (Talking)
const int tolerance = 1;          // Tolerance in milliseconds (+/- 1 ms)
const int digitLength = 50;        // Expected duration of a digit in ms
const int maxBits = (17 * 10);    // Maximum number of binary digits to store

int inBuffer[maxBits];            // Array to store binary digits (BUFFER)
int outBuffer[100];               // Array for output buffer
int bitIndex = 0;                 // Current index in the binary list

// Define system states using enum
enum SystemState {
  Listening,
  Interpreting,
  Talking
};
SystemState state = Listening;    // Initialize to "Listening" state

void setup() {
  pinMode(signalPin, INPUT);      // Configure the signal pin as input
  pinMode(outputPin, OUTPUT);     // Configure the output pin as output
  Serial.begin(9600);             // Start serial communication

}

void loop() {
  // Call the appropriate function based on the current state
  switch (state) {
    case Listening:
      listen();
      break;

    case Interpreting:
      interpret();
      break;

    case Talking:
      talk();
      break;
  }
}

void listen() {
  // Clear the inBuffer
  for (int i = 0; i < maxBits; i++) {
    inBuffer[i] = 0;
  }
  bitIndex = 0;  // Reset the bit index to start saving from the beginning

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

  // Wait for the HIGH signal to complete
  while (digitalRead(signalPin) == HIGH) {
    // Do nothing, wait for the signal to go LOW
  }

  // Calculate the duration of the initial HIGH signal
  unsigned long highDuration = millis() - signalStart;

  // Check if the initial HIGH signal meets the 90% threshold
  if (highDuration >= (0.8 * digitLength)) {
    Serial.println("Initial HIGH signal detected and valid. Listening...");
    addBinaryDigit(1); // Count the initial HIGH signal as the first bit
    bitCount++;
  } else {
    Serial.println("Initial HIGH signal too short. Resetting...");
    return; // Exit if the initial HIGH signal is too short
  }

  while (true) {
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
      }
    } else if (signal == LOW && !isHigh) {
      // Continuous LOW signal
      int duration = currentTime - signalStart;
      if (duration >= digitLength) {
        int bits = round((float)duration / digitLength); // Calculate bits based on duration

        for (int i = 0; i < bits; i++) {
          addBinaryDigit(0);
          bitCount++;
        }
        signalStart = currentTime; // Reset signalStart for the next LOW period
      }
    }

    // Stop listening after filling the buffer
    if (bitIndex >= maxBits) {
      Serial.println("Buffer is full or maximum bits read.");
      state = Interpreting;  // Change state to "Interpreting"
      Serial.println("State changed to 'Interpreting'.");
      return;
    }
  }
}



void interpret() {
  Serial.println("Interpreting");

  // Copy data from inBuffer to outBuffer
  for (int i = 0; i < 100; i++) {
    outBuffer[i] = inBuffer[i];
  }

  // Change state to "Talking"
  state = Talking;
  Serial.println("State changed to 'Talking'.");
}

void talk() {
  Serial.println("Talking...");

  // Send outBuffer as HIGH and LOW signals
  for (int i = 0; i < maxBits; i++) {
    if (outBuffer[i] == 1) {
      digitalWrite(outputPin, HIGH);
    } else {
      digitalWrite(outputPin, LOW);
    }
    delay(digitLength); // Maintain each signal for the duration of one digit
  }

  Serial.println("Message sent.");

  state = Listening;  // Reset state to "Listening"
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