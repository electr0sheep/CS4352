#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void read_sub(char*, char*, int, int, int, int, char*, char*);
int checkName(char*, char*);
int checkTime(int, int, int);
int checkInode(int, int);

// WARNING: I USE SYSTEM() WHICH IS UNSAFE. IF THIS CODE WERE TO GO
// TO PRODUCTION, SYSTEM() CANNOT BE USED.

int main(int argc, char * argv[]){
  char * name = NULL;
  int min = -1;
  int min_option = 0;
  int iNode = -1;
  int delete = 0;
  char * exec = NULL;
  char * newName = NULL;
  // make sure an argument was passed
  if (argc < 2){
    printf("ERROR: program requires at least a single argument\n");
    return -1;
  }
  // make sure first argument is required argument
  if (argv[1][0] == '-'){
    printf("ERROR: provide directory as first argument\n");
    return -1;
  }
  // find optional arguments
  int i;
  if (argc > 2){
    for (i = 2; i < argc; i++){
      if (strcmp(argv[i], "-name") == 0){
        // make sure there is an argument after
        if (argc < i + 2) {
          printf("ERROR: no argument for -name\n");
          return -1;
        }
        name = malloc(sizeof(char) * 2000);
        name = strcpy(name, argv[i+1]);
        i++;
      } else if(strcmp(argv[i], "-mmin") == 0) {
        // make sure there is an argument after
        if (argc < i + 2) {
          printf("ERROR: no argument for -mmin\n");
          return -1;
        }
        if (argv[i+1][0] == '-'){
          min_option = -1;
          min = atoi(argv[i+1]);
          // This will read the number as a negative, which we don't want
          // the following solution may  not be optimal (it definitely isn't)
          // but it was easy.
          min = min * -1;
        } else if (argv[i+1][0] == '+'){
          min_option = 1;
          min = atoi(argv[i+1]);
        } else {
          min_option = 0;
          min = atoi(argv[i+1]);
        }
        i++;
      } else if(strcmp(argv[i], "-inum") == 0) {
        // make sure there is an argument after
        if (argc < i + 2) {
          printf("ERROR: no argument for -inum\n");
          return -1;
        }
        iNode = atoi(argv[i+1]);
        i++;
      } else if(strcmp(argv[i], "-delete") == 0) {
        // exec and delete are mutually exclusive
        if (exec != NULL){
          printf("ERROR: -delete and -exec are mutually exclusive. Please remove one or the other\n");
          return -1;
        }
        delete = 1;
      } else if(strcmp(argv[i], "-exec") == 0) {
        // exec and delete are mutually exclusive
        if (delete){
          printf("ERROR: -delete and -exec are mutually exclusive. Please remove one or the other\n");
          return -1;
        }
        // make sure there are arguments after
        if (argc < i + 4) {
          printf("ERROR: need arguments for -exec\n");
          return -1;
        }
        // if mv, then an extra argument is needed
        if (strcmp(argv[i+1], "mv") == 0) {
          if (argc < i + 5) {
            printf("ERROR: An extra argument is needed for mv\n");
            return -1;
          }
          if (strcmp(argv[i+4], ";") != 0) {
            printf("ERROR: Put \\; at the end for -exec\n");
            return -1;
          }
        } else {
          if (strcmp(argv[i+3], ";") != 0) {
            printf("ERROR: Put \\; at the end for -exec\n");
            return -1;
          }
        }
        // make sure the crap at the end is right
        if (strcmp(argv[i+2], "{}") != 0) {
          printf("ERROR: put '{}' after -exec\n");
          return -1;
        }
        // now we know the arguments are correct
        exec = malloc(sizeof(char) * 2000);
        strcpy(exec, argv[i+1]);
        // NOTE an unneccassary strcmp is USED
        if (strcmp(argv[i+1], "mv") == 0) {
          newName = malloc(sizeof(char) * 2000);
          newName = strcpy(newName, argv[i+3]);
          i = i + 4;
        } else {
          i = i + 3;
        }
      } else {
        printf("ERROR: Unknown argument %s\n", argv[i]);
        return -1;
      }
    }
  }

  // open up stats on the root directory, as we need to possibly check it
  struct stat buf;
  if(stat(argv[1], &buf) != 0){
    printf("Error: can't get stat on %s\n", argv[1]);
    return -1;
  }

  // if we want to do crap to the root directory...
  if (checkName(argv[1], name) && checkTime((int)buf.st_mtime, min, min_option) && checkInode((int)buf.st_ino, iNode)){
    // check if the crap we wanted to do was delete it...
    if (delete){
      // note that remove is after read_sub. you can't remove a non-empty
      // directory, so we first have to empty it.
      read_sub(argv[1], name, min, min_option, iNode, delete, exec, newName);
      remove(argv[1]);
    // or print it.
    } else {
      printf("%s\n", argv[1]);
      read_sub(argv[1], name, min, min_option, iNode, delete, exec, newName);
    }
  // otherwise, just check sub-directories
  } else {
    read_sub(argv[1], name, min, min_option, iNode, delete, exec, newName);
  }
}

/*
*A function that recursively prints all file names
*Input: directory name, i.e., char * sub_dir
*Output: all file names
*/

