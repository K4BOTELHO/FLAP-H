/* --------------------------------------------------------------------------------------------------------------------------------------------------------

Este software é protegido por direitos autorais e leis internacionais. Qualquer cópia não autorizada, distribuição ou uso deste software, total ou parcial,
será considerada uma violação dos direitos autorais e sujeita a medidas legais. 
Conforme estipulado pela Lei de Direitos Autorais, Lei nº 9.610/98, a pirataria de software é estritamente proibida e sujeita a penalidades legais. A cópia
não autorizada deste software constitui uma violação dos direitos de propriedade intelectual, passível de processo civil e criminal.
Ressaltamos que qualquer tentativa de reprodução, distribuição ou uso não autorizado deste software será monitorada e tratada com rigor dentro dos limites 
da lei.

-----------------------------------------------------------------------------------------------------------------------------------------------------------
Nosso sistema IR opera na versão 3.8, pois as versões mais recentes não apresentam melhorias significativas que justifiquem uma atualização,
sem impacto relevante na execução da lógica.
-------------------------------------------------------------------------------------------------------------------------------------------------------- */ 

#include <IRremote.h>                       // Biblioteca do sistema IR.
#include <RCSwitch.h>                       // Biblioteca do sistema RF.
#include <EEPROM.h>                         // Biblioteca EEPROM (Memoria).
  
// --------------------------------------------------------------------------------------------------------------------------------------------------------  

#define SensorAbre 11                       // Fim de curso que identifica o FLAP "Aberto". 
#define SensorFecha 10                      // Fim de curso que identifica o FLAP "Fechado". 

// Já esses #define abaixo são para o Atuador Linear.

#define AvancarMotor 5                      // Atuador Linear - Avançar/Abir.
#define RecuarMotor 4                       // Atuador Linear - Recuar/Fechar.

// --------------------------------------------------------------------------------------------------------------------------------------------------------

int RECV_PIN = 16;                          // Define o pino de leitura do sensor IR.

// --------------------------------------------------------------------------------------------------------------------------------------------------------

unsigned long int tempobotao = 0;           // Variavel que guarda o momento. 
unsigned long lastCode = 0;                 // Variável para armazenar o último código válido.

unsigned long int TBotao = 0;               // Variavel que guarda o momento do botão.
unsigned long int TAbrirFechar = 0;         // Variavel que guarda o momento do "Abrir" e "Fechar" do sistema.

int AMotor = HIGH;                          // Estado inicial do AMotor = AvancarMotor (4).
int RMotor = HIGH;                          // Estado inicial do RMotor = RecuarMotor (5).

float armazenavalor;                        // Variavel valor do IR.

int TMotor = 0;                             // Estado inicial do TMotor.

unsigned long int IgnoreFC = 0;             // Variável para armazenar o tempo do último comando do fim de curso.

unsigned long tempoMotorAtivado = 0;        // Variável para armazenar o tempo de ativação do motor.

bool motorEmMovimento = false;              // Flag para indicar se o motor está em movimento.

// ---- IR ------------------------------------------------------------------ IR ------------------------------------------------------------------ IR ----

IRrecv irrecv(RECV_PIN);
decode_results results;                     // Estrutura para armazenar os resultados da decodificação.

// ---- RF ------------------------------------------------------------------ RF ------------------------------------------------------------------ RF ----

RCSwitch mySwitch = RCSwitch();

// --------------------------------------------------------------------------------------------------------------------------------------------------------

void setup()
{
  pinMode(AvancarMotor, OUTPUT);            // Define o pino de "AvancarMotor" como saida
  pinMode(RecuarMotor, OUTPUT);             // Define o pino de "RecuarMotor" como saida
  digitalWrite(AvancarMotor, AMotor);   
  digitalWrite(RecuarMotor, RMotor);  

  pinMode(SensorAbre, INPUT);               // Define o pino do "SensorAbre" como entrada.
  pinMode(SensorFecha, INPUT);              // Define o pino do "SensorFecha" como entrada.
  
  irrecv.enableIRIn();                      // Inicializa o receptor IR

  mySwitch.enableReceive(0);                // Indica que a interrupção 0 está associada ao pino 2 do microcontrolador.

  Serial.begin(115200);                     // Taxa de atualização da Serial.

}

