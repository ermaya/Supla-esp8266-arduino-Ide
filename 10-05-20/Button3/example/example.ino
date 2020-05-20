#include <Button3.h>
#define button_PIN 0 
Button3 button = Button3(button_PIN);

void setup() {
  // put your setup code here, to run once:
  button.setClickHandler(click);
  button.setLongClickHandler(longClick);
  button.setDoubleClickHandler(doubleClick);
  button.setTripleClickHandler(tripleClick);
  button.setQuatleClickHandler(quatleClick);
  button.setQuintleClickHandler(quintleClick);  
  button.setConfigClickHandler(configClick); // 10 x Click
}

void loop() {
  // put your main code here, to run repeatedly:
  
    button.loop();  
}
//---------------------------------------
void click(Button3& btn) {
  Serial.println("Click");
}
void doubleClick(Button3& btn) {
  Serial.println("doubleClick");
}
void tripleClick(Button3& btn) {
  Serial.println("tripleClick");
}
void quatleClick(Button3& btn) {
  Serial.println("4 Click");
}
void quintleClick(Button3& btn) {
  Serial.println("5 Click");
}
void longClick(Button3& btn) {
  Serial.println("longClick");
}
void configClick(Button3& btn) {
  Serial.println("ConfigClick 10x");
}
