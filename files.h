//defines functions for file access
#ifndef FILES_H
#define FILES_H

#include "graphs.h"

//function to read in a file containing a graph
//returns a vector containing every edge and node
std::vector<Drawable *> loadGraph(string fileName);

//function to save a graph to a file
//writes in a format which loadGraph can read
//  currently label uniqueness is guaranteed only in reading
//  it is technically possible to save a graph which cannot be read.
void saveGraph(std::vector<Drawable *> graph, string fileName);
#endif
