const int RED_PIN = 18;    // RED pin of the LED to PWM pin 4
const int GREEN_PIN = 17 ;  // GREEN pin of the LED to PWM pin 5
const int BLUE_PIN = 16;   // BLUE pin of the LED to PWM pin 6
const int LED_UPDATE_RATE = 20;
int brightness = 0; // LED brightness
int increment = 5;  // brightness increment
long timer;

void setup()
{
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  Serial.begin(115200);

  timer = millis();
}

void loop()
{
  if ((millis() - timer > LED_UPDATE_RATE)) {
    brightness = brightness + increment;  // increment brightness for next loop iteration

    if (brightness <= 0 || brightness >= 255)    // reverse the direction of the fading
    {
      increment = -increment;
    }
    brightness = constrain(brightness, 0, 255);
    analogWrite(RED_PIN, brightness);
    analogWrite(GREEN_PIN, brightness);
    analogWrite(BLUE_PIN, brightness);
    timer = millis();
  }
}