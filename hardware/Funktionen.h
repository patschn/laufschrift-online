/*
    Softwareprojekt - Lauflicht Online

    Funktionen.h - beinhält sämtliche Funktionsprototypen zum Formatieren/Umwandeln des Strings sowie öffnen des Ports zur Ansprache
    des Gerätes
*/

#include <string>
#include <map>
#include <fstream>
#include "serialib.h"
#ifndef _FUNKTIONEN_HPP
#define _FUNKTIONEN_HPP

#define COLOR_FG 0
#define COLOR_BG 1
#define COLOR_FB 2

namespace SWP
{
    /**
		stSequenz: Enthält sowohl die Original, als auch die konvertierte Sequenz
	*/
    struct stSequenz
    {
        std::wstring sOriginal;      //Hier ist der String, der von der Webseite übermittelt wurde
        std::string sKonvertiert;    //Hier ist (nach der Konvertierung) der String als HexCode
    };

    class CLauflicht
    {
        public:
            CLauflicht();

            /**
                bool OeffneRS232(): Zum Öffnen des RS232 Ports.
                Öffnet die RS232-Schnittstelle mit einem vordefinierten ComPort (ttyAMA0)
            */
            bool OeffneRS232();

            /**
                bool KonvertiereString(stSequenz &sBefehl):
                Konvertiert den Befehl, der von der Website an die Software weitergegeben
                wurde, in eine für das Gerät verständliche Sequenz. Gibt zurück, ob die
                Konvertierung erfolgreich war (true) oder fehlgeschlagen (false) ist.
            */
            bool KonvertiereString(stSequenz &sBefehl);

            /**
                void LeseString(stSequenz &sBefehl):
                Liest einen String von der Website ein.
            */
            void LeseString(stSequenz &sBefehl);

            /**
                void SendeString(stSequenz sBefehl): Dient zum Senden des erhaltenen String
                von der Website.
            */
            void SendeString(stSequenz sBefehl);

            /**
                SchliesseRS232();
                RS232-Port (ttyAMA0) schließen und Kommunikation beenden
            */
            void SchliesseRS232();

        private:
            /**
                void InitialisiereTabelle():
                Initialisiert die LauflichtCodetabelle mit den entsprechenden Dezimalwerten,
                damit die Steuerbefehle der Website korrekt übersetzt werden
            */
            void InitialisiereTabelle();

            /**
				int GetCode(std::wstring wTemp):
				Sucht den passenden Zeichencode aus der LauflichtCodetabelle.
             */
            int GetCode(std::wstring wTemp);

            /**
				int GetCode(std::string wTemp, bool bFlagBig):
				Sucht den passenden Zeichencode aus der LauflichtCodetabelle.
				Hierbei wird der Befehl <BIG> berücksichtigt.
            */
            int GetCode(std::wstring wTemp, bool bFlagBig);

            /**
                GetClock(std::wstring sClock):
                Gibt die aktuelle Systemuhrzeit zurück.
            */
            std::string GetClock();//std::wstring sClock);

            /**
				void CheckClock(stSequenz &sBefehl):
				Prüft, ob vor CheckClock eine Animation ist.
				Falls ja: Fehler
				Falls nein: OK
            */
            void CheckClock(stSequenz &sBefehl);

            /**
				void CheckAnimation(stSequenz &sBefehl):
				Prüft, ob vor jeder Zeichenkette eine Anfangsanimation ist.
            */
            void CheckAnimation(stSequenz &sBefehl);

            /**
            	void AutoLeft(stSequenz &sBefehl):
            	Behandelt die Befehle Autocenter und Left. 
            	
            	Autocenter war ursprünglich nur in der Originalsoftware verfügbar,
            	wird aber hier ebenfalls implementiert.
            	
            	Left muss aufgrund undefiniertem Verhalten behandelt werden 
            	(Left wiederholte die Sequenzen ohne dazwischen Leerzeichen einzufügen.)
            */
            void AutoLeft(stSequenz &sBefehl);

            /**
                std::map<std::wstring,int> LauflichtCodetabelle:
                Beinhält sämtliche Codes als Dezimalzahl, die dann zum Lauflicht gesendet werden können
            */
            static std::map<std::wstring,int> LauflichtCodetabelle;

            /**
                int m_iComPort: Der verwendete Comport. Jede Nummer ist jeweils einem Port in /dev zugeordnet.
                Die Liste ist in rs232.c zu finden.
                Hier wird die Nummer 22 für ttyAMA0 verwendet.
            */
            std::string m_sComPort;

			/**
				m_iLetters:
				Anzahl der Zeichen die die Laufschrift anzeigen kann.
				Wird zum zentrieren und auffüllen der Textzeile benötigt
			*/

            int m_iLetters;

            /**
				bool m_bFlagBig:
				Flag, der anzeigt ob <BIG> im Befehlsstring vorkommt. Die "breiten" Zeichen und Befehle sind in der Laufschrift intern (also
				im Speicher) um 128 'Speicherstellen' verschoben gespeichert.
             */

            bool m_bFlagBig;

            /**
            	bool m_bFlagFail:
            	Flag, der anzeigt ob der Konvertierungsvorgang erfolgreich war.
             */
            bool m_bFlagFail;

            /**
                Klasse Serialib:
                Dient der Verwaltung der RS232-Schnittstelle (Öffnen,Schließen,Senden)...
            */
            serialib m_sl;
    };
}


#endif // _FUNKTIONEN_HPP
