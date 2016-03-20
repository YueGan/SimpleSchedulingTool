#include "lists.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Prototype for helper function and asprintf.
int asprintf(char **strp, const char *fmt, ...);
Poll *find_pre(char *name, Poll *head);
Poll *delete_helper(Poll *this_poll);

/* Create a poll with this name and num_slots. 
 * Insert it into the list of polls whose head is pointed to by *head_ptr_add
 * Return 0 if successful 1 if a poll by this name already exists in this list.
 */
int insert_new_poll(char *name, int num_slots, Poll **head_ptr_add) {

  // Construct a temprary poll to store given info in a poll
  struct poll *temp = malloc(sizeof(struct poll));
  // Malloc error handling
  if(temp == NULL){
    perror("malloc");
    exit(1);
  }
  // set a poll pointer pointing to head
  struct poll *current = *head_ptr_add;

  // return 1 if poll by the same name is found
  if(find_poll(name, current) != NULL){
    return 1;
  }
  while(1){

    // If first is empty, then add to the head
    if(current == NULL){
      strcpy(temp->name, name);
      temp->num_slots = num_slots;
      *head_ptr_add = temp;
      return 0; 
    }

    // If next is empty, add to next and link current->next to next
    if(current->next == NULL){
      strcpy(temp->name, name);
      temp->num_slots = num_slots;
      current->next = temp;
      return 0; 
    }

    // Iterate to next Poll
    else{
      current = current->next;
    }
  }

  return 0;
}


/* Return a pointer to the poll with this name in
 * this list starting with head. Return NULL if no such poll exists.
 */
Poll *find_poll(char *name, Poll *head) {

  while(head != NULL){

    // Compare if the given name is same as current poll's name
    if(strcmp(head->name, name) == 0){
      return head;
    }

    // Iterate to next head
    head = head->next;
  }

  return NULL;
}

/* 
 *  Print the names of the current polls one per line.
 */
void print_polls(Poll *head) {

  // Print all the poll's name until no more poll follows.
  while(head != NULL){
    printf("%s\n", head->name);
    head = head->next;
  }
}



/* Reset the labels for the poll by this poll_name to the strings in the
 * array slot_labels.
 * Return 1 if poll does not exist by this name. 
 * Return 2 if poll by this name does not match number of labels provided
 */
int configure_poll(char *poll_name, char **slot_labels, int num_labels, 
                   Poll *head_ptr) {

  // Find the inquired poll using find_poll function.
  struct poll *temp = find_poll(poll_name, head_ptr);

  if(temp == NULL){
    return 1;
  }

  if(num_labels != temp->num_slots){
    return 2;
  }


  // Give space to store slot labels
  temp->slot_labels = malloc(num_labels * sizeof(char *));
  if(temp->slot_labels == NULL){
    perror("malloc");
    exit(1);
  }

  int i;
  for(i = 0; i < num_labels; i++){
    // for every slot lable string, give a space and add into temporary poll
    temp->slot_labels[i] = malloc(strlen(slot_labels[i]) * sizeof(char));
    if(temp->slot_labels[i] == NULL){
      perror("malloc");
      exit(1);
    }
    strcpy(temp->slot_labels[i], slot_labels[i]);
  }

  

  return 0;
}


/* Delete the poll by the name poll_name from the list at *head_ptr_add.
 * Update the head of the list as appropriate and free all dynamically 
 * allocated memory no longer used.
 * Return 0 if successful and 1 if poll by this name is not found in list. 
 */
int delete_poll(char *poll_name, Poll **head_ptr_add) {

  if(find_poll(poll_name, *head_ptr_add) == NULL){
    return 1;
  }

  // Find the inquired poll and its previous poll.
  struct poll *pre = find_pre(poll_name, *head_ptr_add);
  struct poll *current = find_poll(poll_name, *head_ptr_add);

  // If previous poll does not exist, it means its at first
  if(current != NULL &&  pre == NULL){
    *head_ptr_add = current->next;
    current = delete_helper(current);
    current->next = NULL;
    current = NULL;
    free(current);
  }

  // If current poll has no next, then it is at last 
  else if(current->next == NULL){
    current = delete_helper(current);
    pre->next = NULL;
    current = NULL;
    free(current);
  }


  // Otherwise, it is in middle 
  else{
    pre->next = current->next;
    current = delete_helper(current);
    current->next = NULL;
    current = NULL;
    free(current);
  }


    return 0;
}

