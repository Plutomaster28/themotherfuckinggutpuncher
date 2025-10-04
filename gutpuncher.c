/*
 * GutPuncher - Aggressive Malware Removal Tool
 * Targets: Comet AI Browser and associated malware/worms
 * Build: Static with UCRT64 for maximum compatibility
 */

#include <windows.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psapi.h>

#define MAX_TARGETS 100
#define MAX_PATH_LEN 512

// Color codes for console output
#define COLOR_RED 12
#define COLOR_GREEN 10
#define COLOR_YELLOW 14
#define COLOR_WHITE 15
#define COLOR_CYAN 11

typedef struct {
    char process_name[256];
    DWORD pid;
} ProcessInfo;

// Global variables
static int verbose = 1;
static int killed_count = 0;
static int deleted_count = 0;

// Known malware process names (add more as discovered)
const char* malware_processes[] = {
    "comet.exe",
    "cometbrowser.exe",
    "comet-ai.exe",
    "cometai.exe",
    "comet_browser.exe",
    "comet_update.exe",
    "cometupdater.exe",
    "comet_service.exe",
    "comet_helper.exe",
    "comet_crashpad_handler.exe",
    "perplexity.exe",
    NULL
};

// Known malware registry keys
const char* malware_registry_keys[] = {
    "SOFTWARE\\Comet",
    "SOFTWARE\\CometAI",
    "SOFTWARE\\Comet Browser",
    "SOFTWARE\\Perplexity",
    "SOFTWARE\\Perplexity\\Comet",
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\Comet",
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\CometAI",
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\Perplexity",
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Comet",
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Perplexity",
    "SOFTWARE\\WOW6432Node\\Comet",
    "SOFTWARE\\WOW6432Node\\CometAI",
    "SOFTWARE\\WOW6432Node\\Perplexity",
    NULL
};

// Known malware directories (relative paths to search in base directories)
const char* malware_directories[] = {
    "Comet",
    "CometAI",
    "Comet Browser",
    "CometBrowser",
    "Perplexity\\Comet",
    "Perplexity",
    NULL
};

// Cache and temp directory patterns
const char* cache_patterns[] = {
    "Comet",
    "CometAI",
    "Perplexity",
    NULL
};

// File patterns to search and destroy (wildcards)
const char* file_patterns[] = {
    "comet*.exe",
    "comet*.dll",
    "comet*.log",
    "comet*.tmp",
    "perplexity*.exe",
    "perplexity*.dll",
    "perplexity*.log",
    NULL
};

void set_console_color(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void print_banner() {
    set_console_color(COLOR_RED);
    printf("\n");
    printf("  ██████╗ ██╗   ██╗████████╗██████╗ ██╗   ██╗███╗   ██╗ ██████╗██╗  ██╗███████╗██████╗ \n");
    printf(" ██╔════╝ ██║   ██║╚══██╔══╝██╔══██╗██║   ██║████╗  ██║██╔════╝██║  ██║██╔════╝██╔══██╗\n");
    printf(" ██║  ███╗██║   ██║   ██║   ██████╔╝██║   ██║██╔██╗ ██║██║     ███████║█████╗  ██████╔╝\n");
    printf(" ██║   ██║██║   ██║   ██║   ██╔═══╝ ██║   ██║██║╚██╗██║██║     ██╔══██║██╔══╝  ██╔══██╗\n");
    printf(" ╚██████╔╝╚██████╔╝   ██║   ██║     ╚██████╔╝██║ ╚████║╚██████╗██║  ██║███████╗██║  ██║\n");
    printf("  ╚═════╝  ╚═════╝    ╚═╝   ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝\n");
    set_console_color(COLOR_YELLOW);
    printf("              Aggressive Malware Removal Tool v2.0 - REVO MODE\n");
    printf("           Target: Comet AI Browser & Worms + ALL Dependencies\n");
    printf("        Deep Scan: Files, Registry, Services, Cache, Shortcuts & More\n");
    set_console_color(COLOR_WHITE);
    printf("\n");
}

void log_message(int color, const char* prefix, const char* message) {
    set_console_color(color);
    printf("[%s] ", prefix);
    set_console_color(COLOR_WHITE);
    printf("%s\n", message);
}

// Elevate privileges to get debug privileges
BOOL enable_debug_privilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return FALSE;
    }
    
    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
    CloseHandle(hToken);
    
    return GetLastError() == ERROR_SUCCESS;
}

