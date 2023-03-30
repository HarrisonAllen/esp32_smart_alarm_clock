const int BUTTON_PIN = 32;
const int BUTTON_SCAN_RATE = 20;
long timer;

void setup()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.begin(115200);

  timer = millis();
}

void loop()
{
  timer = millis();
  if (!digitalRead(BUTTON_PIN))
  {
    Serial.println("1");
  } else {
    Serial.println("0");
  }
  while (millis() - timer < BUTTON_SCAN_RATE);
}