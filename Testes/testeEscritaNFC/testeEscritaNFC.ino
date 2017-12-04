#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);


void setup(void)
{
  Serial.begin(115200);
  Serial.println("*** Teste LEITURA Modulo PN532 NFC RFID ***");

  nfc.begin();

  //Verifica a conexao do modulo PN532
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata)
  {
    Serial.print("Placa PN53x nao encontrada...");
    while (1); // halt
  }

  //Conexao ok, mostra informacoes do firmware
  Serial.print("Encontrado chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware versao: "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Aguardando cartao ISO14443A...");
  Serial.println("");
}
void loop()
{
	boolean success;
	
}

void escreverTAG()
{

}