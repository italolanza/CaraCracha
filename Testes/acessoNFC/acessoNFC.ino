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
int       formatarTAG(); //formata Tag no formato NdefMessage
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
    
}

void loop()
{
  Serial.println("Aproxime o cartao NFC próximo a regiao indicada...");  
  while(!(nfc.tagPresent())) {/**/} //aguarda o cartão   
  Serial.println("Cartao NFC detectado!");

  newTag = nfc.read(); // le/guarda informações lidas do cartão no sensor
  if (newTag.hasNdefMessage())
  {
      conteudoMensagem = newTag.getNdefMessage(); //pega mensagem recebida do cartão
      if(conteudoMensagem.getRecordCount() >= 2)
      {
          nfcIndentificationCode = getIdentificationCode(conteudoMensagem.getRecord(0));
          if(testarCredencial(nfcIndentificationCode) != 1)
          {        
              nfcAcessType = getAcessType(conteudoMensagem.getRecord(1));
              if(nfcAcessType.equals("01")) //acesso datado
              {
                    if(conteudoMensagem.getRecordCount() == 3)
                    {              
                        inputDay    = getDay(conteudoMensagem.getRecord(3));
                        inputMonth  = getMonth(conteudoMensagem.getRecord(3));
                        inputYear   = getYear(conteudoMensagem.getRecord(3));

                        if(compararDatas(inputDay, inputMonth, inputYear) == 1)
                        {
                            Serial.println("Acesso liberado");
                            liberarAcesso();
                            //TODO: Liberar o acesso a porta
                        }
                        else
                        {
                            Serial.println("Acesso expirado. Procurar a segunça!");
                            negarAcesso();
                            //TODO : Escrever código que 
                        }
                    }
                    else
                    {
                        Serial.println("Cartao gravado incorretamente. Procurar a segunça!");
                    }      
              }
              else if(nfcAcessType.equals("10")) //acesso limitado de vezes
              {
                    countLeft = getCountLeft(conteudoMensagem.getRecord(3));
                    if(countLeft > 0)
                    {
                        Serial.println("Acesso liberado");
                        countLeft = countLeft - 1;
                        atualizarCartao(countLeft);
                        liberarAcesso();
                    }
                    else
                    {
                        Serial.println("Acesso expirado. Procurar a segunça!");
                        negarAcesso();
                    }
              }
              else //acesso ilimitado de vezes
              {  
                  //Serial.println("Acesso liberado");
                  liberarAcesso();
              }
          }
          else
          {
              //Serial.println("Acesso negado!");  
              negarAcesso();
          }
      }
      else
      {
          Serial.println("Cartao gravado incorretamente. Procurar a segunça!");
      }
  }
  else
  {
      Serial.println("Cartao NFC vazio, insira um cartao valido!");
  }

  imprimirInformacoesTAG(newTag);
  


  delay(3000);
}
// --------------------Funções Utilizadas--------------------//--------------------Funções Utilizadas--------------------////
void imprimirInformacoesTAG(NfcTag tag)
{
  if (nfc.tagPresent())
  {
    NfcTag _tag = tag;
    Serial.println(_tag.getTagType());
    Serial.print("UID: ");Serial.println(_tag.getUidString()); // Retrieves the Unique Identification from your tag
  }
  else
    Serial.println("Cartao NFC nao detectado!");  
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int formatarTAG()//formata o cartão no formato NDEFMessagem com um record vazio
{
    if(nfc.tagPresent())
    { 
        Serial.println("Mantenha a TAG proxima ao sensor.");
        Serial.println("Formatando TAG....!");
        if(nfc.format())
        {
            Serial.println("TAG formatada com sucesso!");
            return 1;
        }
        else
        {
            Serial.println("Erro ! TAG nao pode ser formatada!");     
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
    for (int c = 0; c < payloadLength; c++) 
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
    for (int c = 0; c < payloadLength; c++) 
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
    for (int c = 0; c < payloadLength; c++) 
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
void      negarAcesso()
{
    Serial.println("Acesso negado!");  
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void      liberarAcesso()
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