/*
    Softwareprojekt - Lauflicht Online

    Funktionen.h - beinhält sämtliche Funktionsprototypen zum Formatieren/Umwandeln des Strings sowie öffnen des Ports zur Ansprache
    des Gerätes
*/

#include <string>
#include <map>

#ifndef _FUNKTIONEN_HPP
#define _FUNKTIONEN_HPP

namespace SWP
{
    //stSequenz: Enthält sowohl die Original, als auch die konvertierte Sequenz
    struct stSequenz
    {
        std::string sOriginal;      //Hier ist der String, der von der Webseite übermittelt wurde
        std::string sKonvertiert;   //Hier ist (nach der Konvertierung) der String als HexCode
    };

    /**
        OeffneRS232(int iComPort): Zum Öffnen des RS232 Ports.
        Als Parameter wird der zu öffnende Port verlangt und gibt true zurück,
        falls die Verbindung erfolgreich aufgebaut werden konnte, ansonsten false.
    */
    bool OeffneRS232(int iComPort);

    /**
        KonvertiereString(std::string sBefehl):
        Konvertiert den Befehl, der von der Website an die Software weitergegeben
        wurde, in eine für das Gerät verständliche Sequenz als Hexbefehl
    */
    void KonvertiereString(stSequenz &sBefehl);


    void SendeString(stSequenz sBefehl, int iComPort);


    /**
        SchliesseRS232(int iComPort);
        RS232-Port schließen und Kommunikation beenden
    */
    void SchliesseRS232(int iComPort);

    /**
        InitialisiereTabelle():
        Initialisiert die LauflichtCodetabelle mit den entsprechenden Hexwerten, damit die Steuerbefehle der
        Website korrekt in Hexzahlen übersetzt werden
    */
    void InitialisiereTabelle();

    /**
        LauflichtCodetabelle:
        Beinhält sämtliche Codes als Hexzahl, die dann zum Lauflicht gesendet werden können
    */

    static std::map<std::string,std::string> LauflichtCodetabelle;
}


#endif // _FUNKTIONEN_HPP
