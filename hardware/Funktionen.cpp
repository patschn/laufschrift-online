/*
    Softwareprojekt - Lauflicht Online

    Funktionen.cpp - Implementiert die Funktionen zur Ansteuerung der Headerdatei Funktionen.h
*/

#include "Funktionen.h" //Funktionsprototypen
#include "rs232.h"      //Bibliothek von http://www.teuniz.net/RS-232/
#include <iostream>
#include <ctime>
#include <unistd.h>		//Für usleep()

SWP::CLauflicht::CLauflicht()
{
    //Codetabelle initialisieren
    InitialisiereTabelle();

    std::cin.clear();

    //Com-Port festlegen:
    m_iComPort = 22;    //22 = ttyAMA0 laut teunizbibliothek
    iColors[COLOR_FG] = 3;
    iColors[COLOR_BG] = 0;
    iColors[COLOR_FB] = iColors[COLOR_FG] + iColors[COLOR_BG];
    m_iLetters = 0;
    m_bFlagLeft = false;
    m_bFlagBig = false;
    m_bFlagFail = false;
}

bool SWP::CLauflicht::OeffneRS232()
{
    //Port öffnen, Baudrate: 2400 - tty-Rechte benötigt!
    if(RS232_OpenComport(m_iComPort,2400) == 1)
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
    m_debugfile << std::endl;
}

