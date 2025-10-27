#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h> 

#define SERIAL_MAX_TOKEN_LEN 100
#define SERIAL_MAX_COLUMNS 20
#define SERIAL_MAX_LINES 1300
#define SERIAL_READ_CHUNK_SIZE 256

typedef struct {
    char (*buffer)[SERIAL_MAX_COLUMNS][SERIAL_MAX_TOKEN_LEN];
    double *xAxisData;
    double *yAxisData;
    int head, count, capacity;
    size_t cols;
    const char *separator;
} Serial;

void configureTermios(int* fileDescriptor);
void setupSerial(Serial *serial);
void cleanSerial(Serial *serial);
void readSerialLineRaw(Serial *serial, int fileDescriptor);
