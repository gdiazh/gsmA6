/**
 * @brief Simple GSM Sketch.
 	*Receive AT comands and SMS
 	*Use an RGB LED and Buzzer for debugging
 	*It send data through a Software Serial port for data debugging
 */
 
/*Author: Gustavo Diaz Huenupan*/

#include <SoftwareSerial.h>   // Incluimos la libreria  SoftwareSerial
#include <LiquidCrystal.h>

#define MBED_LED 13
#define R_LED 12
#define G_LED 11
#define B_LED 10
#define BUZZER 2
#define WORD_SIZE 30
#define COMANDS_SIZE 20
#define MESSGES_SIZE 10

/*---------AT comands to set up the GSM for send/recv SMS messages---------------*/

char text_mode_cmd[] = "AT+CMGF=1\r\n";
/* this comand set the operating mode of the GSM
  *0 = PDU Mode
  *1 = Text Mode
*/

char encoding_cmd[] = "AT+CSCS=\"GSM\"\r\n";
/* this command selects the character set for messages
  Posible values:
  *"GSM"
  *"PCCP437"
  *"CUSTOM"
  *"HEX"
*/

char in_msg_cmd[] = "+CIEV: \"MESSAGE\",1";
/* this command is received before de message when an sms is received*/

char init_ready_cmd[] = "+CIEV: service,  1";
/* this command is received when the GSM service is available on de module (it has signal)*/

/* Variable definitions*/ 
SoftwareSerial Debugger(7,6);    // Define RX & TX pin on Arduino conected to a serial debuger
LiquidCrystal lcd(9, 8, A3, A2, A1, A0);
char chr;
char last_word[WORD_SIZE];
char comands[COMANDS_SIZE][WORD_SIZE];
char messages[MESSGES_SIZE][WORD_SIZE];
uint8_t init_ready;
uint8_t char_cnt;
uint8_t word_cnt;
uint8_t msg_cnt;
uint8_t msg_received;

/* Auxiliary methods*/
void read_response(void);
void reset_word(char last_word[], uint8_t size);
void add_cmd(char cmd[], uint8_t cmd_sz);
void add_msg(char msg[], uint8_t msg_sz);
void print_lisfOfComands(void);
void print_lisfOfMsg(void);
void beeps(uint8_t n, float time_delay);
void set_led(char rgb);

void setup()
{
  Serial.begin(115200);   // GSM
  Debugger.begin(9600);
  pinMode(MBED_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(MBED_LED, LOW);
  set_led('R');
  lcd.begin(16, 1);
  // lcd.clear();
  /*init variables*/
  init_ready = 0;
  char_cnt = 0;
  word_cnt = 0;
  msg_cnt = 0;
  msg_received = 0;
  Debugger.println("Init Ready");
  lcd.print("SEARCH..");
}
 
void loop()
{
  while(Serial.available())    // Si llega un dato por el puerto GSM6 se envía al debugger
  {
    read_response();
  }
  if(Debugger.available())  // Si llega un dato por el debugger se envía al puerto GSM6
  {
    Serial.write(Debugger.read());
  }
}

void read_response(void)
{
  char chr = Serial.read();
  // Debugger.print("chr = "); Debugger.println(chr, DEC);
  if (chr != '\r' and chr != '\n')
  {
    last_word[char_cnt] = chr;
    char_cnt++;
  }
  if (char_cnt == WORD_SIZE)
  {
    char_cnt = 0;
    Debugger.write("Warn comand too large, overwriting");
  }
  if (chr == '\n')
  {
    last_word[char_cnt] = 0;
    if(last_word[0]!='\r' & last_word[0]!='\n' & last_word[0]!=0) add_cmd(last_word, char_cnt);
    if (strcmp(last_word, init_ready_cmd)==0)
    {
      lcd.print("CONFIG..");
      init_ready = 1;
      set_led('B');
      Debugger.println("Setting text mode ...");
      Serial.write(text_mode_cmd);
      delay(1000);
      Debugger.println("Setting encoding ...");
      Serial.write(encoding_cmd);
      delay(1000);
      Debugger.println("Ready to receive sms");
      digitalWrite(MBED_LED, HIGH);
      set_led('K');
      lcd.print("READY!");
    }
    if (strcmp(last_word, in_msg_cmd)==0) msg_received = 1;
    if (msg_received & last_word[0]!='+' & last_word[0]!='^' & last_word[0]!='\n' & last_word[0]!='\r' & last_word[0]!=0 & last_word[0]=='*')
    {
      /* TO DO: Improve this way "detecting" the message */
      // Debugger.print("last_word[0]="); Debugger.println(last_word[0], DEC);
      // Debugger.print("last_word="); Debugger.println(last_word);
      set_led('G');
      add_msg(last_word, char_cnt);
      msg_received = 0;
      // uint8_t n = last_word[1]-'0';
      uint8_t n =(char_cnt<=8)?char_cnt:8;
      lcd.print(last_word);
      beeps(n, 500);
      set_led('K');
    }
    reset_word(last_word, WORD_SIZE);
  }
  // Debugger.print("last_word (actual) = "); Debugger.println(last_word);
}

void reset_word(char last_word[], uint8_t size)
{
  for (uint8_t i = 0; i < size; i++)
  {
    last_word[i] = '0';
  }
  char_cnt = 0;
}

void add_cmd(char cmd[], uint8_t cmd_sz)
{
  for (uint8_t i = 0; i < cmd_sz; i++)
  {
    comands[word_cnt][i] = cmd[i];
  }
  word_cnt++;
  // Debugger.print("last_word = "); Debugger.println(last_word);
  if (word_cnt==COMANDS_SIZE) word_cnt = 0;
  print_lisfOfComands();
}

void add_msg(char msg[], uint8_t msg_sz)
{
  for (uint8_t i = 0; i < msg_sz; i++)
  {
    messages[msg_cnt][i] = msg[i];
  }
  msg_cnt++;
  // Debugger.print("last_msg = "); Debugger.println(msg);
  if (msg_cnt==MESSGES_SIZE) msg_cnt = 0;
  print_lisfOfMsg();
}

void print_lisfOfComands(void)
{
  Debugger.println("Comands:");
  for (uint8_t i = 0; i < word_cnt; i++)
  {
    Debugger.print("\t"); Debugger.print(i); Debugger.print(":"); Debugger.println(comands[i]);
  }
}

void print_lisfOfMsg(void)
{
  Debugger.println("Messages:");
  for (uint8_t i = 0; i < msg_cnt; i++)
  {
    Debugger.print("\t"); Debugger.print(i); Debugger.print(":"); Debugger.println(messages[i]);
  }
}

void beeps(uint8_t n, float time_delay)
{
  for(uint8_t i = 0; i<n; i++)
  {
      digitalWrite(BUZZER, HIGH);
      delay(time_delay);
      digitalWrite(BUZZER, LOW);
      delay(time_delay);
  }
}

void set_led(char rgb)
{
  if (rgb == 'R')
  {
    digitalWrite(R_LED, HIGH);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, LOW);
  }
  else if (rgb == 'G')
  {
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, HIGH);
    digitalWrite(B_LED, LOW);
  }
  else if (rgb == 'B')
  {
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, HIGH);
  }
  else
  {
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, LOW);
  }
}