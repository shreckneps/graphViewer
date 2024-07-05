//defines datastructures for graph representation
#ifndef GRAPHS_H
#define GRAPHS_H

#include <unordered_map>
#include <vector>

#include <fstream>

#include "drawing.h"

#include "SDL.h"
#include "SDL2/SDL_opengl.h"

using std::string;


//identifiers for traits' types
enum TraitType { 
    NoneT, 
    IntT,
    DoubleT,
    StringT
};


//box to hold traits which can be associated with a node or edge
//each trait has a string label and a value, which may be one of several types
//these traits are for data in the logical graph being represented
//e.g. an int on an edge might be the length, or the speed limit, of a stretch of road
class TraitFrame {
    public:
        //basic constructor
        TraitFrame();

        //copy existing TraitFrame
        TraitFrame(TraitFrame &old);

        //temporary print function
        void tempPrint();
        
        //insert or update an integer trait
        void addInt(string label, int value);

        //insert or update a floating-point trait
        void addDouble(string label, double value);

        //insert or update a string trait
        void addString(string label, string value);

        //returns the label of every trait in the frame
        std::vector<string> listLabels();
        
        //lookup a trait with a certain label
        //return-value is the identifier for the trait's type
        //if this is not NoneT, (*ret) is set to the address of the trait value
        //this allows both reading and updating of existing traits
        TraitType lookup(string label, void **ret);
        
        //function to write the traits to a given file
        void save(std::ofstream &f);

    private:
        //internal containers for traits
        //implementation may change -- public interface functions should not
        std::unordered_map<string, int> traitInts;
        std::unordered_map<string, double> traitDoubles;
        std::unordered_map<string, string> traitStrings;
};

//pre-declare -- help the compiler make pointers from node to edge
class GraphEdge;

//class representing a node in a graph
class GraphNode : public Drawable {
    public:
        GraphNode(double inX, double inY, string inLabel = "");
        int onClick(double inX, double inY) override;
        void draw() override;

        ~GraphNode();
        
        
        //function that creates a link to a target node g
        //returns pointer to the created edge
        //multiplicity is allowed -- this will always create a new edge, even if one already exists
        //self-cycles are allowed -- this may create a link between a node and itself
        GraphEdge *link(GraphNode *g);

        //function that removes an edge from a node's list
        //this is called when an edge is marked for deletion, to prevent orphaned pointers
        void cut(GraphEdge *source);
        
        //current drawing-position of the node in world-space
        double x, y;
        
        //identifier used for the node
        string label;

        TraitFrame traits;

        static int totalNodes;

        //all edges connected to this node
        std::vector<GraphEdge *> edges;
    private:
};

//class representing an edge in a graph
class GraphEdge : public Drawable {
    public:
        //constructor to create an edge linking two nodes
        //this not only sets the nodes[] member of the edge
        //it additionally adds the new edge to the nodes' list of edges
        GraphEdge(GraphNode *n1, GraphNode *n2);
        ~GraphEdge();
        int onClick(double x, double y) override;
        void draw() override;

        //function that returns the other end of an edge
        GraphNode *from(GraphNode *source);
        //function that marks an edge for deletion
        //the caller is used to know which end of the edge is also being deleted
        //a node which isn't also being deleted must have its list of edges trimmed, to remove this one
        void cut(GraphNode *source);
        
        //deliberate overlap in names/usage between GraphNode::cut() and GraphEdge::cut()
        //naming intuition here is an edge as a string tied between two nodes
        //a cut happens at one end
        //dangling string is not desirable, so the other end is also cut, and the string falls.

        TraitFrame traits;
        GraphNode *nodes[2];
    private:
};


#endif

