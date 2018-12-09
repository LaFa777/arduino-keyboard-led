#include <stdio.h>    // Standard input/output definitions
#include <stdlib.h>
#include <string.h>   // String function definitions
#include <unistd.h>   // for usleep()
#include <getopt.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#include "arduino-serial/arduino-serial-lib.h"

void usage(void)
{
    printf("Usage: keyboard-notifier -b <bps> -p <serialport>\n"
    "\n"
    "Options:\n"
    "  -h, --help                 Print this help message\n"
    "  -b, --baud=baudrate        Baudrate (bps) of Arduino (default 9600)\n"
    "  -p, --port=serialport      Serial port Arduino is connected to\n"
    "  -t  --timeout=millis       Timeout for reads in millisecs (default 5000)\n"
    "\n");
    exit(EXIT_SUCCESS);
}

void error(char* msg)
{
    fprintf(stderr, "%s\n",msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    const int buf_max = 256;

    int fd = -1;
    char serialport[buf_max];
    strcpy(serialport, "/dev/ttyUSB0");
    int baudrate = 9600;
    int timeout = 5000;
    int rc;

    if (argc==1) {
        usage();
        exit(EXIT_SUCCESS);
    }

    /* parse options */
    int option_index = 0, opt;
    static struct option loptions[] = {
        {"help",       no_argument,       0, 'h'},
        {"port",       required_argument, 0, 'p'},
        {"baud",       required_argument, 0, 'b'},
        {"timeout",    required_argument, 0, 't'},
        {NULL,         0,                 0, 0}
    };

    while(1) {
        opt = getopt_long (argc, argv, "hp:b:t:",
                           loptions, &option_index);
        if (opt==-1) break;
        switch (opt) {
            case 'h':
                usage();
                break;
            case 'p':
                strcpy(serialport,optarg);
                break;
            case 'b':
                baudrate = strtol(optarg,NULL,10);
                break;
            case 't':
                timeout = strtol(optarg,NULL,10);
                break;
        }
    }

    // открываем порт
    fd = serialport_init(serialport, baudrate);
    if( fd==-1 ) error("couldn't open port");
    printf("opened port %s\n",serialport);
    serialport_flush(fd);

    // инициализируем всё необходимое для иксов
    XkbStateRec state;
    XkbDescPtr desc;
    char *group;
    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    char prevLang; // 'R' - Russian, 'E' - English*
    int sendingbyte;
    while(True)
    {
        // получаем текущий язык в системе
        XkbGetState(dpy, XkbUseCoreKbd, &state);
        desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
        group = XGetAtomName(dpy, desc->names->groups[state.group]);

        // если язык изменился
        if(group[0] != prevLang)
        {
            // Для English отправляем 0
            // Для Russian отправляем 1
            sendingbyte = (group[0] == 'E') ? 0 : 1;

            rc = serialport_writebyte(fd, sendingbyte);
            if(rc==-1) error("error writing");
            
            printf("Changed lang: %s\n", group);

            prevLang = group[0];
        }
    }

    XFree(group);
    XCloseDisplay(dpy);
    serialport_close(fd);

    exit(EXIT_SUCCESS);
}
