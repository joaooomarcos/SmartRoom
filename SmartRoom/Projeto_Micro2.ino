/*--------------------------------------------------------------------------------
 
 Projeto de Microcontroladores 2
 SMART ROOM
 Grupo: 
 - Carolina Calonassi
 - Joao Marcos Ribeiro Araujo
 - Maria Cecilia
 - Ricardo Hochman
 
 Sensores Utilizados:
   - Sensor de Presenca
   - Sensor de Chuva
   - Sensor de temperatura e umidade
   - Sensor de Luz (LDR)
 
---------------------------------------------------------------------------------*/

/*---------------- Inclusao de Bibliotecas --------------------------------------*/
#include <LCD.h>                // LCD
#include <LiquidCrystal_I2C.h>  // LCD I2c
#include <Keypad.h>             // Teclado Matricial
#include <dht.h>                // Sensor DHT11
#include <Wire.h>               // Comunicacao I2C
#include <EEPROM.h>             // Memoria EEPROM
/*-------------------------------------------------------------------------------*/

/*---------------- Definicoes de conexoes ---------------------------------------*/
// Do LCD para o I2C
#define I2C_ADDR       0x27  // Endereco I2C do LCD
#define BACKLIGHT_PIN  3     // Pino do BackLight
#define En_pin         2     // Pino do Enable de escrita do LCD
#define Rw_pin         1     // Pino de Escrita ou Leitura
#define Rs_pin         0     // Pino do Seletor do Registrador
#define D4_pin         4     // Pino do Dado 0
#define D5_pin         5     // Pino do Dado 1
#define D6_pin         6     // Pino do Dado 2
#define D7_pin         7     // Pino do Dado 3

// Teclado Matricial
#define R1  5  // Pino da Linha 1
#define R2  4  // Pino da Linha 2
#define R3  3  // Pino da Linha 3
#define R4  2  // Pino da Linha 4
#define C1  9  // Pino da Coluna 1
#define C2  8  // Pino da Coluna 2
#define C3  7  // Pino da Coluna 3
#define C4  6  // Pino da Coluna 4

// Buzzer
#define buzz 12 // Buzzer porta digital 12

// Sensor de Temperatura DHT11
#define DHTTYPE DHT11   // Tipo do sensor DHT (DHT11)
#define dht_dpin A1     // Pino DATA do Sensor ligado na porta Analogica A1

// Sensor de Luz (LDR)
#define LDR A0   // LDR ligado na porta Analogica A0

// Sensor de Chuva
#define rain 25  // Sensor de chuva ligado na porta digital 25

// Sensor de Presenca
#define presence 30 // Sensor de presenca na porta digital 30

// Ilumicao
#define light 49 // Rele de saida da iluminacao ligado na porta 49

// Ventilacao
#define fan 53   // Rele de saida da ventilacao ligado na porta 53

// Porta de entrada
#define door 45 // Rele de saida da porta ligado na porta 45

// Janela
#define window 41 // Rele de saida da janela ligado na porta 41

// Botao pra sair
#define push_Out 11 // Push button de saida, na porta 11

// LED de Funcionamento
#define ledFunc 13  // Led de indicacao de funcionamento na porta 13

// LED de Presenca
#define ledPresenc 28 // Led de indicacao de presenca
/*-------------------------------------------------------------------------------*/

/*---------------- Criacao das Variaveis ----------------------------------------*/
// Para o LCD
int lcd_row = 2;  // Quantidade de Linhas
int lcd_col = 16; // Quantidade de Colunas

// Para o Teclado Matricial
const byte ROWS = 4;          // Quantidade de Linhas
const byte COLS = 4;          // Quantidade de Colunas
char hexaKeys[ROWS][COLS] = { // Caracteres do Teclado na forma de matriz
  { '1' , '2' , '3' ,'A'  },
  { '4' , '5' , '6' ,'B'  },
  { '7' , '8' , '9' ,'C'  },
  { '#' , '0' , '*' , 'D' }};
byte rowPins[ROWS] = {R1, R2, R3, R4};  // Ligacao das linhas do teclado
byte colPins[COLS] = {C1, C2, C3, C4};  // Ligacao das colunas do teclado

// Sensor de Temperatura
float temperature;    // Variavel para a tempetura lida
float humidity;       // Variavel para a umidade lida
int tempControl = 25; // Variavel para o controle da temperatura

// Sensor de Luz
float lum;              // Variável para armazenar o valor da luminosidade 
int luminosity;         // Variável para a amostragem dos valores de luminosidade 
float samples;          // Amostras da leitura
float readSensor;       // Valor da leitura do sensor
int luminoControl = 50; // Varialvel para o controle da luminosidade

// Senha
char pass[4];            // Senha de 4 Digitos
boolean pass_ok = false; // Controle da Senha

boolean valor = false; 
boolean show = false;
/*-------------------------------------------------------------------------------*/

/*---------------- Inicializacoes -----------------------------------------------*/
// LCD I2C
LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

// Teclado Matricial
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// Sensor DHT11
dht DHT;
/*-------------------------------------------------------------------------------*/

