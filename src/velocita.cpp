#include <iostream>
#include <iomanip>
#include <fstream>
#include <conio.h>
#include <string>
using namespace std;

#include "sierrachart.h"

//fce pro zapis
int zapis(double cas, double hodnota_absolutni, double hodnota_prumeru, SCString symbol) // PREDAM PARAMETRY
{

	SCString FileName;
	FileName.Format("VELOCITA__%s.txt", symbol);

	// Existuje Soubor ? Kdyz Ne Vytvorim !
	ifstream my_file(FileName);																								
	if (!my_file)
	{
		SCString FileName;
		FileName.Format("VELOCITA__%s.txt", symbol);

		std::ofstream ofs;
		ofs.open(FileName.GetChars(), std::ofstream::out | std::ofstream::trunc);
		ofs.close();

	}
	my_file.close();

	SCString OutputPathAndFileName;
	OutputPathAndFileName.Format("VELOCITA__%s.txt", symbol);
	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	DWORD BytesWritten;

	// Otevrit Existujici Soubor
	FileHandle = CreateFile(OutputPathAndFileName.GetChars(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// Chyba Pri Otvirani Souboru
	if (FileHandle == INVALID_HANDLE_VALUE)	return 0;

	SetFilePointer(FileHandle, 0, NULL, FILE_END);
	
	const char *FormatString; FormatString = "%f %f %f %s";
	SCString data;
	SCString konec_textu = "\n";

	data.Format(FormatString, cas, hodnota_absolutni, hodnota_prumeru, symbol);
	//data.Append(datum_aktualni);
	data.Append(konec_textu);
	
	WriteFile(FileHandle, data.GetChars(), (DWORD)data.GetLength(), &BytesWritten, NULL);
	
	CloseHandle(FileHandle);
	return 1;

}

SCDLLName("VELOCITA")

SCSFExport scsf_Velocita(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Velocita = sc.Subgraph[3];
	SCSubgraphRef Prumer = sc.Subgraph[2];

	SCInputRef Perioda = sc.Input[0];

	if (sc.SetDefaults)
	{
		sc.GraphName = "velocita se zapisem do txt";
		sc.StudyDescription = "Meri tikavost ceny, kolikrat se cena zmeni za cas";
		sc.GraphRegion = 1;

		sc.AutoLoop = 1;
		sc.FreeDLL = 1;

		Velocita.Name = "Velocita";
		Velocita.PrimaryColor = RGB(250, 0, 0);
		Velocita.DrawStyle = DRAWSTYLE_BAR;
		Velocita.LineWidth = 3;

		Prumer.Name = "Prumer";
		Prumer.PrimaryColor = RGB(0, 0, 250);
		Prumer.DrawStyle = DRAWSTYLE_LINE;
		Prumer.LineWidth = 1;

		Perioda.Name = "Perioda";
		Perioda.SetInt(10);

		return;
	}

	double AktualniCena = sc.GetLastPriceForTrading();
	int AktualniIndex = sc.Index;

	float& PocetZmen = sc.GetPersistentFloat(0); // perzit var pro udrzeni informaci o poctu zmen ceny
	double& PredchoziCena = sc.GetPersistentDouble(1); // perzist var pro udrzeni predchozi ceny
	int& PredchoziIndex = sc.GetPersistentInt(2); // perzist var pro udrzeni predchoziho indexu

	//vypocet poctu zmen
	if (AktualniCena != PredchoziCena)
	{
		PocetZmen++; 
		PredchoziCena = AktualniCena; 
	}

	Velocita[sc.Index] = PocetZmen * 10; 

	//nulovani na prelomu baru									 
	if (AktualniIndex != PredchoziIndex) 
	{
		PocetZmen = 0;
		PredchoziIndex = sc.Index; 
	}

	float suma = Velocita[sc.Index];

	//prumer
	for (int i = 1; i < Perioda.GetInt(); i++)
	{
		suma += Velocita[sc.Index - i];
	}

	Prumer[sc.Index] = suma / Perioda.GetInt();

	//volani fce pro zapis 
	double cas = COMBINE_DATE_TIME(sc.BaseDateTimeIn[sc.Index].GetDate(), sc.BaseDateTimeIn[sc.Index].GetTime());
	double hodnota_absolutni = Velocita[sc.Index];
	double hodnota_prumeru = Prumer[sc.Index];

	zapis(cas, hodnota_absolutni, hodnota_prumeru, sc.Symbol);
}


SCSFExport scsf_VelocitaGyrus(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Velocita = sc.Subgraph[0];

	if (sc.SetDefaults)
	{
		sc.GraphName = "velocita (gyrus) bez zapisu";
		sc.StudyDescription = "velocita";
		sc.GraphRegion = 1;

		sc.AutoLoop = 1;
		sc.FreeDLL = 1;

		Velocita.Name = "velocita";
		Velocita.PrimaryColor = RGB(250, 0, 0);
		Velocita.DrawStyle = DRAWSTYLE_BAR;
		Velocita.LineWidth = 1;

		return;
	}

	float& PocetZmen = sc.GetPersistentFloat(0);
	double& pred_LastPrice = sc.GetPersistentDouble(1);
	double& stary_index = sc.GetPersistentDouble(2);


	if (sc.IsFullRecalculation == 0) {
		if (sc.Close[sc.Index] != pred_LastPrice)
		{
			PocetZmen++;
			pred_LastPrice = sc.Close[sc.Index];
		}

		Velocita[sc.Index] = PocetZmen;

		if (stary_index != sc.Index)
		{
			PocetZmen = 0;
			stary_index = sc.Index;
		}
	}
}

/*




SCSFExport scsf_VelocitaGyrus(SCStudyInterfaceRef sc)
{
SCSubgraphRef Velocita = sc.Subgraph[0];

if (sc.SetDefaults)
{
sc.GraphName = "velocita gyrus";
sc.StudyDescription = "velocita";
sc.GraphRegion = 1;

sc.AutoLoop = 1;
sc.FreeDLL = 1;

Velocita.Name = "velocita";
Velocita.PrimaryColor = RGB(250, 0, 0);
Velocita.DrawStyle = DRAWSTYLE_BAR;
Velocita.LineWidth = 1;

return;
}

float& PocetZmen = sc.GetPersistentFloat(0);
double& pred_LastPrice = sc.GetPersistentDouble(1);
double& stary_index = sc.GetPersistentDouble(2);


if (sc.IsFullRecalculation == 0) {
if (sc.Close[sc.Index] != pred_LastPrice)
{
PocetZmen++;
pred_LastPrice = sc.Close[sc.Index];
}

Velocita[sc.Index] = PocetZmen;

if (stary_index != sc.Index)
{
PocetZmen = 0;
stary_index = sc.Index;
}
}
}





SCString datum_aktualni = ? ? ? ? ;
double hodnota_absolutni = 0;
double hodnota_prumeru = 0;

zapis(datum_aktualni, hodnota_absolutni, hodnota_prumeru, symbol)
 


cas_aktualni_persist = COMBINE_DATE_TIME(sc.BaseDateTimeIn[sc.Index].GetDate(), sc.BaseDateTimeIn[sc.Index].GetTime());
SCString datum_aktualni = sc.FormatDateTime(sc.BaseDateTimeIn[sc.Index]).GetChars();
zapis_limity(datum_aktualni, cas_aktualni_persist, pocet_zmen_ask_persist, pocet_zmen_bid_persist, sc.Symbol, sc.ChartNumber, rezim_provozu.GetIndex(), (hranice_limitu.GetDouble() / 100)*prumer_ask_bid, prumer_ask_bid);

*/