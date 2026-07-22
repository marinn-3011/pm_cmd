#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h> // Contains unlink() and rmdir()
#include <sys/stat.h> // Required for mkdir on Linux/macOS
#include <sys/types.h>

// Variables to set
char proj_path[] = "/path/to/your/projects/folder/";
char folder_prefix[] = "Project_";


char *dynstrcat(char *str1, const char *str2){
	char *temp = realloc(str1, strlen(str1)+strlen(str2)+1);
	if (temp == NULL){
		printf("Error generating path..\n");
		free(str1);
		return temp;
	}
	str1 = temp;
	strcat(str1, str2);
	return str1;
}

int newproj(const char* path, const char* folder_name, const char *ext){

    int ret;

    // 0777 gives rwx permissions
    ret = mkdir(path, 0777);

    // Check if the folder was successfully created
    if (ret == 0) {
        printf("Folder '%s' created successfully..\n", folder_name);
    }
    else {
        printf("Failed to create folder or it already exists.\n");
        perror("Error");
	return 1;
    }

    size_t path_len = strlen(path)+1;
    char *file_path = malloc(path_len);
    
    if (!file_path) return 1;

    FILE *fp;

    strcpy(file_path, path);
    if((file_path=dynstrcat(file_path, "/Logfile.txt"))==NULL) return 1;
    
    fp = fopen(file_path, "w");
    if (fp == NULL){
        printf("Error in creating logfile. Exiting..\n");
        return 1;
    }
    else{
        printf("Logfile created successfully..\n");
        fclose(fp);
    }

    if (strcmp(ext,"")==0)
        printf("Language unidentified or not specified. Creating empty main file..\n");

    strcpy(file_path, path);
    if((file_path=dynstrcat(file_path, "/main"))==NULL) return 1;
    //strcat(file_path, "/main");
    if((file_path=dynstrcat(file_path, ext))==NULL) return 1;
    //strcat(file_path, ext);

    fp = fopen(file_path, "w");
    if (fp == NULL){
        printf("Error in creating code file. Exiting..\n");
        free(file_path);
        return 1;
	    }
    else{
        printf("Main created successfully.\n");
        fclose(fp);
    }
    free(file_path);
    return 0;
}

int viewdir(const char *base_path, int indent_level){ // Recursive list
	DIR *dir = opendir(base_path);
	struct dirent *entry;
	struct stat statbuf;
	size_t path_len = strlen(base_path);

	if (dir==NULL){
		printf("\e[1;31m[PROJECT MISSING/DOES NOT EXIST]\e[0m\n");
		return 1;
	}

	while ((entry=readdir(dir)) != NULL){
		// Skip . and ..
		if (strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0){
		continue;
		}

		// Dynamically allocate buffer space for the path string
		size_t len = path_len + strlen(entry->d_name) + 2;
		char *path = malloc(len);

		if (!path) return 1;

		snprintf(path, len, "%s/%s", base_path, entry->d_name);

		//Skip if no metadata
		if (stat(path, &statbuf) == -1){
			continue;
		}

		for(int i=0; i<indent_level; i++){
			printf("    ");
		}
		printf("-");


		if (S_ISDIR(statbuf.st_mode)){
			printf("\e[34m[%s]\e[0m\n", entry->d_name);

			viewdir(path, indent_level+1);
		}
		else printf("%s\n", entry->d_name);

		free(path);
	}
	printf("\n");

	closedir(dir);
	return 0;
}

int removedir(const char *base_path){ // Recursive remove
	DIR *dir = opendir(base_path);
	size_t path_len = strlen(base_path);
	int ret = 0;

	if (!dir) return 1;

	struct dirent *entry;

	while(!ret && (entry = readdir(dir))){
		int r = -1;
		
		if (strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0){
			continue;
		}

		// Dynamically allot buffer space for the path string
		size_t len = path_len + strlen(entry->d_name) + 2;
		char *buf = malloc(len);

		if (buf){
			struct stat statbuf;
			snprintf(buf, len, "%s/%s", base_path, entry->d_name);
			
			// Skip if no metadata
			if (stat(buf, &statbuf) == -1) continue;

			if (!lstat(buf, &statbuf)){
				if (S_ISDIR(statbuf.st_mode)){
					// If dir: Enter and recursively delete contents
					r = removedir(buf);
				}
				else{
					// Delete files & symbolic links
					r = unlink(buf);
				}
			}
			free(buf);
		}
		ret = r;
	}

	closedir(dir);

	// Delete the empty parent directory
	if (!ret){
		ret = rmdir(base_path);
	}

	return ret;
}

void listdir(const char *base_path){ // Non-recursive list
	DIR *dir = opendir(base_path);
	struct dirent *entry;
	struct stat statbuf;
	size_t path_len = strlen(base_path);

	if (dir==NULL){
		printf("\e[31mCould not access the projects folder. Exiting..\e[0m\n");
		return;
	}

	while ((entry=readdir(dir)) != NULL){
		// Skip . and ..
		if (strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0){
		continue;
		}

		// Dynamically allocate buffer space for the path string
		size_t len = path_len + strlen(entry->d_name) + 2;
		char *path = malloc(len);

		if (!path) return;

		snprintf(path, len, "%s/%s", base_path, entry->d_name);

		//Skip if no metadata
		if (stat(path, &statbuf) == -1){
			continue;
		}


		if (S_ISDIR(statbuf.st_mode)){
			printf("-\e[34m[%s]\e[0m\n", entry->d_name);
		}
		else printf("-%s\n", entry->d_name);

		free(path);
	}
	printf("\n");

	closedir(dir);
}

