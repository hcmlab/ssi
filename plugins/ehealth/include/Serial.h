#ifndef SERIALCLASS_H_INCLUDED
#define SERIALCLASS_H_INCLUDED

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

class Serial
{
    private:
        //Serial comm handler
        HANDLE hSerial;
        //Connection status
        bool connected;
        //Get various information about the connection
        COMSTAT status;
        //Keep track of last error
        DWORD errors;

    public:
        //Initialize Serial communication with the given COM port
        Serial(const char *portName, DWORD speed = CBR_115200);
        //Close the connection
        //NOTA: for some reason you can't connect again before exiting
        //the program and running it again
        ~Serial();
        //Read data in a buffer. The function return -1 when nothing could
        //be read, the number of bytes actually read.
        int ReadData(char *buffer, unsigned int nbChar);
        //Writes data from a buffer through the Serial connection
        //return true on success.
        bool WriteData(char *buffer, unsigned int nbChar);
        //Check if we are actually connected
        bool IsConnected();
};

#endif // SERIALCLASS_H_INCLUDED