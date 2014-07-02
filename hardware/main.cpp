/*
    Softwareprojekt - Lauflicht Online
    Software zum Ansprechen des Sigma ASC 333
*/

#include "Funktionen.h"
#include <iostream>

int main(int argc, char *argv[])
{
    SWP::stSequenz Sequenz;
    Sequenz.sOriginal = L"";
    Sequenz.sKonvertiert = "";
    SWP::CLauflicht Lauflicht;

    //Sequenz erhalten
    Lauflicht.LeseString(Sequenz);

    //Sequenz konvertieren
    if(Lauflicht.KonvertiereString(Sequenz) == false)
    {
        return 1;
    }

    //Verbindung öffnen
    if(Lauflicht.OeffneRS232() == false)
    {
        //Verbindungsfehler
        return 1;   //Fehler beim Aufbau der Verbindung
    }

    Lauflicht.SendeString(Sequenz);

    //Verbindung schließen
    Lauflicht.SchliesseRS232();

    //Erfolg
    return 0;
}