void enterdir(const char *base_path, const int t){ // Open the directory in file manager
	struct stat statbuf;
	if (stat(base_path, &statbuf) != 0){ // Checks whether folder exists
		printf("\e[1;31m[PROJECT MISSING/DOES NOT EXIST]\e[0m\n");
		printf("Could not open the folder. Exiting..\n");
		return;
	}
	//size_t open_size = strlen(base_path) + 9 + 2; // string "xdg-open path"
	//size_t open_size = strlen(base_path) + 10 + 2; // string "cd path && bash"
	//size_t open_size = strlen(base_path) + 35 + 2; // "gnome-terminal --working-directory=path"
	
	int open_size = snprintf(NULL, 0, "gnome-terminal --working-directory=%s", base_path)+1; //snprintf returns number of chars written
	char openfile[open_size];

	if (t==0){
		snprintf(openfile, open_size, "xdg-open %s", base_path);
	}
	else{ 
		//snprintf(openfile, open_size, "cd %s && bash", base_path);
		snprintf(openfile, open_size, "gnome-terminal --working-directory=%s", base_path);
	}
	#if defined(__linux__)
		system(openfile);
	#else
		printf("Unsupported OS.\n");
	#endif
}

int main(int argc, char *argv[]){

	char *path = strdup(proj_path); // path copy on heap
	char *folder_name = strdup(folder_prefix); // folder copy on heap
    char ext[10]="";  // file extension for main


    if (argc<2){
        printf("Invalid Usage. \e[32m[Format: pm <command> <proj_name> <options>]\e[0m\n");
        return 1;
    }

    if (argc>2){
		if (*argv[2] == '\0'){
	    	printf("Project name cannot be empty.\n");
	    	return 1;
		}
		int i=0;
		while (*(argv[2]+i)){
	   		if (!isalnum((unsigned char)*(argv[2]+i)) && (*(argv[2]+i) != '_') && (*(argv[2]+i) != '-')){
			printf("Unsupported characters used in <proj_name>. \e[32m[Use: (a-z), (A-Z), (0-9), (-,_)]\e[0m\n");
			return 1;
	    	}
	    	i++;
		}
		if((folder_name=dynstrcat(folder_name, argv[2]))==NULL){
			free(path);
			return 1;
		}
    }

    if((path=dynstrcat(path, folder_name))==NULL){
    	free(folder_name);
		return 1;
	}

    int opt;
    int opterr = 0;
    int t = 0, l = 0;
    char *lang=NULL;

    while ((opt=getopt(argc-2, &argv[2], "+l:t")) != -1){
		switch(opt){
		    case 'l':
				l = 1;
				lang = malloc((strlen(optarg)+1)*sizeof(char)); 
				strcpy(lang, optarg);
				break;
		    case 't':
				t = 1;
				break;
			case '?':
				break;
		}
    }


    if (strcmp(argv[1],"new")==0 && argc>2){
        if (l){
            if ((strcmp(lang,"c")==0) || (strcmp(lang,"C")==0))
                strcpy(ext, ".c");
            else if ((strcmp(lang,"python")==0) || (strcmp(lang,"Python")==0) || (strcmp(lang, "py")==0) || (strcmp(lang,"Py")==0))
                strcpy(ext, ".py");
            else if ((strcmp(lang,"java")==0) || (strcmp(lang,"Java")==0))
                strcpy(ext, ".java");
            else if ((strcmp(lang,"c++")==0) || (strcmp(lang,"C++")==0) || (strcmp(lang, "cpp")==0) || (strcmp(lang,"Cpp")==0))
                strcpy(ext, ".cpp");
        }
        free(lang);
        return newproj(path, folder_name, ext);
    }
    else if (strcmp(argv[1],"view")==0 && argc>2){
	printf("Accessing contents of %s..\n", folder_name);
	printf("\n\e[1;34m[%s]\e[0m\n\n", folder_name);
	viewdir(path, 0);
    }
    else if (strcmp(argv[1], "kill")==0 && argc>2){
	printf("Searching for %s..\n", folder_name);
	printf("\n\e[1;34m[%s]\e[0m\n\n", folder_name);
	int view = viewdir(path, 0);
	if(!view){
	    printf("The project \"%s\" will be deleted permanently.\n", folder_name);
        printf("\e[1;31mARE YOU SURE?[y/n]\e[0m\n");
        char choice;
        scanf("%c", &choice);
        switch (choice){
			case 'y': case 'Y':
		    	if (removedir(path)==0){
		    	    printf("Project removed successfully.\n");
		    	}
		    	else{
				// Prints error message in readable form
				perror("Error removing directory.");
	        	    }
	 	    	break;
			case 'n': case 'N':
      		    printf("Aborting..\n");
		    	break;
			default:
		    	printf("Invalid input given. Aborting..\n");
	    }
	}
	else printf("Could not access %s. Exiting..\n", folder_name);
    }
    else if (strcmp(argv[1],"list")==0){
	printf("\n\e[1;32m[List of Projects]\e[0m\n\n");
	listdir(proj_path);
    }
    else if (strcmp(argv[1],"enter")==0){
	printf("Opening %s..\n", folder_name);
	enterdir(path, t);
    }
    else{
        printf("Invalid command given. \e[32m[Usage: pm <command> <proj_name> <options>]\e[0m\n  \
The list of commands are:\n  \
    new: Create a new project \e[31m[Requires: proj_name]\e[0m \e[32m[Options: -l <lang>]\e[0m\n  \
    view: View contents of a project \e[31m[Requires: proj_name]\e[0m\n  \
    kill: Delete a project \e[31m[Requires: proj_name]\e[0m\n  \
    list: List all projets\n  \
    enter: Open a project \e[31m[Requires: proj_name]\e[0m \e[32m[Options: -t (Terminal Mode)]\e[0m\n");
    }
    
    free(path);
    free(folder_name);

    return 0;
}
