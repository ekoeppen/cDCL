#include <DCL/Interfaces/POSIX/Compat/getopt.h>
#include <string.h>
#include <stdio.h>

char* optarg = NULL;
int optind = 1;
int opterr = 1;
int optopt = '?';

int
getopt( int argc, char* const argv[], const char* optstring )
{
    if ( optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0' )
    {
        return -1;
    }

    if ( strcmp ( argv[optind], "--" ) == 0 )
    {
        optind++;
        return -1;
    }

    char opt = argv[optind][1];
    const char* p = strchr ( optstring, opt );

    if ( p == NULL || opt == ':' )
    {
        optopt = opt;
        if ( opterr && *optstring != ':' )
        {
            fprintf ( stderr, "Unknown option -%c\n", opt );
        }
        return '?';
    }

    if ( p[1] == ':' )
    {
        if ( argv[optind][2] != '\0' )
        {
            optarg = &argv[optind][2];
            optind++;
        }
        else if ( optind + 1 < argc )
        {
            optind++;
            optarg = argv[optind];
            optind++;
        }
        else
        {
            optopt = opt;
            if ( opterr && *optstring != ':' )
            {
                fprintf ( stderr, "Option -%c requires an argument\n", opt );
            }
            return ( *optstring == ':' ) ? ':' : '?';
        }
    }
    else
    {
        optind++;
        optarg = NULL;
    }

    return opt;
}