bool SWP::CLauflicht::KonvertiereString(stSequenz &sBefehl)
{
    /**
        int iColors[3]: Dient zur Speicherung der Farbwerte
        iColors[COLOR_FG]: Vordergrundfarbe
        iColors[COLOR_BG]: Hintergrundfarbe
        iColors[COLOR_FB]: Berechnete Farbenkombination (Addition aus Vorder-/Hintergrundfarbe)
    */
    int iColors[3];
	
    std::ofstream dfileend;

	dfileend.open("debugend.txt");
	AutoLeft(sBefehl);

	m_debugfile << "Originalstring nach AutoLeft: " << sBefehl.sOriginal << std::endl;

	//Temporäre Variable zur Verarbeitung anlegen
    std::wstring sTemp;

    //Start der Konvertierung - der fertige Befehl wird in sBefehl.sKonvertiert gespeichert
    for(unsigned int i = 0;i < sBefehl.sOriginal.length() && m_bFlagFail == false;i++)
    {
    	sTemp = L"";

        if(sBefehl.sOriginal[i] == '<') //Befehlsanfang wurde gefunden
        {
            for(;sBefehl.sOriginal[i] != '>';i++)   //Den kompletten Befehl einlesen
            {
                sTemp += sBefehl.sOriginal[i];
            }

            sTemp += '>';   //Da bei '>' die Schleife abgebrochen wird, muss das Zeichen für das Ende des Befehls
                            //hinzugefügt werden
            m_debugfile << "Konvertiere: " << sTemp << std::endl;

            if(sTemp == L"<BIG>")
            {
            	m_bFlagBig = true;
            	continue;
            }
            if(sTemp == L"<NORMAL>")
            {
            	m_bFlagBig = false;
            	continue;
            }

            //Prüfen, ob Farbe vorliegt
            if(sTemp == L"<BGCOLOR b>" || sTemp == L"<BGCOLOR r>" || sTemp == L"<BGCOLOR g>" || sTemp == L"<BGCOLOR y>")
            {
            	m_debugfile << "COLOR_FG vorher: " << COLOR_FG << std::endl;
            	m_debugfile << "COLOR_BG vorher: " << COLOR_BG << std::endl;
            	m_debugfile << "COLOR_FB vorher: " << COLOR_FB << std::endl;

            	if(iColors[COLOR_FB] != 32)
            	{
            		m_debugfile << "Hintergrundfarbe setzen: " << sTemp << std::endl;

					//Farbe in Tabelle nachschauen und Variablen aktualisieren
					iColors[COLOR_BG] = GetCode(sTemp);
					iColors[COLOR_FB] = iColors[COLOR_FG] + iColors[COLOR_BG];
					m_debugfile << "COLOR_FG nachher: " << iColors[COLOR_FG] << std::endl;
					m_debugfile << "COLOR_BG nachher: " << iColors[COLOR_BG] << std::endl;
					m_debugfile << "COLOR_FB nachher: " << iColors[COLOR_FB] << std::endl;
            	}
            }
            else if(sTemp == L"<COLOR b>" || sTemp == L"<COLOR r>" || sTemp == L"<COLOR g>" || sTemp == L"<COLOR y>")
            {
            	m_debugfile << "COLOR_FG vorher: " << iColors[COLOR_FG] << std::endl;
            	m_debugfile << "COLOR_BG vorher: " << iColors[COLOR_BG] << std::endl;
            	m_debugfile << "COLOR_FB vorher: " << iColors[COLOR_FB] << std::endl;

                //Farbe in Tabelle nachschauen und Variablen aktualisieren
            	m_debugfile << "Vordergrundfarbe setzen: " << sTemp << std::endl;
                iColors[COLOR_FG] = GetCode(sTemp);
                iColors[COLOR_FB] = iColors[COLOR_FG] + iColors[COLOR_BG];
                m_debugfile << "COLOR_FG nachher: " << iColors[COLOR_FG] << std::endl;
                m_debugfile << "COLOR_BG nachher: " << iColors[COLOR_BG] << std::endl;
                m_debugfile << "COLOR_FB nachher: " << iColors[COLOR_FB] << std::endl;
            }
            else if(sTemp == L"<COLOR rainbow>")
            {
            	dfileend << "Rainbow!" << std::endl;
            	/*
            		Der Hintergrund muss schwarz sein - Rainbow lässt keinen anderen Hintergrund zu.
            	*/
            	iColors[COLOR_FG] = GetCode(sTemp);
            	iColors[COLOR_BG] = 0;
            	iColors[COLOR_FB] = iColors[COLOR_FG] + iColors[COLOR_BG];
            }
            else if(sTemp == L"<CLOCK24>" || sTemp == L"<CLOCK12>")
            {
                sBefehl.sKonvertiert += 143;    		//Clockbefehl
                sBefehl.sKonvertiert += GetCode(sTemp); //12h oder 24h Uhr
            }
            else if(sTemp.find(L"WAIT") != std::wstring::npos)
            {
                char c = sTemp[sTemp.find(' ') + 1];    //Sekundenanzahl speichern
                if(c != '0')  							//Falls Sekunden != 0, dann WAIT ignorieren
                {
                    sBefehl.sKonvertiert += GetCode(L"<WAIT>");
                    sBefehl.sKonvertiert += c;//GetCode(L&c);
                }
            }
            else if(sTemp.find(L"SPEED") != std::wstring::npos)
            {
            	char c = sTemp[sTemp.find(' ') + 1];    //Geschwindigkeit speichern
				sBefehl.sKonvertiert += GetCode(L"<SPEED>");
				sBefehl.sKonvertiert += c;//GetCode(L&c);
            }
            else
            {
            	m_debugfile << "Befehl konvertieren..." << std::endl;
                //Befehl in der Codetabelle nachschauen und konvertieren:
                sBefehl.sKonvertiert += GetCode(sTemp);
                sBefehl.sKonvertiert += 3;	//Befehl nur bestimmte Farben!
                dfileend << "Befehl::Farbe:: " << iColors[COLOR_FB] << std::endl;
            }
        }		//if(sBefehl.sOriginal[i] == '<')...
        else    //Wenn kein Befehl gefunden wurde, dann muss es normaler Text sein
        {
            sTemp = sBefehl.sOriginal[i];

            m_debugfile << "Zeichen konvertieren: " << sTemp << std::endl;

            if(sTemp == L"Ω" || sTemp == L"Σ" || sTemp == L"¤" || sTemp == L"æ" ||
                        		sTemp == L"£" || sTemp == L"🍷" || sTemp == L"♪" ||
                        		sTemp == L"🚗" || sTemp == L"⛵" || sTemp == L"🕓" ||
                        		sTemp == L"♥" || sTemp == L"⌂" || sTemp == L"◆" ||
                        		sTemp == L"▲" || sTemp == L"▶" || sTemp == L"▼" ||
                        		sTemp == L"◀" || sTemp == L"☉" || sTemp == L"⬆" ||
                        		sTemp == L"⬇" || sTemp == L"⇦" || sTemp == L"⇨")
			{
				sBefehl.sKonvertiert += GetCode(L"<GRAPH>");
				sBefehl.sKonvertiert += GetCode(sTemp, m_bFlagBig);
			}
            //Bei Zeichen kommt erst die Farbe, anschließend das Zeichen
            else if(sTemp == L"\\")   //Escapezeichen
            {
            	i++;    //Wegen Escapezeichen muss weitergeschaltet werden
                sTemp = sBefehl.sOriginal[i]; //Nächstes Zeichen abholen

                if(sTemp == L"\\") //Dieses Zeichen gibt es nur als Grafik
                {
                    sBefehl.sKonvertiert += GetCode(L"<GRAPH>");
                    sBefehl.sKonvertiert += GetCode(L"\\", m_bFlagBig);
                }
                else //Für die Zeichen '<', '>'
                {
                	sBefehl.sKonvertiert += iColors[COLOR_FB];     //Farbe
                	sBefehl.sKonvertiert += GetCode(sTemp, m_bFlagBig);   //Zeichen
                }
            }
            else
            {
                //Wenn es kein \ ist, dann ist es ein normales Zeichen
                sBefehl.sKonvertiert += iColors[COLOR_FB];     	//Farbe
                sBefehl.sKonvertiert += GetCode(sTemp, m_bFlagBig); //Zeichen
                dfileend << "Befehl::Zeichen:: " << iColors[COLOR_FB] << std::endl;
                //m_debugfile << "Aktuelles Zeichen konvertiert: " << GetCode(sTemp)->second << std::endl;
            }
            m_iLetters++;
        }
        //m_debugfile << sBefehl.sKonvertiert << std::endl;
    }

    //Start- und Endsequenz
    std::string SKonvertiertTemp;

    //Lauflicht initialisieren, sonst Gerät nicht ansprechbar
    SKonvertiertTemp += GetCode(L"<INIT>");

	//Sequenzstart signalisieren
    SKonvertiertTemp += GetCode(L"<START>");

	//Programmwahl im Lauflicht (für die Website wird immer Programm A benutzt
    SKonvertiertTemp += GetCode(L"<PROGRAM->");
    SKonvertiertTemp += GetCode(L"A");

    SKonvertiertTemp += sBefehl.sKonvertiert;
    sBefehl.sKonvertiert = SKonvertiertTemp;

    sBefehl.sKonvertiert += GetCode(L"<END>");
    sBefehl.sKonvertiert += 177;
    sBefehl.sKonvertiert += GetCode(L"<END>");

    //m_debugfile << "Endstring: " << sBefehl.sKonvertiert << std::endl;

    dfileend << "Endstring: " << std::endl;
	dfileend << sBefehl.sKonvertiert << std::endl;
	dfileend.close();

    if(m_bFlagFail == false)	//Sequenz erfolgreich konvertiert
    {
    	return true;
    }
    else	//Sequenzkonvertierung fehlgeschlagen
    {
    	return false;
    }
}

