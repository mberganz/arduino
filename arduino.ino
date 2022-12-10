#include <Servo.h>
#include <Ultrasonic.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Servo.h>
#include <Ethernet.h>

#define pino_trigger 4
#define pino_echo 5
#define pino_pir 2
#define SERVO 6

Ultrasonic ultrasonic(pino_trigger, pino_echo);

char sentenca_tempo[128];
char sentenca_report[128];
char sentenca_update_report[128];
char sentenca_open[128];
char sentenca_close[128];
char leitura[8];
char distancia_char[8];
const int intervalo = 10000;
unsigned long tempo_anterior = 0;
unsigned long tempo_atual;
bool open = false;
int acionamento;
Servo s;
int posicao;

const int tipo_vaga = 1;
const int num_vaga = 1;

byte mac_addr[] = { 0x00, 0x27, 0x13, 0xAE, 0x7E, 0xD1 };

IPAddress server_addr(10, 10, 117, 18);
char user[] = "arduino";
char password[] = "arduino";

char UPDATE_TEMPO[] = "UPDATE report SET duration = (%d)";
char UPDATE_REPORT_OPEN[] = "UPDATE report SET open = 1";
char UPDATE_REPORT_CLOSE[] = "UPDATE report SET open = 0";
char UPDATE_OPEN[] = "UPDATE park SET open = 1";
char UPDATE_CLOSE[] = "UPDATE park SET open = 0";
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

  pinMode(pino_pir, INPUT);
  s.attach(SERVO);
  s.write(0);
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

  // ------------------- Checa a distancia ------------------- //

  dtostrf(cmMsec, 4, 2, distancia_char);
  Serial.print("Distancia: ");
  Serial.print(distancia_char);
  Serial.println("cm");

  String distancia_str = String(distancia_char);
  distancia = distancia_str.toInt();
  Serial.println(distancia);

  delay(1000);

  // ------- Verifica se o carro está dentro da vaga -------- //

  if (distancia > 11) {
    Serial.println("Vaga livre");
    sprintf(sentenca_close, UPDATE_OPEN);  // Marca vaga como livre
    Serial.println(sentenca_close);
    cur_mem->execute(sentenca_close);
    
    tempo_anterior = tempo_atual;

    delay(1000);
  } else if (distancia <= 11 && distancia > 10) {
    Serial.println("Entrando");
    delay(1000);
  } else if (distancia <= 10) {
    Serial.println("Dentro");

    Serial.println("Atualizando vaga no banco");
    sprintf(sentenca_open, UPDATE_CLOSE);  // Marca vaga como ocupada
    Serial.println(sentenca_open);
    cur_mem->execute(sentenca_open);

    delay(1000);

    tempo_atual = millis();
    if ((unsigned long)(tempo_atual - tempo_anterior) >= intervalo) {
      String tempo_str = String(tempo_atual);
      Serial.print("Tempo: ");
      Serial.println(tempo_str);

      delay(1000);

      // ---------- Salva o tempo de vaga no banco ---------- //

      Serial.println("Executando sentença tempo");
      sprintf(sentenca_tempo, UPDATE_TEMPO, tempo_atual - tempo_anterior);
      Serial.println(sentenca_tempo);
      cur_mem->execute(sentenca_tempo);

      // ----------------- Reinicia o timer ----------------- //

      // tempo_anterior = tempo_atual;
    }
  }

  delay(1000);

  // ----------------- Sensor de presença ----------------- //

  acionamento = digitalRead(pino_pir);

  if (acionamento == LOW) {
    posicao = 0;
    s.write(posicao);
  } else {
    posicao = 90;
    s.write(posicao);
  }

  // --------------- Reportar para o sistema --------------- //

  if (distancia < 10) {
    open = true;
  } else if (distancia > 11) {
    open = false;
  }

  if (open == true) {
    Serial.println("Executando sentença report");
    sprintf(sentenca_report, UPDATE_REPORT_CLOSE);  // Marca vaga como ocupada
    Serial.println(sentenca_report);
    cur_mem->execute(sentenca_report);
  } else if (open == false) {
    Serial.println("Atualizando report");
    sprintf(sentenca_update_report, UPDATE_REPORT_OPEN);  // Marca vaga como livre
    Serial.println(sentenca_update_report);
    cur_mem->execute(sentenca_update_report);
  }

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
