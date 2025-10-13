#include "core.h"

// Serial functions
void configureTermios(int* fileDescriptor)
{
    struct termios tty;
    if (tcgetattr(*fileDescriptor, &tty) != 0) fprintf(stderr, "tcgetattr failed on fd=%d: %s\n", *fileDescriptor, strerror(errno));

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                     // disable break processing
    tty.c_lflag = 0;                            // no signaling chars, no echo
    tty.c_oflag = 0;                            // no remapping, no delays
    tty.c_cc[VMIN]  = 1;                        // read returns each byte
    tty.c_cc[VTIME] = 0;                        // no timeout
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);     // shut off xon/xoff ctrl
    tty.c_cflag |= (CLOCAL | CREAD);            // ignore modem, enable read
    tty.c_cflag &= ~(PARENB | PARODD);          // no parity
    tty.c_cflag &= ~CSTOPB;                     // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                    // no flow control

    if (tcsetattr(*fileDescriptor, TCSANOW, &tty) != 0) fprintf(stderr, "tcsetattr failed on fd=%d: %s\n", *fileDescriptor, strerror(errno));

    return;
}


void setupSerial(Serial *serial)
{
    memset(serial, 0, sizeof(*serial));

    serial->separator = ",";
    serial->capacity  = SERIAL_BUF_SIZE;

    // Allocate rows dynamically
    serial->buffer = (char (*)[MAX_COLUMNS][MAX_LENGTH]) malloc(SERIAL_BUF_SIZE * sizeof(*serial->buffer));
    serial->doubleListX = (double *)malloc(serial->capacity * sizeof(double));
    serial->doubleListY = (double *)malloc(serial->capacity * sizeof(double));
    serial->doubleListY2 = (double *)malloc(serial->capacity * sizeof(double));

    if (!serial->buffer)
    {
        fprintf(stderr, "ERROR: malloc failed for serial->buffer\n");
        exit(1);
    }

    serial->head = 0;
    serial->count = 0;
    serial->cols = 0;
}


void cleanSerial(Serial *serial)
{
    free(serial->buffer);
    free(serial->doubleListX);
    free(serial->doubleListY);
    free(serial->doubleListY2);
}


void readSerialLineRaw(int run, Serial *serial, int serialFileDescriptor)
{
    if (!run) return;

    static char lineBuffer[READ_BUF_SIZE];
    static size_t linePosition = 0;

    char buffer[64];
    ssize_t bytesRead = read(serialFileDescriptor, buffer, sizeof(buffer));

    if (bytesRead < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK) fprintf(stderr, "read failed on fd=%d: %s\n", serialFileDescriptor, strerror(errno));
        return;
    }

    for (ssize_t i = 0; i < bytesRead; i++)
    {
        char c = buffer[i];

        if (c == '\n') // End of line
        {
            lineBuffer[linePosition] = '\0'; // Add null termination 

            // Parse line into tokens
            int row = serial->head;
            int col = 0;
            char *token = strtok(lineBuffer, serial->separator);

            while (token != NULL && col < MAX_COLUMNS)
            {
                strncpy(serial->buffer[row][col], token, MAX_LENGTH - 1);
                serial->buffer[row][col][MAX_LENGTH - 1] = '\0';
                token = strtok(NULL, serial->separator);
                col++;

                if (col > serial->cols)
                    serial->cols = col;
            }

            // Advance head and count
            serial->head = (serial->head + 1) % serial->capacity;
            if (serial->count < serial->capacity)
                serial->count++;

            linePosition = 0; // Reset line position
            continue;
        }
        
        if (linePosition >= READ_BUF_SIZE - 1)
        {
            fprintf(stderr, "Line buffer overflow\n"); // Discard the the whole line
            linePosition = 0;
            continue;
        }

        lineBuffer[linePosition++] = c;
    }
}
