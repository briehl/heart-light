/* HeartLight
 * ----------
 * I don't have much else to give, so I'm giving her a piece of my heart.
 * 
 * Not literally. Ew. Also, ow.
 * 
 * But I can cast a piece of resin that pulses and beats in rhythm with my heart, and 
 * build a little box around it. This drives an Arduino microcontroller (I'm using 
 * an Elegoo Nano, which is a knockoff of the Arduino Nano with a different USB driver).
 * It pulses two pins - one for "systole" and one for "diastole" - some number of times
 * per minute.
 * 
 * It has a model for heart beat over time, based on a bunch of data I've collected from my 
 * Fitbit. It's pretty simple, though - just a curvefit (yay for SciPy) over a month of averaged
 * heart rate data, fit against a 5th order model. 
 * 
 * That is, f(x) = ax + bx^2 + cx^3 + dx^4 + ex^5 + f
 * f(x) = heart rate at time x where x in range [0, 86400]. So it loops back around after 24 hours.
 * It's reasonably flat. I mean, the averaged range is 60->75. But it's a little piece of me.
 * 
 * Maybe, before I'm done, I'll attach a proximity sensor to it, and have it rise as a
 * function of that signal. That'll be fun.
 */

int systolePin = 3;
int diastolePin = 5;
int defaultRate = 30;
long startTime = 9000;  // start at 20K seconds. loop around at 86400.
long lastMillis = 0;
double a = -1.40006006e-03;
double b = 1.14829493e-07;
double c = -3.27110291e-12;
double d = 4.00145962e-17;
double e = -1.78367819e-22;
double f = 7.03160970e+01;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
}

void loop() {  
  beat();
}

void singleBeat(float riseSpan, float fallSpan, int pin) {
  int pinDelay = 1;
  for (float i=0.0; i<riseSpan; i++) {
    int val = int(255.0 * i / riseSpan);
    analogWrite(pin, val);
    delay(pinDelay);
  }
  analogWrite(pin, 255);
  delay(pinDelay);
  for (float i=0; i<fallSpan; i++) {
    analogWrite(pin, 255 - int(255.0 * i / fallSpan));
    delay(pinDelay);
  }
  analogWrite(pin, 0);
}

void systoleBeat(int rate) {
  int rise = int(rate*0.85);
  int fall = int(rate*0.15);
  
//  Serial.println("systole");
  singleBeat(rise, fall, systolePin);
  // slow rise, quick drop
  // 75% rise, 25% drop?
}

void diastoleBeat(int rate) {
  // quickish rise, slow drop
  singleBeat(int(rate*0.3), int(rate*0.7), diastolePin);
}

float getHeartRate() {
  long x = (startTime + long(millis()/1000)) % 86400;
  
  double rate = (a*x) + (b*x*x) + (c*x*x*x) + (d*x*x*x*x) + (e*x*x*x*x*x) + f;
  Serial.print("time: ");
  Serial.println(x);
  Serial.print("rate: ");
  Serial.println(rate);
  if (rate <= 0) {
    rate = defaultRate;
  }
  return float(rate);
}

void beat() {
  float bpm = getHeartRate();
  float mspb = 60000.0 / bpm;
  float systoleTime = mspb * 0.25;
  float diastoleTime = mspb * 0.5;
  int restTime = int(mspb * 0.2);
  
  systoleBeat(systoleTime);
  diastoleBeat(diastoleTime);
  delay(restTime);
}
