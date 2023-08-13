
const int TIME = 3500;

void setup() 
{
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT);
}

void loop() 
{
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(2, HIGH);
  delay(TIME);                      
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(2, LOW);
  delay(TIME);                      
}