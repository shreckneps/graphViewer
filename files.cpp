#include "files.h"
#include <fstream>

//used for automatically distributing nodes in a circle
#include <math.h>


std::vector<Drawable *> loadGraph(string fileName) {
    std::unordered_map<string, GraphNode *> nodes;
    std::vector<Drawable *> ret;

    string lines[3] = {""};
    std::ifstream f;
    f.open(fileName);
    if(f.fail()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "loadGraph failed to open file.");
        return ret;
    }
    
    GraphEdge *e = NULL;
    GraphNode *n1 = NULL;
    GraphNode *n2 = NULL;
    TraitFrame *activeTraits = NULL;
    bool action;
    
    //this uses a rotating buffer of the last three lines seen
    //a new node is defined in two lines -- Node, and its label
    //a new edge is defined in three lines -- Edge, and two labels for nodes
    //any trait is defined in three lines -- type, label, value
    while(std::getline(f, lines[2])) {
        //assume an action happens -- set to false if all checks fall through
        action = true;
        
        //string matching here may not be sufficient -- typos and spaces in manually-defined graphs?
        if(lines[2] == "") {
            //blank line -- delineates objects, reset active trait frame
            activeTraits = NULL;
        } else if(lines[1] == "Node") {
            //previous line calls for new node
            if(activeTraits) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                             "Attempt to create a new node before finishing previous object.");
                return ret;
            }
            if(nodes.count(lines[2])) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Duplicate node label: \"%s\"",
                             lines[2].c_str());
                return ret;
            } else {
                //render position nondetermined at this stage
                n1 = new GraphNode(0, 0, lines[2]);
                nodes[lines[2]] = n1;
                ret.push_back(n1);
                activeTraits = &(n1->traits);
            }
        } else if(lines[0] == "Edge") {
            //two lines ago calls for new edge
            if(activeTraits) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                             "Attempt to create a new edge before finishing previous object.");
                return ret;
            }
            if(nodes.count(lines[1])) {
                n1 = nodes[lines[1]];
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Unrecognized node label: \"%s\"", lines[1].c_str());
                n1 = NULL;
            }
            if(nodes.count(lines[2])) {
                n2 = nodes[lines[2]];
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Unrecognized node label: \"%s\"", lines[1].c_str());
                n2 = NULL;
            }
            if(!(n1 && n2)) {
                return ret;
            }

            e = new GraphEdge(n1, n2);
            ret.push_back(e);
            activeTraits = &(e->traits);
        } else if(lines[0] == "Int") {
            if(!activeTraits) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Attempted to create a new Int trait with no active frame.");
            } else {
                //TODO -- error checking in trait value parsing
                activeTraits->addInt(lines[1], stoi(lines[2]));
            }
        } else if(lines[0] == "Double") {
            if(!activeTraits) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Attempted to create a new Double trait with no active frame.");
            } else {
                activeTraits->addDouble(lines[1], stod(lines[2]));
            }
        } else if(lines[0] == "String") {
            if(!activeTraits) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                             "Attempted to create a new String trait with no active frame.");
            } else {
                activeTraits->addString(lines[1], lines[2]);
            }
        } else {
            action = false;
        }

        if(action) {
            lines[0] = "";
            lines[1] = "";
        } else {
            lines[0] = lines[1];
            lines[1] = lines[2];
        }
    }
    f.close();
    

    //space all nodes around a circle
    double radius = nodes.size() / 2;
    double theta = 0;
    double stepAngle = (3.1415 * 2.0) / nodes.size();
    for(auto i = nodes.begin(); i != nodes.end(); ++i) {
        i->second->x = radius * cos(theta);
        i->second->y = radius * sin(theta);
        theta += stepAngle;
    }

    return ret;
}

void saveGraph(std::vector<Drawable *> g, string fileName) {
    std::ofstream f;
    f.open(fileName);
    if(f.fail()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "saveGraph failed to open file.");
        return;
    }

    GraphNode *n;
    GraphEdge *e;
    std::vector<GraphEdge *> edges;
    for(int i = 0; i < g.size(); i++) {
        n = dynamic_cast<GraphNode *>(g[i]);
        e = dynamic_cast<GraphEdge *>(g[i]);
        if(n) {
            f << "Node" << std::endl;
            f << n->label << std::endl;
            n->traits.save(f);
        } else if(e) {
            //Current parsing of graph files does only a single pass
            //an edge cannot be declared before either of its nodes
            //the drawable registry does not provide strict guarantees of order
            //standardizing output files to place edges last guarantees readability of file
            edges.push_back(e);
        } else {
            //this is not necessarily an error.
            //future saving will also support context, like drawing position of nodes
            //future drawables could include annotation objects
            //these would not be part of the logical graph, but saving them may be desirable
            SDL_Log("Non-graph drawable present in saveGraph.");
        }
        f << std::endl;
    }
    for(int i = 0; i < edges.size(); i++) {
        SDL_Log("%d", i);
        e = edges[i];
        f << "Edge" << std::endl;
        f << e->nodes[0]->label << std::endl;
        f << e->nodes[1]->label << std::endl;
        e->traits.save(f);
        f << std::endl;
    }
    f.close();
}

