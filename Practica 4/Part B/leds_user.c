#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>

#define LENGTH_LEDS 4

/*
 * Mask for leds. Bits:
 *  0x01 turn on/off num lock
 *  0x02 turn on/off caps lock
 *  0x04 turn on/off scroll lock
 */
unsigned int mask = 0;
// File descriptor for driver /dev/chardev_leds
int file;

/*
 * Update leds status according to mask value
 */
void updateLeds() {
    char leds[LENGTH_LEDS];
    leds[0] = '\0';

    if(mask & 0x01) {
        strcat(leds, "1");
    }
    if(mask & 0x02) {
        strcat(leds, "2");
    }
    if(mask & 0x04) {
        strcat(leds, "3");
    }
    write(file, leds, strlen(leds));
}

/*
 * Handler to turn off the motion led
 */
void motionLedOff(int signum)
{    
    mask &= 0x06;
    updateLeds();
}

int main() {    
    char *key_name[] = {
        "left",
        "middle",
        "right",
        "scroll up",
        "scroll down"
    };
    struct itimerval timer;
    Display *display;
    XEvent xevent;
    Window window;

    // Methods to capture mouse events
    if( (display = XOpenDisplay(NULL)) == NULL )
        return -1;
    window = DefaultRootWindow(display);
    XAllowEvents(display, AsyncBoth, CurrentTime);
    XGrabPointer(display, 
                 window,
                 1, 
                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask , 
                 GrabModeAsync,
                 GrabModeAsync, 
                 None,
                 None,
                 CurrentTime);

    // Open the driver to write
    file = open("/dev/chardev_leds", O_WRONLY);
    
    if(file < 0) {
        printf("Error opening the device");
        return -1;
    }

    /* Configure one-shot the timer to expire after 100 msec... */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    // At the start, the leds are off
    mask = 0;
    updateLeds();
    
    // Register timer handler
    if (signal(SIGALRM, motionLedOff) == SIG_ERR) {
        perror("Unable to catch SIGALRM");
        exit(1);
    }

    while(1) {
        XNextEvent(display, &xevent);

        switch (xevent.type) {
            case MotionNotify:
                mask |= 0x01;
                printf("Mouse move      : [%d, %d]\n", xevent.xmotion.x_root, xevent.xmotion.y_root);
                // Start/Restart a one-shot timer. It counts down whenever this process is executing.
                if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
                    perror("error calling setitimer()");
                    exit(1);
                }
                break;
            case ButtonPress:
                if(xevent.xbutton.button == 1) {
                    mask |= 0x02;
                }
                if(xevent.xbutton.button == 3) {
                    mask |= 0x04;
                }
                printf("Button pressed  : %s\n", key_name[xevent.xbutton.button - 1]);
                break;
            case ButtonRelease:
                if(xevent.xbutton.button == 1) {
                    mask &= 0x05;
                }
                if(xevent.xbutton.button == 3) {
                    mask &= 0x03;
                }
                printf("Button released : %s\n", key_name[xevent.xbutton.button - 1]);
                break;
        }
        updateLeds();
    }

    write(file, "", 0);

    close(file);
    
    return 0;
}
