/*
    Softwareprojekt - Lauflicht Online

    Funktionen.cpp - Implementiert die Funktionen zur Ansteuerung der Headerdatei Funktionen.h
*/

#include "Funktionen.h" //Funktionsprototypen
#include "serialib.h"
#include <iostream>
#include <ctime>
#include <unistd.h>

SWP::CLauflicht::CLauflicht()
{
    //Codetabelle initialisieren
    InitialisiereTabelle();

    //Com-Port festlegen:
    m_sComPort = "/dev/ttyAMA0";
    m_iLetters = 0;
    m_bFlagBig = false;
    m_bFlagFail = false;
}

bool SWP::CLauflicht::OeffneRS232()
{
    //Port öffnen, Baudrate: 2400 - tty-Rechte benötigt!
    if(m_sl.Open(m_sComPort.c_str(),2400) != 1)
    {
        std::cerr << "Fehler beim Öffnen des Com-Ports!" << std::endl;

        return false;
    }
    else
    {
        std::cout << "Verbindung erfolgreich aufgebaut" << std::endl;
    }

    return true;    //Port erfolgreich geöffnet
}

void SWP::CLauflicht::LeseString(stSequenz &sBefehl)
{
    std::locale::global(std::locale("de_DE.UTF-8"));
    std::wcin.clear();

    //String speichern:

    for(std::wstring line; std::getline(std::wcin, line);)
    {
        sBefehl.sOriginal = line;   //String der Website
    }
}

