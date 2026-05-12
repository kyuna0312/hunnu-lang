#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

static int is_valid_project_name(const char* name) {
    if (!name || !*name) return 0;
    for (const char* p = name; *p; p++) {
        if (!((*p >= 'a' && *p <= 'z') ||
              (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') ||
              *p == '-' || *p == '_')) {
            return 0;
        }
    }
    return 1;
}

static int is_valid_package_name(const char* name) {
    if (!name || !*name) return 0;
    for (const char* p = name; *p; p++) {
        if (!((*p >= 'a' && *p <= 'z') ||
              (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') ||
              *p == '-' || *p == '_' || *p == '/')) {
            return 0;
        }
    }
    return 1;
}

int cmd_new(const char* project_name) {
    if (!project_name || !*project_name) {
        fprintf(stderr, "Error: Project name is required\n");
        fprintf(stderr, "Usage: hunnu new <project-name>\n");
        return 1;
    }

    if (!is_valid_project_name(project_name)) {
        fprintf(stderr, "Error: Invalid project name '%s'. Use only letters, numbers, hyphens, and underscores.\n", project_name);
        return 1;
    }

    struct stat st;
    if (stat(project_name, &st) == 0) {
        fprintf(stderr, "Error: Directory '%s' already exists\n", project_name);
        return 1;
    }

    if (mkdir(project_name, 0755) != 0) {
        fprintf(stderr, "Error: Failed to create project directory '%s': %s\n", project_name, strerror(errno));
        return 1;
    }

    char path[1024];

    snprintf(path, sizeof(path), "%s/main.hn", project_name);
    FILE* f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Error: Failed to create %s: %s\n", path, strerror(errno));
        return 1;
    }
    fprintf(f, "import std.io\n\n");
    fprintf(f, "fn main() {\n");
    fprintf(f, "    println(\"Hello from %s!\")\n", project_name);
    fprintf(f, "}\n");
    fclose(f);

    snprintf(path, sizeof(path), "%s/hunnu.json", project_name);
    f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "Error: Failed to create %s: %s\n", path, strerror(errno));
        return 1;
    }
    fprintf(f, "{\n");
    fprintf(f, "  \"name\": \"%s\",\n", project_name);
    fprintf(f, "  \"version\": \"1.0.0\",\n");
    fprintf(f, "  \"dependencies\": {}\n");
    fprintf(f, "}\n");
    fclose(f);

    printf("Created new Hunnu project '%s'\n", project_name);
    printf("  %s/main.hn\n", project_name);
    printf("  %s/hunnu.json\n", project_name);
    printf("\nRun it with: hunnu run %s/main.hn\n", project_name);
    return 0;
}

int cmd_install(const char* package_name) {
    if (!package_name || !*package_name) {
        fprintf(stderr, "Error: Package name is required\n");
        fprintf(stderr, "Usage: hunnu install <package>\n");
        fprintf(stderr, "  <package> can be 'user/repo' (GitHub) or a simple name\n");
        return 1;
    }

    if (!is_valid_package_name(package_name)) {
        fprintf(stderr, "Error: Invalid package name '%s'\n", package_name);
        return 1;
    }

    struct stat st;
    if (stat("hunnu_modules", &st) != 0) {
        if (mkdir("hunnu_modules", 0755) != 0) {
            fprintf(stderr, "Error: Failed to create hunnu_modules directory: %s\n", strerror(errno));
            return 1;
        }
    }

    char module_path[1024];
    snprintf(module_path, sizeof(module_path), "hunnu_modules/%s", package_name);

    if (stat(module_path, &st) == 0) {
        fprintf(stderr, "Error: Package '%s' is already installed in %s\n", package_name, module_path);
        return 1;
    }

    if (strchr(package_name, '/')) {
        // GitHub-style package: user/repo
        char repo_url[1024];
        snprintf(repo_url, sizeof(repo_url), "https://github.com/%s.git", package_name);

        printf("Installing '%s' from GitHub...\n", package_name);
        fflush(stdout);

        char cmd[2048];
        snprintf(cmd, sizeof(cmd), "git clone %s %s 2>/dev/null", repo_url, module_path);

        int status = system(cmd);
        if (status != 0) {
            // Clean up empty directory if clone failed
            rmdir(module_path);
            fprintf(stderr, "Error: Failed to clone repository '%s'.\n", repo_url);
            fprintf(stderr, "  Make sure git is installed and the repository exists.\n");
            return 1;
        }

        printf("Successfully installed '%s'\n", package_name);
    } else {
        // Simple package name - create empty module directory
        if (mkdir(module_path, 0755) != 0) {
            fprintf(stderr, "Error: Failed to create module directory: %s\n", strerror(errno));
            return 1;
        }

        printf("Successfully installed '%s' (empty module)\n", package_name);
    }

    return 0;
}