void read_sub (char* sub_dir, char* name, int time, int time_option, int iNode, int delete, char* exec, char* newName){
  DIR* sub_dp=opendir(sub_dir); //open a directory stream
  struct dirent * sub_dirp; //define
  struct stat buf; //define a file status structure
  char temp1[]=".";
  char temp2[]="..";
  char temp3[]="/";
  // check whether the directory stream is opened successfully
  if (sub_dp != NULL) {
    int x = -1;
    // read one entry each time
    while ((sub_dirp = readdir(sub_dp)) != NULL){
      // print the first entry, a file or a subdirectory
      // check whether the first entry is a subdirectory
      char * temp = sub_dirp->d_name;
      // to avoid recursively searching . and .. in the directory,
      // which would result in endless recursion
      if (strcmp(temp, temp1) != 0 && strcmp(temp, temp2) != 0){

        char *temp_sub= malloc(sizeof(char)*2000);
        temp_sub = strcpy(temp_sub, temp3);
        temp_sub = strcat(temp_sub, temp);
        // now you add the / in front of the entry's name
        char* temp_full_path = malloc(sizeof(char)*2000);
        temp_full_path = strcpy(temp_full_path, sub_dir);
        strcat(temp_full_path, temp_sub);
        // now you get a full path, e.g., testdir/dir1 or testdir/test1
        // lets grab data for it
        if(stat(temp_full_path, &buf) != 0){
          printf("ERROR: can't get stat on %s\n", temp_full_path);
          return;
        }

        // if current directory/file needs to be printed, do it here
        // we don't want to delete a directory prematurely, so we check that
        // later
        if (checkName(temp, name) && checkTime((int)buf.st_mtime, time, time_option) && checkInode((int)buf.st_ino, iNode) && !delete && exec == NULL){
          printf("%s/%s\n", sub_dir, temp);
        }

        //try to open
        DIR* subsubdp = opendir(temp_full_path);
        // if not null, means we find a subdirectory, otherwise, its just a file
        if (subsubdp != NULL){
          // close the stream, because we will reopen it in the recursive call
          closedir(subsubdp);
          read_sub(temp_full_path, name, time, time_option, iNode, delete, exec, newName); // call the recursive function call
        } else {
          // check to see if file should be deleted or if something should be executed
          if (checkName(readdir(opendir(sub_dir))->d_name, name) && checkTime((int)buf.st_mtime, time, time_option) && checkInode((int)buf.st_ino, iNode)){
            if (delete){
              char * file = malloc(sizeof(char) * 2000);
              file = strcpy(file, sub_dir);
              strcat(strcat(file, "/"), temp);
              unlink(file);
              free(file);
            }
            if (exec != NULL){
              char * command = malloc(sizeof(char) * 2000);
              command = strcpy(command, exec);
              command = strcat(command, " ");
              command = strcat(command, sub_dir);
              command = strcat(command, "/");
              command = strcat(command, temp);
              if (newName != NULL){
                command = strcat(command, " ");
                command = strcat(command, sub_dir);
                command = strcat(command, "/");
                command = strcat(command, newName);
              }
              system(command);
              free(command);
            }
          }
        }
        // FREE MEMORY BEFORE IT GOES OUT OF SCOPE
        free(temp_sub);
        free(temp_full_path);
      }

      // possibly delete or execute something on directory after looping through it
      if (checkName(temp, name) && checkTime((int)buf.st_mtime, time, time_option) && checkInode((int)buf.st_ino, iNode)){
        if (delete){
          char * directory = malloc(sizeof(char) * 2000);
          directory = strcpy(directory, sub_dir);
          directory = strcat(directory, "/");
          directory = strcat(directory, temp);
          remove(directory);
          free(directory);
        }
        if (exec != NULL){
          char * command = malloc(sizeof(char) * 2000);
          command = strcpy(command, exec);
          command = strcat(command, " ");
          command = strcat(command, sub_dir);
          command = strcat(command, "/");
          command = strcat(command, temp);
          if (newName != NULL){
            command = strcat(command, " ");
            command = strcat(command, sub_dir);
            command = strcat(command, "/");
            command = strcat(command, newName);
          }
          system(command);
          free(command);
        }
      }
    } // end of while loop
    closedir(sub_dp); // close the stream
  // opendir failed
  } else {
    printf("cannot open directory\n");
    exit(2);
  }
}

// same as strcmp except NULL searchName means automatically true
int checkName(char * fileName, char * searchName){
  if (searchName == NULL){
    return 1;
  }

  if (strcmp(searchName, fileName) == 0){
    return 1;
  }
  return 0;
}

// checks if searchTime is in a range of modifiedTime
int checkTime(int modifiedTime, int searchTime, int option){
  if (searchTime == -1){
    return 1;
  }

  int currentTime = (int)time(NULL);

  if (option == -1){
    return ((currentTime - modifiedTime) < (searchTime * 60));
  } else if (option == 0) {
    return ((currentTime - modifiedTime) < ((searchTime + 1) * 60) && (currentTime - modifiedTime) > ((searchTime - 1) * 60));
  } else if (option == 1) {
    return ((currentTime - modifiedTime) > (searchTime * 60));
  } else {
    return 0;
  }
}

// compares two ints, but searchVal of -1 means auto true
int checkInode(int iNode, int searchVal){
  if (searchVal == -1){
    return 1;
  }

  return (iNode == searchVal);
}
