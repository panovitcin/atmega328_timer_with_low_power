const byte SENSOR_0 = A0; 
const byte SENSOR_1 = A1; 
const byte BUTTON = 2;

const byte SYNHRO = 7;
const byte LATCH = 6;
const byte DATA = 5;

const byte ZERO = 11;
const byte FIRST = 10;
const byte SECOND = 9;
const byte THIRD = 8;


byte all_off = 0b00000000; 
byte figures[] = {0b11111100, 0b01100000, 0b11011010, 0b11110010, 0b01100110, 0b10110110, 0b10111110, 0b11100000, 0b11111110, 0b11110110};      // байт числа 0-9 для сегментного идикатора
byte figures_DP[] = {0b11111101, 0b01100001, 0b11011011, 0b11110011, 0b01100111, 0b10110111, 0b10111111, 0b11100001, 0b11111111, 0b11110111};   // байт числа 0-9 + точка(dp) для сегментного идикатора 


unsigned long CURRENT;

long TIME_1 = 104;              // время работы одного разряда сегментного 4-хразрядного индикатора (динамическая индикация)
unsigned long PREVIOUS_1;       

long TIME_2 = 7000;             // должна быть 0.01 секунда
unsigned long PREVIOUS_2;

long zero_digit;              // 4 переменные, привязанные к каждому разряду индикатора; равны числам, которые будут выводится на соответсвующий им разряд
long first_digit;
long second_digit;
long third_digit;             

int n;                        // секунды с точностью до сотых (то есть максимальное значение до 5999)
int minute;                   // минуты (от 0 до 100, далее сброс минут в 0)

byte k;                       // номер разряда индикатора (0-3)

boolean val_0;                // переменная первого датчика (секундомер измеряет время между срабатыванием датчиков)
boolean val_1;                // переменная второго датчика
boolean button;               // переменная для кнопки

int state;

volatile long counter;        // переменная для отсчета времени, остающегося до ухода в сон (Power-down)
long beforeSleep = 4000;      // время до ухода в сон (просто отсчёт количества тактов)

// THE BEGINNING OF THE CODE

 void myFunction()            // функция обработчика прерывания
 {
  counter = 0;                // обнуляет время до ухода в сон
 }

