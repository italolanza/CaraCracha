/*
  gravarCartaoNFC - arquivo grava no cartao o tipo de acesso escolhido e depois faz a leitura um cartão NFC. 
*/
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <Time.h>
// ----------------------------- ~ Definição dos Objetos ~ ------------------------------------//
  // -------------------------------------------------------------------------------------//
PN532_I2C pn532i2c(Wire); //identifica o módulo NFC
NfcAdapter nfc(pn532i2c); //objeto para manipular o módulo
//NfcTag newTag; //objeto utilizado para manipular o cartão lido
NdefMessage conteudoMensagem; //objeto para manipular as Mensagens Ndef
File arquivoAberto; // variavel arquivo para leitura no cartão micro SD
RTC_DS1307 RTC; //objeto RTC
// ----------------------------- ~ Definição das Constantes ~ -------------------------------//
  // -------------------------------------------------------------------------------------//
const String controleAcesso     = "entries.txt";
const String chaveAcessos       = "keys.txt";
const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// ----------------------------- ~ Definição das funções a serem utilizadas ~ -------------------------------//
      // -------------------------------------------------------------------------------------//
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
int       procurarChave(String key, String fileName); // procura em um arquivo o valor
int       criarArquivo(String name); // cria uma arquivo no cartão SD dado um nome
int       abrirArquivo(String name); // procura e abre um arquivo no cartão SD dado um nome
void      fecharArquivo(); // fecha o arquivo aberto
int       escreverArquivo(char data[], String fileName);
void      imprimeArquivo(String nomeArquivo); // imprime o que tiver no arquivo aberto
int       gravarAcesso(String id); //registra data e hora de quando há um acesso autorizado
void      imprimirAcessos(); //imprime todos os acessos registrados no arquivo de registros
// ------------------------------- ~ Variaveis utilizads ~ ------------------------------------//
    // -------------------------------------------------------------------------------------//
String    nfcAcessType = "";
String    nfcIndentificationCode = "";
//char    nfcInputDay[2];
//char    nfcInputMonth[2];
//char    nfcInputYear[4];
int       inputDay;
int       inputMonth;
int       inputYear;
int       countLeft;
int       thisYear;
int       thisMonth;
int       today;
String    chave;
String    arquivo;
//DateTime  tempo;
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void setup()
{
    Serial.begin(115200);  
    nfc.begin();
    Serial.println("Modulo NFC iniciado com sucesso!");    
    if (!(SD.begin()))
    {
      Serial.println("## Erro: Cartao SD nao pode ser inicializado! ##");
      while (1);
    }
    Serial.println("Modulo cartao SD iniciado com sucesso!");
    if (! RTC.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
    }    
    Serial.println("Modulo RTC iniciado com sucesso!");
    //RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

    DateTime d  = RTC.now();
    thisYear     = (int)d.year();
    thisMonth   = (int)d.month();
    today       = (int)d.day();
    Serial.print(today);
    Serial.print("/");
    Serial.print(thisMonth);
    Serial.print("/");
    Serial.println(thisYear);

}

