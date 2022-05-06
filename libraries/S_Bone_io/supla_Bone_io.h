
#ifndef S_mcp_Bone_h
#define S_mcp_Bone_h

#include <Arduino.h>
#include <S_MCP23017.h>
#include <supla/control/button.h>
#include <supla/control/relay.h>
#include <supla/io.h>

// MCP 1 + 2
#define Bone_in_1 100
#define Bone_in_2 101
#define Bone_in_3 102
#define Bone_in_4 103
#define Bone_in_5 104
#define Bone_in_6 105
#define Bone_in_7 106
#define Bone_in_8 107
#define Bone_in_9 108
#define Bone_in_10 109
#define Bone_in_11 110
#define Bone_in_12 111
#define Bone_in_13 112
#define Bone_in_14 113
#define Bone_in_15 114
#define Bone_in_16 115
#define Bone_in_17 123
#define Bone_in_18 122
#define Bone_in_19 121
#define Bone_in_20 120
#define Bone_in_21 119
#define Bone_in_22 118
#define Bone_in_23 117
#define Bone_in_24 116
#define Bone_in_25 124
#define Bone_in_26 125
#define Bone_in_27 126
#define Bone_in_28 127
#define Bone_in_29 128
#define Bone_in_30 129
#define Bone_in_31 130
#define Bone_in_32 131
// MCP 3 + 4
#define Bone_out_1 147
#define Bone_out_2 146
#define Bone_out_3 145
#define Bone_out_4 144
#define Bone_out_5 143
#define Bone_out_6 142
#define Bone_out_7 141
#define Bone_out_8 140
#define Bone_out_9 163
#define Bone_out_10 162
#define Bone_out_11 161
#define Bone_out_12 160
#define Bone_out_13 159
#define Bone_out_14 158
#define Bone_out_15 157
#define Bone_out_16 156
#define Bone_out_17 132
#define Bone_out_18 133
#define Bone_out_19 134
#define Bone_out_20 135
#define Bone_out_21 136
#define Bone_out_22 137
#define Bone_out_23 138
#define Bone_out_24 139
#define Bone_out_25 148
#define Bone_out_26 149
#define Bone_out_27 150
#define Bone_out_28 151
#define Bone_out_29 152
#define Bone_out_30 153
#define Bone_out_31 154
#define Bone_out_32 155


	MCP23017 mcp1;
    MCP23017 mcp2;
    MCP23017 mcp3;
    MCP23017 mcp4;


class CustomControl : public Supla::Io {
  public:
    void customDigitalWrite(int channelNumber, uint8_t pin, uint8_t val) {     
      if ((pin > 99) && (pin < 116)){
        mcp1.digitalWrite(pin - 100, val);
         return;
      }
      else if ((pin > 115) && (pin < 132)){
        mcp2.digitalWrite(pin - 116, val);
      }
      else if ((pin > 131) && (pin < 148)){
        mcp3.digitalWrite(pin - 132, val);
         return;
      }
      else if ((pin > 147) && (pin < 164)){
        mcp4.digitalWrite(pin - 148, val);
         return;
      }
	  return ::digitalWrite(pin,val);   
   }   
   int customDigitalRead(int channelNumber, uint8_t pin) {
      if ((pin > 99) && (pin < 116)){
        return mcp1.digitalRead(pin - 100);    
      }
      else if ((pin > 115) && (pin < 132)){
        return mcp2.digitalRead(pin - 116);
      }
      else if ((pin > 131) && (pin < 148)){
        return mcp3.digitalRead(pin - 132);    
      }
      else if ((pin > 147) && (pin < 164)){
        return mcp4.digitalRead(pin - 148);    
      } 
	  return ::digitalRead(pin);      
    }
   void customPinMode(int channelNumber, uint8_t pin, uint8_t mode) {
      (void)(channelNumber);     
      if ((pin > 99) && (pin < 116)){
        mcp1.pinMode(pin - 100, mode);
		return;
      }
      else if ((pin > 115) && (pin < 132)){
        mcp2.pinMode(pin - 116, mode);
		return;
      }
      else if ((pin > 131) && (pin < 148)){
        mcp3.pinMode(pin - 132, mode);
		return;
      }
      else if ((pin > 147) && (pin < 164)){
        mcp4.pinMode(pin - 148, mode);
		return;
      } 
	  return ::pinMode(pin, mode);      
    } 
	      
}CustomControl; 


#endif