void setup()
{ 
  pinMode(0, OUTPUT);         // все свободные пины на выход для повышения энергосбережения
  pinMode(1, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);

  
  pinMode(SYNHRO, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(ZERO, OUTPUT);
  pinMode(FIRST, OUTPUT);
  pinMode(SECOND, OUTPUT);
  pinMode(THIRD, OUTPUT);
  pinMode(SENSOR_1, INPUT);
  pinMode(SENSOR_0, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(2), myFunction, FALLING);  // объявление прерывания
}




void sleepMode()              // функция, заладающая параметры ухода в сон
{
  ADCSRA &= ~(1<<7);

 SMCR|=(1<<2);
 SMCR|=1;

 MCUCR |= (3<<5);
 MCUCR = (MCUCR & ~(1<<5)) | (1<<6);
 __asm__ __volatile__ ("sleep");
}


void shift (byte v)           // функция занимается записью байта "v" информации в сдвиговый регистр
{
 for (int i=0; i<8; i++)
 {
  digitalWrite(SYNHRO, LOW);
  digitalWrite(DATA, v & (1<<i));
  digitalWrite(SYNHRO, HIGH);
 }
 digitalWrite(LATCH, HIGH);
 digitalWrite(LATCH, LOW);
}

void set_digits (byte k, long n)  // функция, выполненная на основе case: k равен 0-3, что соотвествует каждому из 4-х разрядов индикатора. РЕЖИМ РАБОТЫ СЕКУНДОМЕРА: в течение первой минуты выводятся на индикатор секунды + сотые и десятые. Затем: минуты и секунды.
{
  switch(k)
  {
    case 0:                       // первый разряд 
    shift(all_off);               // такой процесс записи числа в разряд индикатора позволяет избавиться от паразитной засветки 
    digitalWrite(FIRST, LOW);
    digitalWrite(SECOND, LOW);
    digitalWrite(THIRD, LOW);
    digitalWrite(ZERO, HIGH);
    if (minute<1)                 //выбираем, что записывать. Либо минуты, если их больше одной на данный момент, либо секунды, если идут первые 60 секунд.
    {
      zero_digit = n/1000;        // n - счетчик секунд до 5999, достаем старший разряд числа сенкунд
    }
    else
    {
      zero_digit = minute/10;     // минуты, достаем старший разряд числа минут 
    }
    shift(figures[zero_digit]);   // записываем полученное число с помощью функции shift
   
    break;
  

    case 1:                       // второй разряд; все данные кейсы аналогичны друг другу по способу записи.
    shift(all_off);
    digitalWrite(ZERO, LOW);
    digitalWrite(SECOND, LOW);
    digitalWrite(THIRD, LOW);
    digitalWrite(FIRST, HIGH);
    if (minute<1)
    {
      first_digit = (n-(long)(n/1000)*1000)/100;
      shift(figures_DP[first_digit]); 
    }
    else
    {
      first_digit = minute - ((minute/10)*10);
      shift(figures_DP[first_digit]); 
    }
    
    break;
  
   
    case 2:                      // третий разряд
    shift(all_off); 
    digitalWrite(ZERO, LOW);
    digitalWrite(FIRST, LOW);
    digitalWrite(THIRD, LOW);  
    digitalWrite(SECOND, HIGH);
    if (minute<1)
    {
      second_digit = ((n-(long)(n/1000)*1000)-(long)((n-(long)(n/1000)*1000)/100)*100)/10;
    }
    else
    {
      second_digit = n/1000;
    }
    shift(figures[second_digit]);
    
    break; 
  
   
    case 3:                        // четвертый разряд
    shift(all_off);
    digitalWrite(ZERO, LOW);
    digitalWrite(FIRST, LOW);
    digitalWrite(SECOND, LOW);
    digitalWrite(THIRD, HIGH);
    if (minute<1)
    {
      third_digit = ((n-(long)(n/1000)*1000)-(long)((n-(long)(n/1000)*1000)/100)*100) - (long)(((n-(long)(n/1000)*1000)-(long)((n-(long)(n/1000)*1000)/100)*100)/10)*10;
    }
    else
    {
     third_digit = (n-(long)(n/1000)*1000)/100;
    }  
    shift(figures[third_digit]);

    break; 
  }
}

void loop()
{
  val_1 = digitalRead(SENSOR_1);     // читаем датчики
  val_0 = digitalRead(SENSOR_0);
  button = digitalRead(BUTTON);     // читаем кнопку
  CURRENT = micros();

  if (val_0 == 1 && state == 0)      // примитивное меню управления секундомером: можно сбросить по кнопке. Конечный автомат.
  {
    state = 1;
  }

  if (val_1 == 1 && state == 1)
  {
    state = 2;
  }

  if (button == 1 && state == 1)         // возможность обнулить время с помощью нажатия кнопки и вернуться в состояние 0 
  {
    state = 0;
  }

  if (button == 1 && state == 2)
  {
    state = 0;
  }

  
  if (CURRENT - PREVIOUS_1 >= TIME_1)   // функция для динамической индикации, записываем числа в разряды индикатора поочередно
  {
    PREVIOUS_1 = CURRENT;
    set_digits(k,n);
    k++;
    if (k==4)
    {
      k=0;
    }
  }

  switch (state)  // Свитч, переменной которого является текущее состояние Конечного автомата
  {
    case 0:   // ожидание работы, секундомер выводит 00:00
    n=0;
    minute=0;

     counter++;
     if (counter >= beforeSleep)  // если ничего не происходит, то уходим в сон
     {
      digitalWrite(THIRD, LOW);
      digitalWrite(ZERO, LOW);
      digitalWrite(FIRST, LOW);
      digitalWrite(SECOND, LOW);
      digitalWrite(SYNHRO, LOW);
      digitalWrite(LATCH, LOW);
      digitalWrite(DATA, LOW);
      sleepMode();
     }
  
    break;

  

    case 1:                             // активная работа секундомера
  if (CURRENT - PREVIOUS_2 >= TIME_2)   // пытаемся отсчитывать секунды и минуты
  {
    PREVIOUS_2 = CURRENT;
    n++;
    if (n>5999)
    {
      n=0;
      minute++;
    }
  }

  if (minute == 100)   // обнуление счетчика минут 
  {
    minute = 0;
  }
    break;

    
    case 2:             // остановка секундомера при срабаьывании датчика. Выводит время, котрое было замерено. 

     counter++;   
     if (counter >= beforeSleep) // Через некотрое время уходит в сон
     {
      digitalWrite(THIRD, LOW);
      digitalWrite(ZERO, LOW);
      digitalWrite(FIRST, LOW);
      digitalWrite(SECOND, LOW);
      digitalWrite(SYNHRO, LOW);
      digitalWrite(LATCH, LOW);
      digitalWrite(DATA, LOW);
      sleepMode();
     }
     
    break; 
  }
}
