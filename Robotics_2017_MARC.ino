#include <SPI.h>
#include "Pixy.h"
#include <Wire.h>

TPixy<LinkSPI> pixy; // This is the main Pixy object
unsigned char returnToMaster;

const float filterConst = 0.125f;
float targetPos         = 159.0f;
float targetPos_tmp     = 159.0f;

char buf[32];

void setup() {
  Serial.begin(9600);
  Serial.println("\nStarting...");

  Wire.begin(63);
  Wire.onRequest(requestHandler);

  returnToMaster = (char)255; // Default return value
  pixy.init();
}

void loop() {
  static int count = 0;
  int cumulativeX = 0;

  if (count == 0) {
    uint16_t blocks = pixy.getBlocks();
    if (blocks > 0) {
      
      // Add together all X values
      for (int i = 0; i < blocks; i++) {
        cumulativeX += pixy.blocks[i].returnX();
      }
      
      // Find the average X position of the block system
      cumulativeX /= blocks;

      // Evaluate returnToMaster
      targetPos_tmp = (filterConst) * (float)cumulativeX + 
                      (1.0f - filterConst) * targetPos;
      targetPos = constrain(targetPos_tmp, 0.0f, 319.0f);
    } else {
      // Default return value if no blocks are detected
      cumulativeX = 159;
    }
  }

  count = ((count++) % 50);
}

void requestHandler() {
   returnToMaster = (char)map((uint16_t)targetPos, 0, 319, 0, 254);

   // Display output and write to Master
   Wire.write(returnToMaster);
   sprintf(buf, "Response: %d", returnToMaster);
   Serial.println(buf);
}