/*---------------- Setup Inicial ------------------------------------------------*/
void setup(){
  // Configuracao dos Pinos como entrada ou saida
  pinMode(ledFunc,    OUTPUT); // LED - Indicacao funcionamento
  pinMode(fan,        OUTPUT); // LED e Rele - ventilador
  pinMode(light,      OUTPUT); // LED e Rele - lampada
  pinMode(door,       OUTPUT); // LED e Rele - Porta
  pinMode(window,     OUTPUT); // LED e Rele - Janela
  pinMode(ledPresenc, OUTPUT); // LED - Indicacao de Presenca
  pinMode(push_Out,   INPUT);  // Botao pra sair
  pinMode(presence,   INPUT);  // Sensor de Presenca

  // Para trocar e gravar a senha nova utilize este metodo
  // Faca as alteracoes necessarias para a nova senha
  //firstAccess(); 

  // Incializacao da comunicacao serial
  Serial.begin(9600);  // Baud Rate de 9600 
  
  // Inicializa o LCD
  lcd.begin(lcd_col, lcd_row);
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE); // Seta o pino de BACKLIGHT
  lcd.setBacklight(HIGH); // Liga o BackLight

  // Lendo a EEPROM que possui a senha
  for (int e = 0; e < 5; e++){
    pass[e] = EEPROM.read(e);
  }

  intro(); // Funcao inicial
}

void loop(){
  digitalWrite(door,HIGH); // Abre a porta
  
  while(1){ // Porta fica aberta ate detectar presenca
    if(digitalRead(presence) == HIGH) break;
  }

  digitalWrite(door,LOW); // Fecha a porta
  
  while(1){  // Loop para controle geral da sala
  
    // Get temperatura e umidade
    DHT.read11(dht_dpin);           // Faz leitura do sensor DHT
    temperature = DHT.temperature;  // Pega o valor da temperatura
    humidity = DHT.humidity;        // Pega o valor da umidade
    luminosity = get_luminosity();  // Pega o valor da luminosidade

    // LCD temperatura
    lcd.setCursor (0,0);      // Seta posição do cursor de escrita do LCD
    lcd.print("  Temp = ");          // Escreve no LCD
    lcd.print(temperature,1); // Escreve a temperatura com uma casa decimal no LCD
    //lcd.setCursor (6,0);    // Seta posição do cursor de escrita do LCD
    lcd.print("C");           // Escreve no LCD a letra C, de Celsius

    char customKey = customKeypad.getKey(); // Pega o que foi teclado
    if (customKey == 'D'){                  // Se foi teclado D
      if (show == false) show = true;       // Mostra Umidade
      else show = false;                    // Se nao mostra luminosidade
    }
    
    if (show == true){ // Mostra umidade
      // LCD Umidade
      lcd.setCursor (0,1);       // Seta posição do cursor de escrita do LCD
      lcd.print("  Umidade = "); // Escreve no LCD
      lcd.print(humidity,0);     // Escreve a umidade no LCD
      lcd.print("%");            // Escreve no LCD
    }
    else {  // Mostra luminosidade
      // LCD Luminosidade
      lcd.setCursor (0,1);     // Seta posição do cursor de escrita do LCD
      lcd.print("   Luz = ");  // Escreve no LCD
      lcd.print(luminosity);   // Escreve a luminosidade no LCD
      lcd.print("%    ");      // Escreve no LCD
    }

    // Controle da luminosidade
    if (luminosity <= luminoControl) // se a luminosidade estiver abaixo do valor estipulado
      digitalWrite(light, HIGH);     // liga a iluminacao(rele)
    else  // se nao
      digitalWrite(light, LOW);      // desliga a iluminacao(rele)
      
    // Controle Ventiladores
    if (temperature >= tempControl) // se a temperatura estiver abaixo do valor estipulado
      digitalWrite(fan, HIGH);      // liga os ventiladores (rele)
    else // se nao
      digitalWrite(fan, LOW);       // Desliga os ventiladores (rele)
    
    // Controle da Janela
    controlWindow();
    
    // Verificacao de presenca
    if (digitalRead(presence) == HIGH) 
      digitalWrite(ledPresenc, HIGH);
    else 
      digitalWrite(ledPresenc, LOW);
    
    // Pisca lED de funcionamento e tambem cria um delay para as novas leituras
    delay(200); 
    digitalWrite(13, HIGH); // Liga o led de funcionamento
    delay(200); 
    digitalWrite(13, LOW); // Desliga o led de funcionamento

    // Sair da Sala
    if (digitalRead(push_Out) == LOW) // Se pressionado o botao de saida
    {
      valor = true;
      intro();  // Reinicia o programa
    }
   
    // Exibe os valores de controle via serial
    Serial.print("\nTemperatura = ");
    Serial.print(temperature,1);
    Serial.print("C");
    Serial.print("\tUmidade = ");
    Serial.print(humidity,1);
    Serial.print("%");
    Serial.print("\t\tLuminosidade = ");
    Serial.print(luminosity,1);
    Serial.print("%");
  }
}

