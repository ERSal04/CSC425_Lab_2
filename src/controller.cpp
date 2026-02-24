// // #include <Adafruit_VCNL4040.h>
// #include <M5Unified.h>
// #include <Adafruit_seesaw.h>

// int last_x = 0, last_y = 0;

// Adafruit_seesaw controller;

// int p1Posx = 100, p1Posy = 120;
// int p2Posx = 200, p2Posy = 120;
// int p1Speed = 1, p2Speed = 1;

// #define BUTTON_X 6
// #define BUTTON_Y 2
// #define BUTTON_A 5
// #define BUTTON_B 1
// #define BUTTON_SELECT 0
// #define BUTTON_START 16
// uint32_t button_mask = (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_START) | 
//                        (1UL << BUTTON_A) | (1UL << BUTTON_B) | (1UL << BUTTON_SELECT);

// void showGameOverScreen();
// void updateP1Pos(int last_x, int last_y, uint32_t buttons);
// void updateP2Pos(uint32_t buttons);
// void drawScreen();

// void setup() {
//   Serial.begin(115200);

//   while(!Serial) {
//     delay(10);
//   }

//   M5.begin();

//   // Use adafruit library to connect to your device
//   if (!controller.begin(0x50)) {
//    Serial.println("Could not find Controller locally..."); 
//     while (1);
//   }
//   Serial.println("Found Controller!");

//   controller.pinModeBulk(button_mask, INPUT_PULLUP);
//   controller.setGPIOInterrupts(button_mask, 1);

//   M5.Lcd.clear();
//   M5.Lcd.fillScreen(TFT_BLACK);

//   drawScreen();
// }

// void loop() {
//   M5.update();
//   // delay(100); // delay in loop to slow serial output

//   // Reverse x/y values to match joystick orientation
//   int x = 1023 - controller.analogRead(14);
//   int y = 1023 - controller.analogRead(15);
  
//   if ( (abs(x - last_x) > 3)  ||  (abs(y - last_y) > 3)) {
//     Serial.print("x: "); Serial.print(x); Serial.print(", "); Serial.print("y: "); Serial.println(y);
//     last_x = x;
//     last_y = y;
//   }  

//   uint32_t buttons = controller.digitalReadBulk(button_mask);

//   updateP1Pos(last_x, last_y, buttons);
//   updateP2Pos(buttons);

//   drawScreen();

//   // Check if players are within each others reach
//   if(abs(p1Posx - p2Posx) < 10 && abs(p1Posy - p2Posy) < 10) {
//     // Game Over
//     Serial.print("Game over");
//     showGameOverScreen();
//     while (1) {
//       delay(500);
//     }
//   }
// }

// void drawScreen() {
//   p1Posx = constrain(p1Posx, 1, 319);
//   p1Posy = constrain(p1Posy, 1, 239);
//   p2Posx = constrain(p2Posx, 1, 319);
//   p2Posy = constrain(p2Posy, 1, 239);

//   M5.Lcd.fillScreen(TFT_BLACK);
  
//   // Player 1
//   // Serial.printf("Player1 pos: (x = %d, y = %d) \n", p1Posx, p1Posy);
//   M5.Lcd.fillRect(p1Posx, p1Posy, 4, 4, TFT_RED);

//   // Player 2
//   // Serial.printf("Player2 pos: (x = %d, y = %d) \n", p2Posx, p2Posy);
//   M5.Lcd.fillRect(p2Posx, p2Posy, 4, 4, TFT_BLUE);
// }

// void showGameOverScreen() {
//   M5.Lcd.clear();
//   M5.Lcd.setCursor(80, 120);
//   M5.Lcd.setTextSize(2);
//   M5.Lcd.print("GAME OVER");

//   M5.Lcd.setCursor(80, 140);
//   M5.Lcd.setTextSize(1);
//   M5.Lcd.printf("Time alive: %d seconds", millis() / 1000);
// }

// void updateP1Pos(int last_x, int last_y, uint32_t buttons) {
//   if (last_x > 520) // go right
//   {
//     p1Posx += p1Speed;
//   }
//   if (last_y < 490) // go down
//   {
//     p1Posy += p1Speed;
//   }
//   if (last_x < 490) // go left
//   {
//     p1Posx -= p1Speed;
//   }
//   if (last_y > 520) // go up
//   {
//     p1Posy -= p1Speed;
//   }
//   if (! (buttons & (1UL << BUTTON_START))) {
//     // Serial.println("Button START pressed");
//     if (p1Speed == 5) {
//       p1Speed = 1;
//     } else {
//       p1Speed++;
//     }
//     Serial.printf("p1Speed = %d\n", p1Speed);
//   }
// }

// void updateP2Pos(uint32_t buttons) {
//   if (! (buttons & (1UL << BUTTON_A))) {
//     // Serial.println("Button A pressed");
//     p2Posx += p2Speed;
//   }
//   if (! (buttons & (1UL << BUTTON_B))) {
//     // Serial.println("Button B pressed");
//     p2Posy += p2Speed;
//   }
//   if (! (buttons & (1UL << BUTTON_Y))) {
//     // Serial.println("Button Y pressed");
//     p2Posx -= p2Speed;
//   }
//   if (! (buttons & (1UL << BUTTON_X))) {
//     // Serial.println("Button X pressed");
//     p2Posy -= p2Speed;
//   }
//   if (! (buttons & (1UL << BUTTON_SELECT))) {
//     // Serial.println("Button SELECT pressed");
//     if (p2Speed == 5) {
//       p2Speed = 1;
//     } else {
//       p2Speed++;
//     }
//     Serial.printf("p2Speed = %d\n", p2Speed);
//   }

// }