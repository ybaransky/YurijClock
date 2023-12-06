#pragma

#include <TM1637Display.h>

class Display {

    TM1637Display* _display[3]; // left most is the most significant

    public:
        void    init();

};