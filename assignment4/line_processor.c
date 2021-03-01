#include "line_processor.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

// GLOBAL VARIABLES
// Input thread to space thread
char input_buffer[NUM_LINES][LINE_SIZE]; // 1000 character limit for each line, 49 lines maximum
int ib_p_idx = 0; // index to put next line in
int ib_c_idx = 0; // index to take next line from
int ib_count = 0;
pthread_mutex_t ib_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ib_full = PTHREAD_COND_INITIALIZER;

// Space thread to plus thread
char space_buffer[NUM_LINES][LINE_SIZE];
int sb_p_idx = 0;
int sb_c_idx = 0;
int sb_count = 0;
pthread_mutex_t sb_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sb_full = PTHREAD_COND_INITIALIZER;

// Plus thread to output thread
char output_buffer[OUTPUT_SIZE];
int ob_idx = 0;
int ob_count = 0;
bool should_exit = false;
pthread_mutex_t ob_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ob_full = PTHREAD_COND_INITIALIZER;


// These functions are mostly the same as the sample code that was given for the assignment
void get_ib(char copy_to[LINE_SIZE]) {
   // lock mutex before accessing data in buffer
   pthread_mutex_lock(&ib_mutex);
   while (ib_count == 0) {
      // empty, wait for stuff
      pthread_cond_wait(&ib_full, &ib_mutex);
   }

   char *line = input_buffer[ib_c_idx];
   // Increment consumer index and decrement the count
   ++ib_c_idx;
   --ib_count;

   // Copy the line to the buffer
   strncpy(copy_to, line, LINE_SIZE);
   // unlock mutex, done accessing data
   pthread_mutex_unlock(&ib_mutex);
}

void put_ib(char copy_from[LINE_SIZE]) {
   // lock mutex before accessing data in buffer
   pthread_mutex_lock(&ib_mutex);
   // copy the string to the buffer
   strncpy(input_buffer[ib_p_idx], copy_from, LINE_SIZE);
   // update producer index
   ++ib_p_idx;
   ++ib_count;
   // Signal to consumer that the buffer now contains data
   pthread_cond_signal(&ib_full);
   // Unlock mutex
   pthread_mutex_unlock(&ib_mutex);
}

void get_sb(char copy_to[LINE_SIZE]) {
   // lock mutex before accessing data in buffer
   pthread_mutex_lock(&sb_mutex);
   while (sb_count == 0) {
      // empty, wait for stuff
      pthread_cond_wait(&sb_full, &sb_mutex);
   }

   char *line = input_buffer[sb_c_idx];
   // Increment consumer index and decrement the count
   ++sb_c_idx;
   --sb_count;

   // Copy the line to the buffer
   strncpy(copy_to, line, LINE_SIZE);
   // unlock mutex, done accessing data
   pthread_mutex_unlock(&sb_mutex);
}

void put_sb(char copy_from[LINE_SIZE]) {
   // lock mutex before accessing data in buffer
   pthread_mutex_lock(&sb_mutex);
   // copy the string to the buffer
   strncpy(input_buffer[sb_p_idx], copy_from, LINE_SIZE);
   // update producer index
   ++sb_p_idx;
   ++sb_count;
   // Signal to consumer that the buffer now contains data
   pthread_cond_signal(&sb_full);
   // Unlock mutex
   pthread_mutex_unlock(&sb_mutex);
}

// Returns a pointer to the position in the string at which the buffer got full, or null if everything was copied succesfully
char* put_ob(char *copy_from) {
   // lock mutex before accessing data in buffer
   pthread_mutex_lock(&ob_mutex);
   // concatenate as many bytes as possible from copy_from to the output buffer
   while (ob_idx < OUTPUT_SIZE - 1 && *copy_from != '\0') {
      output_buffer[ob_idx] = *copy_from;
      ++ob_idx;
      ++copy_from;
   }
   // terminate the string
   output_buffer[ob_idx] = '\0';

   // Notify the output thread if the buffer is full
   if (ob_idx == OUTPUT_SIZE - 1) {
      pthread_cond_signal(&ob_full);

      // If there is still something left to be copied, return the address of the start of the leftover data
      if (*copy_from != '\0') {
	 pthread_mutex_unlock(&ob_mutex);
	 return copy_from;
      }
   }
   pthread_mutex_unlock(&ob_mutex);

   // Everything was copied successfully, return NULL.
   return NULL;
}

