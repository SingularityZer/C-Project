#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STUDENTS 1000
#define MAX_NAME_LEN 100
#define MAX_PROGRAMME_LEN 100
#define FILENAME_LEN 100
#define MAX_LINE_LEN 256

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char programme[MAX_PROGRAMME_LEN];
    float mark;
} Student;

typedef struct {
    Student students[MAX_STUDENTS];
    int count;
    char filename[FILENAME_LEN];
    int is_modified;
} Database;

// Function prototypes
void init_database(Database *db, const char *filename);
int open_database(Database *db);
void show_all(const Database *db);
void show_all_sorted(Database *db, const char *sort_by, const char *order);
int insert_student(Database *db);
int query_student(const Database *db, int id);
int update_student(Database *db, int id);
int delete_student(Database *db, int id);
int save_database(Database *db);
void show_summary(const Database *db);
void to_lower_case(char *str);
void trim_whitespace(char *str);
int find_student(const Database *db, int id);
void search_by_name_pattern(const Database *db, const char *pattern);

int main() {
    Database db;
    char command[100];
    
    // File is one level outside - use "../Sample-CMS.txt"
    char filename[] = "../Sample-CMS.txt";
    
    init_database(&db, filename);
    
    printf("\nCMS: Class Management System initialized.\n");
    printf("Using database file: %s\n", filename);
    printf("Type 'HELP' for available commands.\n\n");
    
    // Auto-open the database file on startup
    if (open_database(&db)) {
        printf("CMS: Successfully loaded %d student records.\n", db.count);
    }
    
    while (1) {
        printf("CMS: ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        // Remove newline character
        command[strcspn(command, "\n")] = 0;
        trim_whitespace(command);
        to_lower_case(command);
        
        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
            if (db.is_modified) {
                printf("CMS: You have unsaved changes. Type 'SAVE' to save or 'EXIT' again to quit without saving.\n");
                db.is_modified = 1;
                continue;
            }
            printf("CMS: Goodbye!\n");
            break;
        } else if (strcmp(command, "open") == 0) {
            open_database(&db);
        } else if (strcmp(command, "show all") == 0) {
            show_all(&db);
        } else if (strncmp(command, "show all sort by", 16) == 0) {
            char sort_by[20], order[20];
            if (sscanf(command, "show all sort by %s %s", sort_by, order) == 2) {
                show_all_sorted(&db, sort_by, order);
            } else if (sscanf(command, "show all sort by %s", sort_by) == 1) {
                show_all_sorted(&db, sort_by, "asc");
            } else {
                printf("CMS: Invalid sort command. Usage: SHOW ALL SORT BY [ID|MARK] [ASC|DESC]\n");
            }
        } else if (strcmp(command, "show summary") == 0) {
            show_summary(&db);
        } else if (strcmp(command, "insert") == 0) {
            insert_student(&db);
        } else if (strncmp(command, "query", 5) == 0) {
            int id;
            if (sscanf(command, "query id=%d", &id) == 1) {
                query_student(&db, id);
            } else {
                printf("CMS: Invalid query format. Usage: QUERY ID=student_id\n");
            }
        } else if (strncmp(command, "update", 6) == 0) {
            int id;
            if (sscanf(command, "update id=%d", &id) == 1) {
                update_student(&db, id);
            } else {
                printf("CMS: Invalid update format. Usage: UPDATE ID=student_id\n");
            }
        } else if (strncmp(command, "delete", 6) == 0) {
            int id;
            if (sscanf(command, "delete id=%d", &id) == 1) {
                delete_student(&db, id);
            } else {
                printf("CMS: Invalid delete format. Usage: DELETE ID=student_id\n");
            }
        } else if (strcmp(command, "save") == 0) {
            save_database(&db);
        } else if (strncmp(command, "search name", 11) == 0) {
            char pattern[50];
            if (sscanf(command, "search name=%49s", pattern) == 1) {
                search_by_name_pattern(&db, pattern);
            } else {
                printf("CMS: Invalid search format. Usage: SEARCH NAME=pattern\n");
            }
        } else if (strcmp(command, "help") == 0) {
            printf("\nAvailable Commands:\n");
            printf("OPEN                    - Open database file\n");
            printf("SHOW ALL                - Display all records\n");
            printf("SHOW ALL SORT BY ID [ASC|DESC] - Show sorted by ID\n");
            printf("SHOW ALL SORT BY MARK [ASC|DESC] - Show sorted by mark\n");
            printf("SHOW SUMMARY            - Show statistics\n");
            printf("INSERT                  - Add new student\n");
            printf("QUERY ID=number         - Search student by ID\n");
            printf("UPDATE ID=number        - Update student record\n");
            printf("DELETE ID=number        - Delete student record\n");
            printf("SEARCH NAME=pattern     - Search by name pattern\n");
            printf("SAVE                    - Save to file\n");
            printf("EXIT/QUIT              - Exit program\n\n");
        } else if (strlen(command) > 0) {
            printf("CMS: Unknown command '%s'. Type 'HELP' for available commands.\n", command);
        }
    }
    
    return 0;
}

