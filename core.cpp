#include "core.h"

// Serial functions
void configureTermios(int* fileDescriptor) {

    struct termios options;
    tcgetattr(*fileDescriptor, &options);               // Get the current options
    cfsetispeed(&options, B9600);                       // Set input baud rate to 9600
    cfsetospeed(&options, B9600);                       // Set output baud rate to 9600
    options.c_cflag &= ~PARENB;                         // Disable parity checking
    options.c_cflag &= ~CSTOPB;                         // Use 1 stop bit
    options.c_cflag &= ~CSIZE;                          // Mask the character size
    options.c_cflag |= CS8;                             // Select 8-bit data size
    options.c_cflag |= (CLOCAL | CREAD);                // Enable receiver and ignore modem control lines
    options.c_cflag &= ~CRTSCTS;                        // Disable hardware flow control
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Disable canonical, echoing, signal generation (raw input)
    options.c_iflag &= ~(IXON | IXOFF | IXANY);         // Disable software flow control
    options.c_oflag &= ~OPOST;                          // Select raw output
    options.c_cc[VMIN]  = 0;                            // Return immediately if data is available
    options.c_cc[VTIME] = 10;                           // Return 0 if no data arrives within 1 second
    tcsetattr(*fileDescriptor, TCSANOW, &options);      // Set the new options

    return;
}

void setupSerial(Serial *serial) {    
    memset(serial, 0, sizeof(*serial));
    serial->separator = ",";
    serial->capacity  = SERIAL_MAX_LINES;
    serial->buffer = (char (*)[SERIAL_MAX_COLUMNS][SERIAL_MAX_TOKEN_LEN]) malloc(SERIAL_MAX_LINES * sizeof(*serial->buffer));
    serial->xAxisData = (double *)malloc(serial->capacity * sizeof(double));
    serial->yAxisData = (double *)malloc(serial->capacity * sizeof(double));
    serial->head = 0;
    serial->count = 0;
    serial->cols = 0;
}

void cleanSerial(Serial *serial) {
    free(serial->buffer);
    free(serial->xAxisData);
    free(serial->yAxisData);
}

void readSerialLineRaw(Serial *serial, int fileDescriptor) {

    static char lineBuffer[SERIAL_READ_CHUNK_SIZE];
    static size_t linePosition = 0;
    char buffer[64];
    ssize_t bytesRead = read(fileDescriptor, buffer, sizeof(buffer));

    for (ssize_t i = 0; i < bytesRead; i++) {
        char c = buffer[i];

        if (c == '\n') {
            lineBuffer[linePosition] = '\0'; // Add null termination 

            int row = serial->head;
            int col = 0;
            char *token = strtok(lineBuffer, serial->separator);

            while (token != NULL && col < SERIAL_MAX_COLUMNS) {
                strncpy(serial->buffer[row][col], token, SERIAL_MAX_TOKEN_LEN - 1);
                serial->buffer[row][col][SERIAL_MAX_TOKEN_LEN - 1] = '\0';
                token = strtok(NULL, serial->separator);
                col++;

                if (col > serial->cols)
                    serial->cols = col;
            }

            serial->head = (serial->head + 1) % serial->capacity; // Advance head and count
            if (serial->count < serial->capacity)
                serial->count++;

            linePosition = 0; // Reset line position
            continue;
        }
        
        if (linePosition >= SERIAL_READ_CHUNK_SIZE - 1) { // Line buffer overflow
            linePosition = 0;
            continue;
        }

        lineBuffer[linePosition++] = c;
    }
}
