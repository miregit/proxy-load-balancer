#include "proxylb.h"

// gets random conection
// allocates ip and port
int get_random_connection_params(char **_ip, char **_port, struct node *head, int nr_nodes, int debug ) {
    int rn;
    struct node *ptr;
    int nr_printed;

    // generate random number

    rn = 1+(int) ((double)(nr_nodes)*rand()/(RAND_MAX+1.0));

    // get random connection

    ptr = get_node(rn,nr_nodes,head);

    // modify connection parameters
    
    (*_ip)   = (char *) calloc(strlen(ptr->host_name)+1,sizeof(char));
    strcpy((*_ip),ptr->host_name);    
    (*_port) = (char *) calloc(6,sizeof(char));
    nr_printed = snprintf((*_port),6,"%d",ptr->port);
    (*_port)[5]='\0';
    // debug==1?syslog(LOG_INFO,"Number of characters: %d, port is '%s'",nr_printed,(*_port)):NULL;

    
    // return success
    return 1;
}


