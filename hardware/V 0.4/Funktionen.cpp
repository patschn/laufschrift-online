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

    //Com-Port festlegen:
    m_iComPort = 22;    //22 = ttyAMA0
    m_iColors[0] = 3;
    m_iColors[1] = 0;
    m_iColors[2] = m_iColors[0] + m_iColors[1];
    m_iLetters = 0;
    m_bFlagLeft = false;
}

bool SWP::CLauflicht::OeffneRS232()
{
    //Port öffnen, Baudrate: 2400 - ROOT RECHTE BENÖTIGT!
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
    m_debugfile.open("debug.txt");
    m_debugfile << "Originalstring:" << std::endl;

    //String speichern:
    for(std::string line; std::getline(std::cin, line);)
    {
        m_debugfile << line;        //In Debugdatei schreiben
        sBefehl.sOriginal = line;   //String der Website
    }
    m_debugfile << std::endl << std::endl;
}

void SWP::CLauflicht::KonvertiereString(stSequenz &sBefehl)
{
    //Lauflicht initialisieren, sonst Gerät nicht ansprechbar
	sBefehl.sKonvertiert += LauflichtCodetabelle.find("<INIT>")->second;
	sBefehl.sKonvertiert += LauflichtCodetabelle.find("<INIT>")->second;
	sBefehl.sKonvertiert += LauflichtCodetabelle.find("<INIT>")->second;
	sBefehl.sKonvertiert += LauflichtCodetabelle.find("<INIT>")->second;
	sBefehl.sKonvertiert += LauflichtCodetabelle.find("<INIT>")->second;
	sBefehl.sKonvertiert += LauflichtCodetabelle.find("<INIT>")->second;

    //Sequenzstart signalisieren
    sBefehl.sKonvertiert += LauflichtCodetabelle.find("<START>")->second;

    //Programmwahl im Lauflicht (für die Website wird immer Programm A benutzt
    sBefehl.sKonvertiert += LauflichtCodetabelle.find("<PROGRAM->")->second;
    sBefehl.sKonvertiert += LauflichtCodetabelle.find("A")->second;

    //Temporäre Variable zur Verarbeitung anlegen
    std::string sTemp = "";

    //Start der Konvertierung - der fertige Befehl wird in sBefehl.sKonvertiert gespeichert
    for(unsigned int i = 0;i < sBefehl.sOriginal.length();i++)
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
            //if(sTemp == "<AUTOCENTER>")...

            if(sTemp == "<LEFT>")   //Da Left einfach den String aneinanderkettet, muss der Befehl separat behandelt werden
            {
                m_bFlagLeft = true;
            }

            //Prüfen, ob Farbe vorliegt
            if(sTemp == "<BGCOLOR b>" || sTemp == "<BGCOLOR r>" || sTemp == "<BGCOLOR g>" || sTemp == "<BGCOLOR y>")
            {
                //Farbe in Tabelle nachschauen und Variablen aktualisieren
                m_iColors[COLOR_BG] = LauflichtCodetabelle.find(sTemp)->second;
                m_iColors[COLOR_FB] = m_iColors[COLOR_FG] + m_iColors[COLOR_BG];
            }
            else if(sTemp == "<COLOR b>" || sTemp == "<COLOR r>" || sTemp == "<COLOR g>" || sTemp == "<COLOR y>")
            {
                //Farbe in Tabelle nachschauen und Variablen aktualisieren
                m_iColors[COLOR_FG] = LauflichtCodetabelle.find(sTemp)->second;
                m_iColors[COLOR_FB] = m_iColors[COLOR_FG] + m_iColors[COLOR_BG];
            }
            else if(sTemp == "<CLOCK24>" || sTemp == "<CLOCK12>")
            {
                sBefehl.sKonvertiert += 143;    //Clockbefehl
                sBefehl.sKonvertiert += LauflichtCodetabelle.find(sTemp)->second;   //12h oder 24h Uhr
                sBefehl.sKonvertiert += GetClock(sTemp);
            }
            else if(sTemp.find("WAIT") != std::string::npos)
            {

                char c = sTemp[sTemp.find(' ') + 1];    //Sekundenanzahl speichern
                if(c != '0')  //Falls Sekunden != 0, dann WAIT ignorieren
                {
                    sBefehl.sKonvertiert += LauflichtCodetabelle.find("<WAIT>")->second;
                    sBefehl.sKonvertiert += c;//LauflichtCodetabelle.find(&c)->second;
                }
            }
            else if(sTemp.find("SPEED") != std::string::npos)
            {
                sBefehl.sKonvertiert += LauflichtCodetabelle.find("<SPEED>")->second;
                char c = sTemp[sTemp.find(' ') + 1];
                sBefehl.sKonvertiert += LauflichtCodetabelle.find(&c)->second;
            }
            else
            {
                //Befehl in der Codetabelle nachschauen und konvertieren:
                sBefehl.sKonvertiert += LauflichtCodetabelle.find(sTemp)->second;
                sBefehl.sKonvertiert += 3;//m_iColors[2];
            }
        }//if(sBefehl.sOriginal[i] == '<')...
        else    //Wenn kein Befehl gefunden wurde, dann muss es normaler Text sein
        {
            sTemp = sBefehl.sOriginal[i];
            m_debugfile << "Aktuelles Zeichen normal: " << sTemp << std::endl;

            //Bei Zeichen kommt erst die Farbe, anschließend das Zeichen
            if(sTemp == "\\")   //Escapezeichen
            {
                //i++;    //Wegen Escapezeichen muss weitergeschaltet werden
                sTemp = sBefehl.sOriginal[i]; //Nächstes Zeichen abholen

                if(sTemp == "\\") //Dieses Zeichen gibt es nur als Grafik
                {
                    sBefehl.sKonvertiert += LauflichtCodetabelle.find("<GRAPH>")->second;
                    sBefehl.sKonvertiert += 4;

                    i++;
                }
            }
            else
            {
                //Wenn es kein \ ist, dann ist es ein normales Zeichen
                sBefehl.sKonvertiert += m_iColors[2];                               //Farbe
                sBefehl.sKonvertiert += LauflichtCodetabelle.find(sTemp)->second;   //Zeichen

                m_debugfile << "Aktuelles Zeichen konvertiert: " << LauflichtCodetabelle.find(sTemp)->second;   //Zeichen << std::endl;
            }
            m_iLetters++;
        }//else normaler Text

        sTemp = ""; //Temporären String leeren

    }//for(unsigned int i = 0;i < sBefehl.sOriginal.length();i++)...

    //Flags prüfen
    if(m_bFlagLeft == true)	//Left kommt in der Sequenz vor
    {
        for(;m_iLetters <= 14; m_iLetters++)
        {
            sBefehl.sKonvertiert += m_iColors[2];
            sBefehl.sKonvertiert += LauflichtCodetabelle.find(" ")->second;
        }
    }

    //Endsequenz
    sBefehl.sKonvertiert += LauflichtCodetabelle.find("<END>")->second;
    sBefehl.sKonvertiert += 177;

    m_debugfile << "Endstring: " << sBefehl.sKonvertiert << std::endl;
}