int get_ob(char copy_to[OUTPUT_SIZE]) {
   // Lock the mutex
   pthread_mutex_lock(&ob_mutex);
   // only get output when the buffer is full
   while (ob_idx != OUTPUT_SIZE - 1 && !should_exit) {
      pthread_cond_wait(&ob_full, &ob_mutex);
   }
   // Check to see if this thread should exit
   if (should_exit) {
      return -1;
   }
   // extract data from buffer
   strncpy(copy_to, output_buffer, OUTPUT_SIZE);
   // reset the buffer
   ob_idx = 0;
   output_buffer[0] = '\0';
   // unlock mutex
   pthread_mutex_unlock(&ob_mutex);
   return 0;
}


void* get_input(void *args) {
   char line[LINE_SIZE]; // Buffer to store line in until we can get a lock on input_buffer

   while (1) {
      // Make sure there isn't a buffer overflow issue :)
      if (ib_p_idx >= NUM_LINES - 1) {
	 strncpy(line, "STOP\n", LINE_SIZE);
      }
      else {
	 fgets(line, LINE_SIZE, stdin);
      }
      // put the line in the input buffer
      put_ib(line);

      // Check if this is the stop processing line. If it is, this thread should exit.
      if (strcmp(line, "STOP\n") == 0) {
	 return NULL;
      }
   }
}

void* replace_newlines(void *args) {
   char line[LINE_SIZE];

   while (1) {
      get_ib(line);
      
      if (strcmp(line, "STOP\n") != 0) {
	 // Do the actual processing
	 for (int i = 0; i < strlen(line); ++i) {
	    if (line[i] == '\n') {
	       line[i] = ' ';
	    }
	 }
	 put_sb(line);
      }
      else {
	 // If we receive the stop processing line, this thread can exit after putting it in the buffer
	 put_sb(line);
	 return NULL;
      }
   }
}

void* replace_plus(void *args) {
   char line[LINE_SIZE];

   while (1) {
      get_sb(line);
      
      if (strcmp(line, "STOP\n") != 0) {
	 // Do the actual processing
	 int j = 0;
	 for (int i = 0; i < strlen(line); ++i) {
	    if (line[i] == '+' && line[i + 1] == '+') {
	       line[j] = '^';
	       ++i;
	    }
	    else {
	       line[j] = line[i];
	    }
	    ++j;
	 }
	 line[j] = '\0';

	 // keep filling the output buffer as long as there is more stuff to put in it
	 char* current_position = line;
	 do {
	    current_position = put_ob(current_position);
	 } while (current_position != NULL);
      }
      else {
	 // If we receive the stop processing line, wait for the output thread to finish writing, then stop it.
	 while (1) {
	    pthread_mutex_lock(&ob_mutex);
	    // make sure that the output buffer doesn't have another line that it needs to output
	    if (ob_idx != OUTPUT_SIZE - 1) {
	       // set should_exit flag and signal output thread
	       should_exit = true;
	       pthread_cond_signal(&ob_full);
	       pthread_mutex_unlock(&ob_mutex);
	       break;
	    }
	    pthread_mutex_unlock(&ob_mutex);
	 }
	 return NULL;
      }
   }
}

void* output_stuff(void *args) {
   char line[OUTPUT_SIZE];

   while (get_ob(line) != -1) {
      printf("%s\n", line);
   }
   return NULL;
}

void run_line_processor() {
   // Create the threads
   pthread_t input_thread, space_thread, plus_thread, output_thread;

   pthread_create(&input_thread, NULL, get_input, NULL);
   pthread_create(&space_thread, NULL, replace_newlines, NULL);
   pthread_create(&plus_thread, NULL, replace_plus, NULL);
   pthread_create(&output_thread, NULL, output_stuff, NULL);

   // Wait for threads to terminate
   pthread_join(input_thread, NULL);
   pthread_join(space_thread, NULL);
   pthread_join(plus_thread, NULL);
   pthread_join(output_thread, NULL);
}
