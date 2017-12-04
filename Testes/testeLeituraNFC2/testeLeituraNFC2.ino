#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

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

	if(nfc.tagPresent()) 
  {
  	imprimirInformacoesTAG();
  }
}

void imprimirInformacoesTAG()
{
	NfcTag tag = nfc.read();
	/*
	Serial.print("Tipo de TAG: ");
	Serial.println(nfc.getTagType());
	Serial.print("Tamanho da UID: ");
	Serial.println(nfc.getUidLength());	
	Serial.print("UID da TAG: ");
	Serial.println(nfc.getUidString());
	*/
	
	tag.print();
}