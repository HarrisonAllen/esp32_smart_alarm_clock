const int NOODS_PIN = 19;    // RED pin of the LED to PWM pin 4
const int NOODS_UPDATE_RATE = 20;
int brightness = 0; // LED brightness
int increment = 5;  // brightness increment
long timer;

void setup()
{
  pinMode(NOODS_PIN, OUTPUT);
  Serial.begin(115200);

  timer = millis();
}

void loop()
{
  if ((millis() - timer > NOODS_UPDATE_RATE)) {
    brightness = brightness + increment;  // increment brightness for next loop iteration

    if (brightness <= 0 || brightness >= 255)    // reverse the direction of the fading
    {
      increment = -increment;
    }
    brightness = constrain(brightness, 0, 255);
    analogWrite(NOODS_PIN, brightness);
    timer = millis();
  }
}