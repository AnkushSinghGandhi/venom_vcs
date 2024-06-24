#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#define REPO_DIR ".venom"
#define INDEX_FILE ".venom/index"
#define COMMITS_DIR ".venom/commits"

void init() {
    struct stat st = {0};

    if (stat(REPO_DIR, &st) == -1) {
        mkdir(REPO_DIR, 0700);
        mkdir(COMMITS_DIR, 0700);

        FILE *index = fopen(INDEX_FILE, "w");
        if (index == NULL) {
            perror("Unable to create index file");
            exit(EXIT_FAILURE);
        }
        fclose(index);

        printf("Initialized empty Venom repository in %s\n", REPO_DIR);
    } else {
        printf("Repository already exists.\n");
    }
}

void add(const char *filename) {
    FILE *index = fopen(INDEX_FILE, "a");
    if (index == NULL) {
        perror("Unable to open index file");
        exit(EXIT_FAILURE);
    }

    fprintf(index, "%s\n", filename);
    fclose(index);

    printf("Added '%s' to the index.\n", filename);
}

void commit(const char *message) {
    static int commit_count = 0;
    char commit_dir[256];

    sprintf(commit_dir, "%s/%d", COMMITS_DIR, commit_count);
    mkdir(commit_dir, 0700);

    FILE *index = fopen(INDEX_FILE, "r");
    if (index == NULL) {
        perror("Unable to open index file");
        exit(EXIT_FAILURE);
    }

    char filename[256];
    while (fgets(filename, sizeof(filename), index)) {
        filename[strcspn(filename, "\n")] = 0;  // Remove newline

        char cmd[512];
        sprintf(cmd, "cp %s %s/", filename, commit_dir);
        system(cmd);
    }
    fclose(index);

    char commit_message_file[256];
    sprintf(commit_message_file, "%s/message.txt", commit_dir);

    FILE *message_file = fopen(commit_message_file, "w");
    if (message_file == NULL) {
        perror("Unable to create commit message file");
        exit(EXIT_FAILURE);
    }

    fprintf(message_file, "%s\n", message);
    fclose(message_file);

    commit_count++;
    printf("Committed changes as commit %d\n", commit_count - 1);

    // Clear the index
    index = fopen(INDEX_FILE, "w");
    fclose(index);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [<args>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "init") == 0) {
        init();
    } else if (strcmp(argv[1], "add") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s add <file>\n", argv[0]);
            return EXIT_FAILURE;
        }
        add(argv[2]);
    } else if (strcmp(argv[1], "commit") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s commit <message>\n", argv[0]);
            return EXIT_FAILURE;
        }
        commit(argv[2]);
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
