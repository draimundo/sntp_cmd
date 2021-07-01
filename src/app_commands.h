/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef APP_COMMANDS_H
#define	APP_COMMANDS_H
#include <stdbool.h>
#ifdef	__cplusplus
extern "C" {
#endif
bool APP_Commands_Init();
#define MAX_URL_SIZE 255
extern char APP_DHCP_Server_IP[MAX_URL_SIZE];
extern char APP_DHCP_Server_Port[6];
extern char APP_SNTP_Server_IP[MAX_URL_SIZE];
extern char APP_SNTP_Server_Port[6];
extern bool APP_Get_DHCP;
extern bool APP_Get_SNTP;
#ifdef	__cplusplus
}
#endif

#endif	/* APP_COMMANDS_H */
