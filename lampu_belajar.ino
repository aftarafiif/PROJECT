int A = 11;
int B = 10;
int C = 8;
int D = 12;
int E = 9;

void setup() {
pinMode(A, OUTPUT);
pinMode(B, OUTPUT);
pinMode(C, OUTPUT);
pinMode(D, OUTPUT);
pinMode(E, OUTPUT);
}

void lampumati() {
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);
  digitalWrite(E, LOW);
}

void loop() {

  lampumati();
  digitalWrite(A, HIGH);
  delay(500);

  lampumati();
  digitalWrite(B, HIGH);
  delay(500);
  
  lampumati();
  digitalWrite(C, HIGH);
  delay(500);

  lampumati();
  digitalWrite(D, HIGH);
  delay(500);

  lampumati();
  digitalWrite(E, HIGH);
  delay(500);

  lampumati();
  digitalWrite(D, HIGH);
  delay(500);

  lampumati();
  digitalWrite(C, HIGH);
  delay(500);
  
  lampumati();
  digitalWrite(B, HIGH);
  delay(500);
}
