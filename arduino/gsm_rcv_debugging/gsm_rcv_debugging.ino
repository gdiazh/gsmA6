/**
 * @brief Simple GSM Sketch.
  *Receive AT comands and SMS
  *Usu an LED and Buzzer as debugger
  *It need's more than one hardware Serial port (eg. Arduino Due or Mega)
 */
 
/*Author: Gustavo Diaz Huenupan*/

#define MBED_LED 13
#define BUZZER 2
#define WORD_SIZE 50
#define COMANDS_SIZE 30
#define MESSGES_SIZE 20

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
// SoftwareSerial Debugger(8,7);    // Define RX & TX pin on Arduino conected to a serial debuger
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

void setup()
{
  Serial1.begin(115200);	// GSM A6 Module
  Serial.begin(9600);		// PC Debug
  pinMode(MBED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(MBED_LED, LOW);
  /*init variables*/
  init_ready = 0;
  char_cnt = 0;
  word_cnt = 0;
  msg_cnt = 0;
  msg_received = 0;
  Serial.println("Init Ready");
}
 
void loop()
{
  while(Serial1.available())    // Si llega un dato por el puerto GSM6 se envía al debugger
  {
    read_response();
  }
  if(Serial.available())  // Si llega un dato por el debugger se envía al puerto GSM6
  {
    Serial1.write(Serial.read());
  }
}

void read_response(void)
{
  char chr = Serial1.read();
  // Serial.print("chr = "); Serial.println(chr, DEC);
  if (chr != '\r' and chr != '\n')
  {
    last_word[char_cnt] = chr;
    char_cnt++;
  }
  if (char_cnt == WORD_SIZE)
  {
    char_cnt = 0;
    Serial.write("Warn comand too large, overwriting");
  }
  if (chr == '\n')
  {
    last_word[char_cnt] = 0;
    if(last_word[0]!='\r' & last_word[0]!='\n' & last_word[0]!=0) add_cmd(last_word, char_cnt);
    if (strcmp(last_word, init_ready_cmd)==0)
    {
      init_ready = 1;
      Serial.println("Setting text mode ...");
      Serial1.write(text_mode_cmd);
      delay(1000);
      Serial.println("Setting encoding ...");
      Serial1.write(encoding_cmd);
      delay(1000);
      Serial.println("Ready to receive sms");
      digitalWrite(MBED_LED, HIGH);
    }
    if (strcmp(last_word, in_msg_cmd)==0) msg_received = 1;
    if (msg_received & last_word[0]!='+' & last_word[0]!='^' & last_word[0]!='\n' & last_word[0]!='\r' & last_word[0]!=0)
    {
      /* TO DO: Improve this way "detecting" the message */
      // Serial.print("last_word[0]="); Serial.println(last_word[0], DEC);
      // Serial.print("last_word="); Serial.println(last_word);
      add_msg(last_word, char_cnt);
      msg_received = 0;
      uint8_t n = last_word[0]-'0';
      beeps(n, 500);
    }
    reset_word(last_word, WORD_SIZE);
  }
  // Serial.print("last_word (actual) = "); Serial.println(last_word);
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
  // Serial.print("last_word = "); Serial.println(last_word);
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
  // Serial.print("last_msg = "); Serial.println(msg);
  if (msg_cnt==MESSGES_SIZE) msg_cnt = 0;
  print_lisfOfMsg();
}

void print_lisfOfComands(void)
{
  Serial.println("Comands:");
  for (uint8_t i = 0; i < word_cnt; i++)
  {
    Serial.print("\t"); Serial.print(i); Serial.print(":"); Serial.println(comands[i]);
  }
}

void print_lisfOfMsg(void)
{
  Serial.println("Messages:");
  for (uint8_t i = 0; i < msg_cnt; i++)
  {
    Serial.print("\t"); Serial.print(i); Serial.print(":"); Serial.println(messages[i]);
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