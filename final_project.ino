#include <IRremote.h>
#include <LiquidCrystal.h>
#include <dht_nonblocking.h>
#include <pitches.h>
#include "melodies.cpp"

#define DHT_SENSOR_TYPE DHT_TYPE_11
#define TEMPO 105

//DHT
static const int DHT_SENSOR_PIN = 2;
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

//lcd
LiquidCrystal lcd(7,8,9,10,11,12); //board pins for the lcd screen

//IRreceptor
static const int RECEIVER = 3; //pin for the IR receiver
IRrecv irrecv(RECEIVER); //instance of irrecv

//msg
const String msg[] = {"    HUMEDAD:    ","  TEMPERATURA:  ","  LUMINOSIDAD:  ","  TEMPORIZADOR: "};
int this_msg = 0;
String text;

//fotorresistor
//int sensorHigh=0;
//int sensorLow = 1023;

//buzzer
int wholenote = (60000 * 4) / TEMPO;
int divider = 0, note_duration = 0;

//values of sensors (0)= humidity, 1 = temperature, 2 = light, 3 = seconds)
float vals[4];

//values of the last iteration (0 = humidity, 1 = temperature, 2 = light, 3 = seconds) 
int previous_vals[4];
int previous_msg;
int marker = 0;
int time = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0,INPUT);    //photoresistor pin
  irrecv.enableIRIn();  //start the receiver
  lcd.begin(16,2);      //initializes the lcd screen with 16 columns and 2 rows
  initialize();         //start
}

void loop() {
  // put your main code here, to run repeatedly:
  bool msg_change = info_change(this_msg,&previous_msg);  //checks if the displayed msg has changed
  vals[2] =(int)(analogRead(A0)/10)*10; //converts the light value into a multiple of 10 so it doesn´t vary all the time on the display
  dht_sensor.measure( &vals[1], &vals[0] ); //measures the humidity(0) and temperature(1)

  if(irrecv.decode()){  //checks if it is receiving an IR signal
    translateIR();      //checks what the signal code is
    irrecv.start();     //receive the next value
  } 
 
  //if the current msg is different
  if(msg_change){
    lcd.clear();   //clears the display before showing anything
    lcd.setCursor(0,0);
    lcd.print(msg[this_msg]);   //shows the new msg
  }

  if(marker > 0){
    vals[3] = time - (millis()/1000 - marker);
    if (vals[3] == 0){
      marker = 0;
      int prev = this_msg;
      this_msg = 3;
      print_msg("      TIME      ",msg_change,this_msg);
      play_melody(PACMAN);
      msg_change = true;
      this_msg = prev;
    }
  }
  
  
  lcd.setCursor(0,1);
  
  //displays information according to the current msg
  switch(this_msg){
    case 0: //humidity
      text = "      " + (String)(int)vals[0] + "%      ";
      print_msg(text,msg_change,this_msg);
      break;

    case 1: //temperature
      text = "     " + (String)(int)vals[1] + " deg.C    ";
      print_msg(text,msg_change,this_msg);
      break;

    case 2: //light
      text = "      " + (String)(int)vals[2] + "      ";
      print_msg(text,msg_change,this_msg);
      Serial.println(vals[2]);
      break;

    case 3: //seconds
      text = "      " + (String)(int)vals[3]+ "      ";
      print_msg(text,msg_change,this_msg);
  }
  
}

