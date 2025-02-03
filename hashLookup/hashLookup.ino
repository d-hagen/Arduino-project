#include <Arduino.h>
#include <map>

using namespace std;

// Maps to store function mappings
map<int, String> funcHash = {
    {0, "writeN"},
    {1, "addN"},
    {2, "subN"}
};

map<int, String> endHash = {
    {0, "end1"},
    {1, "end2"}
};

// Maps for character conversions
map<int, char> charHash;
map<char, int> charToBinaryHash;

// Initialize character hashes
void initializeCharHashes() {
    // Populate charHash with integer -> character mappings
    int value = 0; // Incrementing integer for each character
    for (char c = 'A'; c <= 'Z'; ++c) {
        charHash[value] = c;
        charToBinaryHash[c] = value;
        ++value;
    }
    for (char c = 'a'; c <= 'z'; ++c) {
        charHash[value] = c;
        charToBinaryHash[c] = value;
        ++value;
    }
}

// Function to convert a character to an encoded integer with a prefix
int CharToBinary(char c) {
    if (charToBinaryHash.find(c) != charToBinaryHash.end()) {
        return (1 << 16) | charToBinaryHash[c]; // Prefix 0b01 (1 in decimal)
    }
    return -1; // Invalid character
}

// Function to find the binary integer for a function name
int FunctToBinary(const String& funcName) {
    for (const auto& pair : funcHash) {
        if (pair.second == funcName) {
            return (2 << 16) | pair.first; // Prefix 0b10 (2 in decimal)
        }
    }
    return -1; // Unknown function
}



// Maximum size for the resulting array
#define MAX_RESULTS 100

// Function to decode a bitstring
int getValue(const String& bitString, String results[], int maxSize) {
    int resultCount = 0;

    // Process the input in chunks of 18 bits
    for (int i = 0; i < bitString.length() && resultCount < maxSize; i += 18) {
        // Extract an 18-bit chunk
        String chunk = bitString.substring(i, i + 18);
        if (chunk.length() < 18) {
            // Skip incomplete chunks
            continue;
        }

        // Convert the chunk to an integer
        int chunkInt = strtol(chunk.c_str(), nullptr, 2);

        // Determine the prefix
        int prefix = (chunkInt >> 16) & 0b11; // Extract the first 2 bits
        int payload = chunkInt & 0xFFFF;     // Extract the last 16 bits

        if (prefix == 0) { // Integer values
            results[resultCount++] = String(payload);

        } else if (prefix == 1) { // Characters
            if (charHash.find(payload) != charHash.end()) {
                results[resultCount++] = String(charHash[payload]);
            } else {
                results[resultCount++] = "?";
            }

        } else if (prefix == 2) { // Function names
            if (funcHash.find(payload) != funcHash.end()) {
                results[resultCount++] = funcHash[payload];
            } else {
                results[resultCount++] = "?";
            }

        } else if (prefix == 3) { // End markers
            if (endHash.find(payload) != endHash.end()) {
                results[resultCount++] = endHash[payload];
            } else {
                results[resultCount++] = "?";
            }
        }
    }

    return resultCount; // Return the number of results stored
}
