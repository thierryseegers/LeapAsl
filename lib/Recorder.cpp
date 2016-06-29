#include "LeapAsl/Recorder.h"

#include <iostream>

namespace LeapAsl
{
    
void Recorder::onFrame(Leap::Controller const& controller)
{
    auto const serialized_frame = controller.frame().serialize();
    output_stream << serialized_frame.length() << '\n' << serialized_frame << '\n';
}

}