// Check if process name matches malware patterns
BOOL is_malware_process(const char* process_name) {
    for (int i = 0; malware_processes[i] != NULL; i++) {
        if (_stricmp(process_name, malware_processes[i]) == 0) {
            return TRUE;
        }
    }
    
    // Check for partial matches (fuzzy matching)
    char lower_name[256];
    strncpy(lower_name, process_name, sizeof(lower_name) - 1);
    lower_name[sizeof(lower_name) - 1] = '\0';
    _strlwr(lower_name);
    
    if (strstr(lower_name, "comet") != NULL || 
        strstr(lower_name, "perplexity") != NULL) {
        return TRUE;
    }
    
    return FALSE;
}

// Forcefully terminate a process
BOOL kill_process(DWORD pid, const char* process_name) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        return FALSE;
    }
    
    if (TerminateProcess(hProcess, 1)) {
        char msg[512];
        sprintf(msg, "Terminated process: %s (PID: %lu)", process_name, pid);
        log_message(COLOR_GREEN, "KILL", msg);
        killed_count++;
        CloseHandle(hProcess);
        return TRUE;
    }
    
    CloseHandle(hProcess);
    return FALSE;
}

// Scan and kill malware processes
int scan_and_kill_processes() {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe32;
    int found = 0;
    
    log_message(COLOR_CYAN, "SCAN", "Scanning for malware processes...");
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        log_message(COLOR_RED, "ERROR", "Failed to create process snapshot");
        return 0;
    }
    
    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (is_malware_process(pe32.szExeFile)) {
                found++;
                char msg[512];
                sprintf(msg, "Found malware: %s (PID: %lu)", pe32.szExeFile, pe32.th32ProcessID);
                log_message(COLOR_YELLOW, "FOUND", msg);
                
                // Kill it
                if (!kill_process(pe32.th32ProcessID, pe32.szExeFile)) {
                    sprintf(msg, "Failed to kill: %s (PID: %lu)", pe32.szExeFile, pe32.th32ProcessID);
                    log_message(COLOR_RED, "ERROR", msg);
                }
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return found;
}

// Recursively delete directory
BOOL delete_directory_recursive(const char* path) {
    char search_path[MAX_PATH_LEN];
    WIN32_FIND_DATA find_data;
    HANDLE hFind;
    
    sprintf(search_path, "%s\\*.*", path);
    hFind = FindFirstFile(search_path, &find_data);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH_LEN];
        sprintf(full_path, "%s\\%s", path, find_data.cFileName);
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            delete_directory_recursive(full_path);
            RemoveDirectory(full_path);
        } else {
            // Remove read-only attribute
            SetFileAttributes(full_path, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(full_path);
            deleted_count++;
        }
    } while (FindNextFile(hFind, &find_data));
    
    FindClose(hFind);
    
    // Remove the directory itself
    SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
    RemoveDirectory(path);
    
    return TRUE;
}

// Clean temp directories
void clean_temp_directories() {
    char temp_paths[3][MAX_PATH_LEN];
    int path_count = 0;
    
    log_message(COLOR_CYAN, "SCAN", "Cleaning temporary files...");
    
    // Get temp directories
    GetEnvironmentVariable("TEMP", temp_paths[path_count++], MAX_PATH_LEN);
    GetEnvironmentVariable("TMP", temp_paths[path_count++], MAX_PATH_LEN);
    
    // Windows temp
    GetWindowsDirectory(temp_paths[path_count], MAX_PATH_LEN);
    strcat(temp_paths[path_count], "\\Temp");
    path_count++;
    
    // Search for malware files in temp
    for (int i = 0; i < path_count; i++) {
        for (int j = 0; file_patterns[j] != NULL; j++) {
            char search_path[MAX_PATH_LEN];
            sprintf(search_path, "%s\\%s", temp_paths[i], file_patterns[j]);
            
            WIN32_FIND_DATA find_data;
            HANDLE hFind = FindFirstFile(search_path, &find_data);
            
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    char full_path[MAX_PATH_LEN];
                    sprintf(full_path, "%s\\%s", temp_paths[i], find_data.cFileName);
                    
                    SetFileAttributes(full_path, FILE_ATTRIBUTE_NORMAL);
                    if (DeleteFile(full_path)) {
                        char msg[512];
                        sprintf(msg, "Deleted temp file: %s", find_data.cFileName);
                        log_message(COLOR_GREEN, "DELETE", msg);
                        deleted_count++;
                    }
                } while (FindNextFile(hFind, &find_data));
                FindClose(hFind);
            }
        }
    }
}

