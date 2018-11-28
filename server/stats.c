//
// Created by Frixs on 7.10.2018.
//

#include <time.h>
#include <stdio.h>
#include "constants.h"
#include "stats.h"

time_t time_initial, time_current;
long bytes_received = 0;
long bytes_sent = 0;
long messages_received = 0;
long messages_sent = 0;
long messages_bad = 0;

/// Prints the info about the connection.
/// \param stream
void print_info(FILE *stream) {
    time(&time_current);

    long seconds    = (long) difftime(time_current, time_initial);
    long minutes     = seconds / 60;
    long hours       = minutes / 60;

    fprintf(stream, "==================== SERVER STATISTICS ====================\r\n");
    fprintf(stream, "Server started at: %s\r\n", asctime(localtime(&time_initial)));
    fprintf(stream, "Already running: %ld hours %ld minutes %ld seconds\r\n", hours, minutes, seconds%60);
    fprintf(stream, "Number of recieved messages: %ld\r\n", messages_received);
    fprintf(stream, "Number of recieved bytes: %ld\r\n", bytes_received);
    fprintf(stream, "Number of sent messages: %ld\r\n", messages_sent);
    fprintf(stream, "Number of sent bytes: %ld\r\n", bytes_sent);
    fprintf(stream, "Number of recieved messages with bad form: %ld\r\n", messages_bad);
    fprintf(stream, "===========================================================\r\n");
}

/// Write down message into log file and console.
/// \param log
void write_log(char *log) {
    FILE *logs = fopen("server.log", "a");
    fprintf(logs, "%s\t%s\r\n", asctime(localtime(&time_initial)), log);
    fprintf(stdout, "%s", log);
    fclose(logs);
}

/// Write down statistics into stats file.
void write_stats() {
    FILE *f = fopen("stats.log", "w");
    print_info(f);
    fclose(f);
}