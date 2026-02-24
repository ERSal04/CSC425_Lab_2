// #include <Adafruit_VCNL4040.h>
// #include <M5Unified.h>
// // #include <M5Core2.h>

// void getScreenMetrics();

// // vcnl 4040
// Adafruit_VCNL4040 vcnl4040 = Adafruit_VCNL4040();

// int sHeight;
// int sWidth;

// int proximity = 0;

// unsigned long lastBeepTime = 0;
// bool beepState = false;  // Track if beep is on or off

// void setup() {
//   M5.begin();

//   getScreenMetrics();

//   Serial.print("Initializing Speaker");
//   M5.Speaker.begin();
//   M5.Speaker.setVolume(128);

//   Serial.print("Setting Vibration to 0");
//   M5.Power.setVibration(0);

//   Serial.print("Muting Speaker");
//   M5.Speaker.stop();

//   // Use adafruit library to connect to your device
//   Serial.println("Adafruit vcnl4040 config demo");
//   if (!vcnl4040.begin()) {
//    Serial.println("Could not find 4040 locally..."); 
//     while (1);
//   }
//   Serial.println("Found VCNL4040 Chip!");

// }

// void loop() {
  
//   // Library calls
//   Serial.printf("Proximity %d\n",  vcnl4040.getProximity());
//   Serial.printf("Ambient light: %d\n", vcnl4040.getLux());
//   Serial.printf("Raw White light: %d\n\n", vcnl4040.getWhiteLight());

//   proximity = vcnl4040.getProximity();

//    if (proximity > 30) {
//     // Map proximity to frequency: closer = higher pitch
//     // Proximity range ~30-300, map to 500Hz-2000Hz
//     int frequency = map(constrain(proximity, 30, 300), 30, 300, 500, 2000);
    
//     // Map proximity to beep interval: closer = faster beeping
//     // Proximity range ~30-300, map to 100ms-1000ms intervals
//     int beepInterval = map(constrain(proximity, 30, 300), 30, 300, 1000, 100);
    
//     // Set vibration proportional to proximity
//     uint8_t vibLevel = map(constrain(proximity, 30, 300), 30, 300, 50, 200);
//     M5.Power.setVibration(vibLevel);
    
//     // Square wave beeping - toggle on/off at equal intervals
//     if (millis() - lastBeepTime >= beepInterval) {
//       beepState = !beepState;
//       lastBeepTime = millis();
      
//       if (beepState) {
//         M5.Speaker.tone(frequency);
//         Serial.printf("Beep ON - Freq: %d Hz, Interval: %d ms\n", frequency, beepInterval);
//       } else {
//         M5.Speaker.stop();
//         Serial.println("Beep OFF");
//       }
//     }
//   } else {
//      // Object too far, turn everything off
//     M5.Power.setVibration(0);
//     M5.Speaker.stop();
//     beepState = false;
//     Serial.println("Out of range - off");
//     delay(100);  // Small delay when nothing detected
//   }
// }

// void getScreenMetrics() {
//   sHeight = M5.Lcd.height();
//   sWidth = M5.Lcd.width();
// }
