#include <EEPROM.h>
 
#define BUFFERSIZE 80
#define TRUE 1
#define FALSE 0

#define REDPIN 5
#define GREENPIN 6
#define BLUEPIN 7

#define SEQUENCETRIGGER 11  
#define SEQUENCEFINISHED 4

long RedDelay = 10000;
long GreenDelay = 15000;
long BlueDelay = 20000;

long RedOpen = 10000;
long GreenOpen = 15000;
long BlueOpen = 20000;

long RedOpenTime;
long GreenOpenTime;
long BlueOpenTime;

long RedCloseTime;
long GreenCloseTime;
long BlueCloseTime;

long ExposureStart = 0;

int RedAddress = 10;
int GreenAddress = 20;
int BlueAddress = 30;

int TriggerHistory = HIGH;
int TriggerStatus;

char COMMANDEND = 0x0D;

char CommandBuffer[BUFFERSIZE + 1];
int BufferUsed= 0;

void setup() 
{
  Serial.begin(9800);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(SEQUENCETRIGGER, INPUT_PULLUP);
  pinMode(SEQUENCEFINISHED, OUTPUT);
  
  
  digitalWrite(REDPIN, LOW);
  digitalWrite(GREENPIN, LOW);
  digitalWrite(BLUEPIN, LOW);
  digitalWrite(SEQUENCEFINISHED, HIGH);

  EEPROM_read(RedAddress,&RedDelay); 
  EEPROM_read(RedAddress+5,&RedOpen);  
  
  EEPROM_read(GreenAddress,&GreenDelay); 
  EEPROM_read(GreenAddress+5,&GreenOpen);  
  
  EEPROM_read(BlueAddress,&BlueDelay); 
  EEPROM_read(BlueAddress+5,&BlueOpen);  
  
  //ExposureInit();
}

void loop() 
{
  static char Response[40];
  
  while (Serial.available())
  {
    if(BufferUsed < BUFFERSIZE)
    {
      Serial.readBytes(&CommandBuffer[BufferUsed], 1);
      Serial.print(CommandBuffer[BufferUsed]);
      
      if (CommandBuffer[BufferUsed] == '\r' )
      {
        CommandBuffer[BufferUsed] = '\0';
        if(strstr(CommandBuffer,"go"))
        {
          ExposureInit();
        }
        else if(strstr(CommandBuffer,"report"))
        {
          Report();
        }
        else if(strstr(CommandBuffer,"red delay"))
        {
          ChangeRedDelay(&CommandBuffer[9]);
          Report();
        }
        else if(strstr(CommandBuffer,"green delay"))
        {
          ChangeGreenDelay(&CommandBuffer[11]);
          Report();
        }
        else if(strstr(CommandBuffer,"blue delay"))
        {
          ChangeBlueDelay(&CommandBuffer[10]);
          Report();
        }
        else if(strstr(CommandBuffer,"red time"))
        {
          ChangeRedTime(&CommandBuffer[8]);
          Report();
        }
        else if(strstr(CommandBuffer,"green time"))
        {
          ChangeGreenTime(&CommandBuffer[10]);
          Report();
        }
        else if(strstr(CommandBuffer,"blue time"))
        {
          ChangeBlueTime(&CommandBuffer[9]);
          Report();
        }
        else if(strstr(CommandBuffer,"tr"))
        {
          digitalWrite(REDPIN,!digitalRead(REDPIN));
          Serial.println("Toggle Red");
        }
        else if(strstr(CommandBuffer,"tg"))
        {
          digitalWrite(GREENPIN,!digitalRead(GREENPIN));
          Serial.println("Toggle Green");
        }
        else if(strstr(CommandBuffer,"tb"))
        {
          digitalWrite(BLUEPIN,!digitalRead(BLUEPIN));
          Serial.println("Toggle Blue");
        }
        BufferUsed = 0; 
      }
      else
      {
         BufferUsed ++;
      }
    }
    else
    {
      BufferUsed = 0;
    }  
  }
  
  TriggerStatus = digitalRead(SEQUENCETRIGGER);
  if(( TriggerStatus == LOW) && (TriggerHistory== HIGH))
  {
    Serial.println("Edge Detected");
    delay(1000);
    TriggerStatus = digitalRead(SEQUENCETRIGGER);
    if( TriggerStatus == LOW)
    {
      Serial.println("Still High");
      ExposureInit();
    }
    else
    {
      Serial.println("Was just a glitch!!");
    }
  }
  TriggerHistory = TriggerStatus;
  
  RunExposure();
  
}