bool SWP::CLauflicht::KonvertiereString(stSequenz &sBefehl)
{
    if(sBefehl.sOriginal.empty() == true)   //Sequenz ist leer
    {
        std::cerr << "Leere Sequenzen sind ungültig!" << std::endl;
        m_bFlagFail = true;
        return false;
    }

    //Uhrzeitbehandlung (Keine Animationen vor Uhrzeit, wenn kein Text in der Sequenz ist
	/*std::wstring Anfangsanimationen[] =
	{
		L"<LEFT>",L"<RIGHT>",L"<UP>",L"<DOWN>",L"<DOFF>",L"<DOBIG>",L"<FLASH>",
		L"<JUMP>",L"<OPENMID>",L"<OPENRIGHT>",L"<RANDOM>",L"<SHIFTMID>",L"<SNOW>"
	};*/

	std::wstring Anfangsanimationen[] = //Invertiert, da von Clock aus rückwärts gesucht wird
	{
		L">TFEL<",L">THGIR<",L">PU<",L">NWOD<",L">FFOD<",L">GIBOD<",L">HSLAF<",
		L">PMUJ<",L">DIMNEPO<",L">THGIRNEPO<",L">MODNAR<",L">DIMTFIHS<",L">WONS<"
	};

	if(sBefehl.sOriginal.find(L"<CLOCK24>") != std::wstring::npos ||  //Uhrzeitelement gefunden
		sBefehl.sOriginal.find(L"<CLOCK12>") != std::wstring::npos)
	{
		int Clockpos = -1;
		std::wstring wsTemp = L"";

		if(sBefehl.sOriginal.find(L"<CLOCK24>") != std::wstring::npos)
		{
			Clockpos = sBefehl.sOriginal.find(L"<CLOCK24>");
		}
		else
		{
			Clockpos = sBefehl.sOriginal.find(L"<CLOCK12>");
		}

		if(Clockpos > 0)
		{
			bool bTextfound = false;

			//Es muss geprüft werden, ob Befehle vor der Clockanweisung sind, die das Verhalten beeinträchtigen könnten.
			for(int i = Clockpos-1;i > 0;i--)
			{
				//Befehl
				if(sBefehl.sOriginal[i] == '>' && sBefehl.sOriginal[i-1] != '\\')
				{
					//Befehl einlesen
					while(sBefehl.sOriginal[i] != '<')
					{
						wsTemp += sBefehl.sOriginal[i];
						i--;
					}
					wsTemp += '<';

					//Anfangsanimationen durchschalten
					for(int i = 0;i < 13; i++)
					{
						if(Anfangsanimationen[i] == wsTemp)
						{
							std::cerr << "Ungültige Anfangsanimation vor Clock gefunden: Abbruch!" << std::endl;
							m_bFlagFail = true;
							break;
						}
					}

					wsTemp = L"";
				}//</if>
				else
				{
					break;
				}//</else>
			}//</for>
		}//</if>
	}//</if>

	if(m_bFlagFail == true) { return false; }
	else { return true; }

    if(m_bFlagFail == true)
    {
        return false;
    }
//-------------------------------------------------------------------------------
    /**
        int iColors[3]: Dient zur Speicherung der Farbwerte
        iColors[COLOR_FG]: Vordergrundfarbe
        iColors[COLOR_BG]: Hintergrundfarbe
        iColors[COLOR_FB]: Berechnete Farbenkombination (Addition aus Vorder-/Hintergrundfarbe)
    */

    int iColors[3];
    iColors[COLOR_FG] = 3;
    iColors[COLOR_BG] = 0;
    iColors[COLOR_FB] = 3;

    AutoLeft(sBefehl);

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
            	if(iColors[COLOR_FB] != 32)
            	{
					//Farbe in Tabelle nachschauen und Variablen aktualisieren
            	    if((sTemp == L"<BGCOLOR b>") && (iColors[COLOR_FG] == GetCode(L"<COLOR b>")))
            	    {
            	        m_bFlagFail = true;
            	        std::cerr << "Schwarzer Hintergrund und Textfarbe verboten!" << std::endl;
            	    }
            	    else
            	    {
            	        iColors[COLOR_BG] = GetCode(sTemp);
            	        iColors[COLOR_FB] = iColors[COLOR_FG] + iColors[COLOR_BG];
            	    }
            	}
            }
            else if(sTemp == L"<COLOR b>" || sTemp == L"<COLOR r>" || sTemp == L"<COLOR g>" || sTemp == L"<COLOR y>")
            {
                    //Farbe in Tabelle nachschauen und Variablen aktualisieren
                    iColors[COLOR_FG] = GetCode(sTemp);
                    iColors[COLOR_FB] = iColors[COLOR_FG] + iColors[COLOR_BG];
            }
            else if(sTemp == L"<COLOR rainbow>")
            {
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
				sBefehl.sKonvertiert += c;
            }
            else
            {
                //Befehl in der Codetabelle nachschauen und konvertieren:
                sBefehl.sKonvertiert += GetCode(sTemp);
                sBefehl.sKonvertiert += 3;	//Befehl nur bestimmte Farben!
            }
        }		//if(sBefehl.sOriginal[i] == '<')...
        else    //Wenn kein Befehl gefunden wurde, dann muss es normaler Text sein
        {
            sTemp = sBefehl.sOriginal[i];

            if(sTemp == L"Ω" || sTemp == L"Σ" || sTemp == L"¤" || sTemp == L"æ" ||
                        		sTemp == L"£" || sTemp == L"🍷" || sTemp == L"♪" ||
                        		sTemp == L"🚗" || sTemp == L"⛵" || sTemp == L"🕓" ||
                        		sTemp == L"♥" || sTemp == L"⌂" || sTemp == L"◆" ||
                        		sTemp == L"▲" || sTemp == L"▶" || sTemp == L"▼" ||
                        		sTemp == L"◀" || sTemp == L"☉" || sTemp == L"⬆" ||
                        		sTemp == L"⬇" || sTemp == L"⇦" || sTemp == L"⇨" ||
                        		sTemp == L"€")
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
            }
            m_iLetters++;
        }
    }

    //Start- und Endsequenz
    std::string SKonvertiertTemp;

    //Lauflicht initialisieren, sonst Gerät nicht ansprechbar
    for(int i = 0; i < 10; i++) { SKonvertiertTemp += GetCode(L"<INIT>"); }

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
    m_sl.WriteString(sBefehl.sKonvertiert.c_str());
    usleep(300000);
    m_sl.WriteString(GetClock().c_str());
}

void SWP::CLauflicht::SchliesseRS232()
{
    m_sl.Close();
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
	    std::wcerr << L"Ungültige Zeichenkette „" << wTemp << L"“" << std::endl;
		m_bFlagFail = true;
		return 0;
	}
	else
	{
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
		std::wcerr << L"Ungültige Zeichenkette „" << wTemp << L"“" << std::endl;
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

    for(int i = 0; i < 10; i++) { sLocaltime += 170; }  //Init
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

    clockfile << "Konvertierte Uhrzeit: ";
    clockfile << sLocaltime;
    clockfile.close();

    return sLocaltime;
}

