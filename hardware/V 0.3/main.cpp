/*
    Softwareprojekt - Lauflicht Online
    Software zum Ansprechen des Sigma ASC 333
*/

#include "Funktionen.h"
#include <iostream>
#include <fstream>

/*
    Einsprungspunkt der Software:
    Als Parameter werden sowohl die Befehle, als auch der String übermittelt und in argv abgelegt.
*/

int main(int argc, char *argv[])
{
    /* Dient zu Debuggingzwecken: */
    std::ofstream myfile;
    myfile.open("debug.txt");
    myfile << "Originalstring:\n";

    SWP::stSequenz Sequenz;

    //String speichern:
    for(std::string line; std::getline(std::cin, line);)
    {
        myfile << line << "\n"; //DEBUG
        Sequenz.sOriginal = line;
    }

    //Codetabelle initialisieren
    SWP::InitialisiereTabelle();

    //String auf gültige Befehle prüfen
    //ToDo:...

    //Verbindung öffnen
    if(!SWP::OeffneRS232(1))
    {
        //Verbindungsfehler
        std::cerr << "Fehler beim Öffnen des Com-Ports!" << std::endl;

        return 1;
    }

    //Sequenz konvertieren
    SWP::KonvertiereString(Sequenz);
    myfile << "Konvertiert: \n";
    myfile << Sequenz.sKonvertiert;



    //Filestream schließen
    myfile.close();

    //Erfolg
    return 0;
}