void ExposureInit(void)
{
  ExposureStart = millis();
  
  digitalWrite(SEQUENCEFINISHED, LOW);
  
  RedOpenTime = ExposureStart + RedDelay;
  GreenOpenTime = ExposureStart + GreenDelay;
  BlueOpenTime = ExposureStart + BlueDelay;
  
  RedCloseTime = ExposureStart + RedDelay + RedOpen;
  GreenCloseTime = ExposureStart + GreenDelay + GreenOpen;
  BlueCloseTime = ExposureStart + BlueDelay + BlueOpen;
  
}

void RunExposure(void)
{
    long TimeNow;
    
    TimeNow = millis();
    
    if( (TimeNow > (RedCloseTime+1500)) && (TimeNow > (GreenCloseTime+1500)) && (TimeNow > (BlueCloseTime+1500)))
    {
      digitalWrite(SEQUENCEFINISHED,LOW);
      return;
    }

    if( (TimeNow > (RedCloseTime+500)) && (TimeNow > (GreenCloseTime+500)) && (TimeNow > (BlueCloseTime+500)))
    {
      digitalWrite(SEQUENCEFINISHED,HIGH);
      return;
    }
    
    Serial.print((TimeNow-ExposureStart)/1000);
    
    if( (TimeNow > RedOpenTime) && (TimeNow < RedCloseTime))
    {
      digitalWrite(REDPIN,HIGH);
      Serial.print("  Red Shutter Open  ");
    }
    else if ( (TimeNow > RedOpenTime) && (TimeNow > RedCloseTime))
    {
      digitalWrite(REDPIN, LOW);
      Serial.print("  Red Shutter Closed");
    }
    else
    {
      Serial.print("  Red Shutter Closed");
    }
    
    if( (TimeNow > GreenOpenTime) && (TimeNow < GreenCloseTime))
    {
      digitalWrite(GREENPIN,HIGH);
      Serial.print("  Green Shutter Open  ");
    }
    else if ( (TimeNow > GreenOpenTime) && (TimeNow > GreenCloseTime))
    {
      digitalWrite(GREENPIN, LOW);
      Serial.print("  Green Shutter Closed");
    }
    else
    {
      Serial.print("  Green Shutter Closed");
    }
    if( (TimeNow > BlueOpenTime) && (TimeNow < BlueCloseTime))
    {
      digitalWrite(BLUEPIN,HIGH);
      Serial.println("  Blue Shutter Open  ");
    }
    else if ( (TimeNow > BlueOpenTime) && (TimeNow > BlueCloseTime))
    {
      digitalWrite(BLUEPIN, LOW);
      Serial.println("  Blue Shutter Closed");
    }
    else
    {
      Serial.println("  Blue Shutter Closed");
    }
}

void Report(void)
{
  Serial.println("Channel !  Delay  ! Time ");
  Serial.println("-------------------------");
  Serial.print  ("Red     ! ");
  Serial.print  (RedDelay);
  Serial.print  (" ! ");
  Serial.println(RedOpen);
  Serial.print  ("Green   ! ");
  Serial.print  (GreenDelay);
  Serial.print  (" ! ");
  Serial.println(GreenOpen);
  Serial.print  ("Blue    ! ");
  Serial.print  (BlueDelay);
  Serial.print  (" ! ");
  Serial.println(BlueOpen);
}

void ChangeRedDelay(char * command)
{
  long Delay;
  
  Delay = atol(command);
  RedDelay = Delay; 
  EEPROM_write(RedAddress,RedDelay);
}

void ChangeGreenDelay(char * command)
{
  long Delay;
  
  Delay = atol(command);
  GreenDelay = Delay; 
  EEPROM_write(GreenAddress,GreenDelay);
}
void ChangeBlueDelay(char * command)
{
  long Delay;
  
  Delay = atol(command);
  BlueDelay = Delay;
  EEPROM_write(BlueAddress,BlueDelay); 
}  

void ChangeRedTime(char * command)
{
  long Delay;
  
  Delay = atol(command);
  RedOpen = Delay; 
  EEPROM_write(RedAddress+5,RedOpen);
} 

void ChangeGreenTime(char * command)
{
  long Delay;
  
  Delay = atol(command);
  GreenOpen = Delay; 
  EEPROM_write(GreenAddress+5,GreenOpen);
}

void ChangeBlueTime(char * command)
{
  long Delay;
  
  Delay = atol(command);
  BlueOpen = Delay; 
  EEPROM_write(BlueAddress+5,BlueOpen);
}  

  
int EEPROM_write(int ee, long value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(long); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

int EEPROM_read(int ee, long *value)
{
    byte* p = (byte*)(void*)value;
    unsigned int i;
    for (i = 0; i < sizeof(long); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}  


