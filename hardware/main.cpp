/*
    Softwareprojekt - Lauflicht Online
    Software zum Ansprechen des Sigma ASC 333
*/

#include "Funktionen.h"
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>

void timeoutHandler(int n)
{
    std::cerr << "Programm hat sich aufgehängt" << std::endl;
    std::exit(1);
}


int main(int argc, char *argv[])
{
    //Nach 3 Sekunden Programm töten
    timer_t timer;
    if (timer_create(CLOCK_MONOTONIC, NULL, &timer) != 0)
    {
        perror("timer_create");
        return 1;
    }
    struct itimerspec timerSpec;
    timerSpec.it_interval.tv_sec = timerSpec.it_interval.tv_nsec = 0;
    timerSpec.it_value.tv_sec = 3;
    timerSpec.it_value.tv_nsec = 0;
    if (timer_settime(timer, 0, &timerSpec, NULL) != 0)
    {
        perror("timer_settime");
        return 1;
    }
    struct sigaction act;
    std::memset(&act, 0, sizeof(act));
    act.sa_handler = timeoutHandler;
    if (sigaction(SIGALRM, &act, NULL) != 0)
    {
        perror("sigaction");
        return 1;
    }

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