void init_database(Database *db, const char *filename) {
    db->count = 0;
    db->is_modified = 0;
    strncpy(db->filename, filename, FILENAME_LEN - 1);
    db->filename[FILENAME_LEN - 1] = '\0';
}

/*int open_database(Database *db) {
    FILE *file = fopen(db->filename, "r");
    if (!file) {
        printf("CMS: The database file \"%s\" does not exist. A new database will be created.\n", db->filename);
        db->count = 0;
        return 0;
    }
    
    char line[MAX_LINE_LEN];
    db->count = 0;
    
    // Skip header lines (first 4 lines)
    for (int i = 0; i < 4; i++) {
        if (fgets(line, sizeof(line), file) == NULL) {
            fclose(file);
            return 0;
        }
    }
    
    // Read student records
    while (fgets(line, sizeof(line), file) != NULL && db->count < MAX_STUDENTS) {
        Student *s = &db->students[db->count];
        
        // Parse the line according to the Sample-CMS.txt format
        // Format: ID\tName\tProgramme\tMark
        char *token;
        char *rest = line;
        
        // Get ID
        token = strtok_r(rest, "\t", &rest);
        if (token == NULL) continue;
        s->id = atoi(token);
        
        // Get Name
        token = strtok_r(rest, "\t", &rest);
        if (token == NULL) continue;
        strncpy(s->name, token, MAX_NAME_LEN - 1);
        s->name[MAX_NAME_LEN - 1] = '\0';
        trim_whitespace(s->name);
        
        // Get Programme
        token = strtok_r(rest, "\t", &rest);
        if (token == NULL) continue;
        strncpy(s->programme, token, MAX_PROGRAMME_LEN - 1);
        s->programme[MAX_PROGRAMME_LEN - 1] = '\0';
        trim_whitespace(s->programme);
        
        // Get Mark
        token = strtok_r(rest, "\t\n", &rest);
        if (token == NULL) continue;
        s->mark = atof(token);
        
        db->count++;
    }
    
    fclose(file);
    db->is_modified = 0;
    printf("CMS: The database file \"%s\" is successfully opened.\n", db->filename);
    return 1;
}*/

