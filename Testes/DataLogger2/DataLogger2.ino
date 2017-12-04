/*
DataLogger2 - software que escreve procura por uma chave e em um arquivo e caso
encontra a chave, guarda a informação de data e hora de quando foi feito a busca.
Algumas informações
Comunicação SPI (Arduino Mega)
Pinos:
50 (MISO),
51 (MOSI),
52 (SCK),
53 (SS ou CS).
*/
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <Time.h>


// ----------------------------- ~ Definição dos Objetos ~ -------------------------------//
File arquivoAberto; // variavel arquivo para leitura no cartão micro SD
RTC_DS1307 RTC; //objeto RTC
// ----------------------------- ~ Definição das Constantes ~ -------------------------------//
const String controleAcesso			= "entries.txt";
const String chaveAcessos 			= "keys.txt";
const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// ----------------------------- ~ Definição das funções a serem utilizadas ~ -------------------------------//
int 	procurarChave(String key, String fileName); // procura em um arquivo o valor
int 	criarArquivo(String name); // cria uma arquivo no cartão SD dado um nome
int 	abrirArquivo(String name); // procura e abre um arquivo no cartão SD dado um nome
void 	fecharArquivo(); // fecha o arquivo aberto
int 	escreverArquivo(char data[], String fileName);
void 	imprimeArquivo(String nomeArquivo); // imprime o que tiver no arquivo aberto
int 	gravarAcesso(String id); //registra data e hora de quando há um acesso autorizado
void 	imprimirAcessos(); //imprime todos os acessos registrados no arquivo de registros
// ----------------------------- ~ Variaveis utilizads ~ -------------------------------//
String chave;
String arquivo;
DateTime tempo;
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
// ------------------------------------------------------------//
void setup()
{
	Serial.begin(115200);
	// pinMode(pinoSD, OUTPUT);
	if (!(SD.begin()))
	{
		Serial.println("## Erro: Cartao SD nao pode ser inicializado! ##");
		while (1);
	}
	if (! RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
	Serial.println("Modulo cartao SD iniciado com sucesso!");
	Serial.println("Modulo RTC iniciado com sucesso!");
}

void loop()
{
	Serial.println("Insira a chave de acesso: ");	
	while (Serial.available() == 0){/*nao faz nada, aguarda por uma entrada do serial*/}

	if (Serial.available())
	{
		arquivo = chaveAcessos;
		while (Serial.available() == 0){/*nao faz nada, aguarda por uma entrada do serial*/}
		chave = Serial.readString();
		Serial.print("\n");
		Serial.print("Chave length:");
		Serial.println(chave.length());
		Serial.print("Chave inserida:");
		Serial.println(chave);
		Serial.println("Verficiando autenticidade da chave de acesso...");
		if (procurarChave(chave, arquivo) == 1)
		{
			Serial.println("Chave autenticada com sucesso!");
			Serial.println("Registrando acesso...");
			if(gravarAcesso("ID_USUARIO_TESTE") == 1)
				Serial.println("Acesso registrado com sucesso!");
			else
			{
				Serial.println("Erro no registro! Reiniciar sistema!");
				while(1){/*loop infinito*/}

			}
		}
		else
			Serial.println("Chave de acesso invalida! Insira uma chave valida.");
	}
	imprimirAcessos();

	delay(2000);
}

// --------------------Funções Utilizadas--------------------//--------------------Funções Utilizadas--------------------////
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
	String mensagem = "";
	tempo 		= RTC.now();
	mensagem 	= 	id+","
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

/*	Serial.println("Mensagem Array:");
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