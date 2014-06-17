/*
    Softwareprojekt - Lauflicht Online

    Funktionen.cpp - Implementiert die Funktionen zur Ansteuerung der Headerdatei Funktionen.h
*/

#include "Funktionen.h" //Funktionsprototypen
#include "rs232.h"      //Bibliothek von http://www.teuniz.net/RS-232/
#include <iostream>
#include <ctime>

SWP::CLauflicht::CLauflicht()
{
    //Codetabelle initialisieren
    InitialisiereTabelle();

    std::cin.clear();

    //Com-Port festlegen:
    m_iComPort = 22;    //22 = ttyAMA0
    m_iColors[COLOR_FG] = 3;
    m_iColors[COLOR_BG] = 0;
    m_iColors[COLOR_FB] = m_iColors[COLOR_FG] + m_iColors[COLOR_BG];
    m_iLetters = 0;
    m_bFlagLeft = false;
    m_bFlagFail = false;
}

bool SWP::CLauflicht::OeffneRS232()
{
    //Port öffnen, Baudrate: 2400 - tty-Rechte benötigt!
    if(RS232_OpenComport(m_iComPort,2400) == 1)   //Rückgabe von 1 signalisiert Fehler
    {
        std::cerr << "Fehler beim Öffnen des Com-Ports!" << std::endl;

        return false;
    }
    else
    {
        m_debugfile << "Verbindung erfolgreich aufgebaut!" << std::endl;
    }

    return true;    //Port erfolgreich geöffnet
}

void SWP::CLauflicht::LeseString(stSequenz &sBefehl)
{
    std::locale::global(std::locale(""));
    std::wcin.clear();

    m_debugfile.open("debug.txt");
    m_debugfile << "Originalstring:" << std::endl;

    //String speichern:

    for(std::wstring line; std::getline(std::wcin, line);)
    {
        m_debugfile << line << std::endl;      	//In Debugdatei schreiben
        sBefehl.sOriginal = line;   //String der Website
    }
    m_debugfile << std::endl << std::endl;
}