void loop() {
Sensores();                                 // Void para realizar a leitura dos sensores e fazer a parada do sistema.

// ---- Loop do RF ------------------------------------------------------ Loop do RF ------------------------------------------------------ Loop do RF ----

    if (mySwitch.available()) {

    int value = mySwitch.getReceivedValue();
     if (value == 0){
     Serial.print("Codigo desconhecido");

     } else {

if (mySwitch.getReceivedValue() == 1200011) {                  // Comando para "SUBIR" o FLAP via RF. (Botão no controle A)
           tempobotao = millis();
           delay(250);                                         // Tempo em milesegundos para enviar novamente o sinal RF.
                Subir();
}

else if (mySwitch.getReceivedValue() == 1200012) {             // Comando para "PARAR" o FLAP via RF. (Botão no controle B)
           tempobotao = millis();
           delay(250);                                         // Tempo em milesegundos para enviar novamente o sinal RF.
         PararGeral();
 }   
 
else if (mySwitch.getReceivedValue() == 1200013) {             // Comando para "DESCER" o FLAP via RF. (Botão no controle C)
           tempobotao = millis();
           delay(250);                                         // Tempo em milesegundos para enviar novamente o sinal RF.
               Descer();
}             
}   
         
  Serial.print("Código RF recebido: ");
  Serial.println( mySwitch.getReceivedValue() );               // Informa o valor do codigo RF.  
  mySwitch.resetAvailable();

}
  
// ---- Loop do IR ------------------------------------------------------ Loop do IR ------------------------------------------------------ Loop do IR ----

  if (irrecv.decode(&results)) {
    if (results.value != 0xFFFFFFFF) {                                      // Ignora o código de repetição
    lastCode = results.value;                                               // Armazena o novo código
    Serial.print("Código IR recebido: ");                                      // Exibe na serial "Código recebido:"
    Serial.println(results.value, HEX);                                     // Exibe na serial o valor codigo do IR
    }
    armazenavalor = (results.value);

    if (armazenavalor == 0x202D02F || armazenavalor == 0x76B366E3)          // Comando para "SUBIR" o FLAP via IR.
    {
      tempobotao = millis();
      Subir();

    }
    else if (armazenavalor == 0x202708F || armazenavalor == 0x6EDFE961 )    // Comando para "DESCER" o FLAP via IR.
    {
      tempobotao = millis();
      Descer();

    }
    else if (armazenavalor == 0x923F150B || armazenavalor == 0x202B04F)     // Comando para "PARAR" o FLAP via IR.
    {
      tempobotao = millis();
      PararGeral();
    
    }
    irrecv.resume();                                                        // Le o próximo valor
}

// ---- Loop Serial ---------------------------------------------------- Loop Serial ---------------------------------------------------- Loop Serial ----
  
 // Usado apenas ao conectar o computador e executar pela IDE via conexão serial.
 if (Serial.available() > 0) {
 char letra = Serial.read();
   
   if (letra == 'a'){                       // Comando para "SUBIR" o FLAP via Serial.
   Subir();
   }
   
   else if (letra == 's'){                  // Comando para "DESCER" o FLAP via Serial.
   Descer();
   }
   
   else if (letra == 'd'){                  // Comando para "PARAR" o FLAP via Serial.
   PararGeral();
  } 
 }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------

// Usado para fazer o motor ligar no sentido de SUBIR o equipamento.
void Subir() {
    TAbrirFechar = millis();
    TBotao = millis();
    IgnoreFC = millis();                    // Inicia o temporizador para ignorar o fim de curso por 1125ms.
               
         AMotor = HIGH;                     // Desliga o Atuador Linear - Avançar/Abrir.                             
         RMotor = LOW;                      // Liga o Atuador Linear - Recuar/Fechar.                             
    
         digitalWrite(AvancarMotor, AMotor);    
         digitalWrite(RecuarMotor, RMotor);    

         TMotor = 1;

         motorEmMovimento = true;           // Indica que o motor está em movimento.
         tempoMotorAtivado = millis();      // Marca o tempo de ativação do motor.
            
         Serial.println("Comando para SUBIR o FLAP foi acionado");       
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------

// Usado para fazer o motor ligar no sentido de DESCER o equipamento.
void Descer() {
    TAbrirFechar = millis();
    TBotao = millis();
    IgnoreFC = millis();                    // Inicia o temporizador para ignorar o fim de curso por 1125ms.
         
         AMotor = LOW;                      // Liga o Atuador Linear - Avançar/Abrir.                             
         RMotor = HIGH;                     // Desliga o Atuador Linear - Recuar/Fechar.                              

         digitalWrite(AvancarMotor, AMotor);
         digitalWrite(RecuarMotor, RMotor);
         
         TMotor = 1;

         motorEmMovimento = true;           // Indica que o motor está em movimento.
         tempoMotorAtivado = millis();      // Marca o tempo de ativação do motor.

         Serial.println("Comando para DESCER o FLAP foi acionado");
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------

// Usado para parar toda a logica de funcionamento.
void PararGeral() {
         tempobotao = millis();

         AMotor = HIGH;                     // Desliga o Atuador Linear - Avançar/Abrir.                          
         RMotor = HIGH;                     // Desliga o Atuador Linear - Recuar/Fechar.                          
         
         digitalWrite(AvancarMotor, AMotor);
         digitalWrite(RecuarMotor, RMotor);
         
         TMotor = 0;

         motorEmMovimento = false;          // Para indicar que o motor não está mais em movimento.
         
         Serial.println("Comando para PARAR o FLAP foi acionado");
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------

// Usado para parar toda a logica de funcionamento atravez dos fim de curso.
void ParadaFC() {
         tempobotao = millis();

         AMotor = HIGH;                     // Desliga o Atuador Linear - Avançar/Abrir.                          
         RMotor = HIGH;                     // Desliga o Atuador Linear - Recuar/Fechar.                          
         
         digitalWrite(AvancarMotor, AMotor);
         digitalWrite(RecuarMotor, RMotor);
         
         TMotor = 0;

         motorEmMovimento = false;          // Para indicar que o motor não está mais em movimento.
         
         //Serial.println("Comando para PARAR o FLAP via Fim de Curso");
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------

void Sensores() { 
    
       // Ignora o funcionamento dos sensores de fim de curso por 1125ms após o acionamento de um comando.
 if (millis() - IgnoreFC > 1125) {

       // Verifica o estado do Fim de curso ABERTO.
  if (digitalRead(SensorAbre) == LOW ) {
       //Serial.println("Fim de curso ABERTO está acionado");
        ParadaFC(); // Para se o sensor de ABERTURA for acionado.
       //delay(800);               
  }

       // Verifica o estado do Fim de curso FECHADO.
  if (digitalRead(SensorFecha) == LOW ) {
       //Serial.println("Fim de curso FECHADO está acionado");
        ParadaFC(); // Para se o sensor de FECHAMENTO for acionado.
       //delay(800);               
   }

       // Verifica se o motor está ligado por mais de 50 segundos sem acionar nenhum fim de curso.
  if (motorEmMovimento && (millis() - tempoMotorAtivado >= 50000)) {
    PararGeral();
    Serial.println("Motor foi desligado automaticamente após 50 segundos sem atingir qualquer fim de curso.");
  
  }
 }
}
// ---- KB ------------------------------------------------------------------ KB ------------------------------------------------------------------ KB ----