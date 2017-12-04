#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <SD.h>
#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

PN532_I2C pn532i2c(Wire);
NfcAdapter nfc(pn532i2c);

void setup()
{
	Serial.begin(115200);
  	Serial.println("Teste Leitura Cartao NFC");  
  	nfc.begin(); 	
	
}

void loop()
{
	/*
	Lógica:
			- Aguardar algum cartão NFC
			- Receber dados do cartão
			- Checar no cartão SD se é um cartao válido (UID válido)
			- Se sim checar o tipo
				- Se for full time:
					- Liberar acesso (acionar motor e gravar data e hora do acesso)
				- Se for temporario
					- Checar comparar data e hora que tem no cartão com a do dia
						- Se tiver tudo OK, então liberar acesso
						- Caso contrário, não liberar e mandar mensagem de erro
				- Se for de contagem
					- Checar o contador
						- Se o contador for maior que zero, então esta ok. Liberar acesso e diminuir contador
						- Se for igual a zero então não liberar
	*/
}