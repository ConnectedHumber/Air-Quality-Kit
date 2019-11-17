#ifndef WEBSERVER_H

#define	WEBSERVER_H

#include "settings.h"
#include "processes.h"
#include "connectwifi.h"

int startWebServer(struct process * webserverProcess);
int updateWebServer(struct process * webserverProcess);
int stopWebserver(struct process * webserverProcess);
void webserverStatusMessage(struct process * webserverProcess, char * buffer, int bufferLength);

#endif