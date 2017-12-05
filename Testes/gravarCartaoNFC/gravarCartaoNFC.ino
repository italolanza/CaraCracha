/*
  acessoNFC - arquivo testa a liberação de acesso utilizando um cartão NFC. 
  Verifica o código e tipo de acesso.
*/
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
// ----------------------------- ~ Definição dos Objetos ~ -------------------------------//
PN532_I2C pn532i2c(Wire); //identifica o módulo NFC
NfcAdapter nfc(pn532i2c); //objeto para manipular o módulo
NfcTag newTag; //objeto utilizado para manipular o cartão lido
NdefMessage conteudoMensagem; //objeto para manipular as Mensagens Ndef
// ----------------------------- ~ Definição das Constantes ~ -------------------------------//
const int thiYear   = 2017;
const int thisMonth = 12;
const int today     = 4;
// ----------------------------- ~ Definição das funções a serem utilizadas ~ -------------------------------//
void      imprimirInformacoesTAG(); //imprime as informações no cartão
void      formatarTAG(); //formata Tag no formato NdefMessage
int       limparTAG(); //limpa a Tag inserindo um record vazio
String    getIdentificationCode(NdefRecord record);
String    getAcessType(NdefRecord record);
int       getDay(NdefRecord record);
int       getMonth(NdefRecord record);
int       getYear(NdefRecord record);
int       getCountLeft(NdefRecord record);
void      negarAcesso();
void      liberarAcesso();
int       atualizarCartao(int _countLeft);
int       testarCredencial(String ID);
int       compararDatas(int _day, int _month, int _year);
int       gravarCartaoCompleto(String _ID);
int       gravarCartaoTemporario(String _ID, int _day, int _month, int _year);
int       gravarCartaoLimitado(String _ID, int n);
// ----------------------------- ~ Variaveis utilizads ~ -------------------------------//
String nfcAcessType = "";
String nfcIndentificationCode = "";
//char nfcInputDay[2];
//char nfcInputMonth[2];
//char nfcInputYear[4];
int    inputDay;
int    inputMonth;
int    inputYear;
int    countLeft;
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void setup()
{
    Serial.begin(115200);    
    nfc.begin();
    Serial.println("Modulo NFC iniciado com sucesso!");
    //formatarTAG();
}