int open_database(Database *db) {
    FILE *file = fopen(db->filename, "r");
    if (!file) {
        printf("CMS: The database file \"%s\" does not exist. A new database will be created.\n", db->filename);
        db->count = 0;
        return 0;
    }

    char line[MAX_LINE_LEN];
    db->count = 0;

    // Read all lines and only accept properly formatted data lines.
    // Expected data format: ID<TAB>Name<TAB>Programme<TAB>Mark
    while (fgets(line, sizeof(line), file) != NULL && db->count < MAX_STUDENTS) {
        trim_whitespace(line);
        if (line[0] == '\0') continue; // skip empty lines

        int id;
        char name[MAX_NAME_LEN];
        char programme[MAX_PROGRAMME_LEN];
        float mark;

        // Try to parse a data line with tab-separated fields.
        // Limit lengths to avoid buffer overflow.
        if (sscanf(line, "%d\t%99[^\t]\t%99[^\t]\t%f", &id, name, programme, &mark) == 4) {
            Student *s = &db->students[db->count++];
            s->id = id;
            strncpy(s->name, name, MAX_NAME_LEN - 1);
            s->name[MAX_NAME_LEN - 1] = '\0';
            trim_whitespace(s->name);

            strncpy(s->programme, programme, MAX_PROGRAMME_LEN - 1);
            s->programme[MAX_PROGRAMME_LEN - 1] = '\0';
            trim_whitespace(s->programme);

            s->mark = mark;
        } else {
            // Not a valid data line (probably header or malformed) -> skip.
            continue;
        }
    }

    fclose(file);
    db->is_modified = 0;
    printf("CMS: The database file \"%s\" is successfully opened.\n", db->filename);
    return 1;
}

// [Rest of the functions remain exactly the same as in the previous code]
// show_all(), show_all_sorted(), insert_student(), find_student(), query_student(),
// update_student(), delete_student(), save_database(), show_summary(), 
// search_by_name_pattern(), to_lower_case(), trim_whitespace()
// [Include all the same comparison functions and other functions from previous code]

void show_all(const Database *db) {
    if (db->count == 0) {
        printf("CMS: No records found in the table \"StudentRecords\".\n");
        return;
    }
    
    printf("CMS: Here are all the records found in the table \"StudentRecords\".\n");
    printf("%-10s %-20s %-25s %s\n", "ID", "Name", "Programme", "Mark");
    printf("%-10s %-20s %-25s %s\n", "----------", "--------------------", 
           "-------------------------", "----------");
    
    for (int i = 0; i < db->count; i++) {
        const Student *s = &db->students[i];
        printf("%-10d %-20s %-25s %.1f\n", s->id, s->name, s->programme, s->mark);
    }
}

// Comparison functions for sorting
int compare_id_asc(const void *a, const void *b) {
    return ((Student*)a)->id - ((Student*)b)->id;
}

int compare_id_desc(const void *a, const void *b) {
    return ((Student*)b)->id - ((Student*)a)->id;
}

int compare_mark_asc(const void *a, const void *b) {
    float diff = ((Student*)a)->mark - ((Student*)b)->mark;
    return (diff > 0) - (diff < 0);
}

int compare_mark_desc(const void *a, const void *b) {
    float diff = ((Student*)b)->mark - ((Student*)a)->mark;
    return (diff > 0) - (diff < 0);
}

void show_all_sorted(Database *db, const char *sort_by, const char *order) {
    if (db->count == 0) {
        printf("CMS: No records found in the table \"StudentRecords\".\n");
        return;
    }
    
    // Create a temporary array for sorting
    Student temp[MAX_STUDENTS];
    memcpy(temp, db->students, sizeof(Student) * db->count);
    
    // Choose comparison function
    int (*compare)(const void*, const void*) = NULL;
    
    if (strcmp(sort_by, "id") == 0) {
        compare = (strcmp(order, "desc") == 0) ? compare_id_desc : compare_id_asc;
    } else if (strcmp(sort_by, "mark") == 0) {
        compare = (strcmp(order, "desc") == 0) ? compare_mark_desc : compare_mark_asc;
    } else {
        printf("CMS: Invalid sort field. Use 'ID' or 'MARK'.\n");
        return;
    }
    
    qsort(temp, db->count, sizeof(Student), compare);
    
    printf("CMS: Here are all the records sorted by %s (%s).\n", sort_by, order);
    printf("%-10s %-20s %-25s %s\n", "ID", "Name", "Programme", "Mark");
    printf("%-10s %-20s %-25s %s\n", "----------", "--------------------", 
           "-------------------------", "----------");
    
    for (int i = 0; i < db->count; i++) {
        const Student *s = &temp[i];
        printf("%-10d %-20s %-25s %.1f\n", s->id, s->name, s->programme, s->mark);
    }
}