void SWP::CLauflicht::AutoLeft(stSequenz &sBefehl)
{
    int firstchar,lastchar;
    int lastautoleft = 0;
    int iLeerzeichen = 0;
    std::wstring sTemp = L"";
    firstchar = lastchar = -1;

    bool bAutoCenterDone = false, bLeftDone = false, bBig = false;

    for(;(lastautoleft != std::wstring::npos) && bAutoCenterDone != true;)
    {
        lastautoleft = sBefehl.sOriginal.find(L"<AUTOCENTER>");
        firstchar = lastchar = -1;

        if(lastautoleft == 0)   //Autocenter wurde am Anfang platziert
        {
            sBefehl.sOriginal.erase(0,12);
            continue;
        }
        if(lastautoleft == std::wstring::npos)  //<AUTOCENTER> nicht gefunden
        {
            bAutoCenterDone = true;
        }
        else
        {
            //Zeichen ab <AUTOCENTER>-Position suchen
            //lastautoleft -1 wegen dem < Zeichen
            for(int i = lastautoleft-1;i >= 0;)
            {
                //Befehlsanfang nachschauen und überspringen
                if((lastchar == -1) && ((sBefehl.sOriginal[i] == '>') && (sBefehl.sOriginal[i-1] != '\\')))
                {
                    while(sBefehl.sOriginal[i] != '<')
                    {
                        i--;
                    }
                    if(i != 0)
                    {
                        i--;    //Nochmal dekrementieren wegen '<'
                    }
                }
                else if(lastchar != -1 && sBefehl.sOriginal[i] == '>' && sBefehl.sOriginal[i-1] != '\\')
                {
                    break;
                }
                else if(lastchar == -1 && i == 0 && sBefehl.sOriginal[i] == '<')
                {
                    break;
                }
                else//Zeichen
                {
                    if(lastchar == -1)
                    {
                        lastchar = i;
                    }
                    //Escapezeichen behandeln
                    if(((sBefehl.sOriginal[i] == '<') || (sBefehl.sOriginal[i] == '>') || sBefehl.sOriginal[i] == '\\'))
                    {
                        firstchar = i;
                        iLeerzeichen++;
                        i--;    //Nochmal weiterschalten, da \ nicht mitgezählt werden darf
                    }
                    else
                    {
                        firstchar = i;
                        iLeerzeichen++;
                        i--;
                    }
                }
            }
            //AUTOCENTER entfernen
            sBefehl.sOriginal.erase(lastautoleft,12);

            if(lastchar != -1)
            {
                int spaces = 0;
                /*
                    Zwischen BIG und NORMAL unterscheiden:
                    Von aktueller firstchar-Position aus nach <BIG> und <NORMAL> suchen,
                    um Leerzeichen korrekt zu berechnen
                */

                for(int iBigNormal = firstchar; iBigNormal > 0;iBigNormal--)
                {
                    sTemp = L"";
                    if(sBefehl.sOriginal[iBigNormal] == '>' && sBefehl.sOriginal[iBigNormal-1] != '\\')
                    {
                        while(sBefehl.sOriginal[iBigNormal] != '<')
                        {
                            sTemp += sBefehl.sOriginal[iBigNormal];
                            iBigNormal--;
                        }
                        sTemp += '<';
                    }

                    if(sTemp == L">LAMRON<")    { bBig = false; break; }
                    else if (sTemp == L">GIB<") { bBig = true; break; }
                }


                //Leerzeichen berechnen und einfügen
                if(lastchar - firstchar < 7 && bBig == true)
                {
                    iLeerzeichen = 7 - (lastchar - firstchar);
                    iLeerzeichen /= 2;

                    if(((lastchar - firstchar) % 2) == 1)
                    {
                        spaces = 1;
                    }

                    for(; spaces < iLeerzeichen;spaces++) //Rechts auffüllen
                    {
                        sBefehl.sOriginal.insert(lastchar+1,L" ");
                    }

                    for(spaces = 0; spaces < iLeerzeichen;spaces++)    //Links auffüllen
                    {
                        sBefehl.sOriginal.insert(firstchar,L" ");
                    }
                }
                else if(lastchar - firstchar < 14 && bBig == false)
                {
                    iLeerzeichen = 14 - (lastchar - firstchar);
                    iLeerzeichen /= 2;

                    if(((lastchar - firstchar) % 2 + 1) == 1)
                    {
                        spaces = 1;
                    }
                    for(; spaces < iLeerzeichen;spaces++) //Rechts auffüllen
                    {
                        sBefehl.sOriginal.insert(lastchar+1,L" ");
                    }

                    for(spaces = 0; spaces < iLeerzeichen;spaces++) //Links auffüllen
                    {
                        sBefehl.sOriginal.insert(firstchar,L" ");
                    }
                }
            }
        }
    }
    bBig = false;

    //Left behandeln
    while(bLeftDone == false)
    {
        int iTemp = lastautoleft;
        int iAnzahlZeichen = 0;

        firstchar = lastchar = -1;

        //Letztes Vorkommen von Left kopieren
        while(iTemp == lastautoleft)
        {
            lastautoleft = sBefehl.sOriginal.find(L"<LEFT>",lastautoleft+1);
            if(lastautoleft == std::wstring::npos) { break; }
        }

        if(lastautoleft == std::wstring::npos)  //Fehlschlag
        {
            bLeftDone = true;
        }
        else
        {
            //Falls left gefunden, left auf letzte bekannte Position setzen
            for(int i = lastautoleft;i < sBefehl.sOriginal[i];i++)
            {
                //Befehl, falls kein Escapezeichen oder sonstiges gefunden
                if(sBefehl.sOriginal[i] == '<' && sBefehl.sOriginal[i-1] != '\\')
                {
                    while(sBefehl.sOriginal[i] != '>')
                    {
                        i++;
                    }
                }
                else
                {
                    while(sBefehl.sOriginal[i] != '<' && i < sBefehl.sOriginal.length())  //Bis zum nächsten Befehl die Zeichen durchgehen
                    {
                        if(firstchar == -1)
                        {
                            firstchar = i;
                        }
                        else
                        {
                            lastchar = i;
                            iAnzahlZeichen++;
                            i++;
                        }
                        if((sBefehl.sOriginal[i-1] == '\\' && sBefehl.sOriginal[i] == '<') ||
                           (sBefehl.sOriginal[i-1] == '\\' && sBefehl.sOriginal[i] == '>') ||
                           (sBefehl.sOriginal[i-1] == '\\' && sBefehl.sOriginal[i] == '\\') )
                        {
                            /*
                                 i wurde schon weitergeschalten, falls das vorhergehende Element ein Zeichen
                                 war. Deshalb wird mit der vorherigen und aktuellen Position geprüft, ob ein
                                 Sonderzeichen eingeleitet wurde, um nicht irrtümlich einen Befehl anzunehmen.
                            */
                            lastchar = i;
                            i++;
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

                        if(sTemp == L">LAMRON<")    { bBig = false; break; }
                        else if (sTemp == L">GIB<") { bBig = true; break; }
                    }
                    if(iAnzahlZeichen < 14 && bBig == false)   //Leerzeichen einfügen, falls Zeichenanzahl
                    {                                          //kleiner der maximal darstellbaren Charakter
                        /*
                         * Big behandeln:
                         * Wenn vor firstchar noch ein <BIG> kommt (ohne weitere Zeichen dazwischen) dann
                         * Leerzeichen nochmal durch zwei teilen.
                         */

                        iLeerzeichen = 14 - (iAnzahlZeichen); //Fehlende Leerzeichen berechnen

                        for(int spaces = 0;spaces < iLeerzeichen;spaces++)
                        {
                            sBefehl.sOriginal.insert(lastchar+1,L" ");
                        }

                        firstchar = lastchar = -1;

                        break;
                    }
                    else if(iAnzahlZeichen < 7 && bBig == true)   //Leerzeichen einfügen, falls Zeichenanzahl
                    {                                             //kleiner der maximal darstellbaren Charakter
                        /*
                         * Big behandeln:
                         * Wenn vor firstchar noch ein <BIG> kommt (ohne weitere Zeichen dazwischen) dann
                         * Leerzeichen nochmal durch zwei teilen.
                        */

                        iLeerzeichen = 7 - (iAnzahlZeichen); //Fehlende Leerzeichen berechnen

                        for(int spaces = 0;spaces < iLeerzeichen;spaces++)
                        {
                            sBefehl.sOriginal.insert(lastchar+1,L" ");
                        }

                        firstchar = lastchar = -1;

                        break;
                    }
                }//</else>
            }
        }
    }

    return;
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
    LauflichtCodetabelle[L"€"] = 36;
    LauflichtCodetabelle[L"/"] = 47;
    LauflichtCodetabelle[L"#"] = 35;
    LauflichtCodetabelle[L"$"] = 36;
    LauflichtCodetabelle[L"%"] = 37;
    LauflichtCodetabelle[L"&"] = 38;
    LauflichtCodetabelle[L"'"] = 39;
    LauflichtCodetabelle[L"´"] = 39;
    LauflichtCodetabelle[L"`"] = 39;
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
    LauflichtCodetabelle[L"<COLOR rainbow>"] = 32;   //Rainbow

    //Hintergrundfarben:
    LauflichtCodetabelle[L"<BGCOLOR b>"] = 0;   //Schwarz
    LauflichtCodetabelle[L"<BGCOLOR r>"] = 4;   //Rot
    LauflichtCodetabelle[L"<BGCOLOR g>"] = 8;   //Grün
    LauflichtCodetabelle[L"<BGCOLOR y>"] = 12;  //Gelb
}

std::map<std::wstring,int> SWP::CLauflicht::LauflichtCodetabelle;