void SWP::CLauflicht::SendeString(stSequenz sBefehl)
{
	RS232_cputs(m_iComPort,GetClock().c_str());
	usleep(300000);	//200ms
	RS232_cputs(m_iComPort,sBefehl.sKonvertiert.c_str());
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
	m_debugfile << "Inhalt von wTemp: " << wTemp << "|" << std::endl;
	it = LauflichtCodetabelle.find(wTemp);

	//Prüfen ob der die Teilsequenz in der Tabelle gefunden wurde
	if(it == LauflichtCodetabelle.end())
	{
		m_debugfile << "Fehler beim Konvertieren von " << wTemp << std::endl;
		std::cerr << "Fehler beim Konvertieren!"<< std::endl;
		m_bFlagFail = true;
		return 0;
	}
	else
	{
		m_debugfile << "Erfolgreich " << wTemp << "konvertiert!" << std::endl;
		return it->second;
	}
}

int SWP::CLauflicht::GetCode(std::wstring wTemp, bool bFlagBig)
{
	//Iterator anlegen, um die Codetabelle zu durchsuchen
	std::map<std::wstring,int>::iterator it;

	//Code suchen
	it = LauflichtCodetabelle.find(wTemp);

	//Prüfen ob der die Teilsequenz in der Tabelle gefunden wurde
	if(it == LauflichtCodetabelle.end())
	{
		std::cerr << "Fehler beim BigKonvertieren!" << std::endl;
		m_bFlagFail = true;
		return 0;
	}
	else
	{
		if(bFlagBig == false)	//Keine breiten Buchstaben
		{
			return it->second;
		}
		else
		{
			return it->second + 128;
		}
	}
}

