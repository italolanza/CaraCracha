/*
DataLogger - software que escreve em um arquivo uma informaçao no cartao SD
recupera-la.


Algumas informações
Comunicação SPI (Arduino Mega)
Pinos:
 50 (MISO),
 51 (MOSI),
 52 (SCK),
 53 (SS ou CS).

*/
#include <SPI.h>
#include <Wire.h>
#include <SD.h>

//Objetos
File arquivoAberto; //variavel arquivo para leitura no cartão micro SD

//constantes
//const int pinoSD;
String nomeArquivo 			= "data.txt";
//const String logger			= "log.txt";
const String chaveAcessos 	= "keys.txt";

//~Funções a serem utilizadas
int procurarChave(String key, String fileName); //procura em um arquivo o valor
int  criarArquivo(String name); //cria uma arquivo no cartão SD dado um nome
int  abrirArquivo(String name);	//procura e abre um arquivo no cartão SD dado um nome
void fecharArquivo();	//fecha o arquivo aberto
int  escreverArquivo(String name);	//escreve a informação no arquivo aberto escolhido dado o nome
void imprimeArquivo(); //imprime o que tiver no arquivo aberto

void setup()
{
	Serial.begin(115200);
//	pinMode(pinoSD, OUTPUT);
	

	if(!(SD.begin()))
	{
		Serial.println("## Erro: Cartao SD nao pode ser inicializado! ##");	
		return;
	}
	Serial.println("Cartao SD iniciado com sucesso!");
	Serial.println("Insira o nome do arquivo e a chave:");
}

String chave;
String arquivo;

void loop()
{
	if(Serial.available())
	{
		//while(Serial.available() == 0){}
    //arquivo = Serial.readString();
    arquivo = chaveAcessos;
    while(Serial.available() == 0){}
    chave = Serial.readString();
    
    Serial.print("Nome do arquivo inserido: ");
    Serial.println(arquivo);
    Serial.print("Chave length:");      
    Serial.println(chave.length());
    Serial.print("Chave inserida:");
    Serial.println(chave);  
    
      
		if(procurarChave(chave, arquivo) == 1)
		{
			Serial.println("A chave esta presente no arquivo.");
			Serial.println("Gravando data do acesso");
		}
		else
		{
			Serial.println("A chave esta nao presente no arquivo.");
		}
	}
	delay(2000);

}


//--------------------Funções Utilizadas--------------------//--------------------Funções Utilizadas--------------------////



int criarArquivo(String name)
{
	if(SD.exists(name))
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
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
int abrirArquivo(String name)
{	

	if(!(SD.exists(name)))
	{
		Serial.print("## Erro: Arquivo com o nome - ");
		Serial.print(name);
		Serial.println(" - não existe! ##");
		return 0;
	}
	else
	{
		arquivoAberto = SD.open(name);
		if(arquivoAberto)
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
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
void fecharArquivo()
{	
	arquivoAberto.close();
	Serial.println("Arquivo fechado!");
}


int escreverArquivo(String name, String data)
{
	arquivoAberto = SD.open(name, FILE_WRITE);
	if(arquivoAberto)
	{
		Serial.println("Arquivo aberto com sucesso para escrita!");
		Serial.println("Escrevendo no arquivo");
		arquivoAberto.println(data);		
		fecharArquivo();
		return 1;
	}
	else
	{
		Serial.println("## Erro: Arquivo nao pode ser aberto! ##");
		return 0;
	}	
}
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
void imprimeArquivo()
{	
	abrirArquivo(nomeArquivo);
	Serial.println("Imprimindo arquivo!");
	while(arquivoAberto.available())
	{
		Serial.write(arquivoAberto.read());
	}
	Serial.println("<- Fim do arquivo ->");
	fecharArquivo();	
}
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
//procura em um arquivo o valor
int procurarChave(String key, String fileName)
{
	arquivoAberto = SD.open(fileName, FILE_READ);
	if(arquivoAberto)
	{
		while(arquivoAberto.available())
		{//length()
			String input = arquivoAberto.readStringUntil('#');
      //Serial.print("Input length:");      
      //Serial.println(input.length());
      //Serial.print("Input:");      
      //Serial.println(input);
      //Serial.println("#");
			if(input.equals(key))
			{
        //Serial.println("Its a match!");
        arquivoAberto.close();
				return 1;
			}			
		}
		arquivoAberto.close();
		return 0;
	}
	else
	{
		Serial.println("Arquivo nao pode ser aberto ou nao foi encontrado!");   
		return 0;
	}
}
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
//------------------------------------------------------------//
