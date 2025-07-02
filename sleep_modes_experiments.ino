const int LED = 7;
const int BUTTON = 2;


long spacing = 1000;
volatile int counter;

unsigned long CURRENT;
unsigned long PREVIOUS;

boolean flag;
boolean currentButton;
boolean lastButton;


 void myFunction()
 {
  counter = 0; 
 }


void setup() 
{
  
  for (int i=0; i<20; i++)
  {
    if (i !=2)
  pinMode(i, OUTPUT);
  }
  pinMode(BUTTON, INPUT);
  
attachInterrupt(digitalPinToInterrupt(2), myFunction, FALLING);

}

void sleepMode()
{
  ADCSRA &= ~(1<<7);

 SMCR|=(1<<2);
 SMCR|=1;

 MCUCR |= (3<<5);
 MCUCR = (MCUCR & ~(1<<5)) | (1<<6);
 __asm__ __volatile__ ("sleep");
}


void loop() 
{
  CURRENT = millis();
  if (CURRENT - PREVIOUS >= spacing)
  {
    PREVIOUS = CURRENT;
    counter++;
  }

  currentButton = digitalRead(BUTTON);
  if (currentButton == HIGH && lastButton == LOW)
  {
   flag=!flag;
   digitalWrite(LED, flag);
  }
  lastButton = currentButton;
  
  if (counter>= 15)
  {
    digitalWrite(LED, LOW);
    sleepMode();
  }
}
