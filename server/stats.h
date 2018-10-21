//
// Created by Frixs on 7.10.2018.
//

#ifndef SERVER_STATS_H
#define SERVER_STATS_H

extern long bytes_received;
extern long bytes_sent;
extern long messages_received;
extern long messages_sent;
extern long messages_bad;

void print_info(FILE *stream);
void write_log(char *log);
void write_stats();

#endif //SERVER_STATS_H
