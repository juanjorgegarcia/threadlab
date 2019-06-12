#include "raylib.h"
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

int current_bytes = 0;
int current_size_file = 0;
double progress_of_file = 0;
int running = 1;
int number_of_files = 0;
int current_file = 0;
double total_progress = 0;
int err_output_file = 0;

int cp_file(char *input_pathname, char *output_pathname) {
  struct stat st;
  stat(input_pathname, &st);

  int input = open(input_pathname, O_RDONLY, 0600);
  int output = open(output_pathname, O_WRONLY | O_CREAT, 0777);
  current_size_file = st.st_size;

  chmod(output_pathname, st.st_mode);
  printf("This is a file...\n");
  printf("Starting to copy: %s...\n", input_pathname);
  current_file++;
  char c;
  progress_of_file = 0;
  current_bytes = 0;
  if (number_of_files > 0) {
    total_progress = ((current_file * 100) / number_of_files);
  }
  while (read(input, &c, 1) > 0 && running) {

    write(output, &c, 1);
    current_bytes++;

    progress_of_file = ((current_bytes * 100) / current_size_file);
  }

  close(input);
  close(output);

  return (1);
}

int cp_dir(const char *input_path, const char *output_path) {
  struct dirent *de; // Pointer for directory entry

  printf("This is a dir...\n");
  struct stat st, sb;
  stat(input_path, &sb);

  if (stat(output_path, &st) == -1) {
    mkdir(output_path, 0777);
    chmod(output_path, sb.st_mode);
  }

  DIR *dr = opendir(input_path);

  if (dr == NULL) // opendir returns NULL if couldn't open directory
  {
    printf("Could not open current directory");
    return 0;
  }
  struct stat f_sb;

  while ((de = readdir(dr)) != NULL && running) {
    char *i = malloc(sizeof(char) * 100000);
    char *o = malloc(sizeof(char) * 100000);

    if (de->d_type == DT_REG) {
      sprintf(i, "%s/%s", input_path, de->d_name);
      sprintf(o, "%s/%s", output_path, de->d_name);
      int fs = stat(i, &f_sb);

      if (fs == 0 && S_ISREG(f_sb.st_mode)) {
        printf("Copiando arquivo: %s\n", i);
        printf("Output: %s\n", o);
        if (cp_file(i, o)) {
          printf("Success on copying file...\n");
        }
      }
    } else if (de->d_name[0] != '.') {

      sprintf(i, "%s/%s", input_path, de->d_name);
      sprintf(o, "%s/%s", output_path, de->d_name);

      int fs = stat(i, &f_sb);
      if (fs == 0 && S_ISDIR(f_sb.st_mode)) {

        printf("Copiando diretorio: %s\n", i);
        printf("Output: %s\n", o);

        cp_dir(i, o);
      }
    }
    free(i);
    free(o);
  }
  closedir(dr);

  return 1;
}

void cp(char *input_path, char *output_path) {

  struct stat sb, st;
  int f_stat = stat(input_path, &sb);

  if (f_stat == 0 && S_ISDIR(sb.st_mode)) {
    int c = '.';
    char *p = strchr(output_path, c);
    if (p == NULL) {
      int f_out = stat(output_path, &st);
      cp_dir(input_path, output_path);
    } else {
      printf("Destino nao eh uma pasta\n");
      err_output_file = 1;
    }
  } else if (f_stat == 0 && S_ISREG(sb.st_mode)) {
    cp_file(input_path, output_path);
  }
}

int delete_file(char *filepath) {
  int status = remove(filepath);

  if (status == 0) {
    printf("%s file deleted successfully.\n", filepath);
    return 1;
  } else {
    printf("Unable to delete the file\n");
    perror("Following error occurred");
    return 0;
  }
}

int delete_dir(const char *dir_path) {
  struct dirent *de; // Pointer for directory entry

  printf("This is a dir...\n");
  DIR *dr = opendir(dir_path);
  struct stat st, sb;
  stat(dir_path, &sb);

  if (dr == NULL) {
    // opendir returns NULL if couldn't open directory

    printf("Could not open current directory");
    return 0;
  }
  struct stat f_sb;
  while ((de = readdir(dr)) != NULL) {
    char *i = malloc(sizeof(char) * 1024);

    if (de->d_type == DT_REG) {
      sprintf(i, "%s/%s", dir_path, de->d_name);
      int fs = stat(i, &f_sb);

      if (fs == 0 && S_ISREG(f_sb.st_mode)) {
        printf("Deleting file: %s\n", i);
        if (delete_file(i)) {
          printf("Success on deleting file...\n");
        }
      }
    } else if (de->d_name[0] != '.') {

      sprintf(i, "%s/%s", dir_path, de->d_name);

      int fs = stat(i, &f_sb);
      if (fs == 0 && S_ISDIR(f_sb.st_mode)) {
        printf("Deleting dir: %s\n", i);
        delete_dir(i);
      }
    }
    free(i);
  }

  int status = remove(dir_path);

  if (status == 0) {
    printf("%s Dir deleted successfully.\n", dir_path);
    closedir(dr);
    return (1);
  } else {
    printf("Unable to delete the dir: %s\n", dir_path);
    perror("Following error occurred");
    return 0;
  }
}

void cancel_copy(char *output_path) {
  printf("####################################################\n");
  printf("Deleting Files...\n");

  running = 0;
  struct stat sb, st;
  int f_stat = stat(output_path, &sb);

  if (f_stat == 0 && S_ISDIR(sb.st_mode)) {
    delete_dir(output_path);
  } else if (f_stat == 0 && S_ISREG(sb.st_mode)) {
    delete_file(output_path);
  }
}

void count_files(const char *input_path) {
  struct dirent *de; // Pointer for directory entry

  printf("This is a dir...\n");
  struct stat f_sb, sb;
  int f_stat = stat(input_path, &sb);

  if (f_stat == 0 && S_ISREG(sb.st_mode)) {
    number_of_files += 1;
    return;
  }
  DIR *dr = opendir(input_path);

  if (dr == NULL) {
    // opendir returns NULL if couldn't open directory
    printf("Could not open current directory");
  }

  while ((de = readdir(dr)) != NULL && running) {
    char *i = malloc(sizeof(char) * 1024);

    if (de->d_type == DT_REG) {

      sprintf(i, "%s/%s", input_path, de->d_name);
      int fs = stat(i, &f_sb);

      if ((fs == 0) && (S_ISREG(f_sb.st_mode))) {
        number_of_files += 1;
      }
    } else if (de->d_name[0] != '.') {

      sprintf(i, "%s/%s", input_path, de->d_name);

      int fs = stat(i, &f_sb);

      if (fs == 0 && S_ISDIR(f_sb.st_mode)) {
        count_files(i);
      }
    }
    free(i);
  }

  closedir(dr);
}