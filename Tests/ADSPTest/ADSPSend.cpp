/* ADSPSend.cp */

#include <netat/AppleTalk.h>
#include <sys/socket.h>
#include <netat/ADSP.h>

extern "C" {
	#include "at_proto.h"
}

// ANSI C
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

static int CreateADSPSender( at_inet_t* outTarget );

static int CreateADSPSender( at_inet_t* outTarget )
{
	int theClientSocket = -1;
    int	err;
	do {
	    at_inet_t	name;
	    int			namelen = sizeof( at_inet_t );

	    theClientSocket = ADSPsocket(AF_APPLETALK, SOCK_STREAM, 0);
	    if (theClientSocket < 0)
	    {
	        printf("Failed to open ADSP socket %d\n", theClientSocket);
	        break;
	    }
	    err = ADSPbind(theClientSocket, 0, 0);
	    if (err < 0)
	    {
	        printf("Failed to open ADSPbind %d\n", err);
	        break;
	    }
	    err = ADSPgetsockname(theClientSocket, &name, &namelen);
	    if (err < 0)
	    {
	        printf("Failed to open ADSPgetsocketname %d\n", err);
	        break;
	    }
    
	    printf("net number is %d\n", name.net);
	    printf("node number is %d\n", name.node);
	    printf("socket number is %d\n", name.socket);

	    outTarget->net = name.net;
	    outTarget->node = name.node;
	} while ( false );

	
    if ((err) && (theClientSocket >= 0))
    {
		(void) ADSPclose (theClientSocket);
		theClientSocket = err;
	}
    
	return theClientSocket;
}

int main(int argc, char* argv [])
{
	(void) ::printf( "This is ADSPClient\n" );
	
	if ((argc != 1) && (argc != 2) && (argc != 4))
	{
		(void) ::printf( "Bad syntax. Syntax is %s [[name] | [net node socket]]\n", argv[0] );
		::exit(1);
	}
	int err;
	int theClientSocket;
	do {
		at_inet_t target;
		theClientSocket = CreateADSPSender( &target );
		if (theClientSocket < 0)
		{
			err = theClientSocket;
			break;
		}

		if (argc == 4)
		{
			target.net = ::atoi(argv[1]);
			target.node = ::atoi(argv[2]);
			target.socket = ::atoi(argv[3]);
		} else {
			// We do an NBP lookup
			char* theTestMachine = "Test";
			char* theMachine;
			if (argc == 2)
			{
				theMachine = argv[1];
			} else {
				theMachine = theTestMachine;
			}
			char* theType = "Docker";
			char* theZone = "*";

			at_entity_t theMachineEntity;
			err = ::nbp_make_entity(
							&theMachineEntity, theMachine,
							theType, theZone);
	    	if (err < 0)
	    	{
				printf("Failed with nbp_make_entity %i\n", err);
				break;
			}
			
			at_nbptuple theResult;
			int entryCount = ::nbp_lookup(&theMachineEntity, &theResult, 1, NULL /* retry */ );
			if (entryCount != 1)
			{
				(void) ::printf("nbplookup failed and returned %i\n", entryCount);
				break;				
			}
			
			target = theResult.enu_addr;
		}

	    (void) ::printf("server net number is %d\n", target.net);
	    (void) ::printf("server node number is %d\n", target.node);
	    (void) ::printf("server number is %d\n", target.socket);

		err = ADSPconnect(theClientSocket, &target, sizeof(target));
	    if (err < 0)
	    {
	        (void) ::printf("Failed to open ADSPconnect %d\n", err);
	        break;
	    }

	    char* theMessage = "Just a test!";
	    int size = ::strlen(theMessage);
	    int sentCount = ADSPsend(theClientSocket, theMessage, size, 0);

	    if(sentCount < 0)
	    {
	    	err = sentCount;
		    (void) ::printf("Echec lors de l'Žcriture %d\n", err );
	    	break;
		}

		(void) ::printf("sent %d bytes\n", sentCount);
	} while ( false );
	
	if (theClientSocket > 0)
	{
		// On ferme.
		int theCloseErr = ::ADSPclose( theClientSocket );
		if (err == 0)
		{
			err = theCloseErr;
		}
	}
	
	if (err)
	{
		(void) ::printf( "Error: %d - %d\n", err, errno );
	}
	
	return err;
}
