void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  char dbuffer[200];
  int sc = 0;
  while (Serial.available() > 0) {
    dbuffer [sc] = Serial.read(); 
    sc = sc + 1;
  }
 
  for (int i=0; i < sc; i ++){
    Serial.print(dbuffer[i]);
  }
 
}

