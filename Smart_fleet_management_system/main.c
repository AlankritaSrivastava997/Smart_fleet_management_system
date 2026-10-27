#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_DRIVERS 4
#define MAX_PLATE 50
#define MAX_MODEL 100
#define MAX_NAME 100
#define INPUT_SIZE 256

typedef struct Vehicle {
    int id;
    int fuel;
    int status;
    int driverId;
    char plate[MAX_PLATE];
    char model[MAX_MODEL];
    struct Vehicle *next;
} Vehicle;

typedef struct {
    int id;
    char name[MAX_NAME];
    int vehicleId;
} Driver;

typedef struct {
    Vehicle *head;
    int nextId;
    Driver drivers[MAX_DRIVERS];
} Fleet;

/* Input handling */

int readInt(const char *prompt, int *value) {
    char input[INPUT_SIZE];
    char *end;
    long number;

    printf("%s", prompt);

    if (fgets(input, sizeof(input), stdin) == NULL)
        return 0;

    if (strchr(input, '\n') == NULL && !feof(stdin)) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
        return -1;
    }

    errno = 0;
    number = strtol(input, &end, 10);

    if (end == input || errno == ERANGE ||
        number < -2147483647L - 1 ||
        number > 2147483647L)
        return -1;

    while (isspace((unsigned char)*end))
        end++;

    if (*end != '\0')
        return -1;

    *value = (int)number;
    return 1;
}

int readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL)
        return 0;

    if (strchr(buffer, '\n') == NULL && !feof(stdin)) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
        printf("Input is too long. Please try again.\n");
        return -1;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    return 1;
}

void trim(char *str) {
    char *start = str;
    char *end;

    while (isspace((unsigned char)*start))
        start++;

    if (start != str)
        memmove(str, start, strlen(start) + 1);

    end = str + strlen(str);

    while (end > str && isspace((unsigned char)end[-1]))
        end--;

    *end = '\0';
}

/* Fleet initialization */

void initFleet(Fleet *fleet) {
    fleet->head = NULL;
    fleet->nextId = 1;

    fleet->drivers[0] = (Driver){1, "Rahul Sharma", 0};
    fleet->drivers[1] = (Driver){2, "Amit Verma", 0};
    fleet->drivers[2] = (Driver){3, "Priya Nair", 0};
    fleet->drivers[3] = (Driver){4, "Neha Gupta", 0};
}

Vehicle *findVehicle(Fleet *fleet, int id) {
    Vehicle *p = fleet->head;

    while (p != NULL) {
        if (p->id == id)
            return p;
        p = p->next;
    }

    return NULL;
}

int plateExists(Fleet *fleet, const char *plate) {
    Vehicle *p = fleet->head;

    while (p != NULL) {
        if (strcmp(p->plate, plate) == 0)
            return 1;
        p = p->next;
    }

    return 0;
}

const char *statusName(int status) {
    switch (status) {
        case 1: return "On Trip";
        case 2: return "Maintenance";
        default: return "Available";
    }
}

/* Add vehicle */