// Clean browser caches and profiles
void clean_browser_data() {
    char local_appdata[MAX_PATH_LEN];
    
    log_message(COLOR_CYAN, "SCAN", "Cleaning browser profiles and caches...");
    
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, local_appdata);
    
    // Cache locations
    const char* cache_dirs[] = {
        "\\Comet\\User Data",
        "\\Perplexity\\Comet\\User Data",
        "\\Comet\\Cache",
        "\\Perplexity\\Cache",
        "\\Comet\\GPUCache",
        "\\Comet\\Code Cache",
        "\\Comet\\Service Worker",
        NULL
    };
    
    for (int i = 0; cache_dirs[i] != NULL; i++) {
        char target_path[MAX_PATH_LEN];
        sprintf(target_path, "%s%s", local_appdata, cache_dirs[i]);
        
        if (GetFileAttributes(target_path) != INVALID_FILE_ATTRIBUTES) {
            char msg[512];
            sprintf(msg, "Found cache/profile: %s", target_path);
            log_message(COLOR_YELLOW, "FOUND", msg);
            
            if (delete_directory_recursive(target_path)) {
                sprintf(msg, "Deleted: %s", target_path);
                log_message(COLOR_GREEN, "DELETE", msg);
            }
        }
    }
}

// Delete shortcuts from desktop, start menu, etc.
void delete_shortcuts() {
    char shortcut_paths[10][MAX_PATH_LEN];
    int path_count = 0;
    
    log_message(COLOR_CYAN, "SCAN", "Removing shortcuts...");
    
    // Get shortcut locations
    SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, shortcut_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, 0, shortcut_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_STARTMENU, NULL, 0, shortcut_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_COMMON_STARTMENU, NULL, 0, shortcut_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_PROGRAMS, NULL, 0, shortcut_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, 0, shortcut_paths[path_count++]);
    
    const char* shortcut_names[] = {
        "Comet*.lnk",
        "Perplexity*.lnk",
        "Comet AI*.lnk",
        NULL
    };
    
    for (int i = 0; i < path_count; i++) {
        for (int j = 0; shortcut_names[j] != NULL; j++) {
            char search_path[MAX_PATH_LEN];
            sprintf(search_path, "%s\\%s", shortcut_paths[i], shortcut_names[j]);
            
            WIN32_FIND_DATA find_data;
            HANDLE hFind = FindFirstFile(search_path, &find_data);
            
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    char full_path[MAX_PATH_LEN];
                    sprintf(full_path, "%s\\%s", shortcut_paths[i], find_data.cFileName);
                    
                    if (DeleteFile(full_path)) {
                        char msg[512];
                        sprintf(msg, "Deleted shortcut: %s", find_data.cFileName);
                        log_message(COLOR_GREEN, "DELETE", msg);
                        deleted_count++;
                    }
                } while (FindNextFile(hFind, &find_data));
                FindClose(hFind);
            }
        }
    }
}

// Stop and delete Windows services
void clean_services() {
    log_message(COLOR_CYAN, "SCAN", "Checking for malware services...");
    
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (scm == NULL) {
        log_message(COLOR_YELLOW, "WARN", "Could not open Service Control Manager");
        return;
    }
    
    DWORD bytesNeeded, servicesReturned;
    EnumServicesStatusEx(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                        NULL, 0, &bytesNeeded, &servicesReturned, NULL, NULL);
    
    LPBYTE buffer = (LPBYTE)malloc(bytesNeeded);
    if (buffer && EnumServicesStatusEx(scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                                       buffer, bytesNeeded, &bytesNeeded, &servicesReturned, NULL, NULL)) {
        
        LPENUM_SERVICE_STATUS_PROCESS services = (LPENUM_SERVICE_STATUS_PROCESS)buffer;
        
        for (DWORD i = 0; i < servicesReturned; i++) {
            char lower_name[256];
            strncpy(lower_name, services[i].lpServiceName, sizeof(lower_name) - 1);
            _strlwr(lower_name);
            
            if (strstr(lower_name, "comet") != NULL || strstr(lower_name, "perplexity") != NULL) {
                SC_HANDLE service = OpenService(scm, services[i].lpServiceName, SERVICE_ALL_ACCESS);
                if (service) {
                    // Stop the service
                    SERVICE_STATUS status;
                    ControlService(service, SERVICE_CONTROL_STOP, &status);
                    
                    // Delete the service
                    if (DeleteService(service)) {
                        char msg[512];
                        sprintf(msg, "Deleted service: %s", services[i].lpServiceName);
                        log_message(COLOR_GREEN, "DELETE", msg);
                    }
                    CloseServiceHandle(service);
                }
            }
        }
    }
    
    if (buffer) free(buffer);
    CloseServiceHandle(scm);
}

