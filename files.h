//defines functions for file access
#ifndef FILES_H
#define FILES_H

#include "graphs.h"

//function to read in a file containing a graph
//returns a vector containing every edge and node
std::vector<Drawable *> loadGraph(string fileName);

#endif
