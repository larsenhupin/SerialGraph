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
    serial->head = 0;
    serial->separator = ",";
    serial->capacity  = SERIAL_MAX_LINES;
    serial->buffer = (SerialBuffer *) malloc(SERIAL_MAX_LINES * sizeof(*serial->buffer));
    serial->xAxisData = (int *) malloc(serial->capacity * sizeof(int));
    serial->yAxisData = (int *) malloc(serial->capacity * sizeof(int));
}

void cleanSerial(Serial *serial) {
    free(serial->buffer);
    free(serial->xAxisData);
    free(serial->yAxisData);
}

void readSerialLineRaw(Serial *serial, int fileDescriptor) {

    static char line[SERIAL_READ_CHUNK_SIZE];
    static size_t position = 0;
    char readBuffer[64];
    ssize_t readBufferBytes = read(fileDescriptor, readBuffer, sizeof(readBuffer));

    for (ssize_t i = 0; i < readBufferBytes; i++) {
        
        char c = readBuffer[i];

        if (c != '\n') {
            line[position] = c;
            position++;
        }
        else {
            char *token = strtok(line, serial->separator);

            for (int col = 0; token != NULL && col < SERIAL_MAX_COLUMNS; col++) {
                
                strncpy(serial->buffer[serial->head][col], token, SERIAL_MAX_TOKEN_LEN);
                token = strtok(NULL, serial->separator);
            }

            serial->head = (serial->head + 1) % serial->capacity; // Advance head using circular buffer
            position = 0; // Reset line position
        }
    }
}