bool SWP::CLauflicht::KonvertiereString(stSequenz &sBefehl)
{
    //Lauflicht initialisieren, sonst Gerät nicht ansprechbar
	sBefehl.sKonvertiert += GetCode(L"<INIT>");
	sBefehl.sKonvertiert += GetCode(L"<INIT>");

    //Sequenzstart signalisieren
    sBefehl.sKonvertiert += GetCode(L"<START>");

    //Programmwahl im Lauflicht (für die Website wird immer Programm A benutzt
    sBefehl.sKonvertiert += GetCode(L"<PROGRAM->");
    sBefehl.sKonvertiert += GetCode(L"A");

    //Temporäre Variable zur Verarbeitung anlegen
    std::wstring sTemp = L"";

    //Start der Konvertierung - der fertige Befehl wird in sBefehl.sKonvertiert gespeichert
    for(unsigned int i = 0;i < sBefehl.sOriginal.length() && m_bFlagFail == false;i++)
    {
        if(sBefehl.sOriginal[i] == '<') //Befehlsanfang wurde gefunden
        {
            for(;sBefehl.sOriginal[i] != '>';i++)   //Den kompletten Befehl einlesen
            {
                sTemp += sBefehl.sOriginal[i];  //Zeichen in temporären String speichern
            }

            sTemp += '>';   //Da bei '>' die Schleife abgebrochen wird, muss das Zeichen für das Ende des Befehls
                            //hinzugefügt werden

            //AUTOCENTER::
            if (sTemp == L"<AUTOCENTER>")
            {
                m_bFlagAutocenter = true;
            }
            if(sTemp == L"<LEFT>")   //Da Left einfach den String aneinanderkettet, muss der Befehl separat behandelt werden
            {
                m_bFlagLeft = true;
            }

            //Prüfen, ob Farbe vorliegt
            if(sTemp == L"<BGCOLOR b>" || sTemp == L"<BGCOLOR r>" || sTemp == L"<BGCOLOR g>" || sTemp == L"<BGCOLOR y>")
            {
                //Farbe in Tabelle nachschauen und Variablen aktualisieren
                m_iColors[COLOR_BG] = GetCode(sTemp);
                m_iColors[COLOR_FB] = m_iColors[COLOR_FG] + m_iColors[COLOR_BG];
            }
            else if(sTemp == L"<COLOR b>" || sTemp == L"<COLOR r>" || sTemp == L"<COLOR g>" || sTemp == L"<COLOR y>")
            {
                //Farbe in Tabelle nachschauen und Variablen aktualisieren
                m_iColors[COLOR_FG] = GetCode(sTemp);
                m_iColors[COLOR_FB] = m_iColors[COLOR_FG] + m_iColors[COLOR_BG];
            }
            else if(sTemp == L"<CLOCK24>" || sTemp == L"<CLOCK12>")
            {
                sBefehl.sKonvertiert += 143;    //Clockbefehl
                sBefehl.sKonvertiert += GetCode(sTemp);   //12h oder 24h Uhr
                //sBefehl.sKonvertiert += GetClock(sTemp);
            }
            else if(sTemp.find(L"WAIT") != std::wstring::npos)
            {

                char c = sTemp[sTemp.find(' ') + 1];    //Sekundenanzahl speichern
                if(c != '0')  //Falls Sekunden != 0, dann WAIT ignorieren
                {
                    sBefehl.sKonvertiert += GetCode(L"<WAIT>");
                    sBefehl.sKonvertiert += c;//GetCode(L&c);
                }
            }
            else if(sTemp.find(L"SPEED") != std::wstring::npos)
            {
                sBefehl.sKonvertiert += GetCode(L"<SPEED>");
                char c = sTemp[sTemp.find(' ') + 1];
      //          sBefehl.sKonvertiert += GetCode(&c);
            }
            else
            {
                //Befehl in der Codetabelle nachschauen und konvertieren:
                sBefehl.sKonvertiert += GetCode(sTemp);
                sBefehl.sKonvertiert += 3;//m_iColors[2];
            }
        }//if(sBefehl.sOriginal[i] == '<')...
        else    //Wenn kein Befehl gefunden wurde, dann muss es normaler Text sein
        {
            sTemp = sBefehl.sOriginal[i];
            //m_debugfile << "Aktuelles Zeichen normal: " << sTemp << std::endl;

            if(sTemp == L"Ω" || sTemp == L"Σ" || sTemp == L"¤" || sTemp == L"æ" ||
                        		sTemp == L"£" || sTemp == L"🍷" || sTemp == L"♪" ||
                        		sTemp == L"🚗" || sTemp == L"⛵" || sTemp == L"🕓" || sTemp == L"♥" ||
                        		sTemp == L"⌂" || sTemp == L"◆" || sTemp == L"▲" || sTemp == L"▶" ||
                        		sTemp == L"▼" || sTemp == L"◀" || sTemp == L"☉" || sTemp == L"⬆" ||
                        		sTemp == L"⬇" || sTemp == L"⇦" || sTemp == L"⇨")
			{
				sBefehl.sKonvertiert += GetCode(L"<GRAPH>");
				sBefehl.sKonvertiert += GetCode(sTemp);
			}
            //Bei Zeichen kommt erst die Farbe, anschließend das Zeichen
            else if(sTemp == L"\\")   //Escapezeichen
            {
            	i++;    //Wegen Escapezeichen muss weitergeschaltet werden
                sTemp = sBefehl.sOriginal[i]; //Nächstes Zeichen abholen

                if(sTemp == L"\\") //Dieses Zeichen gibt es nur als Grafik
                {
                    sBefehl.sKonvertiert += GetCode(L"<GRAPH>");
                    sBefehl.sKonvertiert += 4;
                }
                else //Für die Zeichen '<', '>'
                {
                	sBefehl.sKonvertiert += m_iColors[2];     //Farbe
                	sBefehl.sKonvertiert += GetCode(sTemp);   //Zeichen
                }
            }
            else
            {
                //Wenn es kein \ ist, dann ist es ein normales Zeichen
            	//if(sBefehl.sOriginal[i] == 'ü'){m_debugfile << "Ü entdeckt!" << std::endl;}
                sBefehl.sKonvertiert += m_iColors[2];                               //Farbe
                sBefehl.sKonvertiert += GetCode(sTemp);   //Zeichen

                //m_debugfile << "Aktuelles Zeichen konvertiert: " << GetCode(sTemp)->second << std::endl;
            }
            m_iLetters++;
        }//else normaler Text
        sTemp = L""; //Temporären String leeren
    }//for(unsigned int i = 0;i < sBefehl.sOriginal.length();i++)...


    //Flags prüfen
    if(m_bFlagLeft == true && m_bFlagAutocenter == false)	//Left kommt in der Sequenz vor
    {
		for (; m_iLetters <= 14; m_iLetters++)
		{
			sBefehl.sKonvertiert += m_iColors[2];
			sBefehl.sKonvertiert += GetCode(L" ");
		}
    }
	else
	{
		int m_iLinks = 0;	//Laufindex für Leerzeichen links des Textes
		int m_iRechts = 0; 	//Laufindex für Leerzeichen rechts des Textes
		m_iLetters = 14 - m_iLetters; //Anzahl der Leerzeichen die benötigt werden
		m_iLetters /= 2; //Anzahl der Leerzeichen die links benötigt werden
		std::string sTempSpace = ""; //String, um vor und nach dem Befehl Leerzeichen einzufügen

		for (; m_iLinks <= m_iLetters; m_iLinks++)
		{
			sTempSpace += m_iColors[2];
			sTempSpace += GetCode(L" ");
		}
		sBefehl.sKonvertiert = sTempSpace + sBefehl.sKonvertiert; //Links Leerzeichen zum normalen Befehl hinzufügen
		sTempSpace = "";  //Leerzeichen String leeren
		m_iLetters = 14 - m_iLinks; //Anzahl der Leerzeichen die rechts benötigt werden
		for (; m_iRechts <= m_iLetters; m_iRechts++)
		{
			sTempSpace += m_iColors[2];
			sTempSpace += GetCode(L" ");
		}
		sBefehl.sKonvertiert = sBefehl.sKonvertiert + sTempSpace; //Rechts Leerzeichen zum normalen Befehl hinzufügen
	}
	/*if (m_bFlagAutocenter == true) //Text soll Zentriert werden
	{
		int m_iLinks = 0; //Laufindex für Leerzeichen links des Textes
		int m_iRechts = 0; //Laufindex für Leerzeichen rechts des Textes
		m_iLetters = 14 - m_iLetters; //Anzahl der Leerzeichen die benötigt werden
		m_iLetters = m_iLetters / 2; //Anzahl der Leerzeichen die links benötigt werden
		std::string sTempSpace = ""; //String, um vor und nach dem Befehl Leerzeichen einzufügen
		for (; m_iLinks <= m_iLetters; m_iLinks++)
		{
			sTempSpace += m_iColors[2];
			sTempSpace += GetCode(L" ");
		}
		sBefehl.sKonvertiert = sTempSpace + sBefehl.sKonvertiert; //Links Leerzeichen zum normalen Befehl hinzufügen
		sTempSpace = "";  //Leerzeichen String leeren
		m_iLetters = 14 - m_iLinks; //Anzahl der Leerzeichen die rechts benötigt werden
		for (; m_iRechts <= m_iLetters; m_iRechts++)
		{
			sTempSpace += m_iColors[2];
			sTempSpace += GetCode(L" ");
		}
		sBefehl.sKonvertiert = sBefehl.sKonvertiert + sTempSpace; //Rechts Leerzeichen zum normalen Befehl hinzufügen
	}*/


    //Endsequenz
    sBefehl.sKonvertiert += GetCode(L"<END>");
    sBefehl.sKonvertiert += 177;
    sBefehl.sKonvertiert += GetCode(L"<END>");

    //m_debugfile << "Endstring: " << sBefehl.sKonvertiert << std::endl;


    if(m_bFlagFail == false)	//Sequenz erfolgreich konvertiert
    {
    	return true;
    }
    else		//Sequenzkonvertierung fehlgeschlagen
    {
    	return false;
    }
}

