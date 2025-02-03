#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Initialize SH1107 128x128 OLED Display
U8G2_SH1107_128X128_1_HW_I2C u8g2(U8G2_R0); // Hardware I2C, no rotation

// Define walls as tuples of start and end points
const int num_walls = 13; // Adjust based on number of walls
const int walls[num_walls][2][2] = {
    {{5, 30}, {5, 120}},   // Left vertical wall
    {{120, 30}, {120, 120}}, // Right vertical wall
    {{5, 120}, {90, 120}}, // Bottom horizontal wall
    {{100, 120}, {120, 120}},
    {{60, 30}, {60, 75}},
    {{60, 85}, {60, 105}}, // A middle vertical wall
    {{60, 100}, {90, 100}},
    {{100, 100}, {120, 100}},
    {{60, 30}, {120, 30}},
    {{60, 30}, {5, 30}},
    {{60, 95}, {5, 95}},
    {{60, 115}, {60, 120}},
    {{20, 95}, {20, 110}}
};

// Function to draw a straight line using Manhattan distance
void drawWall(int x1, int y1, int x2, int y2) {
    int dx = (x2 > x1) ? 1 : (x2 < x1) ? -1 : 0; // Determine x direction
    int dy = (y2 > y1) ? 1 : (y2 < y1) ? -1 : 0; // Determine y direction

    int x = x1, y = y1;
    while (x != x2 || y != y2) {
        u8g2.drawPixel(x, y);
        if (x != x2) x += dx; // Move horizontally if needed
        if (y != y2) y += dy; // Move vertically if needed
    }
    u8g2.drawPixel(x2, y2); // Draw last pixel
}

void setup() {
    u8g2.begin();  // Start the display
    u8g2.setContrast(255); // Max brightness
}

void loop() {
    u8g2.firstPage();
    do {
        // Draw all walls from tuple list
        for (int i = 0; i < num_walls; i++) {
            drawWall(walls[i][0][0], walls[i][0][1], walls[i][1][0], walls[i][1][1]);
        }
    } while (u8g2.nextPage()); // Update the display
}

