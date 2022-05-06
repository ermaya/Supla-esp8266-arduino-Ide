#define supla_lib_config_h_  // silences debug messages
#include <Wire.h>
#include <SuplaDevice.h>
#include <supla_Bone_io.h>

//
//#include <supla/network/wt32_eth01.h>
//Supla::WT32_ETH01 Eth(1);
//
#include <supla/network/esp_wifi.h>
Supla::ESPWifi wifi("your_wifi_ssid", "your_wifi_password");
//

void setup() {

  Serial.begin(115200);

  Wire.begin(14, 15);  // Wire.begin(sda, scl);
  Wire.setClock(400000);
  
  if (! mcp1.begin(0))Serial.println("MCP23017 1 not found!"); // begin(uint8_t address) 
  if (! mcp2.begin(1))Serial.println("MCP23017 2 not found!"); // begin(uint8_t address) 
  if (! mcp3.begin(2))Serial.println("MCP23017 3 not found!"); // begin(uint8_t address) 
  if (! mcp4.begin(3))Serial.println("MCP23017 4 not found!"); // begin(uint8_t address) 

  // Replace the falowing GUID with value that you can retrieve from https://www.supla.org/arduino/get-guid
  char GUID[SUPLA_GUID_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  // Replace the following AUTHKEY with value that you can retrieve from: https://www.supla.org/arduino/get-authkey
  char AUTHKEY[SUPLA_AUTHKEY_SIZE] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 
  /*
     the outputs are defined as: "Bone_out_1" to "Bone_out_32"
     and correspond to the numbers of the "BoneIO" outputs.
     So the BoneIO rele_1 with output active at high level is:
     Supla::Control::Relay(Bone_out_1, true);
  */
    
    auto relay_1 = new Supla::Control::Relay(Bone_out_1, true);
    auto relay_2 = new Supla::Control::Relay(Bone_out_2, true);
    auto relay_3 = new Supla::Control::Relay(Bone_out_3, true);
    auto relay_4 = new Supla::Control::Relay(Bone_out_4, true);
    auto relay_5 = new Supla::Control::Relay(Bone_out_5, true);
    auto relay_6 = new Supla::Control::Relay(Bone_out_6, true);
    auto relay_7 = new Supla::Control::Relay(Bone_out_7, true);
    auto relay_8 = new Supla::Control::Relay(Bone_out_8, true);
    auto relay_9 = new Supla::Control::Relay(Bone_out_9, true);
    auto relay_10 = new Supla::Control::Relay(Bone_out_10, true);
    auto relay_11 = new Supla::Control::Relay(Bone_out_11, true);
    auto relay_12 = new Supla::Control::Relay(Bone_out_12, true);
    auto relay_13 = new Supla::Control::Relay(Bone_out_13, true);
    auto relay_14 = new Supla::Control::Relay(Bone_out_14, true);
    auto relay_15 = new Supla::Control::Relay(Bone_out_15, true);
    auto relay_16 = new Supla::Control::Relay(Bone_out_16, true);

    auto relay_17 = new Supla::Control::Relay(Bone_out_17, true);
    auto relay_18 = new Supla::Control::Relay(Bone_out_18, true);
    auto relay_19 = new Supla::Control::Relay(Bone_out_19, true);
    auto relay_20 = new Supla::Control::Relay(Bone_out_20, true);
    auto relay_21 = new Supla::Control::Relay(Bone_out_21, true);
    auto relay_22 = new Supla::Control::Relay(Bone_out_22, true);
    auto relay_23 = new Supla::Control::Relay(Bone_out_23, true);
    auto relay_24 = new Supla::Control::Relay(Bone_out_24, true);
    auto relay_25 = new Supla::Control::Relay(Bone_out_25, true);
    auto relay_26 = new Supla::Control::Relay(Bone_out_26, true);
    auto relay_27 = new Supla::Control::Relay(Bone_out_27, true);
    auto relay_28 = new Supla::Control::Relay(Bone_out_28, true);
    auto relay_29 = new Supla::Control::Relay(Bone_out_29, true);
    auto relay_30 = new Supla::Control::Relay(Bone_out_30, true);
    auto relay_31 = new Supla::Control::Relay(Bone_out_31, true);
    auto relay_32 = new Supla::Control::Relay(Bone_out_32, true);


  /*
     the inputs are defined as: "BBone_in_1" to "Bone_in_32"
     and correspond to the numbers of the "BoneIO" inputs.
  */

    auto button_1 = new Supla::Control::Button(Bone_in_1, true, true); 
    auto button_2 = new Supla::Control::Button(Bone_in_2, true, true); 
    auto button_3 = new Supla::Control::Button(Bone_in_3, true, true); 
    auto button_4 = new Supla::Control::Button(Bone_in_4, true, true); 
    auto button_5 = new Supla::Control::Button(Bone_in_5, true, true); 
    auto button_6 = new Supla::Control::Button(Bone_in_6, true, true); 
    auto button_7 = new Supla::Control::Button(Bone_in_7, true, true); 
    auto button_8 = new Supla::Control::Button(Bone_in_8, true, true);
    auto button_9 = new Supla::Control::Button(Bone_in_9, true, true); 
    auto button_10 = new Supla::Control::Button(Bone_in_10, true, true); 
    auto button_11 = new Supla::Control::Button(Bone_in_11, true, true); 
    auto button_12 = new Supla::Control::Button(Bone_in_12, true, true); 
    auto button_13 = new Supla::Control::Button(Bone_in_13, true, true); 
    auto button_14 = new Supla::Control::Button(Bone_in_14, true, true); 
    auto button_15 = new Supla::Control::Button(Bone_in_15, true, true); 
    auto button_16 = new Supla::Control::Button(Bone_in_16, true, true); 

    auto button_17 = new Supla::Control::Button(Bone_in_17, true, true); 
    auto button_18 = new Supla::Control::Button(Bone_in_18, true, true); 
    auto button_19 = new Supla::Control::Button(Bone_in_19, true, true); 
    auto button_20 = new Supla::Control::Button(Bone_in_20, true, true); 
    auto button_21 = new Supla::Control::Button(Bone_in_21, true, true); 
    auto button_22 = new Supla::Control::Button(Bone_in_22, true, true); 
    auto button_23 = new Supla::Control::Button(Bone_in_23, true, true); 
    auto button_24 = new Supla::Control::Button(Bone_in_24, true, true);
    auto button_25 = new Supla::Control::Button(Bone_in_25, true, true); 
    auto button_26 = new Supla::Control::Button(Bone_in_26, true, true); 
    auto button_27 = new Supla::Control::Button(Bone_in_27, true, true); 
    auto button_28 = new Supla::Control::Button(Bone_in_28, true, true); 
    auto button_29 = new Supla::Control::Button(Bone_in_29, true, true); 
    auto button_30 = new Supla::Control::Button(Bone_in_30, true, true); 
    auto button_31 = new Supla::Control::Button(Bone_in_31, true, true); 
    auto button_32 = new Supla::Control::Button(Bone_in_32, true, true);
      
    button_1->addAction(Supla::TOGGLE, relay_1, Supla::ON_PRESS);
    button_1->setSwNoiseFilterDelay(50);
    button_2->addAction(Supla::TOGGLE, relay_2, Supla::ON_PRESS);
    button_2->setSwNoiseFilterDelay(50);
    button_3->addAction(Supla::TOGGLE, relay_3, Supla::ON_PRESS);
    button_3->setSwNoiseFilterDelay(50);
    button_4->addAction(Supla::TOGGLE, relay_4, Supla::ON_PRESS);
    button_4->setSwNoiseFilterDelay(50);
    button_5->addAction(Supla::TOGGLE, relay_5, Supla::ON_PRESS);
    button_5->setSwNoiseFilterDelay(50);
    button_6->addAction(Supla::TOGGLE, relay_6, Supla::ON_PRESS);
    button_6->setSwNoiseFilterDelay(50);
    button_7->addAction(Supla::TOGGLE, relay_7, Supla::ON_PRESS);
    button_7->setSwNoiseFilterDelay(50);
    button_8->addAction(Supla::TOGGLE, relay_8, Supla::ON_PRESS);
    button_8->setSwNoiseFilterDelay(50);
    button_9->addAction(Supla::TOGGLE, relay_9, Supla::ON_PRESS);
    button_9->setSwNoiseFilterDelay(50);
    button_10->addAction(Supla::TOGGLE, relay_10, Supla::ON_PRESS);
    button_10->setSwNoiseFilterDelay(50);
    button_11->addAction(Supla::TOGGLE, relay_11, Supla::ON_PRESS);
    button_11->setSwNoiseFilterDelay(50);
    button_12->addAction(Supla::TOGGLE, relay_12, Supla::ON_PRESS);
    button_12->setSwNoiseFilterDelay(50);
    button_13->addAction(Supla::TOGGLE, relay_13, Supla::ON_PRESS);
    button_13->setSwNoiseFilterDelay(50);
    button_14->addAction(Supla::TOGGLE, relay_14, Supla::ON_PRESS);
    button_14->setSwNoiseFilterDelay(50);
    button_15->addAction(Supla::TOGGLE, relay_15, Supla::ON_PRESS);
    button_15->setSwNoiseFilterDelay(50);
    button_16->addAction(Supla::TOGGLE, relay_16, Supla::ON_PRESS);
    button_16->setSwNoiseFilterDelay(50);

    button_17->addAction(Supla::TOGGLE, relay_17, Supla::ON_PRESS);
    button_17->setSwNoiseFilterDelay(50);
    button_18->addAction(Supla::TOGGLE, relay_18, Supla::ON_PRESS);
    button_18->setSwNoiseFilterDelay(50);
    button_19->addAction(Supla::TOGGLE, relay_19, Supla::ON_PRESS);
    button_19->setSwNoiseFilterDelay(50);
    button_20->addAction(Supla::TOGGLE, relay_20, Supla::ON_PRESS);
    button_20->setSwNoiseFilterDelay(50);
    button_21->addAction(Supla::TOGGLE, relay_21, Supla::ON_PRESS);
    button_21->setSwNoiseFilterDelay(50);
    button_22->addAction(Supla::TOGGLE, relay_22, Supla::ON_PRESS);
    button_22->setSwNoiseFilterDelay(50);
    button_23->addAction(Supla::TOGGLE, relay_23, Supla::ON_PRESS);
    button_23->setSwNoiseFilterDelay(50);
    button_24->addAction(Supla::TOGGLE, relay_24, Supla::ON_PRESS);
    button_24->setSwNoiseFilterDelay(50);
    button_25->addAction(Supla::TOGGLE, relay_25, Supla::ON_PRESS);
    button_25->setSwNoiseFilterDelay(50);
    button_26->addAction(Supla::TOGGLE, relay_26, Supla::ON_PRESS);
    button_26->setSwNoiseFilterDelay(50);
    button_27->addAction(Supla::TOGGLE, relay_27, Supla::ON_PRESS);
    button_27->setSwNoiseFilterDelay(50);
    button_28->addAction(Supla::TOGGLE, relay_28, Supla::ON_PRESS);
    button_28->setSwNoiseFilterDelay(50);
    button_29->addAction(Supla::TOGGLE, relay_29, Supla::ON_PRESS);
    button_29->setSwNoiseFilterDelay(50);
    button_30->addAction(Supla::TOGGLE, relay_30, Supla::ON_PRESS);
    button_30->setSwNoiseFilterDelay(50);
    button_31->addAction(Supla::TOGGLE, relay_31, Supla::ON_PRESS);
    button_31->setSwNoiseFilterDelay(50);
    button_32->addAction(Supla::TOGGLE, relay_32, Supla::ON_PRESS);
    button_32->setSwNoiseFilterDelay(50);
 
  /*
   * SuplaDevice Initialization.
   * Server address is available at https://cloud.supla.org 
   * If you do not have an account, you can create it at https://cloud.supla.org/account/create
   * SUPLA and SUPLA CLOUD are free of charge
   * 
   */
 
  SuplaDevice.begin(GUID,              // Global Unique Identifier 
                    "svr1.supla.org",  // SUPLA server address
                    "email@address",   // Email address used to login to Supla Cloud
                    AUTHKEY);          // Authorization key
    
}

void loop() {
  SuplaDevice.iterate();
  delay(25);
}
