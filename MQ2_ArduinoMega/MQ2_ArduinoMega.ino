#define MQ2_PIN A0

void setup() {

  Serial.begin(9600);

  pinMode(MQ2_PIN, INPUT);

  Serial.println("MQ-2 Gas Detector Started");
}

void loop() {

  int gasValue = analogRead(MQ2_PIN);

  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  if(gasValue > 400) {

    Serial.println("Gas Leakage Detected!");
  }

  delay(1000);
}
