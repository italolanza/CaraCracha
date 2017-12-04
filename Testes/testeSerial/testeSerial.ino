void setup()
{
	Serial.begin(9600);
//	pinMode(pinoSD, OUTPUT);
	Serial.println("Insira o nome do arquivo e a chave:");
}

String chave;
String arquivo;

void loop()
{
	while(Serial.available() == 0){}
  arquivo = Serial.readString();
  while(Serial.available() == 0){}
	chave = Serial.readString();
		
		Serial.print("Nome do arquivo inserido: ");
		Serial.println(arquivo);
		Serial.print("Chave inserida: ");
		Serial.println(chave);		
	
}
