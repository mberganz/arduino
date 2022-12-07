#include <Ultrasonic.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Servo.h>
#include <Ethernet.h>

#define pino_trigger 4
#define pino_echo 5

Ultrasonic ultrasonic(pino_trigger, pino_echo);

char sentenca_tempo[128];
char sentenca_report[128];
char sentenca_open[128];
char leitura[8];
char distancia_char[8];
const int intervalo = 1000;
unsigned long tempo_anterior = 0;

const int tipo_vaga = 1;
const int num_vaga = 1;

byte mac_addr[] = { 0xF4, 0x6B, 0x8C, 0x68, 0xB5, 0xF9 };

IPAddress server_addr(10, 10, 204, 24);
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
  while (!Serial)
    ;
  Ethernet.begin(mac_addr);
  Serial.println("Conectando...");
  if (conn.connect(server_addr, 3306, user, password)) {
    Serial.println("Conectado!");

    delay(1000);

    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    cur_mem->execute(BANCODEDADOS);
    delete cur_mem;
  } else {
    Serial.println("A conexão falhou");
    conn.close();
  }
}

void loop() {
  Serial.println("");
  Serial.println("------------------------------------------------");

  Serial.println("Alocando memória");
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  delay(1000);

  Serial.println("Lendo dados do sensor...");

  // ------------------------------------------------ //

  float cmMsec;
  int distancia;
  long microsec = ultrasonic.timing();
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);

  // ------------------------------------------------ //

  dtostrf(cmMsec, 4, 2, distancia_char);
  Serial.print("Distancia: ");
  Serial.print(distancia_char);
  Serial.println("cm");

  String distancia_str = String(distancia_char);
  distancia = distancia_str.toInt();
  Serial.println(distancia);

  delay(1000);

  // ------------------------------------------------ //

  if (distancia <= 11 && distancia > 10) {
    Serial.println("Entrando");
  } else if (distancia > 10 && distancia < 11) {
    Serial.println("Saindo");
  } else if (distancia <= 10) {
    Serial.println("Dentro");
    unsigned long tempo_atual = millis();
    if ((unsigned long)(tempo_atual - tempo_anterior) >= intervalo) {
      String tempo_str = String(tempo_atual, DEC);
      Serial.print("Tempo: ");
      Serial.println(tempo_str);

      // ------------------------------------------------ //

      Serial.println("Executando sentença tempo");
      sprintf(sentenca_tempo, INSERIR_TEMPO, tempo_str);
      Serial.println(sentenca_tempo);
      cur_mem->execute(sentenca_tempo);

      // ------------------------------------------------ //

      tempo_anterior = tempo_atual;
    }
  }

  delay(1000);

  // ------------------------------------------------ //

  Serial.println("Executando sentença report");
  sprintf(sentenca_report, INSERIR_REPORT, "report");
  Serial.println(sentenca_report);
  cur_mem->execute(sentenca_report);

  delay(1000);

  // ------------------------------------------------ //

  Serial.println("Liberando memória...");
  delete cur_mem;
  delay(1000);
  Serial.println("Memória liberada");

  delay(1000);

  Serial.println("------------------------------------------------");
  Serial.println("");
}
