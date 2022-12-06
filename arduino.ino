#include <Ultrasonic.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Servo.h>
#include <Ethernet.h>

#define pino_trigger 4
#define pino_echo 5

Ultrasonic ultrasonic(pino_trigger, pino_echo);

char sentenca[128];
char leitura[8];

byte mac_addr[] = { 0xF4, 0x6B, 0x8C, 0x68, 0xC0, 0xF3 };

IPAddress server_addr(10,10,204,22);
char user[] = "arduino";
char password[] = "arduino";

char INSERIR_TEMPO[] = "INSERT INTO arduino (testee) VALUES (%s)";
char BANCODEDADOS[] = "USE arduino";

EthernetClient client;
MySQL_Connection conn((Client *)&client);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Ethernet.begin(mac_addr);
  Serial.println("Conectando...");
  if (conn.connect(server_addr, 3306, user, password)) {
    Serial.println("Conexão feita com sucesso");
    
    delay(1000);
    
    MySQL_Cursor *cur_mem = new MySQL_Cursor (&conn);
    cur_mem->execute(BANCODEDADOS);
    delete cur_mem;
  } else {
    Serial.println("A conexão falhou");
    conn.close();
  }
}

void loop() {
  Serial.println("Lendo dados do sensor...");
  
  float cmMsec;
  unsigned long tempo;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);
  Serial.print("Distancia em cm: ");
  Serial.println(cmMsec);

  if (cmMsec < 10) {
    tempo = millis();
    
  }
  
  delay(1000);

  Serial.println("Executando sentença");
  
  dtostrf(cmMsec, 4, 2, leitura);
  Serial.print("Valor leitura: ");
  Serial.println(leitura);
  sprintf(sentenca, INSERIR_TEMPO, leitura);
  
  Serial.println(sentenca);
  
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  cur_mem->execute(sentenca);
  delete cur_mem;
  
  delay(2000);

  Serial.println("Sentença executada");
}
