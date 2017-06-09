/**
 * @brief Simple Echo Sketch.
 */
 
void setup()
{
  Serial1.begin(115200);   // Conected to Arduino Nano (receive/send data from/to GSM driver)
  Serial.begin(115200);   // PC debugger
}
 
void loop()
{
  while(Serial1.available())
  {
    Serial.write(Serial1.read());
  }
  /*if(Serial.available())
  {
    Serial1.write(Serial.read());
  }*/
}