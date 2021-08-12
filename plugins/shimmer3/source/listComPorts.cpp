#include "listComPorts.h"


int listComPorts(std::vector<USBserialDevice*> *devices)
{
	//int verbose = 1;

    //if(verbose)
       // printf("Searching for COM ports...\n");

    DISPATCH_OBJ(wmiSvc);
    DISPATCH_OBJ(colDevices);

    dhInitialize(TRUE);
    dhToggleExceptions(FALSE);
 
    dhGetObject(L"winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2",
    //dhGetObject(L"winmgmts:\\\\.\\root\\cimv2",
                NULL, &wmiSvc);
    dhGetValue(L"%o", &colDevices, wmiSvc, 
               L".ExecQuery(%S)",  
               L"Select * from Win32_PnPEntity");


    int port_count = 0;

    FOR_EACH(objDevice, colDevices, NULL) {
        
        char* name = NULL;
        char* pnpid = NULL;
        char* manu = NULL;
        char* match;

        dhGetValue(L"%s", &name,  objDevice, L".Name");
        dhGetValue(L"%s", &pnpid, objDevice, L".PnPDeviceID");
                                                
        //if(verbose>1) printf("'%s'.\n", name);
        if( name != NULL && ((match = strstr( name, "(COM" )) != NULL) ) { // look for "(COM23)"
            // 'Manufacturuer' can be null, so only get it if we need it
            dhGetValue(L"%s", &manu, objDevice,  L".Manufacturer");
            port_count++;
            char* comname = strtok( match, "()");

			devices->push_back(new USBserialDevice(comname, manu, pnpid));
            //printf("%s - %s - %s\n",comname, manu, pnpid);
            dhFreeString(manu);
        }
        
        dhFreeString(name);
        dhFreeString(pnpid);
        
    } NEXT(objDevice);
    
    SAFE_RELEASE(colDevices);
    SAFE_RELEASE(wmiSvc);
    
    dhUninitialize(TRUE);
    
    //if(verbose) 
        //printf("Found %d port%s\n",port_count,(port_count==1)?"":"s");
    return 0;
}


