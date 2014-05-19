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

void Testfunktion()
{
    SWP::stSequenz s;

    s.sOriginal = "<START><LEFT>1553<END>";
    std::cout << "Originalstring: " << s.sOriginal << std::endl;
    SWP::InitialisiereTabelle();
    SWP::KonvertiereString(s);

    std::cout << "Konvertierter string: " << s.sKonvertiert;
}


int main(int argc, char *argv[])
{
    /*Dient zu Debuggingzwecken: */
    std::ofstream myfile;
    myfile.open("debug.txt");
    myfile << "Originalstring:\n";

    SWP::stSequenz Sequenz;

    //String speichern:
    for(std::string line; std::getline(std::cin, line);)
    {
        myfile << line;
    }

    //Codetabelle initialisieren
    SWP::InitialisiereTabelle();

    //String auf gültige Befehle prüfen
    //ToDo:...

    //Verbindung öffnen
    //if(RS232_OpenComport(1,115200) == 1)
    /*{
        //Verbindungsfehler
        return 1;   //Fehler beim Aufbau der Verbindung
    }*/

    //Sequenz konvertieren
    SWP::KonvertiereString(Sequenz);
    myfile << Sequenz.sKonvertiert;





    //Filestream schließen
    myfile.close();

    //Erfolg
    return 0;
}