void loop()
{
  Serial.println("Aproxime o cartao NFC próximo a regiao indicada...");  
  while(!(nfc.tagPresent())) {/**/} //aguarda o cartão   
  Serial.println("Cartao NFC detectado!");

  NfcTag newTag = nfc.read(); // le/guarda informações lidas do cartão no sensor
  String UID = newTag.getUidString();
  if (newTag.hasNdefMessage())
  {
      conteudoMensagem = newTag.getNdefMessage(); //pega mensagem recebida do cartão
      if(conteudoMensagem.getRecordCount() >= 2)
      {
          nfcIndentificationCode = getIdentificationCode(conteudoMensagem.getRecord(0));
          if(testarCredencial(nfcIndentificationCode) != 1)
          {    
              Serial.println("Cartao com identificador valido!");    
              nfcAcessType = getAcessType(conteudoMensagem.getRecord(1));
              //Serial.print("nfcAcessType: ");
              //Serial.println(nfcAcessType);    
              if(nfcAcessType.equals("01")) //acesso datado
              {
                    Serial.println("Cartao tipo temporario!");
                    if(conteudoMensagem.getRecordCount() == 3)
                    {              
                        inputDay    = getDay(conteudoMensagem.getRecord(2));
                        inputMonth  = getMonth(conteudoMensagem.getRecord(2));
                        inputYear   = getYear(conteudoMensagem.getRecord(2));
                        //Serial.print(inputDay);
                        //Serial.print("/");
                        //Serial.print(inputMonth);
                        //Serial.print("/");
                        //Serial.println(inputYear);

                        if(compararDatas(inputDay, inputMonth, inputYear) == 1)
                        {
                            Serial.println("Acesso liberado");                            
                            liberarAcesso();
                            if(gravarAcesso(UID) == 1)
                            {
                                Serial.println("Registro de acesso feito com sucesso!");
                            }
                            else
                            {
                                Serial.println("Erro no registro do acesso!");
                            }
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
                    Serial.println("Cartao tipo limitado!");
                    countLeft = getCountLeft(conteudoMensagem.getRecord(2));
                    Serial.print("countLeft: ");
                    Serial.println(countLeft);
                    if(countLeft > 0)
                    {
                        Serial.println("Acesso liberado");
                        countLeft = countLeft - 1;
                        atualizarCartao(countLeft);
                        if(gravarAcesso(UID) == 1)
                        {
                            Serial.println("Registro de acesso feito com sucesso!");
                        }
                        else
                        {
                            Serial.println("Erro no registro do acesso!"); 
                        }
                        liberarAcesso();
                        //TODO: Liberar o acesso a porta
                    }
                    else
                    {
                        Serial.println("Acesso expirado. Procurar a segunça!");
                        negarAcesso();
                    }
              }
              else //acesso ilimitado
              {  
                  Serial.println("Cartao tipo ilimitado!");
                  if(gravarAcesso(UID) == 1)
                  {
                      Serial.println("Registro de acesso feito com sucesso!");
                  }
                  else
                  {
                      Serial.println("Erro no registro do acesso!"); 
                  }
                  liberarAcesso();//TODO: Liberar o acesso a porta
              }
          }
          else
          {
              Serial.println("Acesso negado!");  
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
  //imprimirInformacoesTAG(newTag);
  imprimirAcessos();
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
    for (int c = payloadLength-2; c < payloadLength; c++) 
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
    for (int c = payloadLength-2; c < payloadLength; c++) 
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
    for (int c = 3; c < 5; c++) 
    {
          day[c] = (char)payload[c];
    }
    char d[2];
    d[0] = day[3];
    d[1] = day[4];
    return atoi(d);
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
    for (int c = 6; c < 8; c++) 
    {
          month[c] = (char)payload[c];
    }
    char m[2];
    m[0] = month[6];
    m[1] = month[7];
    return atoi(m);
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
    for (int c = 9; c < payloadLength; c++) 
    {
          year[c] = (char)payload[c];
    }
    char y[4];
    y[0] = year[9];
    y[1] = year[10];
    y[2] = year[11];
    y[3] = year[12];
    return atoi(y);
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
    //Serial.print("payloadLength: ");
    //Serial.println(payloadLength);
    //Serial.println("count[c]: ");
    for (int c = 3; c < payloadLength; c++) 
    {
          count[c] = (char)payload[c];
          //Serial.println(count[c]);
          //Serial.println(c);
    }
    return atoi(&count[3]);   
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

            //Serial.println("#####################################################");
            //Serial.println("Mensagem gravada:");
            //message.print();
            //Serial.println("#####################################################");

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
   if(_year >= thisYear)
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
int criarArquivo(String name)
{
  if (SD.exists(name))
  {
    Serial.println("## Erro: Arquivo com nome duplicado! ##");
    return 0;
  }
  else
  {
    arquivoAberto = SD.open(name, FILE_WRITE);
    Serial.println("Arquivo criado com sucesso!");
    fecharArquivo();
    return 1;
  }
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int abrirArquivo(String name)
{
  if (!(SD.exists(name)))
  {
    Serial.print("## Erro: Arquivo com o nome - ");
    Serial.print(name);
    Serial.println(" - não existe! ##");
    return 0;
  }
  else
  {
    arquivoAberto = SD.open(name);
    if (arquivoAberto)
    {
      Serial.println("Arquivo aberto com sucesso!");
      return 1;
    }
    else
    {
      Serial.println("## Erro: Arquivo nao pode ser aberto! ##");
      return 0;
    }
  }
}

// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void fecharArquivo()
{
  arquivoAberto.close();
  Serial.println("Arquivo fechado!");
}

int escreverArquivo(char data[], String fileName)
{
  arquivoAberto = SD.open(fileName, FILE_WRITE);
  if (arquivoAberto)
  {
    Serial.println("Arquivo aberto com sucesso para escrita!");
    Serial.println("Escrevendo no arquivo");
    arquivoAberto.write(data);
    fecharArquivo();
    return 1;
  }
  else
  {
    Serial.println("## Erro: Arquivo nao pode ser aberto! ##");
    fecharArquivo();
    return 0;
  }
}

// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void imprimeArquivo(String nomeArquivo)
{
  abrirArquivo(nomeArquivo);
  Serial.println("Imprimindo arquivo!");
  while (arquivoAberto.available())
  {
    Serial.write(arquivoAberto.read());
  }
  Serial.println("<- Fim do arquivo ->");
  fecharArquivo();
}

// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// procura em um arquivo o valor
int procurarChave(String key, String fileName)
{
  arquivoAberto = SD.open(fileName, FILE_READ);
  if (arquivoAberto)
  {
    while (arquivoAberto.available())
    {
      // length()
      String input = arquivoAberto.readStringUntil('#');
      // Serial.print("Input length:");
      // Serial.println(input.length());
      // Serial.print("Input:");
      // Serial.println(input);
      // Serial.println("#");
      if (input.equals(key))
      {
        // Serial.println("Its a match!");
        arquivoAberto.close();
        return 1;
      }
    }
    arquivoAberto.close();
    return 0;
  }
  else
  {
    arquivoAberto.close();
    Serial.println("Arquivo nao pode ser aberto ou nao foi encontrado!");
    return 0;
  }
}

// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
int gravarAcesso(String id)
{
  String mensagem  = "";
  DateTime tempo   = RTC.now();
  mensagem  =   id+","
                +String(daysOfTheWeek[tempo.dayOfTheWeek()])+","
                +String(tempo.day(), DEC)+","
                +String(tempo.month(), DEC)+","
                +String(tempo.year(), DEC)+","
                +String(tempo.hour(), DEC)+","
                +String(tempo.minute(), DEC);

  //Serial.println("Mensagem:");
  //Serial.print(mensagem);
  char data[mensagem.length()];
  mensagem.toCharArray(data, mensagem.length());

/*  Serial.println("Mensagem Array:");
  Serial.print(data);*/

  File registro = SD.open(controleAcesso, FILE_WRITE);
  if(registro)
  {
    Serial.println("Arquivo aberto com sucesso para escrita!");
    Serial.println("Escrevendo no arquivo");
    registro.println(data);
    registro.close();
    //fecharArquivo();
    return 1;
  }
  else
  {
    Serial.println("## Erro: Arquivo nao pode ser aberto! ##");
    registro.close();
    //fecharArquivo();
    return 0;
  }
  /*char data[mensagem.length()];
  //mensagem.toCharArray(data, mensagem.length());
  if(escreverArquivo(data, controleAcesso) == 1)
  {
    return 1;
  }
  else
  {
    return 0;
  }*/
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void imprimirAcessos()
{
  File registro = SD.open(controleAcesso, FILE_READ);
  if(registro)
  {
    Serial.println("Imprimindo registro de acessos!");
    Serial.println("###################################################################");
    while (registro.available())
    {
      Serial.println(registro.readString());
    }   
    Serial.println("##################################################################");
    Serial.println("<- Fim do arquivo ->");
    registro.close(); 
  }
  else
  {
    Serial.println("## Erro: Arquivo de registro nao pode ser aberto! ##");
    registro.close();
    //fecharArquivo();
  }  
}
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//