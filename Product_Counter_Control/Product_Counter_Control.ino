#include <Servo.h>
#include <EEPROM.h> 

#define	SENSOR1		2
#define	SENSOR2		3
#define SERVO1		5
#define SERVO2		6
#define SHIFT		7
#define LOAD		8
#define DIO			9
#define BUZZER		11
#define RELAYT		12
#define BUTTON1		A5
#define BUTTON2		A4
#define RELAY1A		A0
#define RELAY1B		A1
#define RELAY2A		A2
#define RELAY2B		A3

#define DEBOUNCE	200
#define LONG_PRESS	3000
#define SHORT_PRESS	150
#define TIME_OUT	5000

unsigned long tick1 = 0;
unsigned long tick2 = 0;
unsigned long b1tick = 0;
unsigned long b2tick = 0;
unsigned long timeout = 0;

uint8_t mode = 0;
uint8_t count1 = 0;
uint8_t count2 = 0;
uint8_t maxCount1 = 0;
uint8_t maxCount2 = 0;

bool button1State = true;
bool button2State = true;
bool changeState = false;

uint8_t numbers[] = 
    {// 0    1     2      3   4    5     6      7   8    9         -
       0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90
    };

Servo servo1, servo2;
	

void sensor1Event(){
	if(millis() - tick1 >= DEBOUNCE && count1 < maxCount1 && mode == 0){
		count1++;
		buzzerPop();
		tick1 = millis();
	}
}

void sensor2Event(){
	if(millis() - tick2 >= DEBOUNCE && count2 < maxCount2 && mode == 0){
		count2++;
		buzzerPop();
		tick2 = millis();
	}
}

void load(){
	maxCount1 = EEPROM.read(0);
	maxCount2 = EEPROM.read(1);
}

void save(){
	EEPROM.write(0, maxCount1);
	EEPROM.write(1, maxCount2);
}

void button1Event(){
	count1 = 0;
}

void button2Event(){
	count2 = 0;
}

void overload1Event(){	
	//servo 1
	buzzerPop();
	
	servo1.write(170);
	delay(100);

	digitalWrite(RELAY1B, LOW);
	digitalWrite(RELAY1A, HIGH);
	
	delay(1000);
	
	buzzerPop();
	digitalWrite(RELAY1A, LOW);
	digitalWrite(RELAY1B, HIGH);
	
	delay(1000);
	
	digitalWrite(RELAY1A, LOW);
	digitalWrite(RELAY1B, LOW);
	
	servo1.write(90);
	delay(100);
	
	buzzerPop();
	delay(200);
	buzzerPop();
}

void overload2Event(){		
	//servo 2
	buzzerPop();
	
	servo2.write(10);
	delay(100);

	digitalWrite(RELAY2A, HIGH);
	digitalWrite(RELAY2B, LOW);
	
	delay(1000);
	
	buzzerPop();
	digitalWrite(RELAY2A, LOW);
	digitalWrite(RELAY2B, HIGH);
	
	delay(1000);
	
	digitalWrite(RELAY2A, LOW);
	digitalWrite(RELAY2B, LOW);
	
	servo2.write(90);
	delay(100);
	
	buzzerPop();
	delay(200);
	buzzerPop();
}

void showLed(){
	int number;
	
	if(mode == 0){
		number = count1*100 + count2;
	}else{
		number = maxCount1*100 + maxCount2;
	}	

	for(int i = 0; i <4; i++){
		int digit = 0, pos = 0;
		digit = numbers[number%10];
		number = number/10;
		for(int j = 0; j<8; j++){
			if(digit & 0x80){
				digitalWrite(DIO, HIGH);
			}else{
				digitalWrite(DIO, LOW);
			}
			digit = digit << 1;
			digitalWrite(SHIFT, LOW);
			digitalWrite(SHIFT, HIGH);
		}

		pos = 0xf7 >> i;
		
		if(mode == 1 && i < 2){
			pos = 0xff;
		}else if(mode == 2 && i > 1){
			pos = 0xff;
		}

		for(int j = 0; j<8; j++){
			if(pos & 0x80){
				digitalWrite(DIO, HIGH);
			}else{
				digitalWrite(DIO, LOW);
			}
			pos = pos << 1;
			digitalWrite(SHIFT, LOW);
			digitalWrite(SHIFT, HIGH);
		}
		
		digitalWrite(LOAD, LOW);
		digitalWrite(LOAD, HIGH);
		delay(3);
	}
}

