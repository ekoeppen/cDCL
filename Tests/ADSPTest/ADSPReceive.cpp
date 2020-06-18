/* ADSPReceive.cp */

#include <netat/AppleTalk.h>
#include <sys/socket.h>
#include <netat/ADSP.h>

extern "C" {
	#include "at_proto.h"
}

// ANSI C
#include <stdio.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <string.h>

// bpf
#include <net/bpf.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int CreateADSPServer(void);

static int CreateADSPServer(void)
{
	int theServerSocket = -1;
    int	err;
	do {
	    at_inet_t	name;
	    int			namelen = sizeof( at_inet_t );

	    theServerSocket = ADSPsocket(AF_APPLETALK, SOCK_STREAM, 0);
	    if (theServerSocket < 0)
	    {
	        printf("Failed to open ADSP socket %d\n", theServerSocket);
	        break;
	    }
	    err = ADSPbind(theServerSocket, 0, 0);
	    if (err < 0)
	    {
	        printf("Failed to open ADSPbind %d\n", err);
	        break;
	    }
    
	    err = ADSPgetsockname(theServerSocket, &name, &namelen);
	    if (err < 0)
	    {
	        printf("Failed to open ADSPgetsocketname %d\n", err);
	        break;
	    }
    
	    printf("net number is %d\n", name.net);
	    printf("node number is %d\n", name.node);
	    printf("socket number is %d\n", name.socket);
    
		char *theMachine = "Test";
		char *theType = "Docker";
		char *theZone = "*";

		at_entity_t theMachineEntity;
		err = ::nbp_make_entity(
						&theMachineEntity, theMachine,
						theType, theZone);
    	if (err < 0)
    	{
			printf("Failed to open nbp_make_entity %i\n", err);
			break;
		}

		err = ::nbp_reg_lookup( &theMachineEntity, NULL /* retry */ );
    	if (err != 0)
    	{
			printf("Failed to open nbp_reg_lookup %i\n", err);
			break;
		}

		err = ::nbp_register( &theMachineEntity, theServerSocket, NULL /* retry */ );
    	if (err != 0)
    	{
			printf("Failed to open nbp_register %i\n", err);
			break;
		}

    	err = ADSPlisten(theServerSocket, 1);
    	if (err < 0)
    	{
			printf("Failed to open ADSPlisten %i\n", err);
			break;
		}
	} while ( false );
	
    if ((err) && (theServerSocket >= 0))
    {
		(void) ADSPclose (theServerSocket);
		theServerSocket = err;
	}
    
	return theServerSocket;
}

int main(int /* argc */, char * /* argv */[])
{
	(void) ::printf( "This is ADSPServer\n" );
	
	// Code trs largement inspirŽ de libpcap
	char theBPFDevicePath[sizeof "/dev/bpf0000000000"];
	int bpf_fd = -1;
	int indexBpf = 0;
	while (true)
	{
		(void) ::snprintf( theBPFDevicePath, sizeof(theBPFDevicePath), "/dev/bpf%d", indexBpf++ );
		
		// On essaye d'ouvrir.
		bpf_fd = ::open( theBPFDevicePath, O_RDONLY );
		
		if (bpf_fd >= 0)
		{
			break;	// RŽussi.
		}
		
		if (errno != EBUSY)
		{
			// Echec en erreur.
			break;
		}
		
		// Sinon, on essaye le suivant.
	}
	
	if (bpf_fd < 0)
	{
		(void) ::printf( "Impossible d'ouvrir un bpf (%i)\n", errno );
	} else {
		// On associe ˆ en1 (Airport) -- il faudrait rŽcupŽrer depuis IOKit.
		struct ifreq ifr;
		(void) ::strncpy(ifr.ifr_name, "en1", sizeof(ifr.ifr_name));
		if (ioctl(bpf_fd, BIOCSETIF, &ifr) < 0)
		{
			(void) ::printf( "Echec avec BIOCSETIF (%i)\n", errno );
		} else {
			// On configure en promiscuous.
			if (::ioctl(bpf_fd, BIOCPROMISC, NULL) < 0)
			{
				(void) ::printf( "Echec avec BIOCPROMISC (%i)\n", errno );
			}
		}
	}

	int err = 0;
	int theServerSocket = -1;
	int theClientSocket = -1;
	do {
		theServerSocket = CreateADSPServer();
		if (theServerSocket < 0)
		{
			err = theServerSocket;
	        printf("error occurred calling CreateADSPServer %i\n", err);
			break;
		}
		
	    at_inet_t	name;
	    int			namelen = sizeof( at_inet_t );
	    theClientSocket = ::ADSPaccept(theServerSocket, &name, &namelen);
	    
	    if (theClientSocket < 0)
	    {
	    	err = theClientSocket;
	        printf("error occurred calling ADSPAccept %i\n", err);
			break;
	    }

        printf("sender net number is %d\n", name.net);
        printf("sender node number is %d\n", name.node);
        printf("sender socket number is %d\n", name.socket);
	    
	    char tmpstr[64];
	    int size = sizeof(tmpstr);
	    int recvCount = ::ADSPrecv(theClientSocket,tmpstr,size,0);

	    if(recvCount < 0)
	    {
	    	err = recvCount;
		    ::printf("Echec lors de la lecture %d\n", err );
	    	break;
		}

		::printf("received %d bytes\n", recvCount);
	} while ( false );
	
	if (theServerSocket > 0)
	{
		// On ferme.
		int theCloseErr = ::ADSPclose( theServerSocket );
		if ((err == 0) && (theCloseErr))
		{
			err = theCloseErr;
			::printf("Erreur ˆ la fermeture du serveur %d\n", err );
		}
	}
	
	if (theClientSocket > 0)
	{
		// On ferme.
		int theCloseErr = ::ADSPclose( theClientSocket );
		if ((err == 0) && (theCloseErr))
		{
			err = theCloseErr;
			::printf("Erreur ˆ la fermeture du client %d\n", err );
		}
	}

	if (err)
	{
		::printf( "Error: %i - %i\n", err, errno );
	}
	
	// On ferme le bpf.
	if (bpf_fd >= 0)
	{
		(void) ::close( bpf_fd );
	}
	
	return err;
}
