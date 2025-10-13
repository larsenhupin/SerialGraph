#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h> 

#define MAX_LENGTH 100
#define MAX_COLUMNS 20
#define SERIAL_BUF_SIZE 1300
#define READ_BUF_SIZE 256

// Serial interface
typedef struct {
    char (*buffer)[MAX_COLUMNS][MAX_LENGTH];
    double *doubleListX;
    double *doubleListY;
    double *doubleListY2;
    int head, count, capacity;
    size_t cols;
    const char *separator;
} Serial;

void configureTermios(int* fileDescriptor);
void setupSerial(Serial *serial);
void cleanSerial(Serial *serial);
void readSerialLineRaw(int run, Serial *serial, int serialFd);