std::string SWP::CLauflicht::GetClock()
{
	std::ofstream clockfile;
	clockfile.open("clockfile.txt");

    time_t tTime = time(0);
    struct tm now;
    now = *localtime(&tTime);  //Aktuelle Zeit einlesen

    char buf[20];
    memset(buf,0,sizeof(buf));	//Buffer auf 0 setzen

    strftime(buf,sizeof(buf),"%y%m%d%H%M%S",&now);  //Uhrzeit im passenden Format kopieren

    clockfile << "Uhrzeit: ";
    clockfile << buf << std::endl;

    std::string sLocaltime;

    sLocaltime += 170;
	sLocaltime += 190;
	sLocaltime += 50;

    for(unsigned int pos = 0;pos < strlen(buf); pos++)
    {
    	sLocaltime += buf[pos];
    }

    sLocaltime += 191;
	sLocaltime += 177;
	sLocaltime += 191;
	sLocaltime += 177;
	sLocaltime += 191;

    /* Gültige Sequenz
    sLocaltime += 170;
	sLocaltime += 190;
	sLocaltime += 50;
	sLocaltime += 49;
	sLocaltime += 52;
	sLocaltime += 48;
	sLocaltime += 54;
	sLocaltime += 49;
	sLocaltime += 54;
	sLocaltime += 50;
	sLocaltime += 51;
	sLocaltime += 53;
	sLocaltime += 55;
	sLocaltime += 51;
	sLocaltime += 48;
	sLocaltime += 191;
	sLocaltime += 177;
	sLocaltime += 191;
	sLocaltime += 177;
	sLocaltime += 191;
	*/

    clockfile << "Konvertierte Uhrzeit: ";
    clockfile << sLocaltime;
    clockfile.close();

    return sLocaltime;
}