void SWP::CLauflicht::SendeString(stSequenz sBefehl)
{
	for(unsigned int i = 0; i < sBefehl.sKonvertiert.length();i++)
	{
		RS232_SendByte(m_iComPort, sBefehl.sKonvertiert[i]);
		m_debugfile << "sende......" << sBefehl.sKonvertiert[i] << std::endl;
	}

//	RS232_cputs(m_iComPort,sBefehl.sKonvertiert.c_str());
}

void SWP::CLauflicht::SchliesseRS232()
{
    RS232_CloseComport(m_iComPort);

    //Filestream schließen
    m_debugfile.close();
}



int SWP::CLauflicht::GetCode(std::wstring wTemp)
{
	//Iterator anlegen, um die Codetabelle zu durchsuchen
	std::map<std::wstring,int>::iterator it;

	//Code suchen
	it = LauflichtCodetabelle.find(wTemp);

	//Prüfen ob der die Teilsequenz in der Tabelle gefunden wurde
	if(it == LauflichtCodetabelle.end())
	{
		std::cerr << "Fehler beim Konvertieren!" << std::endl;
		m_bFlagFail = true;
		return 0;
	}
	else
	{
		return it->second;
	}
}

std::string SWP::CLauflicht::GetClock(std::string sClock)
{
    time_t tTime = time(0);
    struct tm now;
    now = *localtime(&tTime);  //Aktuelle Zeit einlesen
    char buf[20];
    memset(buf,0,sizeof(buf));

    if(sClock == "<CLOCK24>")
    {
        strftime(buf,sizeof(buf),"%d%m%Y%H%M%S",&now);  //Uhrzeit im passenden Format kopieren
    }
    else
    {
        strftime(buf,sizeof(buf),"%d%m%Y%I%M%S",&now);  //Uhrzeit im passenden Format kopieren
    }
    std::string sLocaltime(buf);

    return sLocaltime;
}