void addVehicle(Fleet *fleet, const char *plate,
                const char *model, int fuel) {
    Vehicle *v;
    Vehicle *p;

    if (plate[0] == '\0' || model[0] == '\0') {
        printf("Plate and model cannot be empty.\n");
        return;
    }

    if (fuel < 0 || fuel > 100) {
        printf("Fuel must be between 0 and 100.\n");
        return;
    }

    if (plateExists(fleet, plate)) {
        printf("This plate number already exists.\n");
        return;
    }

    v = malloc(sizeof(Vehicle));

    if (v == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    v->id = fleet->nextId++;
    v->fuel = fuel;
    v->status = 0;
    v->driverId = 0;

    snprintf(v->plate, sizeof(v->plate), "%s", plate);
    snprintf(v->model, sizeof(v->model), "%s", model);

    v->next = NULL;

    if (fleet->head == NULL) {
        fleet->head = v;
    } else {
        p = fleet->head;

        while (p->next != NULL)
            p = p->next;

        p->next = v;
    }

    printf("Vehicle added successfully. ID: %d\n", v->id);
}

/* Display vehicles */

void displayVehicles(Fleet *fleet) {
    Vehicle *p = fleet->head;
    int found = 0;

    if (p == NULL) {
        printf("No vehicles available.\n");
        return;
    }

    printf("\n========== ALL VEHICLES ==========\n");

    while (p != NULL) {
        found = 1;

        printf("\nID: %d\n", p->id);
        printf("Plate: %s\n", p->plate);
        printf("Model: %s\n", p->model);
        printf("Fuel: %d%%\n", p->fuel);
        printf("Status: %s\n", statusName(p->status));

        if (p->driverId == 0)
            printf("Driver: Unassigned\n");
        else
            printf("Driver: %s\n",
                   fleet->drivers[p->driverId - 1].name);

        printf("---------------------------------\n");
        p = p->next;
    }

    if (!found)
        printf("No vehicles available.\n");
}

/* Search vehicle */

void searchVehicle(Fleet *fleet) {
    int id, result;
    Vehicle *v;

    result = readInt("Enter vehicle ID: ", &id);

    if (result <= 0) {
        if (result == 0)
            printf("\nInput ended.\n");
        else
            printf("Invalid ID.\n");
        return;
    }

    v = findVehicle(fleet, id);

    if (v == NULL) {
        printf("Vehicle not found.\n");
        return;
    }

    printf("\nVehicle found!\n");
    printf("ID: %d\n", v->id);
    printf("Plate: %s\n", v->plate);
    printf("Model: %s\n", v->model);
    printf("Fuel: %d%%\n", v->fuel);
    printf("Status: %s\n", statusName(v->status));

    if (v->driverId == 0)
        printf("Driver: Unassigned\n");
    else
        printf("Driver: %s\n",
               fleet->drivers[v->driverId - 1].name);
}

/* Update vehicle and assign driver */

void updateVehicle(Fleet *fleet) {
    int id, fuel, status, driverId;
    int result, i;
    Vehicle *v;

    result = readInt("Enter vehicle ID: ", &id);

    if (result <= 0) {
        printf(result == 0 ? "\nInput ended.\n" : "Invalid ID.\n");
        return;
    }

    v = findVehicle(fleet, id);

    if (v == NULL) {
        printf("Vehicle not found.\n");
        return;
    }

    result = readInt("Enter fuel percentage (0-100): ", &fuel);

    if (result <= 0 || fuel < 0 || fuel > 100) {
        printf("Invalid fuel percentage.\n");
        return;
    }

    result = readInt(
        "Status (0=Available, 1=On Trip, 2=Maintenance): ",
        &status
    );

    if (result <= 0 || status < 0 || status > 2) {
        printf("Invalid status.\n");
        return;
    }

    printf("\nAvailable drivers:\n");

    for (i = 0; i < MAX_DRIVERS; i++) {
        printf("%d. %s", fleet->drivers[i].id,
               fleet->drivers[i].name);

        if (fleet->drivers[i].vehicleId != 0 &&
            fleet->drivers[i].vehicleId != id)
            printf(" (Assigned)");

        printf("\n");
    }

    result = readInt("Driver ID (0=Unassigned): ", &driverId);

    if (result <= 0 || driverId < 0 ||
        driverId > MAX_DRIVERS) {
        printf("Invalid driver ID.\n");
        return;
    }

    if (status == 1 && driverId == 0) {
        printf("Assign a driver before starting a trip.\n");
        return;
    }

    if (status == 1 && v->status == 2) {
        printf("Vehicle is under maintenance.\n");
        return;
    }

    if (status == 1 && fuel == 0) {
        printf("Vehicle needs fuel before a trip.\n");
        return;
    }

    if (driverId != 0 &&
        fleet->drivers[driverId - 1].vehicleId != 0 &&
        fleet->drivers[driverId - 1].vehicleId != id) {
        printf("Driver already has another vehicle.\n");
        return;
    }

    /* Remove the old driver assignment */

    for (i = 0; i < MAX_DRIVERS; i++) {
        if (fleet->drivers[i].vehicleId == id)
            fleet->drivers[i].vehicleId = 0;
    }

    /* Set the new driver assignment */

    if (driverId != 0)
        fleet->drivers[driverId - 1].vehicleId = id;

    v->fuel = fuel;
    v->status = status;
    v->driverId = driverId;

    printf("Vehicle updated successfully.\n");
}

/* Delete vehicle */

void deleteVehicle(Fleet *fleet) {
    int id, result, i;
    Vehicle *current = fleet->head;
    Vehicle *previous = NULL;

    result = readInt("Enter vehicle ID to delete: ", &id);

    if (result <= 0) {
        printf(result == 0 ? "\nInput ended.\n" : "Invalid ID.\n");
        return;
    }

    while (current != NULL && current->id != id) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Vehicle not found.\n");
        return;
    }

    for (i = 0; i < MAX_DRIVERS; i++) {
        if (fleet->drivers[i].vehicleId == id)
            fleet->drivers[i].vehicleId = 0;
    }

    if (previous == NULL)
        fleet->head = current->next;
    else
        previous->next = current->next;

    free(current);

    printf("Vehicle deleted successfully.\n");
}

/* Sort vehicles by fuel (ascending) */

