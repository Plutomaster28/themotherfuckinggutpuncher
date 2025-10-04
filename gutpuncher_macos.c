/*
 * GutPuncher - Aggressive Malware Removal Tool (macOS Version)
 * Targets: Comet AI Browser and associated malware/worms
 * Build: Static for maximum compatibility
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <libproc.h>

#define MAX_PATH_LEN 1024
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_CYAN    "\033[0;36m"
#define COLOR_WHITE   "\033[0;37m"
#define COLOR_RESET   "\033[0m"

// Global counters
static int killed_count = 0;
static int deleted_count = 0;

// Known malware process names
const char* malware_processes[] = {
    "comet",
    "Comet",
    "CometBrowser",
    "comet-ai",
    "CometAI",
    "Comet Helper",
    "Comet Browser",
    "Perplexity",
    NULL
};

// Known malware directories in user space
const char* malware_dirs[] = {
    "/Applications/Comet.app",
    "/Applications/CometAI.app",
    "/Applications/Comet Browser.app",
    "/Applications/Perplexity.app",
    "~/Library/Application Support/Comet",
    "~/Library/Application Support/CometAI",
    "~/Library/Application Support/Perplexity",
    "~/Library/Caches/Comet",
    "~/Library/Caches/CometAI",
    "~/Library/Caches/Perplexity",
    "~/Library/Preferences/com.comet.*",
    "~/Library/Preferences/com.perplexity.*",
    NULL
};

void print_banner() {
    printf(COLOR_RED);
    printf("\n");
    printf("  ██████╗ ██╗   ██╗████████╗██████╗ ██╗   ██╗███╗   ██╗ ██████╗██╗  ██╗███████╗██████╗ \n");
    printf(" ██╔════╝ ██║   ██║╚══██╔══╝██╔══██╗██║   ██║████╗  ██║██╔════╝██║  ██║██╔════╝██╔══██╗\n");
    printf(" ██║  ███╗██║   ██║   ██║   ██████╔╝██║   ██║██╔██╗ ██║██║     ███████║█████╗  ██████╔╝\n");
    printf(" ██║   ██║██║   ██║   ██║   ██╔═══╝ ██║   ██║██║╚██╗██║██║     ██╔══██║██╔══╝  ██╔══██╗\n");
    printf(" ╚██████╔╝╚██████╔╝   ██║   ██║     ╚██████╔╝██║ ╚████║╚██████╗██║  ██║███████╗██║  ██║\n");
    printf("  ╚═════╝  ╚═════╝    ╚═╝   ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n");
    printf(COLOR_RESET);
    printf(COLOR_YELLOW);
    printf("              Aggressive Malware Removal Tool v2.0 - macOS Edition\n");
    printf("           Target: Comet AI Browser & Worms + ALL Dependencies\n");
    printf("        Deep Scan: Processes, Files, Cache, LaunchAgents & More\n");
    printf(COLOR_RESET);
    printf("\n");
}

void log_message(const char* color, const char* prefix, const char* message) {
    printf("%s[%s]%s %s\n", color, prefix, COLOR_RESET, message);
}

// Check if running as root
int check_root() {
    return (geteuid() == 0);
}

// Check if process name matches malware
int is_malware_process(const char* name) {
    for (int i = 0; malware_processes[i] != NULL; i++) {
        if (strcasestr(name, malware_processes[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}

// Kill process by PID
int kill_process(pid_t pid, const char* name) {
    if (kill(pid, SIGKILL) == 0) {
        char msg[512];
        snprintf(msg, sizeof(msg), "Terminated process: %s (PID: %d)", name, pid);
        log_message(COLOR_GREEN, "KILL", msg);
        killed_count++;
        return 1;
    }
    return 0;
}

// Scan and kill malware processes
void scan_and_kill_processes() {
    log_message(COLOR_CYAN, "SCAN", "Scanning for malware processes...");
    
    int num_pids = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    if (num_pids <= 0) {
        log_message(COLOR_RED, "ERROR", "Failed to get process list");
        return;
    }
    
    pid_t* pids = malloc(sizeof(pid_t) * num_pids);
    num_pids = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pid_t) * num_pids);
    
    for (int i = 0; i < num_pids; i++) {
        if (pids[i] == 0) continue;
        
        char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
        if (proc_pidpath(pids[i], pathbuf, sizeof(pathbuf)) > 0) {
            char* basename = strrchr(pathbuf, '/');
            char* name = basename ? basename + 1 : pathbuf;
            
            if (is_malware_process(name)) {
                char msg[512];
                snprintf(msg, sizeof(msg), "Found malware: %s (PID: %d)", name, pids[i]);
                log_message(COLOR_YELLOW, "FOUND", msg);
                
                if (!kill_process(pids[i], name)) {
                    snprintf(msg, sizeof(msg), "Failed to kill: %s (PID: %d)", name, pids[i]);
                    log_message(COLOR_RED, "ERROR", msg);
                }
            }
        }
    }
    
    free(pids);
}

// Recursively delete directory
int delete_directory(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return 0;
    
    struct dirent* entry;
    char filepath[MAX_PATH_LEN];
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        
        struct stat statbuf;
        if (lstat(filepath, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                delete_directory(filepath);
            } else {
                unlink(filepath);
                deleted_count++;
            }
        }
    }
    
    closedir(dir);
    rmdir(path);
    return 1;
}

// Expand tilde in path
void expand_path(const char* path, char* expanded, size_t size) {
    if (path[0] == '~') {
        const char* home = getenv("HOME");
        if (!home) {
            struct passwd* pw = getpwuid(getuid());
            home = pw->pw_dir;
        }
        snprintf(expanded, size, "%s%s", home, path + 1);
    } else {
        strncpy(expanded, path, size - 1);
        expanded[size - 1] = '\0';
    }
}

// Delete application bundle
void delete_application(const char* app_path) {
    char msg[512];
    
    struct stat st;
    if (stat(app_path, &st) == 0) {
        snprintf(msg, sizeof(msg), "Found application: %s", app_path);
        log_message(COLOR_YELLOW, "FOUND", msg);
        
        if (delete_directory(app_path)) {
            snprintf(msg, sizeof(msg), "Deleted: %s", app_path);
            log_message(COLOR_GREEN, "DELETE", msg);
        } else {
            snprintf(msg, sizeof(msg), "Failed to delete: %s", app_path);
            log_message(COLOR_RED, "ERROR", msg);
        }
    }
}

// Search and destroy malware files
void search_and_destroy_files() {
    log_message(COLOR_CYAN, "SCAN", "Searching for malware files...");
    
    for (int i = 0; malware_dirs[i] != NULL; i++) {
        char expanded[MAX_PATH_LEN];
        expand_path(malware_dirs[i], expanded, sizeof(expanded));
        
        // Handle wildcards
        if (strstr(expanded, "*") != NULL) {
            char* dir_part = strdup(expanded);
            char* wildcard = strrchr(dir_part, '/');
            if (wildcard) {
                *wildcard = '\0';
                char* pattern = wildcard + 1;
                
                DIR* dir = opendir(dir_part);
                if (dir) {
                    struct dirent* entry;
                    while ((entry = readdir(dir)) != NULL) {
                        if (strstr(entry->d_name, "comet") != NULL ||
                            strstr(entry->d_name, "Comet") != NULL ||
                            strstr(entry->d_name, "perplexity") != NULL ||
                            strstr(entry->d_name, "Perplexity") != NULL) {
                            char full_path[MAX_PATH_LEN];
                            snprintf(full_path, sizeof(full_path), "%s/%s", dir_part, entry->d_name);
                            
                            struct stat st;
                            if (stat(full_path, &st) == 0) {
                                if (S_ISDIR(st.st_mode)) {
                                    delete_application(full_path);
                                } else {
                                    unlink(full_path);
                                    deleted_count++;
                                }
                            }
                        }
                    }
                    closedir(dir);
                }
            }
            free(dir_part);
        } else {
            delete_application(expanded);
        }
    }
}

// Clean LaunchAgents and LaunchDaemons
void clean_launch_agents() {
    log_message(COLOR_CYAN, "SCAN", "Cleaning LaunchAgents and LaunchDaemons...");
    
    const char* agent_paths[] = {
        "~/Library/LaunchAgents",
        "/Library/LaunchAgents",
        "/Library/LaunchDaemons",
        "/System/Library/LaunchAgents",
        "/System/Library/LaunchDaemons",
        NULL
    };
    
    for (int i = 0; agent_paths[i] != NULL; i++) {
        char expanded[MAX_PATH_LEN];
        expand_path(agent_paths[i], expanded, sizeof(expanded));
        
        DIR* dir = opendir(expanded);
        if (!dir) continue;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strstr(entry->d_name, "comet") != NULL ||
                strstr(entry->d_name, "Comet") != NULL ||
                strstr(entry->d_name, "perplexity") != NULL ||
                strstr(entry->d_name, "Perplexity") != NULL) {
                
                char plist_path[MAX_PATH_LEN];
                snprintf(plist_path, sizeof(plist_path), "%s/%s", expanded, entry->d_name);
                
                if (unlink(plist_path) == 0) {
                    char msg[512];
                    snprintf(msg, sizeof(msg), "Deleted LaunchAgent: %s", entry->d_name);
                    log_message(COLOR_GREEN, "DELETE", msg);
                    deleted_count++;
                }
            }
        }
        closedir(dir);
    }
}

// Clean browser profiles and caches
void clean_browser_data() {
    log_message(COLOR_CYAN, "SCAN", "Cleaning browser caches and profiles...");
    
    const char* cache_paths[] = {
        "~/Library/Caches/Comet",
        "~/Library/Caches/CometAI",
        "~/Library/Caches/Perplexity",
        "~/Library/Application Support/Comet",
        "~/Library/Application Support/CometAI",
        "~/Library/Application Support/Perplexity",
        "~/Library/Saved Application State/com.comet.*",
        "~/Library/Saved Application State/com.perplexity.*",
        NULL
    };
    
    for (int i = 0; cache_paths[i] != NULL; i++) {
        char expanded[MAX_PATH_LEN];
        expand_path(cache_paths[i], expanded, sizeof(expanded));
        delete_application(expanded);
    }
}

// Main execution
int main(void) {
    print_banner();
    
    // Check for root privileges
    if (!check_root()) {
        printf("%s[!] WARNING: Not running as root!%s\n", COLOR_RED, COLOR_RESET);
        printf("%s[!] Some features may not work properly.%s\n", COLOR_RED, COLOR_RESET);
        printf("%s[!] Please run with sudo for full functionality.%s\n\n", COLOR_RED, COLOR_RESET);
        
        printf("Press Enter to continue anyway, or Ctrl+C to exit...");
        getchar();
    }
    
    log_message(COLOR_CYAN, "START", "Beginning malware removal...");
    printf("\n");
    
    // Phase 1: Kill processes (multiple iterations for respawn detection)
    for (int i = 0; i < 3; i++) {
        scan_and_kill_processes();
        if (i < 2) {
            log_message(COLOR_YELLOW, "INFO", "Waiting for respawn detection...");
            sleep(2);
        }
    }
    printf("\n");
    
    // Phase 2: Delete application bundles
    search_and_destroy_files();
    printf("\n");
    
    // Phase 3: Clean browser data
    clean_browser_data();
    printf("\n");
    
    // Phase 4: Clean LaunchAgents
    clean_launch_agents();
    printf("\n");
    
    // Summary
    printf(COLOR_GREEN);
    printf("========================================\n");
    printf("           CLEANUP COMPLETE\n");
    printf("========================================\n");
    printf(COLOR_RESET);
    printf("Processes terminated: %d\n", killed_count);
    printf("Files deleted: %d\n", deleted_count);
    printf("\n");
    
    printf(COLOR_YELLOW);
    printf("[RECOMMENDATION] Restart your Mac to complete cleanup.\n");
    printf(COLOR_RESET);
    
    printf("\nPress Enter to exit...");
    getchar();
    
    return 0;
}
