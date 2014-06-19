#include "proxylb.h"

// inserts node at a start of a list
int insert_node( char *host_name, int port, int *ptr_nr_nodes, struct node **ptr_head, int debug ) {
    struct node *ptr;

    // debug==1?syslog(LOG_INFO,"insert_node got host_name '%s' port '%d'",host_name,port):NULL;

    /* we don't need to debug so hard, do we ? :)

    if (debug == 1) {
        if ((*ptr_head)==NULL) {
           syslog(LOG_INFO,"head is NULL, number of elements is %d",(*ptr_nr_nodes));
        } else {
           syslog(LOG_INFO,"head is NOT null, number of elements is %d",(*ptr_nr_nodes));
        }
    }

    */
 
    ptr = (struct node *) calloc( 1, sizeof(struct node ) );
    if( ptr == NULL ) {                     /* error allocating node?      */
        exit(1);                            /*   exit, else                */
    } else {                                /* allocated node successfully */
       // make host_name string
       ptr->host_name = (char *) calloc(strlen(host_name)+1,sizeof(char));
       strcpy(ptr->host_name,host_name);
       ptr->port   = port;                  /* copy port, set head         */
       ptr->next   = (struct node *) (*ptr_head);
       (*ptr_head) = ptr;
       ++(*ptr_nr_nodes);                   /* ++ number, return true      */ 
   }

   // debug==1?syslog(LOG_INFO,"insert_node end of function"):NULL;
 
   return 1;                         
 
}

// gets node by integer position
// position 1 is the first position
struct node *get_node( int position, int nr_nodes, struct node *head ) {

    struct node *ptr=head;    

    if ( (position>nr_nodes) || (position<1) ) return NULL;
    
    while ( (ptr != NULL) && (--position) ) {
       
        ptr = ptr->next;
        
    }

    return ptr;
}


// destroys list
int kill_list(struct node **ptr_head, int *ptr_nr_nodes, int debug) {

    struct node *ptr=(*ptr_head);
    struct node *tmp;    
    int counter=0;

    
    while ( ptr != NULL ) {
        debug==1?syslog(LOG_INFO,"--- freeing list, pass %d",++counter):NULL;
        tmp = ptr;
        ptr = ptr->next;
        free((char *) tmp->host_name);
        free((struct node *) tmp);
        
    }

    (*ptr_head)     = NULL;
    (*ptr_nr_nodes) = 0;

    return 1;
}