// Clean Windows Prefetch (execution traces)
void clean_prefetch() {
    log_message(COLOR_CYAN, "SCAN", "Cleaning prefetch traces...");
    
    char windows_dir[MAX_PATH_LEN];
    GetWindowsDirectory(windows_dir, MAX_PATH_LEN);
    
    char prefetch_path[MAX_PATH_LEN];
    sprintf(prefetch_path, "%s\\Prefetch", windows_dir);
    
    const char* prefetch_patterns[] = {
        "COMET*.pf",
        "PERPLEXITY*.pf",
        NULL
    };
    
    for (int i = 0; prefetch_patterns[i] != NULL; i++) {
        char search_path[MAX_PATH_LEN];
        sprintf(search_path, "%s\\%s", prefetch_path, prefetch_patterns[i]);
        
        WIN32_FIND_DATA find_data;
        HANDLE hFind = FindFirstFile(search_path, &find_data);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                char full_path[MAX_PATH_LEN];
                sprintf(full_path, "%s\\%s", prefetch_path, find_data.cFileName);
                
                if (DeleteFile(full_path)) {
                    char msg[512];
                    sprintf(msg, "Deleted prefetch: %s", find_data.cFileName);
                    log_message(COLOR_GREEN, "DELETE", msg);
                    deleted_count++;
                }
            } while (FindNextFile(hFind, &find_data));
            FindClose(hFind);
        }
    }
}

// Clean file associations
void clean_file_associations() {
    log_message(COLOR_CYAN, "SCAN", "Cleaning file associations...");
    
    HKEY roots[] = {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};
    const char* root_names[] = {"HKCU", "HKLM"};
    
    const char* assoc_paths[] = {
        "SOFTWARE\\Classes",
        "SOFTWARE\\RegisteredApplications",
        "SOFTWARE\\Clients\\StartMenuInternet",
        NULL
    };
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; assoc_paths[j] != NULL; j++) {
            HKEY hKey;
            if (RegOpenKeyEx(roots[i], assoc_paths[j], 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS) {
                char subkey_name[256];
                DWORD index = 0;
                
                while (RegEnumKey(hKey, index, subkey_name, sizeof(subkey_name)) == ERROR_SUCCESS) {
                    char lower_name[256];
                    strncpy(lower_name, subkey_name, sizeof(lower_name) - 1);
                    _strlwr(lower_name);
                    
                    if (strstr(lower_name, "comet") != NULL || strstr(lower_name, "perplexity") != NULL) {
                        char full_path[512];
                        sprintf(full_path, "%s\\%s", assoc_paths[j], subkey_name);
                        
                        if (RegDeleteTree(roots[i], full_path) == ERROR_SUCCESS) {
                            char msg[512];
                            sprintf(msg, "Deleted file association: %s\\%s", root_names[i], full_path);
                            log_message(COLOR_GREEN, "DELETE", msg);
                        }
                    } else {
                        index++;
                    }
                }
                RegCloseKey(hKey);
            }
        }
    }
}

// Clean downloaded installers
void clean_installers() {
    log_message(COLOR_CYAN, "SCAN", "Searching for malware installers...");
    
    char download_paths[5][MAX_PATH_LEN];
    int path_count = 0;
    
    // Common download locations
    SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, download_paths[path_count]);
    strcat(download_paths[path_count++], "\\Downloads");
    
    SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, download_paths[path_count++]);
    
    GetEnvironmentVariable("TEMP", download_paths[path_count++], MAX_PATH_LEN);
    
    const char* installer_patterns[] = {
        "*comet*.exe",
        "*comet*.msi",
        "*perplexity*.exe",
        "*perplexity*.msi",
        "CometSetup*.exe",
        "CometInstaller*.exe",
        NULL
    };
    
    for (int i = 0; i < path_count; i++) {
        for (int j = 0; installer_patterns[j] != NULL; j++) {
            char search_path[MAX_PATH_LEN];
            sprintf(search_path, "%s\\%s", download_paths[i], installer_patterns[j]);
            
            WIN32_FIND_DATA find_data;
            HANDLE hFind = FindFirstFile(search_path, &find_data);
            
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    char full_path[MAX_PATH_LEN];
                    sprintf(full_path, "%s\\%s", download_paths[i], find_data.cFileName);
                    
                    SetFileAttributes(full_path, FILE_ATTRIBUTE_NORMAL);
                    if (DeleteFile(full_path)) {
                        char msg[512];
                        sprintf(msg, "Deleted installer: %s", find_data.cFileName);
                        log_message(COLOR_GREEN, "DELETE", msg);
                        deleted_count++;
                    }
                } while (FindNextFile(hFind, &find_data));
                FindClose(hFind);
            }
        }
    }
}