void SWP::CLauflicht::InitialisiereTabelle()
{
    //Verwendet wird die Liste, die auf http://sigma.haufe.org/index.php?content=funktionen zu finden ist,
    //denn diese Liste beinhaltet gleich die ASCII-Codes als Hexzahl

    LauflichtCodetabelle[L"<INIT>"] = 170;
    LauflichtCodetabelle[L"<START>"] = 187;      //Programm-Start
    LauflichtCodetabelle[L"<PROGRAM->"] = 175;   //Speicherwahl
    LauflichtCodetabelle[L"<CLEAR>"] = 142;      //Anzeige leeren
    LauflichtCodetabelle[L"<WAIT>"] = 161;       //Pause bei der Anzeige; muss von einer Zahl von 1-9 gefolgt werden,
                                                //wobei 1 = schnell und 9 = langsam
    LauflichtCodetabelle[L"<SPEED>"] = 160;      //Geschwindigkeit der Anzeige muss von einer Zahl von 1-9 gefolgt werden,
                                                //wobei 1 = schnell und 9 = langsam
    LauflichtCodetabelle[L"<END>"] = 191;           //Programm-Ende
    LauflichtCodetabelle[L"<GRAPH>"] = 16;

    //Einführungsbefehle
    LauflichtCodetabelle[L"<LEFT>"] = 128;       //Text scrollt nach links
    LauflichtCodetabelle[L"<RIGHT>"] = 129;      //Text scrollt nach rechts
    LauflichtCodetabelle[L"<UP>"] = 130;         //Text scrollt nach oben
    LauflichtCodetabelle[L"<DOWN>"] = 131;       //Text scrollt nach unten
    LauflichtCodetabelle[L"<JUMP>"] = 139;       //Text erscheint sofort ohne Effekt
    LauflichtCodetabelle[L"<OPENMID>"] = 132;    //Text "von innen öffnen"
    LauflichtCodetabelle[L"<OPENRIGHT>"] = 134;
    LauflichtCodetabelle[L"<FLASH>"] = 138;      //Text blinkt
    LauflichtCodetabelle[L"<DOFF>"] = 140;       //Buchstaben erscheinen einzeln, von links nach rechts
    LauflichtCodetabelle[L"<DOBIG>"] = 141;      //Fettschrift scrollt von rechts nach links
    LauflichtCodetabelle[L"<RANDOM>"] = 163;     //Zufällige Auswahl des Effektes
    LauflichtCodetabelle[L"<SNOW>"] = 144;       //Schneeeffekt
    LauflichtCodetabelle[L"<DSNOW>"] = 145;
    LauflichtCodetabelle[L"<SHIFTMID>"] = 136;  //Text öffnet sich in der Mitte

    //Endbefehle
    LauflichtCodetabelle[L"<CLOSEMID>"] = 133;   //Text "von außen öffnen"
    LauflichtCodetabelle[L"<CLOSERIGHT>"] = 135; //Text "von außen öffnen"

    //Clock (Dez.: 143, Arg.) unterscheidet sich im zweiten Argument:
    LauflichtCodetabelle[L"<CLOCK24>"] = 7;
    LauflichtCodetabelle[L"<CLOCK12>"] = 3;
    LauflichtCodetabelle[L"<SQUEEZEMID>"] = 137;

    LauflichtCodetabelle[L"0"] = 48;
    LauflichtCodetabelle[L"1"] = 49;
    LauflichtCodetabelle[L"2"] = 50;
    LauflichtCodetabelle[L"3"] = 51;
    LauflichtCodetabelle[L"4"] = 52;
    LauflichtCodetabelle[L"5"] = 53;
    LauflichtCodetabelle[L"6"] = 54;
    LauflichtCodetabelle[L"7"] = 55;
    LauflichtCodetabelle[L"8"] = 56;
    LauflichtCodetabelle[L"9"] = 57;

    LauflichtCodetabelle[L"A"] = 65;   LauflichtCodetabelle[L"N"] = 78;
    LauflichtCodetabelle[L"B"] = 66;   LauflichtCodetabelle[L"O"] = 79;
    LauflichtCodetabelle[L"C"] = 67;   LauflichtCodetabelle[L"P"] = 80;
    LauflichtCodetabelle[L"D"] = 68;   LauflichtCodetabelle[L"Q"] = 81;
    LauflichtCodetabelle[L"E"] = 69;   LauflichtCodetabelle[L"R"] = 82;
    LauflichtCodetabelle[L"F"] = 70;   LauflichtCodetabelle[L"S"] = 83;
    LauflichtCodetabelle[L"G"] = 71;   LauflichtCodetabelle[L"T"] = 84;
    LauflichtCodetabelle[L"H"] = 72;   LauflichtCodetabelle[L"U"] = 85;
    LauflichtCodetabelle[L"I"] = 73;   LauflichtCodetabelle[L"V"] = 86;
    LauflichtCodetabelle[L"J"] = 74;   LauflichtCodetabelle[L"W"] = 87;
    LauflichtCodetabelle[L"K"] = 75;   LauflichtCodetabelle[L"X"] = 88;
    LauflichtCodetabelle[L"L"] = 76;   LauflichtCodetabelle[L"Y"] = 89;
    LauflichtCodetabelle[L"M"] = 77;   LauflichtCodetabelle[L"Z"] = 90;

    LauflichtCodetabelle[L"a"] = 97;   LauflichtCodetabelle[L"n"] = 110;
    LauflichtCodetabelle[L"b"] = 98;   LauflichtCodetabelle[L"o"] = 111;
    LauflichtCodetabelle[L"c"] = 99;   LauflichtCodetabelle[L"p"] = 112;
    LauflichtCodetabelle[L"d"] = 100;  LauflichtCodetabelle[L"q"] = 113;
    LauflichtCodetabelle[L"e"] = 101;  LauflichtCodetabelle[L"r"] = 114;
    LauflichtCodetabelle[L"f"] = 102;  LauflichtCodetabelle[L"s"] = 115;
    LauflichtCodetabelle[L"g"] = 103;  LauflichtCodetabelle[L"t"] = 116;
    LauflichtCodetabelle[L"h"] = 104;  LauflichtCodetabelle[L"u"] = 117;
    LauflichtCodetabelle[L"i"] = 105;  LauflichtCodetabelle[L"v"] = 118;
    LauflichtCodetabelle[L"j"] = 106;  LauflichtCodetabelle[L"w"] = 119;
    LauflichtCodetabelle[L"k"] = 107;  LauflichtCodetabelle[L"x"] = 120;
    LauflichtCodetabelle[L"l"] = 108;  LauflichtCodetabelle[L"y"] = 121;
    LauflichtCodetabelle[L"m"] = 109;  LauflichtCodetabelle[L"z"] = 122;
    LauflichtCodetabelle[L" "] = 58;

    //Sonderzeichen:
    LauflichtCodetabelle[L"◆"] = 1;
    LauflichtCodetabelle[L"⌂"] = 2;
    LauflichtCodetabelle[L"Ȧ"] = 3;
    LauflichtCodetabelle[L"\\"] = 4;
    LauflichtCodetabelle[L"⬆"] = 5;
    LauflichtCodetabelle[L"⬇"] = 47;
    LauflichtCodetabelle[L"£"] = 6;
    LauflichtCodetabelle[L"¤"] = 7;
    LauflichtCodetabelle[L"Ω"] = 8;
    LauflichtCodetabelle[L"⇦"] = 9;
    LauflichtCodetabelle[L"⇨"] = 10;
    LauflichtCodetabelle[L"🚗"] = 11;   //Auto
    LauflichtCodetabelle[L"♪"] = 12;
    LauflichtCodetabelle[L"☉"] = 13;
    LauflichtCodetabelle[L"🕓"] = 17;   //Uhr
    LauflichtCodetabelle[L"⛵"] = 28;
    LauflichtCodetabelle[L"♥"] = 33;

    LauflichtCodetabelle[L"'"] = 0; //Hochkomma
    LauflichtCodetabelle[L"´"] = 0; //Hochkomma
    LauflichtCodetabelle[L"`"] = 0; //Hochkomma

    LauflichtCodetabelle[L"Ö"] = 15;

    LauflichtCodetabelle[L"∅"] = 59;
    LauflichtCodetabelle[L"ȧ"] = 19;
    LauflichtCodetabelle[L"§"] = 20;
    LauflichtCodetabelle[L"æ"] = 22;
    LauflichtCodetabelle[L"Σ"] = 23;
    LauflichtCodetabelle[L"🍷"] = 24;   //Weinglas



    LauflichtCodetabelle[L":"] = 32;
    LauflichtCodetabelle[L"!"] = 33;
    LauflichtCodetabelle[L"/"] = 47;
    LauflichtCodetabelle[L"#"] = 35;
    LauflichtCodetabelle[L"$"] = 36;
    LauflichtCodetabelle[L"%"] = 37;
    LauflichtCodetabelle[L"&"] = 38;
    LauflichtCodetabelle[L"'"] = 39;
    LauflichtCodetabelle[L"("] = 40;
    LauflichtCodetabelle[L")"] = 41;
    LauflichtCodetabelle[L"*"] = 42;
    LauflichtCodetabelle[L"+"] = 43;
    LauflichtCodetabelle[L","] = 44;
    LauflichtCodetabelle[L"-"] = 45;
    LauflichtCodetabelle[L"."] = 46;

    LauflichtCodetabelle[L"⌀"] = 16; //großes Durchschnittszeichen
    LauflichtCodetabelle[L"Æ"] = 60;
    LauflichtCodetabelle[L"="] = 61;
    LauflichtCodetabelle[L"?"] = 63;

    LauflichtCodetabelle[L"@"] = 64;
    LauflichtCodetabelle[L"Ü"] = 91;
    LauflichtCodetabelle[L"ù"] = 92;
    LauflichtCodetabelle[L"è"] = 93;
    LauflichtCodetabelle[L"^"] = 94;

    LauflichtCodetabelle[L"É"] = 96;

    LauflichtCodetabelle[L";"] = 123;
    LauflichtCodetabelle[L"Ñ"] = 124;
    LauflichtCodetabelle[L"ñ"] = 125;    //ACHTUNG: Ähnlich 124
    LauflichtCodetabelle[L"Ä"] = 126;
    LauflichtCodetabelle[L"ä"] = 127;
    LauflichtCodetabelle[L"<"] = 95;
    LauflichtCodetabelle[L">"] = 62;

    //Grafiken
    LauflichtCodetabelle[L"▲"] = 25;
    LauflichtCodetabelle[L"▶"] = 44;
    LauflichtCodetabelle[L"◀"] = 45;
    LauflichtCodetabelle[L"▼"] = 46;


    //Vordergrundfarben:
    LauflichtCodetabelle[L"<COLOR b>"] = 0;   //Schwarz
    LauflichtCodetabelle[L"<COLOR r>"] = 1;   //Rot
    LauflichtCodetabelle[L"<COLOR g>"] = 2;   //Grün
    LauflichtCodetabelle[L"<COLOR y>"] = 3;   //Gelb

    //Hintergrundfarben:
    LauflichtCodetabelle[L"<BGCOLOR b>"] = 0;   //Schwarz
    LauflichtCodetabelle[L"<BGCOLOR r>"] = 4;   //Rot
    LauflichtCodetabelle[L"<BGCOLOR g>"] = 8;   //Grün
    LauflichtCodetabelle[L"<BGCOLOR y>"] = 12;  //Gelb
}

std::map<std::wstring,int> SWP::CLauflicht::LauflichtCodetabelle;
