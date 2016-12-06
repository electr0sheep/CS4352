#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void read_sub(char*, char*);

int main(int argc, char * argv[]){
  int name_index = -1;
  // make sure an argument was passed
  if (argc < 2){
    printf("ERROR: program requires at least a single argument\n");
    return -1;
  }
  // find optional arguments
  int i;
  for (i = 2; i < argc; i++){
    if (strcmp(argv[i], "-name") == 0){
      name_index = i+1;
      i = i + 1;
    } else {
      printf("ERROR: Unknown argument %s\n", argv[i]);
      return -1;
    }
  }
  if (name_index != -1){
    if (strcmp(argv[1], argv[name_index]) == 0){
      printf("%s\n", argv[1]);
    }
    read_sub(argv[1], argv[name_index]);
  } else {
    printf("%s\n", argv[1]);
    read_sub(argv[1], NULL);
  }
}

/*
*A function that recursively prints all file names
*Input: directory name, i.e., char * sub_dir
*Output: all file names
*/

void read_sub (char* sub_dir, char* name){
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
//      if(stat(sub_dirp->d_name,&buf) == 0)
//        printf("TEST: %d\n", (int)buf.st_size);
      // print the first entry, a file or a subdirectory
      // check whether the first entry is a subdirectory
      char * temp = sub_dirp->d_name;
      // to avoid recursively searching . and .. in the directory
      if (strcmp(temp, temp1) != 0 && strcmp(temp, temp2) != 0){
        if (name != NULL && strcmp(temp, name) == 0){
          printf("%s/%s\n", sub_dir, temp);
        } else if (name == NULL){
          printf("%s/%s\n", sub_dir, temp);
        }


        // NEXT LINE PRODUCES A SHALLOW COPY!!!
        // I fixed this issue, now program works as intended

        
        char *temp_sub= malloc(sizeof(char)*2000);
        temp_sub = strcpy(temp_sub, temp3);
        temp_sub = strcat(temp_sub, temp);
        // now you add the / in front of the entry's name
        char* temp_full_path = malloc(sizeof(char)*2000);
        temp_full_path = strcpy(temp_full_path, sub_dir);
        strcat(temp_full_path, temp_sub);
        // now you get a full path, e.g., testdir/dir1 or testdir/test1
        //try to open
        DIR* subsubdp = opendir(temp_full_path);
        // if not null, means we find a subdirectory, otherwise, its just a file
        if (subsubdp != NULL){
          // close the stream, because we will reopen it in the recursive call
          closedir(subsubdp);
          read_sub(temp_full_path, name); // call the recursive function call
        }
        // FREE MEMORY BEFORE IT GOES OUT OF SCOPE
        free(temp_sub);
        free(temp_full_path);
      }
    } // end of while loop
    closedir(sub_dp); // close the stream
  }
  else {
    printf("cannot open directory\n");
    exit(2);
  }
}