int insert_student(Database *db) {
    if (db->count >= MAX_STUDENTS) {
        printf("CMS: Database is full. Cannot add more students.\n");
        return 0;
    }
    
    Student new_student;
    printf("CMS: Please enter student ID: ");
    if (scanf("%d", &new_student.id) != 1) {
        printf("CMS: Invalid ID format.\n");
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');
    
    // Check if ID already exists
    if (find_student(db, new_student.id) != -1) {
        printf("CMS: The record with ID=%d already exists.\n", new_student.id);
        return 0;
    }
    
    printf("CMS: Please enter student name: ");
    fgets(new_student.name, MAX_NAME_LEN, stdin);
    trim_whitespace(new_student.name);
    
    printf("CMS: Please enter programme: ");
    fgets(new_student.programme, MAX_PROGRAMME_LEN, stdin);
    trim_whitespace(new_student.programme);
    
    printf("CMS: Please enter mark: ");
    if (scanf("%f", &new_student.mark) != 1) {
        printf("CMS: Invalid mark format.\n");
        while (getchar() != '\n');
        return 0;
    }
    while (getchar() != '\n');
    
    db->students[db->count++] = new_student;
    db->is_modified = 1;
    printf("CMS: A new record with ID=%d is successfully inserted.\n", new_student.id);
    return 1;
}

int find_student(const Database *db, int id) {
    for (int i = 0; i < db->count; i++) {
        if (db->students[i].id == id) {
            return i;
        }
    }
    return -1;
}

int query_student(const Database *db, int id) {
    int index = find_student(db, id);
    if (index == -1) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return 0;
    }
    
    const Student *s = &db->students[index];
    printf("CMS: The record with ID=%d is found in the data table.\n", id);
    printf("%-10s %-20s %-25s %s\n", "ID", "Name", "Programme", "Mark");
    printf("%-10s %-20s %-25s %s\n", "----------", "--------------------", 
           "-------------------------", "----------");
    printf("%-10d %-20s %-25s %.1f\n", s->id, s->name, s->programme, s->mark);
    return 1;
}

int update_student(Database *db, int id) {
    int index = find_student(db, id);
    if (index == -1) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return 0;
    }
    
    Student *s = &db->students[index];
    printf("CMS: Updating record for ID=%d. Press Enter to keep current value.\n", id);
    
    printf("CMS: Current name: %s. New name: ", s->name);
    char new_name[MAX_NAME_LEN];
    fgets(new_name, MAX_NAME_LEN, stdin);
    trim_whitespace(new_name);
    if (strlen(new_name) > 0) {
        strcpy(s->name, new_name);
    }
    
    printf("CMS: Current programme: %s. New programme: ", s->programme);
    char new_programme[MAX_PROGRAMME_LEN];
    fgets(new_programme, MAX_PROGRAMME_LEN, stdin);
    trim_whitespace(new_programme);
    if (strlen(new_programme) > 0) {
        strcpy(s->programme, new_programme);
    }
    
    printf("CMS: Current mark: %.1f. New mark: ", s->mark);
    char mark_input[20];
    fgets(mark_input, sizeof(mark_input), stdin);
    trim_whitespace(mark_input);
    if (strlen(mark_input) > 0) {
        float new_mark;
        if (sscanf(mark_input, "%f", &new_mark) == 1) {
            s->mark = new_mark;
        } else {
            printf("CMS: Invalid mark format. Keeping current value.\n");
        }
    }
    
    db->is_modified = 1;
    printf("CMS: The record with ID=%d is successfully updated.\n", id);
    return 1;
}

int delete_student(Database *db, int id) {
    int index = find_student(db, id);
    if (index == -1) {
        printf("CMS: The record with ID=%d does not exist.\n", id);
        return 0;
    }
    
    printf("CMS: Are you sure you want to delete record with ID=%d? Type \"Y\" to Confirm or type \"N\" to cancel: ", id);
    char confirmation[10];
    fgets(confirmation, sizeof(confirmation), stdin);
    trim_whitespace(confirmation);
    to_lower_case(confirmation);
    
    if (strcmp(confirmation, "y") == 0) {
        // Shift all elements after the deleted one
        for (int i = index; i < db->count - 1; i++) {
            db->students[i] = db->students[i + 1];
        }
        db->count--;
        db->is_modified = 1;
        printf("CMS: The record with ID=%d is successfully deleted.\n", id);
        return 1;
    } else {
        printf("CMS: The deletion is cancelled.\n");
        return 0;
    }
}