void loop()
{
  String tipo = "";
  Serial.println("Selecione o tipo de cartao a ser gravado: ");
  Serial.println("00. Accesso completo;");
  Serial.println("01. Accesso temporario (valido até a data indicada);");
  Serial.println("10. Accesso limitado (numero finito de vezes).");

  while (Serial.available() == 0){/*nao faz nada, aguarda por uma entrada do serial*/}
  tipo = Serial.readString();

  Serial.print("Tipo escolhido: ");
  Serial.println(tipo);

  Serial.println("Aproxime o cartao NFC próximo a regiao indicada...");  
  while(!(nfc.tagPresent())) {/**/} //aguarda o cartão   
  Serial.println("Cartao NFC detectado!");

  if(tipo == "00")
  {
      if(gravarCartaoCompleto("00010000"))
          Serial.println("Cartao gravado com Acesso Completo");
  }
  else if(tipo == "01")
  {
      if(gravarCartaoTemporario("00010000", 4, 12, 2017))
        Serial.println("Cartao gravado com Acesso Temporario");

  }
  else if(tipo == "10")
  {
      if(gravarCartaoLimitado("00010000", 2))
        Serial.println("Cartao gravado com Acesso Limitado");
  }
  else
  {
      Serial.println("Opcao invalida!");
  }

  delay(5000);

  Serial.println("-----------------------------------------------------------------------");
  Serial.println("Aproxime o cartao NFC próximo a regiao indicada...");  
  while(!(nfc.tagPresent())) {/**/} //aguarda o cartão   
  Serial.println("Cartao NFC detectado!");
  newTag = nfc.read();
  imprimirInformacoesTAG(newTag);

  /*
  Serial.println("##################################################");  
  Serial.println("##################################################");  

  Serial.println("Aproxime o cartao NFC próximo a regiao indicada...");  
  //while(!(nfc.tagPresent())) {} //aguarda o cartão   
  Serial.println("Cartao NFC detectado!");
  */

  delay(5000);
}
// --------------------Funções Utilizadas--------------------//--------------------Funções Utilizadas--------------------////
void imprimirInformacoesTAG(NfcTag tag)
{
  if (nfc.tagPresent())
  {
    Serial.println(tag.getTagType());
    Serial.print("UID: ");Serial.println(tag.getUidString()); // Retrieves the Unique Identification from your tag
     if (tag.hasNdefMessage()) // If your tag has a message
    {

      NdefMessage message = tag.getNdefMessage();
      Serial.print("\nThis Message in this Tag is ");
      Serial.print(message.getRecordCount());
      Serial.print(" NFC Tag Record");
      if (message.getRecordCount() != 1) { 
        Serial.print("s");
      }
      Serial.println(".");

      // If you have more than 1 Message then it wil cycle through them
      int recordCount = message.getRecordCount();
      for (int i = 0; i < recordCount; i++)
      {
        Serial.print("\nNDEF Record ");Serial.println(i+1);
        NdefRecord record = message.getRecord(i);

        int payloadLength = record.getPayloadLength();
        byte payload[payloadLength];
        record.getPayload(payload);


        String payloadAsString = ""; // Processes the message as a string vs as a HEX value
        for (int c = 0; c < payloadLength; c++) {
          payloadAsString += (char)payload[c];
        }
        Serial.print("  Information (as String): ");
        Serial.println(payloadAsString);


        String uid = record.getId();
        if (uid != "") {
          Serial.print("  ID: ");Serial.println(uid); // Prints the Unique Identification of the NFC Tag
        }
      }
    }
  }
  else
    Serial.println("Cartao NFC nao detectado!");  
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void formatarTAG()//formata o cartão no formato NDEFMessagem com um record vazio
{
    Serial.println("Mantenha a TAG proxima ao sensor.");
    while(!(nfc.tagPresent())) {/**/} //aguarda o cartão   
    if(nfc.tagPresent())
    { 

        Serial.println("Formatando TAG....!");
        if(nfc.format())
        {
            Serial.println("TAG formatada com sucesso!");
        }
        else
        {
            Serial.println("Erro ! TAG nao pode ser formatada!");
        }

    }
    else
    {
        Serial.println("TAG nao esta presente!");
    }
    delay(2000);
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int limparTAG()
{
    if(nfc.tagPresent())
    { 
        Serial.println("Mantenha a TAG proxima ao sensor.");
        Serial.println("Limpando TAG....!");
        if(nfc.erase())
        {
            Serial.println("TAG limpa com sucesso!");
            return 1;
        }
        else
        {
            Serial.println("Erro ! TAG nao pode ser limpa!");     
            return 0;
        }

    }
    else
    {
        Serial.println("TAG nao esta presente!");
        return 0;
    } 
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
String    getIdentificationCode(NdefRecord record)
{
    String ID = "";
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    for (int c = 0; c < payloadLength; c++) 
    {
          ID += (char)payload[c];
    }
    return ID;
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
String    getAcessType(NdefRecord record)
{
    String accessCode = "";
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    for (int c = 0; c < payloadLength; c++) 
    {
          accessCode += (char)payload[c];
    }
    return accessCode; 
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       getDay(NdefRecord record)
{
    char day[record.getPayloadLength()];
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    for (int c = 0; c < 2; c++) 
    {
          day[c] = (char)payload[c];
    }
    return atoi(day);
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       getMonth(NdefRecord record)
{
    char month[record.getPayloadLength()];
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    for (int c = 3; c < 5; c++) 
    {
          month[c] = (char)payload[c];
    }
    return atoi(month);
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       getYear(NdefRecord record)
{
    char year[record.getPayloadLength()];
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    for (int c = 6; c < payloadLength; c++) 
    {
          year[c] = (char)payload[c];
    }
    return atoi(year);
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       getCountLeft(NdefRecord record)
{
    char count[record.getPayloadLength()];
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    for (int c = 0; c < payloadLength; c++) 
    {
          count[c] = (char)payload[c];
    }
    return atoi(count);   
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void      negarAcesso() //TODO: Fazer lógica de acionamento
{
    Serial.println("Acesso negado!");  
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void      liberarAcesso()//TODO: Fazer lógica de acionamento
{
    Serial.println("Acesso liberado");
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       atualizarCartao(int _countLeft)
{
    Serial.print("Quantidade de acessos restantes: ");
    Serial.println(_countLeft);
    if(limparTAG())
    {
        if (nfc.tagPresent()) 
        {
            NdefMessage message = NdefMessage();
            message.addTextRecord(nfcIndentificationCode); // Codigo de Identificação
            message.addTextRecord(nfcAcessType);  // Tipo de Acesso
            message.addTextRecord(String(_countLeft));  // Acessos Restantes

            Serial.println("#####################################################");
            Serial.println("Mensagem gravada:");
            message.print();
            Serial.println("#####################################################");

            boolean success = nfc.write(message);

            if (success) 
            {
                Serial.println("Cartao atualizado com sucesso!"); // if it works you will see this message 
                return 1;
            } 
            else 
            {
                Serial.println("Atualizacao falhou!"); // If the the rewrite failed you will see this message
                return 0;
            }
        }
        else
        {
            return 0;          
        }
    }
    else
    {
        return 0;
    }
    

}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       testarCredencial(String ID) 
{
    if (ID.equals("00010000")) //TODO: Implementar lógica certa
      return 1;
    else
      return 0;
    
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       compararDatas(int _day, int _month, int _year)
{
    if(_year >= thiYear)//TODO: Implementar lógica certa
    {
        if(_month >= thisMonth )
        {
            if(_day >= today)
            {
                return 1;
            }
            else
            {
              return 0;
            }
        }
        else
        {
          return 0;
        }
    }
    else
    {
      return 0;
    }
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       gravarCartaoCompleto(String _ID)
{
    if(limparTAG())
    {
        if (nfc.tagPresent()) 
        {
            NdefMessage message = NdefMessage();
            message.addTextRecord(_ID); // Codigo de Identificação
            message.addTextRecord("00");  // Tipo de Acesso
            //message.addTextRecord(String(_countLeft));  // Acessos Restantes

            Serial.println("#####################################################");
            Serial.println("Mensagem gravada:");
            message.print();
            Serial.println("#####################################################");

            boolean success = nfc.write(message);

            if (success) 
            {
                Serial.println("Cartao gravado com sucesso!"); // if it works you will see this message 
                return 1;
            } 
            else 
            {
                Serial.println("Gravacao falhou!"); // If the the rewrite failed you will see this message
                return 0;
            }
        }
        else
        {
            return 0;          
        }
    }
    else
    {
        return 0;
    }
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       gravarCartaoTemporario(String _ID, int _day, int _month, int _year)
{
    if(limparTAG())
    {
        if (nfc.tagPresent()) 
        {
            NdefMessage message = NdefMessage();
            message.addTextRecord(_ID); // Codigo de Identificação
            message.addTextRecord("01");  // Tipo de Acesso
            message.addTextRecord(String(_day)+"/"+String(_month)+"/"+String(_year));  // Dia/Mes/Ano
           // message.addTextRecord();  // 
            //message.addTextRecord();  //

            Serial.println("#####################################################");
            Serial.println("Mensagem gravada:");
            message.print();
            Serial.println("#####################################################");

            boolean success = nfc.write(message);

            if (success) 
            {
                Serial.println("Cartao gravado com sucesso!"); // if it works you will see this message 
                return 1;
            } 
            else 
            {
                Serial.println("Gravacao falhou!"); // If the the rewrite failed you will see this message
                return 0;
            }
        }
        else
        {
            return 0;          
        }
    }
    else
    {
        return 0;
    }
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int       gravarCartaoLimitado(String _ID, int n)
{
    if(limparTAG())
    {
        if (nfc.tagPresent()) 
        {
            NdefMessage message = NdefMessage();
            message.addTextRecord(_ID); // Codigo de Identificação
            message.addTextRecord("10");  // Tipo de Acesso
            message.addTextRecord(String(n));  // Acessos Restantes

            Serial.println("#####################################################");
            Serial.println("Mensagem gravada:");
            message.print();
            Serial.println("#####################################################");

            boolean success = nfc.write(message);

            if (success) 
            {
                Serial.println("Cartao gravado com sucesso!"); // if it works you will see this message 
                return 1;
            } 
            else 
            {
                Serial.println("Gravacao falhou!"); // If the the rewrite failed you will see this message
                return 0;
            }
        }
        else
        {
            return 0;          
        }
    }
    else
    {
        return 0;
    }
    
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//