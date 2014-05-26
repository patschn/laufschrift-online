/*
    Softwareprojekt - Lauflicht Online

    Funktionen.cpp - Implementiert die Funktionen zur Ansteuerung der Headerdatei Funktionen.h
*/

#include "Funktionen.h" //Funktionsprototypen
#include "rs232.h"      //Bibliothek von http://www.teuniz.net/RS-232/
#include <iostream>

#define RS232_PORT_NUMBER 1

bool SWP::OeffneRS232(int iComPort)
{
    //Port öffnen
    if(RS232_OpenComport(iComPort,2400) == 1)   //Rückgabe von 1 signalisiert Fehler
    {
        return false;
    }

    return true;    //Port erfolgreich geöffnet
}

void SWP::KonvertiereString(stSequenz &sBefehl)
{
    //sBefehl.sKonvertiert = "AAAAAAAAAAAAAAAAAAAA";  //Lauflicht initialisieren, sonst Gerät nicht ansprechbar

    std::string sTemp = "";
    std::string sColor = "03"; //Standardfarbe (Vordergrund: Rot, Hintergrund: Schwarz) initialisieren, da eine Farbe benötigt wird

    /*
        Start der Konvertierung - der fertige Befehl wird in sBefehl.sKonvertiert gespeichert
    */
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
            //Befehl in der Codetabelle nachschauen und konvertieren:
            sBefehl.sKonvertiert += LauflichtCodetabelle.find(sTemp)->second + sColor;
            if(sTemp == "<COLOR b>"||sTemp == "<COLOR r>"||sTemp == "<COLOR y>"||sTemp == "<COLOR g>")
            {
                sColor = LauflichtCodetabelle.find(sTemp)->second;
            }
            else
            {

            }
        }
        else    //Wenn kein Befehl gefunden, dann muss es normaler Text sein
        {
            if(sBefehl.sOriginal[i] == '/') //Falls "<", ">" oder "/" im Text einfefügt werden soll
            {
                sTemp = sBefehl.sOriginal[i] + sBefehl.sOriginal[i+1]; // "<", ">" und "/" werden als "/*zeichen*" im Befehlsstring gepseichert, daher werden 2 Zeichen benötigt
                sBefehl.sKonvertiert += sColor + LauflichtCodetabelle.find(sTemp)->second;
                i++;
            }
            else
            {
                sTemp = sBefehl.sOriginal[i];
                sBefehl.sKonvertiert += sColor + LauflichtCodetabelle.find(sTemp)->second;
            }
        sTemp = "";
    }
}
}

void SWP::SendeString(stSequenz sBefehl, int iComPort)
{
    /*
        ToDo: Hier String an Gerät senden
        Der String muss in einzelne Bytefolgen zerlegt und dann gesendet werden.

        --> Hexzahl im Format AA hat ein Byte
    */

	for(unsigned int i = 0; i < sBefehl.sKonvertiert.length();i++)
	{
		RS232_SendByte(iComPort, sBefehl.sKonvertiert[i]);
	}
}


void SWP::SchliesseRS232(int iComPort)
{
    RS232_CloseComport(iComPort);
}

