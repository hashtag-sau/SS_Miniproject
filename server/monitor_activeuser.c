#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_KEY 1234
#define SHM_SIZE sizeof(SharedData)

// Shared structure in shared memory
typedef struct {
    long int active_clients;
} SharedData;

int main() {
    // Get the shared memory segment
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach to the shared memory
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (SharedData *)-1) {
        perror("shmat failed");
        exit(1);
    }
    long int peak = 0;
    while (1) {
        system("clear");
        if (peak < shared_data->active_clients) {
            peak = shared_data->active_clients;
        }
        printf("Active Clients: %ld, Peak Clients: %ld\n",
               shared_data->active_clients, peak);
        sleep(1);  // Update every second
    }

    // Detach from shared memory
    shmdt(shared_data);

    return 0;
}
