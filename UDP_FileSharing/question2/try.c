#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    FILE *file;
    char buffer[255];
    char search[255]; 
    strcpy(search, "vibhav_104"); 
    // Open the file in read mode
    file = fopen("users.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Read each line from the file
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Print the line to the console
        buffer[strcspn(buffer, "\n")] = '\0';
        if(strcmp(buffer, search)==0) printf("matched %s", buffer); 
    }

    // Close the file
    fclose(file);

    return 0;
}