void SWP::InitialisiereTabelle()
{
    //Verwendet wird die Liste, die auf http://sigma.haufe.org/index.php?content=funktionen zu finden ist,
    //denn diese Liste beinhaltet gleich die ASCII-Codes als Hexzahl

    LauflichtCodetabelle["<START>"] = "BB";     //Programm-Start
    LauflichtCodetabelle["<END>"] = "80";       //Programm-Ende
    LauflichtCodetabelle["<CLEAR>"] = "8C";     //Anzeige leeren
    LauflichtCodetabelle["<LEFT>"] = "81";      //Text scrollt nach links
    LauflichtCodetabelle["<RIGHT>"] = "82";     //Text scrollt nach rechts
    LauflichtCodetabelle["<UP>"] = "83";        //Text scrollt nach oben
    LauflichtCodetabelle["<DOWN>"] = "84";      //Text scrollt nach unten
    LauflichtCodetabelle["<JUMP>"] = "85";      //Text erscheint sofort ohne Effekt
    LauflichtCodetabelle["<OPEN>"] = "86";      //Text "von innen öffnen"
    LauflichtCodetabelle["<CLOSE>"] = "87";     //Text "von außen öffnen"
    LauflichtCodetabelle["<FLASH>"] = "88";     //Text blinkt
    LauflichtCodetabelle["<FLSHG>"] = "89";     //Text scrollt blinkend von rechts nach links
    LauflichtCodetabelle["<DOFF>"] = "8A";      //Buchstaben erscheinen einzeln, von links nach rechts
    LauflichtCodetabelle["<BIG>"] = "8B";       //Fettschrift scrollt von rechts nach links
    LauflichtCodetabelle["<RANDM>"] = "8E";     //Zufällige Auswahl des Effektes
    LauflichtCodetabelle["<SPEED>"] = "8D";     //Geschwindigkeit der Anzeige muss von einer Zahl von 1-9 gefolgt werden,
                                                //wobei 1 = schnell und 9 = langsam
    LauflichtCodetabelle["<WAIT>"] = "8F";        //Pause bei der Anzeige; muss von einer Zahl von 1-9 gefolgt werden,
                                                //wobei 1 = schnell und 9 = langsam
    LauflichtCodetabelle["<COLOR r>"] = "AB";       //FEHLER
    LauflichtCodetabelle["<SNOW>"] = "CD";        //FEHLER

    LauflichtCodetabelle["0"] = "30";
    LauflichtCodetabelle["1"] = "31";
    LauflichtCodetabelle["2"] = "32";
    LauflichtCodetabelle["3"] = "33";
    LauflichtCodetabelle["4"] = "34";
    LauflichtCodetabelle["5"] = "35";
    LauflichtCodetabelle["6"] = "36";
    LauflichtCodetabelle["7"] = "37";
    LauflichtCodetabelle["8"] = "38";
    LauflichtCodetabelle["9"] = "39";

    LauflichtCodetabelle["A"] = "41";   LauflichtCodetabelle["N"] = "4E";
    LauflichtCodetabelle["B"] = "42";   LauflichtCodetabelle["O"] = "4F";
    LauflichtCodetabelle["C"] = "43";   LauflichtCodetabelle["P"] = "50";
    LauflichtCodetabelle["D"] = "44";   LauflichtCodetabelle["Q"] = "51";
    LauflichtCodetabelle["E"] = "45";   LauflichtCodetabelle["R"] = "52";
    LauflichtCodetabelle["F"] = "46";   LauflichtCodetabelle["S"] = "53";
    LauflichtCodetabelle["G"] = "47";   LauflichtCodetabelle["T"] = "54";
    LauflichtCodetabelle["H"] = "48";   LauflichtCodetabelle["U"] = "55";
    LauflichtCodetabelle["I"] = "49";   LauflichtCodetabelle["V"] = "56";
    LauflichtCodetabelle["J"] = "4A";   LauflichtCodetabelle["W"] = "57";
    LauflichtCodetabelle["K"] = "4B";   LauflichtCodetabelle["X"] = "58";
    LauflichtCodetabelle["L"] = "4C";   LauflichtCodetabelle["Y"] = "59";
    LauflichtCodetabelle["M"] = "4D";   LauflichtCodetabelle["Z"] = "5A";

    LauflichtCodetabelle["a"] = "61";   LauflichtCodetabelle["n"] = "6E";
    LauflichtCodetabelle["b"] = "62";   LauflichtCodetabelle["o"] = "6F";
    LauflichtCodetabelle["c"] = "63";   LauflichtCodetabelle["p"] = "70";
    LauflichtCodetabelle["d"] = "64";   LauflichtCodetabelle["q"] = "71";
    LauflichtCodetabelle["e"] = "65";   LauflichtCodetabelle["r"] = "72";
    LauflichtCodetabelle["f"] = "66";   LauflichtCodetabelle["s"] = "73";
    LauflichtCodetabelle["g"] = "67";   LauflichtCodetabelle["t"] = "74";
    LauflichtCodetabelle["h"] = "68";   LauflichtCodetabelle["u"] = "75";
    LauflichtCodetabelle["i"] = "69";   LauflichtCodetabelle["v"] = "76";
    LauflichtCodetabelle["j"] = "6A";   LauflichtCodetabelle["w"] = "77";
    LauflichtCodetabelle["k"] = "6B";   LauflichtCodetabelle["x"] = "78";
    LauflichtCodetabelle["l"] = "6C";   LauflichtCodetabelle["y"] = "79";
    LauflichtCodetabelle["m"] = "6D";   LauflichtCodetabelle["z"] = "7A";
    //Sonderzeichen:
    LauflichtCodetabelle["Ȧ"] = "03";
    LauflichtCodetabelle["£"] = "06";
    LauflichtCodetabelle["¤"] = "07";
    LauflichtCodetabelle["Ω"] = "08";
    LauflichtCodetabelle["Ö"] = "0F";

    LauflichtCodetabelle["ø"] = "10";
    LauflichtCodetabelle["ȧ"] = "13";
    LauflichtCodetabelle["§"] = "14";
    LauflichtCodetabelle["卍"] = "16"; //Hakenkreuz
    LauflichtCodetabelle["Σ"] = "17";

    LauflichtCodetabelle[":"] = "20";
    LauflichtCodetabelle["!"] = "21";
    LauflichtCodetabelle["\""] = "22";
    LauflichtCodetabelle["#"] = "23";
    LauflichtCodetabelle["$"] = "24";
    LauflichtCodetabelle["%"] = "25";
    LauflichtCodetabelle["&"] = "26";
    LauflichtCodetabelle["'"] = "27";
    LauflichtCodetabelle["("] = "28";
    LauflichtCodetabelle[")"] = "29";
    LauflichtCodetabelle["*"] = "2A";
    LauflichtCodetabelle["+"] = "2B";
    LauflichtCodetabelle[","] = "2C";
    LauflichtCodetabelle["-"] = "2D";
    LauflichtCodetabelle["."] = "2E";
    LauflichtCodetabelle["/"] = "2F";

    LauflichtCodetabelle["∅"] = "3B"; //großes Durchschnittszeichen
    LauflichtCodetabelle["Æ"] = "3C";
    LauflichtCodetabelle["="] = "3D";
    LauflichtCodetabelle["?"] = "3F";

    LauflichtCodetabelle["@"] = "40";

    LauflichtCodetabelle["Ü"] = "5B";
    LauflichtCodetabelle["ù"] = "5C";
    LauflichtCodetabelle["è"] = "5D";
    LauflichtCodetabelle["^"] = "5E";

    LauflichtCodetabelle["É"] = "60";

    LauflichtCodetabelle[";"] = "7B";
    LauflichtCodetabelle["Ñ"] = "7C";
    LauflichtCodetabelle["ñ"] = "7D";
    LauflichtCodetabelle["Ä"] = "7E";
    LauflichtCodetabelle["ä"] = "7F";
}