void translateIR(){ //identifies the signal
  
  //output the IR code on the serial monitor
  Serial.print("IR code:0x");
  Serial.println(irrecv.decodedIRData.decodedRawData, HEX);

  //map the IR code to the remote key
  switch (irrecv.decodedIRData.decodedRawData)
  {
    case 0xBA45FF00: Serial.println("POWER");
      initialize();
      break;
    case 0xB847FF00: Serial.println("FUNC/STOP");
      //plays PACMAN song
      play_melody(PACMAN);
      break;
    case 0xB946FF00: Serial.println("VOL+");
      break;
    case 0xBB44FF00: Serial.println("FAST BACK");
      break;
    case 0xBF40FF00: Serial.println("PAUSE");
      start_timer();
      break;
    case 0xBC43FF00: Serial.println("FAST FORWARD");
      break;
    case 0xF807FF00: Serial.println("DOWN");
      //sets current msg to msg + 1
      get_info("DOWN");
      break;
    case 0xEA15FF00: Serial.println("VOL-");
      break;
    case 0xF609FF00: Serial.println("UP"); 
    //sets current msg to msg - 1 
      get_info("UP");
      break;
    case 0xE619FF00: Serial.println("EQ");
      //deletes last digit
      if(this_msg == 3)
        set_timer(-1);
      break;
    case 0xF20DFF00: Serial.println("ST/REPT");  
      break;
    case 0xE916FF00: Serial.println("0"); 
      if(this_msg == 3)
        set_timer(0); 
      break;
    case 0xF30CFF00: Serial.println("1");  
      if(this_msg == 3)
        set_timer(1);
      break;
    case 0xE718FF00: Serial.println("2");  
      if(this_msg == 3)
        set_timer(2);
      break;
    case 0xA15EFF00: Serial.println("3"); 
      if(this_msg == 3)
        set_timer(3); 
      break;
    case 0xF708FF00: Serial.println("4");  
      if(this_msg == 3)
        set_timer(4);
      break;
    case 0xE31CFF00: Serial.println("5");  
      if(this_msg == 3)
        set_timer(5);
      break;
    case 0xA55AFF00: Serial.println("6");  
      if(this_msg == 3)
        set_timer(6);
      break;
    case 0xBD42FF00: Serial.println("7"); 
      if(this_msg == 3)
        set_timer(7);
      break;
    case 0xAD52FF00: Serial.println("8"); 
      if(this_msg == 3)
        set_timer(8); 
      break;
    case 0xB54AFF00: Serial.println("9");  
      if(this_msg == 3)
        set_timer(9);
      break;
    default:
      Serial.println(" other button   ");
  }// End Case
  delay(100); // Do not get immediate repeat
} //END translateIR


//Shows the start of the menu
void initialize(){
  this_msg = 0;       //sets the initial msg to display
  previous_msg = -1;  //previous msg != current msg so it shows on the lcd
  vals[3] = 0;
  time = 0;
  marker = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("INICIALIZANDO...");
  while(dht_sensor.measure( &vals[1], &vals[0]) == false);  //measures values for temperature(1) and humidity(0)
  delay(2000);
  lcd.clear();
  get_info(""); //displays the info
}

//gets the next position of the array of msgs according to the pressed key
void get_info(String action){
  if(action == "UP")
    this_msg = (this_msg + 3) % 4;
  else if(action == "DOWN")
    this_msg = (this_msg + 1) % 4;
  tone(13,NOTE_F4,100);
}

//checks if there has been any change on the parameter 'info' since last iteration
bool info_change(int info, int *previous_info){
  if(info != *previous_info){
    *previous_info = info;
    return true;
  }
  else 
    return false;
}

//plays the melody
void play_melody(int melody[]){
  int notes = sizeof(PACMAN)/sizeof(PACMAN[0]) / 2;
  for(int this_note = 0; this_note < notes * 2; this_note = this_note + 2){
      divider = melody[this_note + 1];
      if(divider > 0){
        note_duration = (wholenote/divider);
      }
      else if(divider < 0){
        note_duration = (wholenote) / abs(divider);
        note_duration *= 1.5;
      }

      tone(13,melody[this_note],note_duration * 0.9);
      delay(note_duration);
      noTone(13);
    }
}

//prints the info of the respective msg
void print_msg(String text, bool msg_change,int current){
  if(msg_change)
    lcd.print(text);
  else if(info_change(vals[current],&previous_vals[current]))
    lcd.print(text);
}

void set_timer(int digit){
  if(vals[3] <= 999 && digit >= 0){
    vals[3] = vals[3]*10 + digit;
    tone(13,NOTE_F5,100);
  }
  else if(digit == -1){
    vals[3] = (int)vals[3]/10;
  } 
}

void start_timer(){
  if(marker == 0){
    marker = millis()/1000;
    time = vals[3];
  }
  else {
    marker = 0;
  }
}

