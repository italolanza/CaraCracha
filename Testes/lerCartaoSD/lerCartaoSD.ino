/*
lerCartaoSD - software para testar a leitura e escrita em cartões micro SD
utilizando o módulo.


Algumas informações
Comunicação SPI (Arduino Mega)
Pinos:
 50 (MISO),
 51 (MOSI),
 52 (SCK),
 53 (SS ou CS).

*/
//#include <SPI.h>
//#include <Wire.h>
#include <SD.h>

//Objetos
File arquivoAberto; //variavel arquivo para leitura no cartão micro SD

//constates
//const int pinoSD;
String nomeArquivo = "data.txt";


//~Funções a serem utilizadas
void criarArquivo(String name); //cria uma arquivo no cartão SD dado um nome
void abrirArquivo(String name);	//procura e abre um arquivo no cartão SD dado um nome
void fecharArquivo();	//fecha o arquivo aberto
void escreverArquivo(String name);	//escreve a informação no arquivo aberto escolhido dado o nome
void imprimeArquivo(); //imprime o que tiver no arquivo aberto

void setup()
{
	Serial.begin(9600);
//	pinMode(pinoSD, OUTPUT);
	
	if(!(SD.begin()))
	{
		Serial.println("## Erro: Cartao SD nao pode ser inicializado! ##");	
		return;
	}
	Serial.println("Cartao SD iniciado com sucesso!");

	criarArquivo(nomeArquivo);
	delay(1000);
	escreverArquivo(nomeArquivo);
	delay(1000);
	//abrirArquivo(nomeArquivo);
	imprimeArquivo();
	delay(1000);
	Serial.println("<--- Fim do Programa --->");
}

void loop()
{
	//nao faz nada	
}



//--------------------Funções Utilizadas--------------------//
void criarArquivo(String name)
{
	if(SD.exists(name))
	{
		Serial.println("## Erro: Arquivo com nome duplicado! ##");
		return;
	}
	else
	{
		arquivoAberto = SD.open(name, FILE_WRITE);
		Serial.println("Arquivo criado com sucesso!");
		fecharArquivo();
	}
}
void abrirArquivo(String name)
{	

	if(!(SD.exists(name)))
	{
		Serial.print("## Erro: Arquivo com o nome - ");
		Serial.print(name);
		Serial.println(" - não existe! ##");
		return;
	}
	else
	{
		arquivoAberto = SD.open(name);
		if(arquivoAberto)
		{
			Serial.println("Arquivo aberto com sucesso!");
		}
		else
		{
			Serial.println("## Erro: Arquivo nao pode ser aberto! ##");
		}
		//fecharArquivo();
	}
}
void fecharArquivo()
{	
	arquivoAberto.close();
	Serial.println("Arquivo fechado!");
}
void escreverArquivo(String name)
{
	arquivoAberto = SD.open(name, FILE_WRITE);
	if(arquivoAberto)
	{
		Serial.println("Arquivo aberto com sucesso para escrita!");
		Serial.println("Escrevendo no arquivo");
		arquivoAberto.println("Hello World");
		arquivoAberto.println("Ola Mundo");
		arquivoAberto.println("42");
		fecharArquivo();
	}
	else
	{
		Serial.println("## Erro: Arquivo nao pode ser aberto! ##");
		return;
	}	
}

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