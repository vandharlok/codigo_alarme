// Define sensor and LED pins
#define SonarPin A0
const int LedPin10 = 10;
const int LedPin11 = 11;




// Quanto mais próximo o objeto estiver do sensor, maior será a tensão, com isso, maior o valor retornado da funcao analogread, 
//com isso retornara um valor de tensao, ai devemos multiplicar por um valor de acordo com o datasheet
void setupADC() {
    //  A0 como entrada analógica (ADC0)
    ADMUX = (1 << REFS0); // Definir a referência de tensão para AVcc

    // Ativa o ADC e define o prescaler para 128  
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}
int readADC() {
    // Inicia a conversão, o qual define 1 para o ADSC, o qual definido e responsavel por iniciar a conversao 
    ADCSRA |= (1 << ADSC);

    // Espera a conversão ser concluída
    while (ADCSRA & (1 << ADSC));

    // Retornar o valor convertido
    return ADC;
}
float convertToDistance(int sensorValue) {
    // convertendo a leitura do sensor para voltage
    float voltage = sensorValue * (5.0 / 1023.0);

    
    // no datasheet diz que a saida do sensor eh ~4.9mV/cm, entao 
    float distance_cm = (voltage / 0.0049) * 2;

    return distance_cm;
  }
boolean IsRangeWithinLimit() {
    float averageValue = 0.0;

    for (int i = 0; i < 3; i++) {
        int sensorValue = readADC(); // Usar a função readADC
        float distance = convertToDistance(sensorValue);
        averageValue += distance;
    }
    averageValue /= 3;
    Serial.println("the value is : ");
    Serial.print(averageValue);
    Serial.println("\n");
    return averageValue > 50;
}
void showZeroInDisplay() {
    //desliga todos os segmentos antes de comecar
    PORTD &= B00000011; // Desliga os segmentos A, B, C, D, E, F, G, DP
    PORTB &= B11111100; // Common Cathode (pinos 8 e 9) desligados

    //mostra 0 no display
    PORTD |= B11111100; 
}
void showOneInDisplay(){
    //desliga todos os segmentos antes de comecar
    PORTD &= B00000011; // Desliga os segmentos A, B, C, D, E, F, G, DP
    PORTB &= B11111100; // Common Cathode (pinos 8 e 9) desligados

    //mostra 1 no display
    PORTD |= B11000000; 
  
  
  }
// Function to blink LED

void BlinkLed(int pin, unsigned long blinkInterval) {
  unsigned long previousMillis = 0;
  int ledState = LOW;

  while (true) { 
    //pega o tempo atual e seta um tempo de intervalo,compara esse intervalo com umas flags de tempo recebida por parametro,
    //para que o led fique piscando caso passe desse intervalo
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;

      //fazendo o led piscar alterando o estado dele
      ledState = (ledState == LOW) ? HIGH : LOW;
      if (ledState == HIGH) {
        PORTB |= (1 << PORTB3); // Turn on the LED (pin 10)
      } else {
        PORTB &= ~(1 << PORTB3); // Turn off the LED (pin 10)
      }
    }

    // Check for the condition to exit the loop
    if (IsRangeWithinLimit()) {
      // Turn off the LED before exiting
      PORTB &= ~(1 << PORTB3); // desliga o led (pin 11)
      break;
    }
  }
}

void setup() {
  setupADC();
  Serial.begin(1200);
  // O 1 << DDC0 desloca o bit 1 para a posição 0 (correspondente ao A0),
  //e a negação ~ inverte os bits, resultando em todos os bits como 1 exceto o bit 0. O operador AND com DDRC então limpa somente o bit 0, deixando os outros como estão.
  DDRC &= ~(1 << DDC0); // Configura o pino A0 (PC0) como entrada
  //pinMode(SonarPin, INPUT)
  DDRB |= (1 << DDB2)| (1 << DDB0) | (1 << DDB1)|(1 << DDB3); //define os pinos como output
  DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7); // Configura os pinos 2 a 7 como saídas

}

void loop() {
  boolean isWithinRange = IsRangeWithinLimit();
  if(isWithinRange) {
   // configura o pino B2 do microcontrolador como uma saída em nível alto,
    PORTB |= (1 << PB2); // Define o pino 10 para HIGH   
    showZeroInDisplay();
    PORTB &=  ~(1 << PB3); //define o pino 11 para low 
    delay(50);
    
  } else {
    PORTB &= ~(1 << PB2); //define o pino 10 para low 
    showOneInDisplay();
    PORTB |=  (1 << PB3);  // define o pino 11 para high
    BlinkLed(LedPin11, 200);
  }
}