// Primeira Funcao
void intro() {
  valor = false;

  digitalWrite(fan, LOW);   // Desliga os ventiladores (rele)
  digitalWrite(light, LOW); // Desliga a iluminacao (rele)

  // Som de inicio
  tone(buzz,3000,400);
  delay(450);
  tone(buzz,3000,400);

  // LCD
  lcd.clear();             // Limpa o LCD
  lcd.setCursor(3,0);      // Seta o cursor
  lcd.print("Smart Room"); // Escreve no LCD
  
  // Escreve na serial
  Serial.println("SMART ROOM");
  
  delay(3000);  // Aguarda um instante
  
  lcd.setCursor(0,1);           // Seta o cursor
  lcd.print("Tecle p/ entrar"); // Escreve no LCD
  
  // Escreve na serial
  Serial.println("Tecle algo no teclado para entrar");
  
  delay(500); // Aguarda um instante

  // Loop para aguardar algo ser pressionado
  while (1) {  
    char customKey = customKeypad.getKey(); // Pega o que foi teclado
    if (customKey != NO_KEY)                // Se foi teclado algo
      break;                                // Sai do Loop
 
    delay(50);       // Aguarda um instante para verificar novamente
    controlWindow(); // Controla a Janela
  }
  
  lcd.clear(); // Limpa o LCD
  
  // Loop aguarda senha correta
  while(1){    
    pass_ok = password(); // Verifica se a senha digita esta correta
    if (pass_ok)          // Se sim
      break;              // Sai do loop
    controlWindow();      // Controla a Janela
  }
}

// Verificacao da Senha
boolean password(){
  int count = 0;   // Variavel para quantidade de digitos certos da senha
  int entrada = 0; // variável de apoio para o numero de entradas feita pelo teclado
  
  // LCD
  lcd.print("     Senha:"); // Escreve no LCD
  lcd.setCursor ( 6, 2 );   // Seta a posicao do cursor
  
  // Loop aguarda os quatro numeros da senha
  while (count < 4 ){ 
    char key = customKeypad.getKey(); // obtém informação do teclado
    if (key != NO_KEY){               // se foi teclado algo
      entrada += 1;                   // aumenta contador de entradas
      lcd.print("*");                 // Escreve * no LCD
      // Escreve na serial
      Serial.println("Digitando Senha");
      tone(buzz,3500,100);             // Beep no buzzer
      
      delay(200);                     
      
      if (key == pass[count]) // Verifica na sequencia da senha
        count  += 1;          // Se correto aumenta o contador
        
      if ( count == 4 ) { // Apos os quatro digitos corretos da senha
        //LCD
        lcd.clear();             // Limpa o LCD
        lcd.setCursor(3,0);      // Seta o cursor
        lcd.print("Smart Room"); // Escreve no LCD
        lcd.setCursor (3, 1);    // Seta o cursor
        lcd.print("Bem vindo!"); // Escreve no LCD
        
        // Escreve na serial
        Serial.println("Senha correta");
        
        valor = false;
        
        // Beep no Buzzer
        tone(buzz,2800,300);
        delay(330);
        tone(buzz,2800,300);
        delay(2000);
        
        lcd.clear(); // Limpa o lCD
        return true; // Retorna true pois a senha esta correta
        break;       // sai do loop
      }
      
      if(entrada == 4 && count != 4){ // Quatro entradas, e digitos incorretos da senha
        count = 0;  // Zera o contador
        
        //LCD
        lcd.clear();                 // Limpa o LCD
        lcd.print("     Senha:");    // Escreve no LCD
        lcd.setCursor ( 1, 2 );      // Seta o cursor do LCD
        lcd.print("Senha Invalida"); // Escreve no LCD
        
        // Escreve na serial
        Serial.println("Senha incorreta");
        
        // Beep Buzzer
        tone(buzz,210,900);
        delay(2000);
        
        lcd.clear();  // Limpa o LCD
        return false; // Retorna falso, pois a senha esta incorreta
        break;        // sai loop
      }
      controlWindow(); // Controla a janela
    }
  }
}

// Leitura da Luminosidade
float get_luminosity(){
  samples = 0;    // Amostras
  readSensor = 0; // Valor de leitura
  
  // Amostragem
  for(int i=0; i < 50 ; i++)
  {   
    readSensor = analogRead(LDR);            // Armazena o valor lido pelo LDR   
    readSensor = (readSensor / 1024) * 100;  // Transforma o valor lido em porcentagem   
    samples = samples + readSensor;          // Armazena na variável amostra   
  }  
  return 100 - (samples/50);   // Retorna o valor lido apos a amostragem
}

// Controle da Janela
void controlWindow(){
  if (digitalRead(rain) == HIGH) digitalWrite(window, LOW);
  else digitalWrite(window,HIGH);
}

// Primeiro Acesso
// Este metodo e para a troca da senha, devendo ser executado apenas uma vez e depois retirado
void firstAccess() {
  char newPass[4] = {'7', '6', '1', '0'  };   // Nova Senha
  for (int p = 0; p < 5; p++) 
    EEPROM.write(p,newPass[p]);  // Salva na EEPROM a nova senha
}
