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
char distancia[8];

const int tipo_vaga = 1;
const int num_vaga = 1;

byte mac_addr[] = { 0xF4, 0x6B, 0x8C, 0x68, 0xB5, 0xF9 };

IPAddress server_addr(10,10,204,24);
char user[] = "arduino";
char password[] = "arduino";

char INSERIR_TEMPO[] = "INSERT INTO report (duration) VALUES (%s)";
char INSERIR_REPORT[] = "INSERT INTO report (open) VALUES 1";
char UPDATE_OPEN[] = "UPDATE park SET open = 1 WHERE park_id = 'num_vaga'";
char UPDATE_CLOSE[] = "UPDATE park SET open = 0 WHERE park_id != 'num_vaga'";
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
  dtostrf(cmMsec, 4, 2, distancia);
  Serial.print("Distancia em cm: ");
  Serial.println(distancia);

  if (distancia < "10") {
    tempo = millis();
  }
  
  String tempo_str = String(tempo, DEC);  
  Serial.print("Tempo: ");
  Serial.println(tempo_str);           
  
  delay(1000);

  Serial.println("Executando sentença");
  
  dtostrf(cmMsec, 4, 2, leitura);
  Serial.print("Valor leitura: ");
  Serial.println(leitura);
  sprintf(sentenca, INSERIR_TEMPO, tempo_str);
  
  Serial.println(sentenca);
  
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  cur_mem->execute(sentenca);
  delete cur_mem;
  
  delay(2000);

  Serial.println("Sentença executada");
}
