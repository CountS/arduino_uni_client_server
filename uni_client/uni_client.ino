
// Скетч клиента/сервера, управляемого командами 
// Код единый для сервера и клиента, роль задается константой

/*
  Алгоритм сетевого обмена:
  Клиент  -  Сервер
  Режим A (0)
          <- cmd
  reply  ->

  Режим B (1)
  сmd_rq -> 
          <- cmd
  reply  ->
  Режим C (2)
  сmd -> 
          <- cmd
  
  то есть клиет может быть в режиме А, то есть в режиме ожидания команды от сервера 
  или в режиме Б - тогда клиент сам запрашивает сервер о командах, которые надо выполнить,
  или в режиме C - тогда клиент по своей инициативе передает команды серверу
  Режим A предполагает опрос сервером клиентов.
  Режим B - постоянный диалог с инициативой клиента. (по умолчанию)
  Режим С - команды от клиента.
  
  
 
  Команды:
  клиент - серверу
  0 - команда-расширение.
  1 - получить команду. Нагрузка - нет
  3 - изменение состояния выходов. Нагрузка - старое и новое состояние выходов (по байтам, только логические 1-0)  - описать подробнее
  5 - изменение температуры. Нагрузка - номер датчика, старое и новое состояние температуры
  7 - изменение состояние аналогового входа. Нагрузка - номер датчика, старое и новое состояние
  8 - считана карта. Нагрузка - считанный номер.
  
  сервер - клиенту
  0 - команда-расширение.
  1 - NOP 
  2 - изменить состояние входов. Передается новое стостояние (по байтам, только логические 1-0) - описать подробнее
  4 - изменить состояние аналогового выхода. Нагрузка - номер выхода (6), новое состояние(7) 
  6 - вывести текст на LCD. Нагрузка - текст для вывода (включая команды форматирования и служебные)

    
*/


#include <VirtualWire.h>
const int led_pin = 13;
const int transmit_pin = 12;
const int receive_pin = 10;
const int transmit_en_pin = 3;


String RSinputString = "";         // a string to hold incoming data
boolean RSinputStringComplete = false;  // whether the string is complete

const boolean server = true; //set "true" if it's a server 


byte count = 1;
byte device = 0;
byte mode=1; //0 - режим A, 1 - режим B, 2 -режим C
int i;
int cmd;
int from;
char msg[26] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
//               0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5  
//               0 - номер посылки
//               1 - адрес отправителя
//               2 - резерв
//               3 - адрес получателя.  если 0 то это бродкаст
//               4 - резерв / контрольный 
//               5 - тип команды
//               6 - F нагрузки
//          включить 9й диод на клиенте 2 - 00020291



int freeRam() {
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int) __brkval);  
}


void setup()
{
  // Initialise the IO and ISR
  pinMode(transmit_pin, OUTPUT);      // sets the digital pin as output
  pinMode(receive_pin, INPUT);   // sets the digital pin as output
  pinMode(led_pin, OUTPUT);      // sets the digital pin as output
  
  vw_set_tx_pin(transmit_pin);
  vw_set_rx_pin(receive_pin);
  vw_set_ptt_pin(transmit_en_pin);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);	 // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
  Serial.begin(9600);
  Serial.println("start....");
  Serial.println("");  
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
        // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') 
    {
      RSinputStringComplete = true;
    }
    else
    {
      RSinputString += inChar;
    } 
  }
}