void SWP::CLauflicht::AutoLeft(stSequenz &sBefehl)
{
	std::wstring sTemp = L"";

	int firstchar,lastchar, lastauto;
	int iLeerzeichen;
	firstchar = lastchar = lastauto = iLeerzeichen = -1;

	bool bAutoCenterDone = false, bLeftDone = false, bBig = false;

	while(bAutoCenterDone == false)
	{
		//Autocenter suchen
		lastauto = sBefehl.sOriginal.find(L"<AUTOCENTER>");

		if(lastauto == std::string::npos)	//Fehlschlag
		{
			bAutoCenterDone = true;
		}
		else
		{
			for(int i = lastauto-1;i > 0;i--)
			{
				//Befehl, falls kein Escapezeichen oder sonstiges gefunden
				if(sBefehl.sOriginal[i] == '>' && sBefehl.sOriginal[i-1] != '\\')
				{
					while(sBefehl.sOriginal[i] != '<')
					{
						i--;
					}
				}
				else
				{
					while(sBefehl.sOriginal[i] != '>')	//Bis zum nächsten Befehl
					{
						if(lastchar == -1)
						{
							lastchar = i;
						}
						else
						{
							firstchar = i;
							i--;
						}
					}

					/*
						Zwischen BIG und NORMAL unterscheiden:
						Von aktueller firstchar-Position aus nach <BIG> und <NORMAL> suchen,
						um Leerzeichen korrekt zu berechnen
					*/

					for(int iBigNormal = firstchar; iBigNormal > 0;iBigNormal--)
					{
						sTemp = L"";
						if(sBefehl.sOriginal[iBigNormal] == '>')
						{
							while(sBefehl.sOriginal[iBigNormal] != '<')
							{
								sTemp += sBefehl.sOriginal[iBigNormal];
								iBigNormal--;
							}
							sTemp += '<';
						}

						if(sTemp == L">LAMRON<")	{ bBig = false; break; }
						else if (sTemp == L">GIB<")	{ bBig = true; break; }
					}

					if((lastchar - firstchar < 7) && bBig == true)
					{
						iLeerzeichen = (14 - (lastchar - firstchar));
						iLeerzeichen /= 4;

						for(int spaces = 0;spaces < iLeerzeichen;spaces++)
						{
							sBefehl.sOriginal.insert(lastchar+1,L" ");
						}

						for(int spaces = 0;spaces < iLeerzeichen;spaces++)
						{
							sBefehl.sOriginal.insert(firstchar,L" ");
						}

						//<AUTOCENTER> entfernen
						sBefehl.sOriginal.erase(lastauto + iLeerzeichen*2,12);
						firstchar = lastchar = lastauto = -1;

						break;
					}
					else if((lastchar - firstchar < 14) && bBig == false)	//Leerzeichen einfügen, falls Zeichenanzahl
					{								//kleiner der maximal darstellbaren Charakter
						/*
						 * Big behandeln:
						 * Wenn vor firstchar noch ein <BIG> kommt (ohne weitere Zeichen dazwischen) dann
						 * Leerzeichen nochmal durch zwei teilen.
						 */

						int iLeerzeichen = 0;
						iLeerzeichen = 14 - (lastchar - firstchar);	//Fehlende Leerzeichen berechnen
						if(bBig == true)
						{
							iLeerzeichen /= 4;	//Durch 4 teilen für links und rechts
						}
						else
						{
							iLeerzeichen /= 2;	//Durch 2 teilen für links und rechts
						}

						for(int spaces = 0;spaces < iLeerzeichen;spaces++)
						{
							sBefehl.sOriginal.insert(lastchar+1,L" ");
						}

						for(int spaces = 0;spaces < iLeerzeichen;spaces++)
						{
							sBefehl.sOriginal.insert(firstchar,L" ");
						}

						//<AUTOCENTER> entfernen
						sBefehl.sOriginal.erase(lastauto + iLeerzeichen*2,12);
						firstchar = lastchar = lastauto = -1;

						break;
					}
					else
					{
						sBefehl.sOriginal.erase(lastauto,12);
						firstchar = lastchar = lastauto = -1;
						break;
					}
				}//</else>

				if(i == 0)	//Sollte <AUTOCENTER> unsinnigerweise am Anfang gesetzt oder keine Zeichen gefunden werden
				{
					sBefehl.sOriginal.erase(lastauto,12);
					break;
				}
			}
		}
	}
	return;

	/*
		if(m_bFlagLeft == true)// && m_bFlagAutocenter == false)	//Left kommt in der Sequenz vor
		{
			for (; m_iLetters <= 14; m_iLetters++)
			{
				sBefehl.sKonvertiert += iColors[COLOR_FB];
				sBefehl.sKonvertiert += GetCode(L" ");
			}
		}
	*/
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
    LauflichtCodetabelle[L"<END>"] = 191;        //Programm-Ende
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



    LauflichtCodetabelle[L"ö"] = 31;
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

    LauflichtCodetabelle[L"⌀"] = 16;
    LauflichtCodetabelle[L"Æ"] = 60;
    LauflichtCodetabelle[L"="] = 61;
    LauflichtCodetabelle[L"?"] = 63;

    LauflichtCodetabelle[L"@"] = 64;
    LauflichtCodetabelle[L"Ü"] = 91;
    LauflichtCodetabelle[L"ü"] = 91;
    LauflichtCodetabelle[L"ù"] = 92;
    LauflichtCodetabelle[L"è"] = 93;
    LauflichtCodetabelle[L"^"] = 94;

    LauflichtCodetabelle[L"É"] = 96;

    LauflichtCodetabelle[L";"] = 123;
    LauflichtCodetabelle[L"Ñ"] = 124;
    LauflichtCodetabelle[L"ñ"] = 125;
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
    LauflichtCodetabelle[L"<COLOR rainbow>"] = 32;   //Gelb

    //Hintergrundfarben:
    LauflichtCodetabelle[L"<BGCOLOR b>"] = 0;   //Schwarz
    LauflichtCodetabelle[L"<BGCOLOR r>"] = 4;   //Rot
    LauflichtCodetabelle[L"<BGCOLOR g>"] = 8;   //Grün
    LauflichtCodetabelle[L"<BGCOLOR y>"] = 12;  //Gelb
}

std::map<std::wstring,int> SWP::CLauflicht::LauflichtCodetabelle;
