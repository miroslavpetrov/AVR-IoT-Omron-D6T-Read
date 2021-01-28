/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"
#include "util/delay.h"
// 7bit I2C Address of Omron D6T in binary format
#define D6T_ADDR 0b0001010
// 7bit I2C Read command for Omron D6T in binary format
#define D6T_CMD 0b1001100
// The number of consecutive temperature readings with increase in temperature equal or bigger than the value defined in threshHoldInc
#define comparingNumInc 3
// The number of consecutive temperature readings with decrease in temperature equal or smaller than the value defined in threshHoldDec
#define comparingNumDec 3
// The threshold value in Deg C. The temperature reading needs to increase with this value in order to trigger "occupancy: true" state.
#define threshHoldInc 5
// The threshold value in Deg C. The temperature reading needs to decrease with this value in order to trigger "occupancy: false" state.
#define threshHoldDec 10

// Initializing variables for occupancy algorithm.
uint16_t lastReading = 0;
int16_t seqData[40] = {0};
bool occuPix = 0;
bool occuPixFlag = false;
uint8_t resultOccupancy = 0;
uint16_t totalCount = 0;

bool judge_Occupancy(void) { 
  int j = 0; 
  for (j = 0; j < 39; j++){
    seqData[39 - j] = seqData[38 - j];
  }
  seqData[0] = lastReading;            
  if (totalCount <= comparingNumInc){
    totalCount++;
  }
  if (totalCount > comparingNumInc){    
    if (occuPix == false){
      if ((int16_t)(seqData[0] - seqData[comparingNumInc]) >= (int16_t)threshHoldInc){
        occuPix = true;
      }
    }
    else{   //resultOccupancy == true
      if ((int16_t)(seqData[comparingNumDec] - seqData[0]) >= (int16_t)threshHoldDec){
        occuPix = false;
      }
    }
    if (resultOccupancy == 0) {                
        if(occuPix == true){
          resultOccupancy = 1;
        }
    }
    else{
      occuPixFlag = false;
      if (occuPix == true){
        occuPixFlag = true;
      }
      if (occuPixFlag == false){
        resultOccupancy = 0;
      }
    }
  }
  return true;
}
/*
    Main application
*/
int main(void)



{
   
    SYSTEM_Initialize();
    uint16_t sensorValue = 0;
     
    
    while (1){
        
        _delay_ms(1000);
        //Send read command to the sensor
        i2c_write1ByteRegister(D6T_ADDR, D6T_CMD, 1);
        //Create buffer to store the raw data read from the sensor.
        unsigned char buf[5]; 
        //Read the sensor and store the data in the buffer.
        i2c_readNBytes(D6T_ADDR, buf, 5);
        
        //Convert the raw hex values to integer according to the sensor's documentation.     
        sensorValue = 256*buf[3] + buf[2] ;
        //Assign pix_data variable the value of the acquired temperature reading. This is needed for the occupancy algorithm.
        lastReading = sensorValue;
        //Call the judge occupancy function which will return the occupancy state.
        judge_Occupancy();
        //Print to UART the reading of the sensor in Degrees Celsius.
        printf("\nTemp: %d.%02d", (sensorValue/10), abs(sensorValue)%100);
        //Print the occupancy state.
        printf("\nOccupancy: %u     ",(resultOccupancy));

    }
}
/**
    End of File
*/