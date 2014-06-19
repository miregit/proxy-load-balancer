#include "proxylb.h"


int read_config(struct node **ptr_head, int *ptr_nr_nodes, char *conf_file, int debug) {

    FILE   *fp;          // declare file pointer 
    int    port, weight;
    char   *host_name; 
    // struct node *ptr;
    int    i=0;
    
    (*ptr_nr_nodes) = 0;
    (*ptr_head)     = NULL;

    fp = fopen ((conf_file==NULL)?"./config.txt":conf_file, "r");  /* open the file for reading */

    if (fp == NULL) {
        debug==1?syslog(LOG_ERR, "can't open input file '%s'!",
            (conf_file==NULL)?"./config.txt":conf_file):NULL;
        exit(1);
    }

    // string host_name is automatically allocated
    while (fscanf(fp, "%as %d %d", &host_name, &port, &weight) != EOF) {
       
        debug==1?syslog(LOG_INFO, "'%s' %d %d", host_name, port, weight):NULL;

        
        for(i=0;i<weight;i++) {
            debug==1?syslog(LOG_INFO, "--- inserting new node for host_name '%s', weight:%d", host_name, weight):NULL;
            insert_node(host_name,port,ptr_nr_nodes,ptr_head,debug);
        }

        
        free((char *) host_name); // free the host_name string

        
    }

    fclose(fp);  


    // debugging
    /* we don't have to debug that hard ;)
    if (debug==1) {
        for(i=0;i<(*ptr_nr_nodes);i++) {
            ptr = get_node(i+1,(*ptr_nr_nodes),(*ptr_head));    
            syslog(LOG_INFO, "number %4d, host_name: '%s', port: '%d'", i+1, ptr->host_name, ptr->port);
        }
    }
    */

    return 1;    
}

