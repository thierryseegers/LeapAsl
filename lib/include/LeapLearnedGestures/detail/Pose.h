#pragma once

#include <LeapSDK/Leap.h>

#include <array>
#include <iosfwd>
#include <limits>
#include <string>

namespace LearnedGestures { namespace detail
{
    
class Pose
{
public:
    using joint_position = Leap::Vector;
    using finger_capture = std::array<joint_position, 4>;
    using fingers_capture = std::array<finger_capture, 5>;

    static fingers_capture normalized(Leap::Hand const& hand);
    
    std::string& name();
    std::string const& name() const;
    
    void capture(fingers_capture const& hand);
    
    float match(fingers_capture const& hand, float const error_cap = std::numeric_limits<float>::max()) const;
    
private:
    friend std::ostream& operator<<(std::ostream&, Pose const&);
    friend std::istream& operator>>(std::istream&, Pose&);
    
    std::string name_;
    fingers_capture fingers_capture_;
};

}}