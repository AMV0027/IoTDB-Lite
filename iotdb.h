#ifndef IOTDB_H
#define IOTDB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_RECORDS 1000
#define RECORD_FILE "iotdb.bin"

typedef struct {
    int id;                 // Record ID
    char timestamp[20];     // Timestamp
    double value;           // Sensor value
    char metadata[50];      // Additional info
} Record;

typedef struct {
    Record records[MAX_RECORDS];
    int record_count;
} IoTDB;

IoTDB* init_database();
int insert_record(IoTDB* db, const char* timestamp, double value, const char* metadata);
int query_data(IoTDB* db, const char* start_time, const char* end_time, Record* results, int max_results);
int save_to_file(IoTDB* db);
int load_from_file(IoTDB* db);

#endif
