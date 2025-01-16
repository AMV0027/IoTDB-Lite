#include "iotdb.h"

// Initialize the database
IoTDB* init_database() {
    IoTDB* db = (IoTDB*)malloc(sizeof(IoTDB));
    db->record_count = 0;
    load_from_file(db);
    return db;
}

// Insert a new record
int insert_record(IoTDB* db, const char* timestamp, double value, const char* metadata) {
    if (db->record_count >= MAX_RECORDS) {
        save_to_file(db);
        db->record_count = 0;
    }
    Record* rec = &db->records[db->record_count++];
    rec->id = db->record_count;
    strncpy(rec->timestamp, timestamp, sizeof(rec->timestamp) - 1);
    rec->value = value;
    strncpy(rec->metadata, metadata, sizeof(rec->metadata) - 1);
    return 0;  // Success
}

// Query records by timestamp range
int query_data(IoTDB* db, const char* start_time, const char* end_time, Record* results, int max_results) {
    int count = 0;
    for (int i = 0; i < db->record_count; i++) {
        if (strcmp(db->records[i].timestamp, start_time) >= 0 &&
            strcmp(db->records[i].timestamp, end_time) <= 0) {
            if (count < max_results) {
                results[count++] = db->records[i];
            } else {
                break;
            }
        }
    }
    return count;  // Return number of matches
}

// Save records to file
int save_to_file(IoTDB* db) {
    FILE* file = fopen(RECORD_FILE, "wb");
    if (!file) return -1;
    fwrite(db->records, sizeof(Record), db->record_count, file);
    fclose(file);
    return 0;  // Success
}

// Load records from file
int load_from_file(IoTDB* db) {
    FILE* file = fopen(RECORD_FILE, "rb");
    if (!file) return -1;
    db->record_count = fread(db->records, sizeof(Record), MAX_RECORDS, file);
    fclose(file);
    return 0;  // Success
}
