#include "iotdb.h"
#include <MQTTClient.h>
#include <stdint.h>

#define MQTT_BROKER "tcp://localhost:1883"
#define MQTT_CLIENT_ID "IoTDB_Client"
#define MQTT_TOPIC_REQUEST "iotdb/request"
#define MQTT_TOPIC_RESPONSE "iotdb/response"
#define COMPRESSED_FILE "iotdb_compressed.bin"

IoTDB* db;
MQTTClient client;

// Compress and save records to file using run-length encoding
int save_compressed(IoTDB* db) {
    FILE* file = fopen(COMPRESSED_FILE, "wb");
    if (!file) return -1;

    for (int i = 0; i < db->record_count; i++) {
        Record* curr = &db->records[i];
        // Write fixed-length fields
        fwrite(&curr->id, sizeof(int), 1, file);
        
        // Write variable-length fields with their lengths
        uint8_t ts_len = strlen(curr->timestamp);
        uint8_t md_len = strlen(curr->metadata);
        
        fwrite(&ts_len, sizeof(uint8_t), 1, file);
        fwrite(curr->timestamp, 1, ts_len, file);
        fwrite(&curr->value, sizeof(double), 1, file);
        fwrite(&md_len, sizeof(uint8_t), 1, file);
        fwrite(curr->metadata, 1, md_len, file);
    }
    
    fclose(file);
    return 0;
}

// Load compressed records from file
int load_compressed(IoTDB* db) {
    FILE* file = fopen(COMPRESSED_FILE, "rb");
    if (!file) return -1;
    
    db->record_count = 0;
    
    while (!feof(file) && db->record_count < MAX_RECORDS) {
        Record* curr = &db->records[db->record_count];
        
        // Read fixed-length fields
        if (fread(&curr->id, sizeof(int), 1, file) != 1) break;
        
        // Read variable-length fields
        uint8_t ts_len, md_len;
        
        if (fread(&ts_len, sizeof(uint8_t), 1, file) != 1) break;
        if (fread(curr->timestamp, 1, ts_len, file) != ts_len) break;
        curr->timestamp[ts_len] = '\0';
        
        if (fread(&curr->value, sizeof(double), 1, file) != 1) break;
        
        if (fread(&md_len, sizeof(uint8_t), 1, file) != 1) break;
        if (fread(curr->metadata, 1, md_len, file) != md_len) break;
        curr->metadata[md_len] = '\0';
        
        db->record_count++;
    }
    
    fclose(file);
    return 0;
}

void message_arrived(MQTTClient_message* message) {
    char* payload = (char*)message->payload;
    char response[1024] = {0};

    // Extract command from payload
    char command[20] = {0};
    if (sscanf(payload, "%19s", command) != 1) {
        snprintf(response, sizeof(response), "Error: Invalid message format");
        goto send_response;
    }

    if (strcmp(command, "INSERT") == 0) {
        char timestamp[20], metadata[50];
        double value;
        if (sscanf(payload, "INSERT %19s %lf %49s", timestamp, &value, metadata) != 3) {
            snprintf(response, sizeof(response), "Error: Invalid INSERT format. Expected: INSERT <timestamp> <value> <metadata>");
        } else {
            insert_record(db, timestamp, value, metadata);
            save_compressed(db);  // Save compressed data after insert
            snprintf(response, sizeof(response), "INSERT OK: %s", timestamp);
        }

    } else if (strcmp(command, "QUERY") == 0) {
        char start_time[20], end_time[20];
        if (sscanf(payload, "QUERY %19s %19s", start_time, end_time) != 2) {
            snprintf(response, sizeof(response), "Error: Invalid QUERY format. Expected: QUERY <start_time> <end_time>");
        } else {
            Record results[10];
            int count = query_data(db, start_time, end_time, results, 10);
            if (count == 0) {
                snprintf(response, sizeof(response), "No records found in specified time range");
            } else {
                for (int i = 0; i < count; i++) {
                    char temp[128];
                    snprintf(temp, sizeof(temp), "ID:%d TS:%s Value:%.2f Metadata:%s\n",
                             results[i].id, results[i].timestamp, results[i].value, results[i].metadata);
                    strncat(response, temp, sizeof(response) - strlen(response) - 1);
                }
            }
        }

    } else {
        snprintf(response, sizeof(response), "Error: Unknown command '%s'. Supported commands: INSERT, QUERY", command);
    }

send_response:
    MQTTClient_message pub_msg = MQTTClient_message_initializer;
    pub_msg.payload = response;
    pub_msg.payloadlen = (int)strlen(response);
    pub_msg.qos = 1;
    MQTTClient_publishMessage(client, MQTT_TOPIC_RESPONSE, &pub_msg, NULL);
}

int main() {
    db = init_database();
    load_compressed(db);  // Load compressed data at startup

    MQTTClient_create(&client, MQTT_BROKER, MQTT_CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect to MQTT broker.\n");
        return -1;
    }

    MQTTClient_subscribe(client, MQTT_TOPIC_REQUEST, 1);
    printf("Listening for messages on '%s'...\n", MQTT_TOPIC_REQUEST);

    while (1) {
        MQTTClient_message* message;
        char* topic_name;
        int topic_len;
        MQTTClient_receive(client, &topic_name, &topic_len, &message, 1000);
        if (message) {
            message_arrived(message);
            MQTTClient_freeMessage(&message);
            MQTTClient_free(topic_name);
        }
    }

    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    save_compressed(db);  // Save compressed data before exit
    free(db);
    return 0;
}
