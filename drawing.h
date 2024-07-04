//interface definitions for drawable objects 
#ifndef DRAWING_H
#define DRAWING_H

//identifiers for the state of a drawable object
enum DrawableState {
    ExpiredS,
    ActiveS
};

//Interface representing a drawable, potentially-clickable object
class Drawable {
    public:
        //function for an object to process a click at a given location in world-space coordinates
        //returns nonzero iff the object acted in response to the click
        virtual int onClick(double x, double y) = 0;
        //function to draw an object
        virtual void draw() = 0;
        DrawableState getState();
        //make virtual destructor -- enable polymorphism for destructors in implementation classes
        virtual ~Drawable();
    private:
    protected:
        DrawableState state;
};



#endif
