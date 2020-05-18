#ifndef PARSER_IN_H
#define PARSER_IN_H
#include "darknet.h"
#include "network.h"


typedef struct size_params
{
    int batch;
    int inputs;
    int h;
    int w;
    int c;
    int index;
    int time_steps;
    network *net;
} size_params;

void save_network(network net, char *filename);
void save_weights_double(network net, char *filename);

#endif