void SWP::CLauflicht::SendeString(stSequenz sBefehl)
{
	for(unsigned int i = 0; i < sBefehl.sKonvertiert.length();i++)
	{
		RS232_SendByte(m_iComPort, sBefehl.sKonvertiert[i]);
		m_debugfile << "sende......" << sBefehl.sKonvertiert[i] << std::endl;
	}
}

void SWP::CLauflicht::SchliesseRS232()
{
    RS232_CloseComport(m_iComPort);

    //Filestream schließen
    m_debugfile.close();
}

void SWP::CLauflicht::InitialisiereTabelle()
{
    //Verwendet wird die Liste, die auf http://sigma.haufe.org/index.php?content=funktionen zu finden ist,
    //denn diese Liste beinhaltet gleich die ASCII-Codes als Hexzahl

    LauflichtCodetabelle["<INIT>"] = 170;
    LauflichtCodetabelle["<START>"] = 187;      //Programm-Start
    LauflichtCodetabelle["<PROGRAM->"] = 175;   //Speicherwahl
    LauflichtCodetabelle["<CLEAR>"] = 142;      //Anzeige leeren
    LauflichtCodetabelle["<WAIT>"] = 161;       //Pause bei der Anzeige; muss von einer Zahl von 1-9 gefolgt werden,
                                                //wobei 1 = schnell und 9 = langsam
    LauflichtCodetabelle["<SPEED>"] = 160;      //Geschwindigkeit der Anzeige muss von einer Zahl von 1-9 gefolgt werden,
                                                //wobei 1 = schnell und 9 = langsam
    LauflichtCodetabelle["<END>"] = 191;           //Programm-Ende
    LauflichtCodetabelle["<GRAPH>"] = 16;

    //Einführungsbefehle
    LauflichtCodetabelle["<LEFT>"] = 128;       //Text scrollt nach links
    LauflichtCodetabelle["<RIGHT>"] = 129;      //Text scrollt nach rechts
    LauflichtCodetabelle["<UP>"] = 130;         //Text scrollt nach oben
    LauflichtCodetabelle["<DOWN>"] = 131;       //Text scrollt nach unten
    LauflichtCodetabelle["<JUMP>"] = 139;       //Text erscheint sofort ohne Effekt
    LauflichtCodetabelle["<OPENMID>"] = 132;    //Text "von innen öffnen"
    LauflichtCodetabelle["<OPENRIGHT>"] = 134;
    LauflichtCodetabelle["<FLASH>"] = 138;      //Text blinkt
    LauflichtCodetabelle["<DOFF>"] = 140;       //Buchstaben erscheinen einzeln, von links nach rechts
    LauflichtCodetabelle["<DOBIG>"] = 141;      //Fettschrift scrollt von rechts nach links
    LauflichtCodetabelle["<RANDOM>"] = 163;     //Zufällige Auswahl des Effektes
    LauflichtCodetabelle["<SNOW>"] = 144;       //Schneeeffekt
    LauflichtCodetabelle["<DSNOW>"] = 145;
    LauflichtCodetabelle["<SHIFTMID>"] = 136;	//Text öffnet sich in der Mitte

    //Endbefehle
    LauflichtCodetabelle["<CLOSEMID>"] = 133;   //Text "von außen öffnen"
    LauflichtCodetabelle["<CLOSERIGHT>"] = 135; //Text "von außen öffnen"

    //Clock (Dez.: 143, Arg.) unterscheidet sich im zweiten Argument:
    LauflichtCodetabelle["<CLOCK24>"] = 7;
    LauflichtCodetabelle["<CLOCK12>"] = 3;
    LauflichtCodetabelle["<SQUEEZEMID>"] = 137;

    LauflichtCodetabelle["0"] = 48;
    LauflichtCodetabelle["1"] = 49;
    LauflichtCodetabelle["2"] = 50;
    LauflichtCodetabelle["3"] = 51;
    LauflichtCodetabelle["4"] = 52;
    LauflichtCodetabelle["5"] = 53;
    LauflichtCodetabelle["6"] = 54;
    LauflichtCodetabelle["7"] = 55;
    LauflichtCodetabelle["8"] = 56;
    LauflichtCodetabelle["9"] = 57;

    LauflichtCodetabelle["A"] = 65;   LauflichtCodetabelle["N"] = 78;
    LauflichtCodetabelle["B"] = 66;   LauflichtCodetabelle["O"] = 79;
    LauflichtCodetabelle["C"] = 67;   LauflichtCodetabelle["P"] = 80;
    LauflichtCodetabelle["D"] = 68;   LauflichtCodetabelle["Q"] = 81;
    LauflichtCodetabelle["E"] = 69;   LauflichtCodetabelle["R"] = 82;
    LauflichtCodetabelle["F"] = 70;   LauflichtCodetabelle["S"] = 83;
    LauflichtCodetabelle["G"] = 71;   LauflichtCodetabelle["T"] = 84;
    LauflichtCodetabelle["H"] = 72;   LauflichtCodetabelle["U"] = 85;
    LauflichtCodetabelle["I"] = 73;   LauflichtCodetabelle["V"] = 86;
    LauflichtCodetabelle["J"] = 74;   LauflichtCodetabelle["W"] = 87;
    LauflichtCodetabelle["K"] = 75;   LauflichtCodetabelle["X"] = 88;
    LauflichtCodetabelle["L"] = 76;   LauflichtCodetabelle["Y"] = 89;
    LauflichtCodetabelle["M"] = 77;   LauflichtCodetabelle["Z"] = 90;

    LauflichtCodetabelle["a"] = 97;   LauflichtCodetabelle["n"] = 110;
    LauflichtCodetabelle["b"] = 98;   LauflichtCodetabelle["o"] = 111;
    LauflichtCodetabelle["c"] = 99;   LauflichtCodetabelle["p"] = 112;
    LauflichtCodetabelle["d"] = 100;  LauflichtCodetabelle["q"] = 113;
    LauflichtCodetabelle["e"] = 101;  LauflichtCodetabelle["r"] = 114;
    LauflichtCodetabelle["f"] = 102;  LauflichtCodetabelle["s"] = 115;
    LauflichtCodetabelle["g"] = 103;  LauflichtCodetabelle["t"] = 116;
    LauflichtCodetabelle["h"] = 104;  LauflichtCodetabelle["u"] = 117;
    LauflichtCodetabelle["i"] = 105;  LauflichtCodetabelle["v"] = 118;
    LauflichtCodetabelle["j"] = 106;  LauflichtCodetabelle["w"] = 119;
    LauflichtCodetabelle["k"] = 107;  LauflichtCodetabelle["x"] = 120;
    LauflichtCodetabelle["l"] = 108;  LauflichtCodetabelle["y"] = 121;
    LauflichtCodetabelle["m"] = 109;  LauflichtCodetabelle["z"] = 122;
    LauflichtCodetabelle[" "] = 58;

    //Sonderzeichen:
    LauflichtCodetabelle["Ȧ"] = 3;
    LauflichtCodetabelle["£"] = 6;
    LauflichtCodetabelle["¤"] = 7;
    LauflichtCodetabelle["Ω"] = 8;
    LauflichtCodetabelle["Ö"] = 15;

    LauflichtCodetabelle["ø"] = 16;
    LauflichtCodetabelle["ȧ"] = 19;
    LauflichtCodetabelle["§"] = 20;
    LauflichtCodetabelle["æ"] = 22;
    LauflichtCodetabelle["Σ"] = 23;

    LauflichtCodetabelle[":"] = 32;
    LauflichtCodetabelle["!"] = 33;
    LauflichtCodetabelle["/"] = 47;
    LauflichtCodetabelle["#"] = 35;
    LauflichtCodetabelle["$"] = 36;
    LauflichtCodetabelle["%"] = 37;
    LauflichtCodetabelle["&"] = 38;
    LauflichtCodetabelle["'"] = 39;
    LauflichtCodetabelle["("] = 40;
    LauflichtCodetabelle[")"] = 41;
    LauflichtCodetabelle["*"] = 42;
    LauflichtCodetabelle["+"] = 43;
    LauflichtCodetabelle[","] = 44;
    LauflichtCodetabelle["-"] = 45;
    LauflichtCodetabelle["."] = 46;

    LauflichtCodetabelle["∅"] = 59; //großes Durchschnittszeichen
    LauflichtCodetabelle["Æ"] = 60;
    LauflichtCodetabelle["="] = 61;
    LauflichtCodetabelle["?"] = 63;

    LauflichtCodetabelle["@"] = 64;

    LauflichtCodetabelle["Ü"] = 91;
    LauflichtCodetabelle["ù"] = 92;
    LauflichtCodetabelle["è"] = 93;
    LauflichtCodetabelle["^"] = 94;

    LauflichtCodetabelle["É"] = 96;

    LauflichtCodetabelle[";"] = 123;
    LauflichtCodetabelle["Ñ"] = 124;
    LauflichtCodetabelle["ñ"] = 125;    //ACHTUNG: Ähnlich 124
    LauflichtCodetabelle["Ä"] = 126;
    LauflichtCodetabelle["ä"] = 127;
    LauflichtCodetabelle["<"] = 95;
    LauflichtCodetabelle[">"] = 62;

    //Vordergrundfarben:
    LauflichtCodetabelle["<COLOR b>"] = 0;   //Schwarz
    LauflichtCodetabelle["<COLOR r>"] = 1;   //Rot
    LauflichtCodetabelle["<COLOR g>"] = 2;   //Grün
    LauflichtCodetabelle["<COLOR y>"] = 3;   //Gelb

    //Hintergrundfarben:
    LauflichtCodetabelle["<BGCOLOR b>"] = 0;   //Schwarz
    LauflichtCodetabelle["<BGCOLOR r>"] = 4;   //Rot
    LauflichtCodetabelle["<BGCOLOR g>"] = 8;   //Grün
    LauflichtCodetabelle["<BGCOLOR y>"] = 12;  //Gelb
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

std::map<std::string,int> SWP::CLauflicht::LauflichtCodetabelle;