/* A helper function for delete to remove all contents in given Poll
 * including its participants. The original poll is returned as well.
 */
Poll *delete_helper(Poll *this_poll){

  // Check if slot_labels was ever malloced, if so, free all of them.
  if(this_poll->slot_labels != NULL){

    int i;
    for(i = 0; i < this_poll->num_slots; i++){
      this_poll->slot_labels[i] = NULL;
      free(this_poll->slot_labels[i]);
    }
    free(this_poll->slot_labels);
    this_poll->num_slots = 0;
  }


  // Check if the poll has participants, if so, free all of them.
  if(this_poll->participants != NULL){
    struct participant *current = this_poll->participants;
    struct participant *temp;
    while(current != NULL){

      if(current->comment != NULL){
        current->comment = NULL;
        free(current->comment);
      }

      if(current->availability != NULL){
        current->availability = NULL;
        free(current->availability);
      }

      temp = current;
      current = current->next;
      free(temp);
    }
  }
  return this_poll;
}

/* 
 *  Helper function to find the previous poll. Return null if its not found.
 */

Poll *find_pre(char *name, Poll *head){

  // Instead of comparing the current, compare its next and return its current
  while(head->next != NULL){
    
    if(strcmp(head->next->name, name) == 0){

      return head;
    }
    head = head->next;    
  }
  return NULL;
}

/* Add a participant with this part_name to the participant list for the poll
   with this poll_name in the list at head_pt. Duplicate participant names
   are not allowed. Set the availability of this participant to avail.
   Return: 0 on success 
           1 for poll does not exist with this name
           2 for participant by this name already in this poll
           3 for availability string is wrong length for this poll. 
             Particpant not added
*/
int add_participant(char *part_name, char *poll_name, Poll *head_ptr, char* avail) {

  // Construct a temp_poll to find the given poll.
  struct poll *temp_poll = find_poll(poll_name, head_ptr);

  if(temp_poll == NULL){
    return 1;
  }

  if(find_part(part_name, temp_poll) != NULL){
    return 2;
  }


  if(strlen(avail) != temp_poll->num_slots){
    return 3;
  }  


  // Construct a temporary participant to store given info.
  struct participant *temp_part = malloc(sizeof(struct participant));
  if(temp_part == NULL){
    perror("malloc");
    exit(1);
  }

  // Create a participant pointer point to head of participants in poll.
  struct participant *head = temp_poll->participants;

  // Copy its name and availability to temporary.
  strcpy(temp_part->name, part_name);
  temp_part->availability = malloc(strlen(avail) * sizeof(char));
  if(temp_part->availability == NULL){
      perror("malloc");
      exit(1);
  }
  strcpy(temp_part->availability, avail);

  if(head != NULL){

    temp_part->next = head;
    
  }

  // iterate to next participants
  temp_poll->participants = temp_part;
  return 0;
}

/* Add a comment from the participant with this part_name to the poll with
 * this poll_name. Replace existing comment if one exists. 
 * Return values:
 *    0 on success
 *    1 no poll by this name
 *    2 no participant by this name for this poll
 */
int add_comment(char *part_name, char *poll_name, char *comment, Poll *head_ptr) {

  // Construct a temp_poll to find the given poll.
  struct poll *temp_poll = find_poll(poll_name, head_ptr);

  if(temp_poll == NULL){
    return 1;
  }

  struct participant *temp_part = find_part(part_name, temp_poll);
  if(temp_part == NULL){
    return 2;
  }

  if(temp_part->comment == NULL){

    // allocate space to store the comments if it dosent have it already.
    temp_part->comment = malloc(strlen(comment) * sizeof(char));
    if(temp_part->comment == NULL){
      perror("malloc");
      exit(1);
    }

  }
  strcpy(temp_part->comment, comment);

  return 0;
}

/* Add availability for the participant with this part_name to the poll with
 * this poll_name. Return values:
 *    0 success
 *    1 no poll by this name
 *    2 no participant by this name for this poll
 *    3 avail string is incorrect size for this poll 
 */
int update_availability(char *part_name, char *poll_name, char *avail, 
          Poll *head_ptr) {

  // Construct a temp_poll to find the given poll.
  struct poll *temp_poll = find_poll(poll_name, head_ptr);

  if(temp_poll == NULL){
    return 1;
  }

  struct participant *temp_part = find_part(part_name, temp_poll);
  if(temp_part == NULL){
    return 2;
  }

  if(strlen(avail) != temp_poll->num_slots){
    return 3;
  }

  strcpy(temp_part->availability, avail);

    return 0;
}