void loop()
{
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
//  delay(200);
//  Serial.println(mode); 

  digitalWrite(led_pin, true); // Flash a light to show received good message

  if (mode==0){
  }
  else if (mode==1){
    if (server){ //код сервера
//      vw_wait_rx();
      if (vw_wait_rx_max(1000)){
        if (vw_get_message(buf, &buflen)) // Non-blocking
        {
          
          if (int(buf[3])==device || int(buf[3])==0)
          {
            if (int(buf[5])==3) //DIG - изменилось состояни логического входа
            {  
              Serial.println("client send DIG ");
              Serial.println(int(buf[6]));                                        
              Serial.println(int(buf[7]));                                                      
              
            } 
 
            if (int(buf[5])==1) //GET Command
            {  
//            Serial.println("server recived GET COMMAND from d#"||buf[3]||" n#"||buf[0]);
              Serial.println("server recived GET COMMAND");
              if (RSinputStringComplete)
              {
                Serial.println("server has something to tell... ");
                Serial.println((RSinputString[3]));
                Serial.println((RSinputString[5]));
                Serial.println((RSinputString[6]));
                Serial.println((RSinputString[7]));                
                msg[0] = count; count++;
                msg[1] = device;
                msg[3] = ((RSinputString[3]))-48;
                msg[5] = ((RSinputString[5]))-48;
                msg[6] = ((RSinputString[6]))-48;
                msg[7] = ((RSinputString[7]))-48;
                vw_send((uint8_t *)msg, 26);
                vw_wait_tx(); // Wait until the whole message is gone  
                RSinputStringComplete=false;
                RSinputString="";
                Serial.println("server send command");
              }
              else
              {
              Serial.println("server has nothing to send ");  
              msg[0] = count; count++;
              msg[1] = device;
              msg[3] = buf[1];
              msg[4] = 9;
              msg[5] = 1;
              vw_send((uint8_t *)msg, 26);
              vw_wait_tx(); // Wait until the whole message is gone  
              Serial.println("server send NOP ");
              }  
            
            } 
            if (buf[5]==13) //ANALOG - изменилось состояние аналогового входа
            {  
            } 
            if (buf[5]==14) //CARD - считана карта на клиенте
            {  
            } 
       
          }
          

        }
        
      }
    }
    else { //код клиента
    //послать команду серверу "что делать"
    //получить ответ
    //выполнить команду
    msg[0] = count; count++;
    msg[1] = device;
    msg[3] = 0;
    msg[4] = 9;
    msg[5] = 1;
    vw_send((uint8_t *)msg, 26);
    vw_wait_tx(); // Wait until the whole message is gone 
    Serial.println("client send GET COMMAND (1), waiting...");
//  vw_wait_rx();   
    if (vw_wait_rx_max(1000)){
      if (vw_get_message(buf, &buflen)) // Non-blocking
      {
              Serial.println(">>>");                      
              Serial.write(buf[0]);            
              Serial.write(buf[1]);                          
              Serial.write(buf[3]);                        
              Serial.write(buf[5]);                                        
              Serial.write(buf[6]);                                        
        
        if (buf[3]==device || buf[3]==0)
        {
          if (buf[5]==1) //NOP
          {  
            Serial.println("client recived NOP ");
            msg[0] = count; count++;
            msg[1] = device;
            msg[3] = 0;
            msg[4] = 9;
            msg[5] = 1;
            vw_send((uint8_t *)msg, 26);
            vw_wait_tx(); // Wait until the whole message is gone  
            Serial.println("client confirmed ");           
          } 
          if (buf[5]==2) //DIG
          {  
            Serial.println("client recived SET DIGITAL ");
            digitalWrite(buf[6], buf[7]);
            
            msg[0] = count; count++;
            msg[1] = device;
            msg[3] = 0;
            msg[4] = 9;
            msg[5] = 11;
            msg[6] = buf[6];
            msg[7] = buf[7];
            vw_send((uint8_t *)msg, 26);
            vw_wait_tx(); // Wait until the whole message is gone  
            Serial.println("client confirmed ");
          } 
          if (buf[5]==4) //ANALOG
          {  
          } 
          if (buf[5]==6) //LCD
          {  
          } 
       
        }

       
        
      }
    
      
    }
    }

  }
  else if (mode==2){
    if (server){ //код сервера
    
    }
    else { //код клиента
    
    }
    
  }
  else {
    
    for (i = 0; i < 10; i++){
    digitalWrite(led_pin, true); // Flash a light to show received good message
    delay(200);
    digitalWrite(led_pin, false); // Flash a light to show received good message
    delay(200);
 
     }
    mode=1;
  }
  
  
  /*  const char *msg = "hello";
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    // Wait for a message
    vw_wait_rx();
    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
	int i;
	const char *msg = "goodbye";

        digitalWrite(13, true); // Flash a light to show received good message
	// Message with a good checksum received, dump it.
	Serial.print("Got: ");
	
	for (i = 0; i < buflen; i++)
	{
	    Serial.print(buf[i], HEX);
	    Serial.print(" ");
	}
	Serial.println("");

	// Send a reply
	vw_send((uint8_t *)msg, strlen(msg));
        digitalWrite(13, false);
    }
*/
}


