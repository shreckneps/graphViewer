#include "graphs.h"

//initialize total nodes to zero
int GraphNode::totalNodes = 0;

TraitFrame::TraitFrame() {
    //no special action needed -- current internal containers handle themselves
}

TraitFrame::TraitFrame(TraitFrame &old) {
    traitInts = old.traitInts;
    traitDoubles = old.traitDoubles;
    traitStrings = old.traitStrings;
}

//temporary print function
void TraitFrame::tempPrint() {
    SDL_Log("Ints:");
    for(auto i = traitInts.begin(); i != traitInts.end(); ++i) {
        SDL_Log("\t%s: %d", i->first.c_str(), i->second);
    }
    SDL_Log("Doubles:");
    for(auto i = traitDoubles.begin(); i != traitDoubles.end(); ++i) {
        SDL_Log("\t%s: %lf", i->first.c_str(), i->second);
    }
    SDL_Log("Strings:");
    for(auto i = traitStrings.begin(); i != traitStrings.end(); ++i) {
        SDL_Log("\t%s: %s", i->first.c_str(), i->second.c_str());
    }
}

void TraitFrame::addInt(string label, int value) {
    if(traitDoubles.count(label) || traitStrings.count(label)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Tried to add duplicate trait: %s", label.c_str());
    } else {
        traitInts[label] = value;
    }
}

void TraitFrame::addDouble(string label, double value) {
    if(traitInts.count(label) || traitStrings.count(label)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Tried to add duplicate trait: %s", label.c_str());
    } else {
        traitDoubles[label] = value;
    }
}

void TraitFrame::addString(string label, string value) {
    if(traitInts.count(label) || traitDoubles.count(label)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Tried to add duplicate trait: %s", label.c_str());
    } else {
        traitStrings[label] = value;
    }
}

std::vector<string> TraitFrame::listLabels() {
    std::vector<string> ret;
    for(auto i = traitInts.begin(); i != traitInts.end(); ++i) {
        ret.push_back(i->first);
    }
    for(auto i = traitDoubles.begin(); i != traitDoubles.end(); ++i) {
        ret.push_back(i->first);
    }
    for(auto i = traitStrings.begin(); i != traitStrings.end(); ++i) {
        ret.push_back(i->first);
    }
    return ret;
}

TraitType TraitFrame::lookup(string label, void **ret) {
    if(traitInts.count(label)) {
        //inefficient to count and then retrieve separately -- the internal lookup is done twice
        //hashmaps are efficient, so this is unlikely to matter
        //may cause slowdowns with scripts later
        *ret = &traitInts[label];
        return IntT;
    } else if(traitDoubles.count(label)) {
        *ret = &traitDoubles[label];
        return DoubleT;
    } else if(traitStrings.count(label)) {
        *ret = &traitStrings[label];
        return StringT;
    }
    return NoneT;
}

//function to write the traits to a given file
void TraitFrame::save(std::ofstream &f) {
    for(auto i = traitInts.begin(); i != traitInts.end(); ++i) {
        f << "Int" << std::endl << i->first << std::endl << i->second << std::endl;
    }

    for(auto i = traitDoubles.begin(); i != traitDoubles.end(); ++i) {
        f << "Double" << std::endl << i->first << std::endl << i->second << std::endl;
    }

    for(auto i = traitStrings.begin(); i != traitStrings.end(); ++i) {
        f << "String" << std::endl << i->first << std::endl << i->second << std::endl;
    }
}


GraphNode::GraphNode(double inX, double inY, string inLabel) {
    x = inX;
    y = inY;
    state = NormalS;
    if(inLabel == "") {
        label = "Node " + std::to_string(totalNodes);
    } else {
        label = inLabel;
    }

    traits.addInt("times_clicked", 0);
    traits.addInt("node_id", totalNodes);
    traits.addDouble("value", 0.5);
    traits.addString("type", "A Node");


    totalNodes++;
}

GraphNode::~GraphNode() {
    SDL_Log("Deleted node labeled \"%s\"", label.c_str());
}