// Search and destroy malware files
void search_and_destroy_files() {
    char base_paths[6][MAX_PATH_LEN];
    int path_count = 0;
    
    log_message(COLOR_CYAN, "SCAN", "Searching for malware files...");
    
    // Get common installation directories
    SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, 0, base_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, base_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, base_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, base_paths[path_count++]);
    SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, base_paths[path_count++]);
    
    // Also check user profile directly (some malware installs here)
    char user_profile[MAX_PATH_LEN];
    if (GetEnvironmentVariable("USERPROFILE", user_profile, sizeof(user_profile)) > 0) {
        strncpy(base_paths[path_count++], user_profile, MAX_PATH_LEN - 1);
    }
    
    // Search in each base path
    for (int i = 0; i < path_count; i++) {
        for (int j = 0; malware_directories[j] != NULL; j++) {
            char target_path[MAX_PATH_LEN];
            sprintf(target_path, "%s\\%s", base_paths[i], malware_directories[j]);
            
            if (GetFileAttributes(target_path) != INVALID_FILE_ATTRIBUTES) {
                char msg[512];
                sprintf(msg, "Found malware directory: %s", target_path);
                log_message(COLOR_YELLOW, "FOUND", msg);
                
                if (delete_directory_recursive(target_path)) {
                    sprintf(msg, "Deleted: %s", target_path);
                    log_message(COLOR_GREEN, "DELETE", msg);
                } else {
                    sprintf(msg, "Failed to delete: %s", target_path);
                    log_message(COLOR_RED, "ERROR", msg);
                }
            }
        }
    }
}

// Delete registry keys
void clean_registry() {
    HKEY roots[] = {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};
    const char* root_names[] = {"HKCU", "HKLM"};
    
    log_message(COLOR_CYAN, "SCAN", "Cleaning registry...");
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; malware_registry_keys[j] != NULL; j++) {
            LONG result = RegDeleteTree(roots[i], malware_registry_keys[j]);
            
            if (result == ERROR_SUCCESS) {
                char msg[512];
                sprintf(msg, "Deleted registry key: %s\\%s", root_names[i], malware_registry_keys[j]);
                log_message(COLOR_GREEN, "DELETE", msg);
            }
        }
    }
}

// Clean startup entries
void clean_startup() {
    log_message(COLOR_CYAN, "SCAN", "Cleaning startup entries...");
    
    const char* startup_keys[] = {
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run",
        "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        NULL
    };
    
    HKEY roots[] = {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE};
    const char* root_names[] = {"HKCU", "HKLM"};
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; startup_keys[j] != NULL; j++) {
            HKEY hKey;
            if (RegOpenKeyEx(roots[i], startup_keys[j], 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
                char value_name[256];
                char value_data[MAX_PATH_LEN];
                DWORD value_name_len, value_data_len;
                DWORD index = 0;
                
                while (1) {
                    value_name_len = sizeof(value_name);
                    value_data_len = sizeof(value_data);
                    
                    if (RegEnumValue(hKey, index, value_name, &value_name_len, NULL, NULL, 
                                    (LPBYTE)value_data, &value_data_len) != ERROR_SUCCESS) {
                        break;
                    }
                    
                    // Check if value contains malware references
                    char lower_name[256], lower_data[MAX_PATH_LEN];
                    strncpy(lower_name, value_name, sizeof(lower_name) - 1);
                    strncpy(lower_data, value_data, sizeof(lower_data) - 1);
                    lower_name[sizeof(lower_name) - 1] = '\0';
                    lower_data[sizeof(lower_data) - 1] = '\0';
                    _strlwr(lower_name);
                    _strlwr(lower_data);
                    
                    if (strstr(lower_name, "comet") != NULL || 
                        strstr(lower_data, "comet") != NULL ||
                        strstr(lower_name, "perplexity") != NULL || 
                        strstr(lower_data, "perplexity") != NULL) {
                        
                        if (RegDeleteValue(hKey, value_name) == ERROR_SUCCESS) {
                            char msg[512];
                            sprintf(msg, "Removed startup entry: %s\\%s\\%s", 
                                   root_names[i], startup_keys[j], value_name);
                            log_message(COLOR_GREEN, "DELETE", msg);
                        }
                    } else {
                        index++;
                    }
                }
                
                RegCloseKey(hKey);
            }
        }
    }
}

