/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <time.h>
#include "app.h"
#include "app_commands.h"

#include "tcpip/tcpip.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// print buffer
char printBuffer[1024];

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_TCPIP_WAIT_INIT;
    appData.tmoMs = 1000;
    appData.ntpAddr.Val = 0;
    /* TODO: Initialize your application's state machine and other
     */
    APP_Commands_Init();
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    SYS_STATUS          tcpipStat;
    const char          *netName, *netBiosName;
    static IPV4_ADDR    dwLastIP[2] = { {-1}, {-1} };
    IPV4_ADDR           ipAddr;
    TCPIP_NET_HANDLE    netH;
    int                 i, nNets;
    /* Check the application's current state. */
    switch ( appData.state )
    {
        case APP_TCPIP_WAIT_INIT:
            tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);
            if(tcpipStat < 0)
            {   // some error occurred
                SYS_CONSOLE_MESSAGE(" APP: TCP/IP stack initialization failed!\r\n");
                appData.state = APP_TCPIP_ERROR;
            }
            else if(tcpipStat == SYS_STATUS_READY)
            {
                // now that the stack is ready we can check the 
                // available interfaces 
                nNets = TCPIP_STACK_NumberOfNetworksGet();
                for(i = 0; i < nNets; i++)
                {

                    netH = TCPIP_STACK_IndexToNet(i);
                    netName = TCPIP_STACK_NetNameGet(netH);
                    netBiosName = TCPIP_STACK_NetBIOSName(netH);

#if defined(TCPIP_STACK_USE_NBNS)
                    SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS enabled\r\n", netName, netBiosName);
#else
                    SYS_CONSOLE_PRINT("    Interface %s on host %s - NBNS disabled\r\n", netName, netBiosName);
#endif  // defined(TCPIP_STACK_USE_NBNS)
                    (void)netName;          // avoid compiler warning 
                    (void)netBiosName;      // if SYS_CONSOLE_PRINT is null macro

                }
                appData.state = APP_TCPIP_WAIT_FOR_IP;

            }
            break;


        case APP_TCPIP_WAIT_FOR_IP:

            // if the IP address of an interface has changed
            // display the new value on the system console
            nNets = TCPIP_STACK_NumberOfNetworksGet();

            for (i = 0; i < nNets; i++)
            {
                netH = TCPIP_STACK_IndexToNet(i);
				if(!TCPIP_STACK_NetIsReady(netH))
				{
					return; // interface not ready yet!
				}
                ipAddr.Val = TCPIP_STACK_NetAddress(netH);
                if(dwLastIP[i].Val != ipAddr.Val)
                {
                    dwLastIP[i].Val = ipAddr.Val;

                    SYS_CONSOLE_MESSAGE(TCPIP_STACK_NetNameGet(netH));
                    SYS_CONSOLE_MESSAGE(" IP Address: ");
                    SYS_CONSOLE_PRINT("%d.%d.%d.%d \r\n", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]);
                
                }
			}
			// all interfaces ready. Could start transactions!!!
			appData.state = APP_TCPIP_WAITING_FOR_COMMAND;
            SYS_CONSOLE_MESSAGE("Waiting for command type: getdhcp\r\n");
            
			break;
        case APP_TCPIP_WAITING_FOR_COMMAND:
        {
            if(APP_Get_DHCP){
                APP_Get_DHCP = false;
                TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);
                TCPIP_DHCP_INFO pDhcpInfo;
                TCPIP_DHCP_InfoGet(netH, &pDhcpInfo);
                SYS_CONSOLE_PRINT("Number of NTP Servers: %d \r\n", pDhcpInfo.ntpServersNo);
                for(int i = 0; i < pDhcpInfo.ntpServersNo; i++)
                {
                    SYS_CONSOLE_PRINT("NTP server nb. %d : %d.%d.%d.%d \r\n", pDhcpInfo.ntpServersNo, 
                        pDhcpInfo.ntpServers[i].v[0], pDhcpInfo.ntpServers[i].v[1],
                        pDhcpInfo.ntpServers[i].v[2], pDhcpInfo.ntpServers[i].v[3]);
                }
                if(pDhcpInfo.ntpServersNo > 0)
                {
                    appData.ntpAddr.Val = pDhcpInfo.ntpServers[0].Val;
                }
            }
            
            if(APP_Get_SNTP){
               APP_Get_SNTP = false;
               if(appData.ntpAddr.Val > 0)
               {
                    TCPIP_NET_HANDLE netH = TCPIP_STACK_IndexToNet(0);
                    char ntpAddrStr[20];
                    TCPIP_Helper_IPAddressToString(&appData.ntpAddr, ntpAddrStr, 20);
                    TCPIP_SNTP_ConnectionParamSet(netH, IP_ADDRESS_TYPE_IPV4, ntpAddrStr);
                    //if(TCPIP_SNTP_ConnectionInitiate() == SNTP_RES_OK)
                    //{
                        //TCPIP_SNTP_TIME_STAMP tStamp;
                        //uint32_t lastUpdate;
                        //TCPIP_SNTP_RESULT res = TCPIP_SNTP_TimeStampGet(&tStamp, &lastUpdate);
                        //if(res == SNTP_RES_OK){
                            //SYS_CONSOLE_PRINT("Current timestamp: %lu\r\n", (uint32_t) (tStamp>>32));
                            //SYS_CONSOLE_PRINT("Last update: %lu\r\n", lastUpdate);
                        //}
                        TCPIP_SNTP_ConnectionInitiate();
                        uint32_t UTCSeconds;
                        uint32_t ms;
                        TCPIP_SNTP_TimeGet(&UTCSeconds, &ms);
                        time_t rawtime = UTCSeconds;
                        struct tm *info;
                        time( &rawtime );
                        info = localtime( &rawtime );
                        SYS_CONSOLE_PRINT("Current UTC secs: %lu\r\n", UTCSeconds);
                        SYS_CONSOLE_PRINT("Current local time and date: %s", asctime(info));
                    //}
               }
            }
            appData.state = APP_TCPIP_WAITING_FOR_COMMAND;
        }
        break;
            
        case APP_TCPIP_WAIT_FOR_CONNECTION:
        {
            if (!TCPIP_UDP_IsConnected(appData.socket))
            {
                break;
            }
            if(TCPIP_UDP_PutIsReady(appData.socket) == 0)
            {
                break;
            }
            //TCPIP_UDP_ArrayPut(appData.socket, (uint8_t*)APP_Message_Buffer, strlen(APP_Message_Buffer));
            //TCPIP_UDP_Flush(appData.socket);
            appData.mTimeOut = SYS_TMR_SystemCountGet() + (SYS_TMR_SystemCountFrequencyGet() * (uint64_t)appData.tmoMs) / 1000ull;
            appData.state = APP_TCPIP_WAIT_FOR_RESPONSE;
        }
        break;

        case APP_TCPIP_WAIT_FOR_RESPONSE:
        {
            memset(printBuffer, 0, sizeof(printBuffer));
            if (SYS_TMR_SystemCountGet() > appData.mTimeOut)
            {
                SYS_CONSOLE_MESSAGE("\r\nTime out waiting for response\r\n");
                TCPIP_UDP_Close(appData.socket);
                appData.state = APP_TCPIP_WAITING_FOR_COMMAND;
                break;
            }
            if (!TCPIP_UDP_IsConnected(appData.socket))
            {
                SYS_CONSOLE_MESSAGE("\r\nConnection Closed\r\n");
                appData.state = APP_TCPIP_WAITING_FOR_COMMAND;
                break;
            }
            if (TCPIP_UDP_GetIsReady(appData.socket))
            {
                TCPIP_UDP_ArrayGet(appData.socket, (uint8_t*)printBuffer, sizeof(printBuffer) - 1);
                TCPIP_UDP_Discard(appData.socket);
                SYS_CONSOLE_PRINT("%s", printBuffer);
                TCPIP_UDP_Close(appData.socket);
                appData.state = APP_TCPIP_WAITING_FOR_COMMAND;
            }
        }

        break;
        default:
            break;
    }
}



/*******************************************************************************
 End of File
 */