void sortByFuel(Fleet *fleet) {
    Vehicle *p;
    Vehicle **array;
    int count = 0, i, j;
    Vehicle *temp;

    for (p = fleet->head; p != NULL; p = p->next)
        count++;

    if (count == 0) {
        printf("No vehicles available.\n");
        return;
    }

    array = malloc((size_t)count * sizeof(Vehicle *));

    if (array == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    i = 0;

    for (p = fleet->head; p != NULL; p = p->next)
        array[i++] = p;

    for (i = 0; i < count - 1; i++) {
        for (j = 0; j < count - i - 1; j++) {
            if (array[j]->fuel > array[j + 1]->fuel) {
                temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }

    printf("\n===== VEHICLES SORTED BY FUEL =====\n");

    for (i = 0; i < count; i++) {
        printf("%s | %s | %d%% fuel\n",
               array[i]->plate,
               array[i]->model,
               array[i]->fuel);
    }

    free(array);
}

/* Fleet dashboard */

void dashboard(Fleet *fleet) {
    Vehicle *p;
    int total = 0, available = 0;
    int trips = 0, maintenance = 0, lowFuel = 0;

    for (p = fleet->head; p != NULL; p = p->next) {
        total++;

        if (p->status == 0)
            available++;

        if (p->status == 1)
            trips++;

        if (p->status == 2)
            maintenance++;

        if (p->fuel < 20)
            lowFuel++;
    }

    printf("\n======= FLEET DASHBOARD =======\n");
    printf("Total Vehicles: %d\n", total);
    printf("Available: %d\n", available);
    printf("On Trip: %d\n", trips);
    printf("Under Maintenance: %d\n", maintenance);
    printf("Low Fuel Alerts: %d\n", lowFuel);

    if (lowFuel > 0) {
        printf("\nVehicles needing fuel:\n");

        for (p = fleet->head; p != NULL; p = p->next) {
            if (p->fuel < 20)
                printf("%s - %d%%\n", p->plate, p->fuel);
        }
    }

    printf("===============================\n");
}

/* Free all allocated memory */

void freeFleet(Fleet *fleet) {
    Vehicle *temp;

    while (fleet->head != NULL) {
        temp = fleet->head;
        fleet->head = fleet->head->next;
        free(temp);
    }
}

/* Main menu */

void menu(Fleet *fleet) {
    int choice, result;
    char plate[MAX_PLATE];
    char model[MAX_MODEL];
    int fuel;

    do {
        printf("\n====================================\n");
        printf(" SMART FLEET MANAGEMENT SYSTEM\n");
        printf("====================================\n");
        printf("1. Add Vehicle\n");
        printf("2. Display All Vehicles\n");
        printf("3. Search Vehicle\n");
        printf("4. Update Vehicle and Assign Driver\n");
        printf("5. Delete Vehicle\n");
        printf("6. Sort Vehicles by Fuel\n");
        printf("7. Fleet Dashboard\n");
        printf("0. Exit\n");

        result = readInt("Enter your choice: ", &choice);

        if (result == 0) {
            printf("\nNo more input. Exiting.\n");
            break;
        }

        if (result < 0) {
            printf("Please enter a valid number.\n");
            continue;
        }

        switch (choice) {
            case 1:
                result = readLine("Enter plate number: ",
                                  plate, sizeof(plate));
                if (result <= 0)
                    break;

                trim(plate);

                result = readLine("Enter vehicle model: ",
                                  model, sizeof(model));
                if (result <= 0)
                    break;

                trim(model);

                result = readInt(
                    "Enter fuel percentage (0-100): ", &fuel
                );

                if (result <= 0) {
                    printf("Invalid fuel input.\n");
                    break;
                }

                addVehicle(fleet, plate, model, fuel);
                break;

            case 2:
                displayVehicles(fleet);
                break;

            case 3:
                searchVehicle(fleet);
                break;

            case 4:
                updateVehicle(fleet);
                break;

            case 5:
                deleteVehicle(fleet);
                break;

            case 6:
                sortByFuel(fleet);
                break;

            case 7:
                dashboard(fleet);
                break;

            case 0:
                printf("Thank you for using Smart Fleet Management.\n");
                break;

            default:
                printf("Invalid choice. Try again.\n");
        }

    } while (choice != 0);
}

int main(void) {
    Fleet fleetSystem;

    initFleet(&fleetSystem);

    printf("Smart Fleet Management and Vehicle Monitoring System\n");

    addVehicle(&fleetSystem, "UP16 AB 1021", "Tata Ace", 85);
    addVehicle(&fleetSystem, "DL01 CD 4432", "Mahindra Bolero", 62);
    addVehicle(&fleetSystem, "UP14 GH 3321", "Eicher Pro 2049", 15);
    addVehicle(&fleetSystem, "UP16 LM 5566", "Force Traveller", 30);

    menu(&fleetSystem);

    freeFleet(&fleetSystem);

    return 0;
}