int GraphNode::onClick(double inX, double inY) {
    //simple check if within unit circle for now
    //complex draw-shapes later on will require rework for precise behavior
    //in massive graphs, the current query-every-clickable could be an issue
    //revisit in drawing-rework and when redoing ui
    double dx = inX - x;
    double dy = inY - y;
    if((dx * dx) + (dy * dy) < 1) {
        SDL_Log("Node labeled \"%s\" clicked. Traits:", label.c_str());
        traits.tempPrint();
        SDL_Log("Node has %d edges.", edges.size());

        //temporary trait manipulation
        void *p;
        if(traits.lookup("times_clicked", &p) == IntT) {
            int *ip = (int *)p;
            (*ip)++;
        }
        if(traits.lookup("times_clicked", &p) == DoubleT) {
            int *ip = (int *)p;
            (*ip) = -37;
        }
        if(traits.lookup("value", &p) == DoubleT) {
            double *dp = (double *)p;
            (*dp) *= 2;
        }
        if(traits.lookup("type", &p) == StringT) {
            string *sp = (string *)p;
            (*sp) = "Cat Hode";
        }
        
        for(int i = 0; i < edges.size(); i++) {
            SDL_Log("");
            SDL_Log("Edge to \"%s\" has traits:", edges[i]->from(this)->label.c_str());
            edges[i]->traits.tempPrint();
        }


        SDL_Log("");
        SDL_Log("");

        //if shift is pressed, mark the node for deletion, rather than for making a new edge
        if(SDL_GetModState() & KMOD_SHIFT) {
            state = ExpiredS;
            //mark all connected edges for deletion as well
            for(int i = 0; i < edges.size(); i++) {
                edges[i]->cut(this);
            }
        } else {
            state = ActiveS;
        }
        return 1;
    }
    return 0;
}

void GraphNode::draw() {
    //draw octagon inside unit-circle for now
    //support for fancier shapes later, in drawing-rework
    if(state == ExpiredS) {
        return;
    }

    glBegin(GL_LINE_LOOP);
    glColor3d(0, 0, 0);
    glVertex2d(x + 1, y);
    glVertex2d(x + 0.707, y + 0.707);
    glVertex2d(x, y + 1);
    glVertex2d(x - 0.707, y + 0.707);
    glVertex2d(x - 1, y);
    glVertex2d(x - 0.707, y - 0.707);
    glVertex2d(x, y - 1);
    glVertex2d(x + 0.707, y - 0.707);
    glEnd();

    if(state == ActiveS) {
        //indicate active node for forming connections
        glBegin(GL_LINES);
        glVertex2d(x - 0.5, y);
        glVertex2d(x + 0.5, y);
        glVertex2d(x, y - 0.5);
        glVertex2d(x, y + 0.5);
        glEnd();
    }
}


GraphEdge *GraphNode::link(GraphNode *g) {
    return new GraphEdge(this, g);
}

void GraphNode::cut(GraphEdge *source) {
    for(int i = 0; i < edges.size(); i++) {
        if(edges[i] == source) {
            //GraphEdge *temp = edges[i];
            edges[i] = edges.back();
            //edges[edges.size() - 1] = temp;
            edges.pop_back();
            return;
        }
    }
}

GraphEdge::GraphEdge(GraphNode *n1, GraphNode *n2) {
    nodes[0] = n1;
    nodes[1] = n2;
    state = NormalS;
    
    n1->edges.push_back(this);
    n2->edges.push_back(this);
}

GraphEdge::~GraphEdge() {
    SDL_Log("Deleted edge.");
}

int GraphEdge::onClick(double x, double y) {
    return 0;
}

void GraphEdge::draw() {
    //simple line from n[0] to n[1]
    //how to show self-cycles? multiplicity?
    //more complex, let be invisible now, handle in drawing-rework
    if(state != ExpiredS && nodes[0] && nodes[1]) {
        glBegin(GL_LINES);
        glColor3d(0, 0, 0);
        glVertex2d(nodes[0]->x, nodes[0]->y);
        glVertex2d(nodes[1]->x, nodes[1]->y);
        glEnd();
    }
}

GraphNode *GraphEdge::from(GraphNode *source) {
    if(source == nodes[0]) {
        return nodes[1];
    } else if(source == nodes[1]) {
        return nodes[0];
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Tried to traverse an edge from a node the edge does not touch.");
    return NULL;
}

void GraphEdge::cut(GraphNode *source) {
    state = ExpiredS;
    if(source == nodes[0]) {
        //cutting happens when a thing marks itself for deletion
        nodes[0] = NULL;
    } else {
        nodes[0]->cut(this);
    }
    //support for self-cycles -- these two ifs are not mutually exclusive
    //also supports cutting an edge to delete it cleanly without deleting its nodes
    if(source == nodes[1]) {
        nodes[1] = NULL;
    } else {
        nodes[1]->cut(this);
    }
}



