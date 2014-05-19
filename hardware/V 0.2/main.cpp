/*
    Softwareprojekt - Lauflicht Online
    Software zum Ansprechen des Sigma ASC 333
*/

#include "Funktionen.h"
#include <iostream>

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


int main(int argc, char **argv)
{
    Testfunktion();

    return 0;
}
