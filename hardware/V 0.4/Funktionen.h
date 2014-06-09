/*
    Softwareprojekt - Lauflicht Online

    Funktionen.h - beinhält sämtliche Funktionsprototypen zum Formatieren/Umwandeln des Strings sowie öffnen des Ports zur Ansprache
    des Gerätes
*/

#include <string>
#include <map>
#include <fstream>

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

    class CLauflicht
    {
        public:
            CLauflicht();

            /**
                OeffneRS232(int iComPort): Zum Öffnen des RS232 Ports.
                Als Parameter wird der zu öffnende Port verlangt und gibt true zurück,
                falls die Verbindung erfolgreich aufgebaut werden konnte, ansonsten false.
            */
            bool OeffneRS232();

            /**
                KonvertiereString(std::string sBefehl):
                Konvertiert den Befehl, der von der Website an die Software weitergegeben
                wurde, in eine für das Gerät verständliche Sequenz
            */
            void KonvertiereString(stSequenz &sBefehl);

            /**
                void LeseString(stSequenz &sBefehl): Liest einen String von der Website ein.
            */
            void LeseString(stSequenz &sBefehl);

            /**
                SendeString(stSequenz sBefehl, int iComPort): Dient zum Senden des erhaltenen String von der
                Website.
            */
            void SendeString(stSequenz sBefehl);


            /**
                SchliesseRS232(int iComPort);
                RS232-Port schließen und Kommunikation beenden
            */
            void SchliesseRS232();

        private:
            /**
                InitialisiereTabelle():
                Initialisiert die LauflichtCodetabelle mit den entsprechenden Hexwerten, damit die Steuerbefehle der
                Website korrekt in Hexzahlen übersetzt werden
            */
            void InitialisiereTabelle();

            /**
                LauflichtCodetabelle:
                Beinhält sämtliche Codes als Dezimalzahl, die dann zum Lauflicht gesendet werden können
            */
            static std::map<std::string,int> LauflichtCodetabelle;

            /**
                m_iComPort: Der verwendete Comport. Jede Nummer ist jeweils einem Port in /dev zugeordnet.
                Die Liste ist in rs232.c zu finden.
                Hier wird die Nummer 22 für ttyAMA0 verwendet.
            */
            int m_iComPort;

            /**
                m_iColors[0]: Vordergrundfarbe
                m_iColors[1]: Hintergrundfarbe
                m_iColors[2]: Berechnete Farbenkombination (Addition aus Vorder-/Hintergrundfarbe)
            */
            int m_iColors[3];

            /*
                Dient zu Debuggingzwecken:
                Damit kann nachgeprüft werden, was wie (und ob etwas überhaupt) konvertiert wurde.
            */
            std::ofstream m_debugfile;
            int iLetters;

            /**
            Die Anzahl an Zeichen, die die Laufschrift überhauüt anzeigen kann
            */

    };
}


#endif // _FUNKTIONEN_HPP