/*  Return pointer to participant with this name from this poll or
 *  NULL if no such participant exists.
 */
Participant *find_part(char *name, Poll *poll) {
  
  // Construct a head pointer pointing to head participant
  struct participant *head = poll->participants;

  while(head != NULL){

    if(strcmp(head->name, name) == 0){
      return head;
    }
    head = head->next;
  }

  return NULL;

}
    

/* For the poll by the name poll_name from the list at head_ptr,
 * prints the name, number of slots and each label and each participant.
 * For each participant, prints name and availability.
 * Prints the summary of votes returned by the poll_summary function.
 * See assignment handout for formatting example.
 * Return 0 if successful and 1 if poll by this name is not found in list. 
 */
int print_poll_info(char *poll_name, Poll *head_ptr) {

  // Construct a temp_poll to find the given poll.
  struct poll *temp_poll = find_poll(poll_name, head_ptr);

  if(temp_poll == NULL){
    return 1;
  }

  struct participant *temp_part = temp_poll->participants;

  // print participant info by iterating through participant, print commnet if exists
  while(temp_part != NULL){
    printf("Participant: %s\t%s\n", temp_part->name, temp_part->availability);
    if(temp_part->comment != NULL){
      printf("  Comment: %s\n", temp_part->comment);
    }
    temp_part = temp_part->next;
  }


  // print the Availabilty summary by calling poll_summary, free the space after
  printf("\nAvailability Summary\n");
  char * to_print = poll_summary(temp_poll);
  printf("%s\n", to_print);
  to_print = NULL;
  free(to_print);
  return 0;
}


/* Builds and returns a string for this poll that summarizes the answers.
 * Summary information includes the total number of Y,N and M votes
 * for each time slot. See an example in the assignment handout.km
 */
char *poll_summary(Poll *poll) {

  int i;
  int space_needed = 0;

  // Allocate some space for numerical storage.
  int **avail = malloc(sizeof(int*) * poll->num_slots);
  if(avail == NULL){
    perror("malloc");
    exit(1);
  }

  int max_size = 0;
  int line_size = 0;
  struct participant *temp_part;
  char *result = NULL;
  char *temp;

  // iterate through number of slots
  for(i = 0; i < poll->num_slots; i++){

    // give space for each availability, initializing them to be 0.
    avail[i] = malloc(sizeof(int*) * 3);
    avail[i][0] = 0;
    avail[i][1] = 0;
    avail[i][2] = 0;
    if(avail[i] == NULL){
      perror("malloc");
      exit(1);
    }

    // reset to head participants after each loop
    temp_part = poll->participants;

    while(temp_part != NULL){

      // collect the information and add correspondingly
      if(temp_part->availability[i] == 'Y'){
        avail[i][0]++;
      }
      else if(temp_part->availability[i] == 'N'){
        avail[i][1]++;
      }
      else if(temp_part->availability[i] == 'M'){
        avail[i][2]++;
      }

      // asprintf function will return the strlen of the string 
      line_size = asprintf(&temp, "%s\tY:%d N:%d M:%d\n", poll->slot_labels[i], avail[i][0], avail[i][1], avail[i][2]);

      // find a maximum size for later use
      if(line_size > max_size){
        max_size = line_size;
      }

      // sum up the spaces
      space_needed += line_size;
      temp_part = temp_part->next;
    }
    
  }

  // Give space for result string
  result = malloc(space_needed);
  if(result == NULL){
    perror("malloc");
    exit(1);
  }

  // using max size to allocate temprary string for each line.
  temp = malloc(max_size);
  if(temp == NULL){
    perror("malloc");
    exit(1);
  }

  for(i = 0; i < poll->num_slots; i++){

    // recollect the information and assign them to string. Cat string to result.
    asprintf(&temp, "%s\tY:%d N:%d M:%d\n", poll->slot_labels[i], avail[i][0], avail[i][1], avail[i][2]);
    strcat(result, temp);
    // free the storage 
    free(avail[i]);
  }

  // free the storage pointer.
  avail = NULL;
  free(avail);

  // free the temporary string
  free(temp);
  temp = NULL;


  return result;
}