void buzzerPop(){
	digitalWrite(BUZZER, HIGH);
	delay(20);
	digitalWrite(BUZZER, LOW);
}

void setup() {
	
	pinMode(SENSOR1, INPUT_PULLUP);
	pinMode(SENSOR2, INPUT_PULLUP);
	pinMode(BUTTON1, INPUT);
	pinMode(BUTTON2, INPUT);
	pinMode(LOAD, OUTPUT);
	pinMode(SHIFT, OUTPUT);
	pinMode(DIO, OUTPUT);
	pinMode(BUZZER, OUTPUT);
	pinMode(RELAYT, OUTPUT);
	pinMode(RELAY1A, OUTPUT);
	pinMode(RELAY1B, OUTPUT);
	pinMode(RELAY2A, OUTPUT);
	pinMode(RELAY2B, OUTPUT);
	
	digitalWrite(RELAY1A, LOW);
	digitalWrite(RELAY1B, LOW);
	digitalWrite(RELAY2A, LOW);
	digitalWrite(RELAY2B, LOW);
	digitalWrite(RELAYT, LOW);
	
	servo1.attach(SERVO1);
	servo2.attach(SERVO2);
	servo1.write(90);
	servo2.write(90);
	delay(100);

	Serial.begin(9600);
	Serial.println("Started");
	attachInterrupt(digitalPinToInterrupt(SENSOR1), sensor1Event, RISING);
	attachInterrupt(digitalPinToInterrupt(SENSOR2), sensor2Event, RISING);
	
	timeout = b1tick = b2tick = tick1 = tick2 = millis();
	
	load();
	buzzerPop();
	delay(200);
	buzzerPop();
}

void loop() {
	
	showLed();

	bool b1 = digitalRead(BUTTON1);
	bool b2 = digitalRead(BUTTON2);
		
	if(b1 != button1State){
		if(b1 == LOW){
			if(mode == 0){	
				button1Event();
			}else if(mode == 1){
				maxCount1 = maxCount1==1?100:maxCount1-1;
				timeout = millis();
			}else if(mode == 2){
				maxCount2 = maxCount2==1?100:maxCount2-1;
				timeout = millis();
			}
			changeState = false;
			buzzerPop();
		}
		button1State = b1;
		b1tick = millis();
	}else{
		if(mode == 0){
			if(b1 == LOW && millis() -  b1tick >= LONG_PRESS){
				mode = 1;
				changeState = true;
				b1tick = millis();
				timeout = millis();
				buzzerPop();
				delay(200);
				buzzerPop();
			}
		}else{
			if(b1 == LOW && millis() -  b1tick >= SHORT_PRESS && changeState == false){
				if(mode == 1){
					maxCount1 = maxCount1==1?100:maxCount1-1;
				}else if(mode == 2){
					maxCount2 = maxCount2==1?100:maxCount2-1;
				}
				timeout = millis();
				b1tick = millis();
			}
		}
	}
	
	if(b2 != button2State){
		if(b2 == LOW){
			if(mode == 0){	
				button2Event();
			}else if(mode == 1){
				maxCount1 = maxCount1==100?1:maxCount1+1;
				timeout = millis();
			}else if(mode == 2){
				maxCount2 = maxCount2==100?1:maxCount2+1;
				timeout = millis();
			}
			changeState = false;
			buzzerPop();
		}
		button2State = b2;
		b2tick = millis();
	}else{
		if(mode == 0){
			if(b2 == LOW && millis() -  b2tick >= LONG_PRESS){
				mode = 2;
				changeState = true;
				b2tick = millis();
				timeout = millis();
				buzzerPop();
				delay(200);
				buzzerPop();
			}
		}else{
			if(b2 == LOW && millis() -  b2tick >= SHORT_PRESS && changeState == false){
				if(mode == 1){
					maxCount1 = maxCount1==100?1:maxCount1+1;
				}else if(mode == 2){
					maxCount2 = maxCount2==100?1:maxCount2+1;
				}
				timeout = millis();
				b2tick = millis();
			}
		}
	}
	
	if(mode == 0){
		if(count1 == maxCount1){
			overload1Event();
			count1 = 0;
		}else if(count2 == maxCount2){
			overload2Event();
			count2 = 0;
		}
	}else{
		if(millis() - timeout > TIME_OUT){
			mode = 0;
			save();
			buzzerPop();
			delay(200);
			buzzerPop();
		}
	}
}