int save_database(Database *db) {
    FILE *file = fopen(db->filename, "w");
    if (!file) {
        printf("CMS: Error: Cannot open file \"%s\" for writing.\n", db->filename);
        return 0;
    }
    
    // Write header matching Sample-CMS.txt format
    fprintf(file, "Database Name: %s\n", db->filename);
    fprintf(file, "Authors: Your Team Name\n\n");
    fprintf(file, "Table Name: StudentRecords\n");
    fprintf(file, "ID\tName\t\tProgramme\t\tMark\n");
    
    // Write student records in tab-separated format
    for (int i = 0; i < db->count; i++) {
        const Student *s = &db->students[i];
        fprintf(file, "%d\t%s\t%s\t%.1f\n", s->id, s->name, s->programme, s->mark);
    }
    
    fclose(file);
    db->is_modified = 0;
    printf("CMS: The database file \"%s\" is successfully saved.\n", db->filename);
    return 1;
}

void show_summary(const Database *db) {
    if (db->count == 0) {
        printf("CMS: No records available for summary.\n");
        return;
    }
    
    float total_marks = 0;
    float highest_mark = db->students[0].mark;
    float lowest_mark = db->students[0].mark;
    char highest_name[MAX_NAME_LEN] = "";
    char lowest_name[MAX_NAME_LEN] = "";
    
    strcpy(highest_name, db->students[0].name);
    strcpy(lowest_name, db->students[0].name);
    
    for (int i = 0; i < db->count; i++) {
        const Student *s = &db->students[i];
        total_marks += s->mark;
        
        if (s->mark > highest_mark) {
            highest_mark = s->mark;
            strcpy(highest_name, s->name);
        }
        if (s->mark < lowest_mark) {
            lowest_mark = s->mark;
            strcpy(lowest_name, s->name);
        }
    }
    
    float average_mark = total_marks / db->count;
    
    printf("CMS: Summary Statistics\n");
    printf("======================\n");
    printf("Total number of students: %d\n", db->count);
    printf("Average mark: %.2f\n", average_mark);
    printf("Highest mark: %.1f (%s)\n", highest_mark, highest_name);
    printf("Lowest mark: %.1f (%s)\n", lowest_mark, lowest_name);
}

// Enhanced feature: Search by name pattern
void search_by_name_pattern(const Database *db, const char *pattern) {
    if (db->count == 0) {
        printf("CMS: No records found.\n");
        return;
    }
    
    printf("CMS: Searching for names containing '%s'\n", pattern);
    printf("%-10s %-20s %-25s %s\n", "ID", "Name", "Programme", "Mark");
    printf("%-10s %-20s %-25s %s\n", "----------", "--------------------", 
           "-------------------------", "----------");
    
    int found = 0;
    char lower_pattern[MAX_NAME_LEN];
    char lower_name[MAX_NAME_LEN];
    
    strcpy(lower_pattern, pattern);
    to_lower_case(lower_pattern);
    
    for (int i = 0; i < db->count; i++) {
        const Student *s = &db->students[i];
        strcpy(lower_name, s->name);
        to_lower_case(lower_name);
        
        if (strstr(lower_name, lower_pattern) != NULL) {
            printf("%-10d %-20s %-25s %.1f\n", s->id, s->name, s->programme, s->mark);
            found++;
        }
    }
    
    if (!found) {
        printf("CMS: No records found matching the pattern '%s'.\n", pattern);
    } else {
        printf("CMS: Found %d record(s) matching the pattern.\n", found);
    }
}

// Utility functions
void to_lower_case(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void trim_whitespace(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
    
    // Trim leading spaces
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}