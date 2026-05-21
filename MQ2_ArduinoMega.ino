#define MQ2_PIN A0
#define LDR_PIN A1

void setup() {

  Serial.begin(9600);

  pinMode(MQ2_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  Serial.println("Environment Monitoring System Started");
}

void loop() {

  int gasValue = analogRead(MQ2_PIN);
  int lightValue = analogRead(LDR_PIN);

  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  Serial.print("Light Value: ");
  Serial.println(lightValue);

  if(gasValue > 400) {

    Serial.println("Gas Leakage Detected!");
  }

  if(lightValue < 300) {

    Serial.println("Dark Environment Detected");
  }
  else {

    Serial.println("Bright Light Detected");
  }

  Serial.println("-------------------");

  delay(1000);
}
