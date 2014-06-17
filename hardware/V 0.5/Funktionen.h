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

#define COLOR_FG 0
#define COLOR_BG 1
#define COLOR_FB 2

namespace SWP
{
    //stSequenz: Enthält sowohl die Original, als auch die konvertierte Sequenz
    struct stSequenz
    {
        std::wstring sOriginal;      //Hier ist der String, der von der Webseite übermittelt wurde
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
            bool KonvertiereString(stSequenz &sBefehl);

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
				GetCode(std::string wTemp):
				Sucht den passenden Zeichencode aus der LauflichtCodetabelle.
             */
            int GetCode(std::wstring wTemp);

            /**
				GetCode(std::string wTemp, bool bFlagBig):
				Sucht den passenden Zeichencode aus der LauflichtCodetabelle.
				Hierbei wird der Befehl <BIG> berücksichtigt
            */
            int GetCode(std::wstring wTemp, bool bFlagBig);

            /**
                GetClock(std::string sClock):
            */
            std::string GetClock(std::string sClock);


            /**
                LauflichtCodetabelle:
                Beinhält sämtliche Codes als Dezimalzahl, die dann zum Lauflicht gesendet werden können
            */
            static std::map<std::wstring,int> LauflichtCodetabelle;

            /**
                m_iComPort: Der verwendete Comport. Jede Nummer ist jeweils einem Port in /dev zugeordnet.
                Die Liste ist in rs232.c zu finden.
                Hier wird die Nummer 22 für ttyAMA0 verwendet.
            */
            int m_iComPort;

            /**
                m_iColors[COLOR_FG]: Vordergrundfarbe
                m_iColors[COLOR_BG]: Hintergrundfarbe
                m_iColors[COLOR_FB]: Berechnete Farbenkombination (Addition aus Vorder-/Hintergrundfarbe)
            */
            int m_iColors[3];

			/**
			m_iLetters:
			Anzahl der Zeichen die die Laufschrift anzeigen kann
			Wird zum zentrieren und auffüllen der Textzeile benötigt
			*/

            int m_iLetters;

			/**
			m_bFlagLeft:
			Flag, der anzeigt ob <LEFT> im Befehlsstring vorkommt
			true: kommt vor, false: kommt nicht vor
			*/

            bool m_bFlagLeft;

            /**
                    m_bFlagAutocenter:
                    Flag, der anzeigt ob <AUTOCENTER> im Befehlsstring vorkommt
                    true: kommt vor, false: kommt nicht vor
            */

            bool m_bFlagAutocenter;

            /**
                    m_bFlagBig:
                    Flag, der anzeigt ob <BIG> im Befehlsstring vorkommt
             */

            bool m_bFlagBig;

            /**
            	m_bFlagFail:
            	Flag, der anzeigt ob der Konvertierungsvorgang erfolgreich war
             */
            bool m_bFlagFail;
            /*
                Dient zu Debuggingzwecken:
                Damit kann nachgeprüft werden, was wie (und ob etwas überhaupt) konvertiert wurde.
            */
            std::wofstream m_debugfile;
    };
}


#endif // _FUNKTIONEN_HPP