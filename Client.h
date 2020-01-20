#ifndef CLIENT_H
#define CLIENT_H
/*
# Description - Client_Run func inits the client and run client thread
# Parameters - serverPortAddr - port number. inputFile - file to log. isHuman - read from text or human
# Returns - success_code if good, failure_code else
*/
int Client_Run(unsigned short serverPortAddr, FILE *inputFile, BOOL isHuman);

#endif