// Clean scheduled tasks
void clean_scheduled_tasks() {
    log_message(COLOR_CYAN, "SCAN", "Cleaning scheduled tasks...");
    
    // Use schtasks command to find and delete malware tasks
    system("schtasks /query /fo LIST /v > nul 2>&1 | findstr /i \"comet perplexity\" > tasks.tmp");
    
    FILE* fp = fopen("tasks.tmp", "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "TaskName:") != NULL) {
                char* task_name = strchr(line, '\\');
                if (task_name) {
                    task_name[strcspn(task_name, "\r\n")] = 0;
                    
                    char cmd[512];
                    sprintf(cmd, "schtasks /delete /tn \"%s\" /f > nul 2>&1", task_name);
                    
                    if (system(cmd) == 0) {
                        char msg[512];
                        sprintf(msg, "Deleted scheduled task: %s", task_name);
                        log_message(COLOR_GREEN, "DELETE", msg);
                    }
                }
            }
        }
        fclose(fp);
    }
    DeleteFile("tasks.tmp");
}

// Main execution
int main(void) {
    print_banner();
    
    // Check for admin privileges
    BOOL isAdmin = FALSE;
    PSID adminGroup;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    
    if (!isAdmin) {
        set_console_color(COLOR_RED);
        printf("\n[!] WARNING: Not running as Administrator!\n");
        printf("[!] Some features may not work properly.\n");
        printf("[!] Please run as Administrator for full functionality.\n\n");
        set_console_color(COLOR_WHITE);
        
        printf("Press Enter to continue anyway, or Ctrl+C to exit...");
        getchar();
    }
    
    // Enable debug privileges
    if (!enable_debug_privilege()) {
        log_message(COLOR_YELLOW, "WARN", "Could not enable debug privileges");
    }
    
    log_message(COLOR_CYAN, "START", "Beginning malware removal...");
    printf("\n");
    
    // Phase 1: Kill processes (do this multiple times to catch respawns)
    for (int i = 0; i < 3; i++) {
        int found = scan_and_kill_processes();
        if (found > 0 && i < 2) {
            log_message(COLOR_YELLOW, "INFO", "Waiting for respawn detection...");
            Sleep(2000);
        }
    }
    printf("\n");
    
    // Phase 2: Delete files
    search_and_destroy_files();
    printf("\n");
    
    // Phase 3: Clean temp directories
    clean_temp_directories();
    printf("\n");
    
    // Phase 4: Clean browser data
    clean_browser_data();
    printf("\n");
    
    // Phase 5: Delete shortcuts
    delete_shortcuts();
    printf("\n");
    
    // Phase 6: Clean services
    clean_services();
    printf("\n");
    
    // Phase 7: Clean prefetch
    clean_prefetch();
    printf("\n");
    
    // Phase 8: Clean file associations
    clean_file_associations();
    printf("\n");
    
    // Phase 9: Clean installers
    clean_installers();
    printf("\n");
    
    // Phase 10: Clean registry
    clean_registry();
    printf("\n");
    
    // Phase 11: Clean startup
    clean_startup();
    printf("\n");
    
    // Phase 12: Clean scheduled tasks
    clean_scheduled_tasks();
    printf("\n");
    
    // Summary
    set_console_color(COLOR_GREEN);
    printf("========================================\n");
    printf("           CLEANUP COMPLETE\n");
    printf("========================================\n");
    set_console_color(COLOR_WHITE);
    printf("Processes terminated: %d\n", killed_count);
    printf("Files deleted: %d\n", deleted_count);
    printf("\n");
    
    set_console_color(COLOR_YELLOW);
    printf("[RECOMMENDATION] Restart your computer to complete cleanup.\n");
    set_console_color(COLOR_WHITE);
    
    printf("\nPress Enter to exit...");
    getchar();
    
    return 0;
}
