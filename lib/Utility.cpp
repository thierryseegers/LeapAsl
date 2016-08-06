#include "LeapAsl/Utility.h"

#include <LeapSDK/Leap.h>
#include <LeapSDK/LeapMath.h>
#include <LeapSDK/util/LeapUtilGL.h>

#include <algorithm>
#include <iostream>

namespace LeapAsl
{

using namespace std;
    
double distance_squared(Leap::Vector const& first, Leap::Vector const& second)
{
    return pow(first.x - second.x, 2.) + pow(first.y - second.y, 2.) + pow(first.z - second.z, 2.);
}

double difference(fingers_position const& a, fingers_position const& b, double const delta_cap)
{
    double delta = 0.;
    
    for(auto i = 0; i != a.size() && delta < delta_cap; ++i)
    {
        for(auto j = 0; j != a[i].size() && delta < delta_cap; ++j)
        {
            delta += distance_squared(a[i][j], b[i][j]);
        }
    }
    
    return delta;
}

double difference(bones_directions const& a, bones_directions const& b, double const delta_cap)
{
    double delta = 0.;
    
    for(auto i = 0; i != a.size() && delta < delta_cap; ++i)
    {
        delta += a[i].xBasis.angleTo(b[i].xBasis) + a[i].yBasis.angleTo(b[i].yBasis) + a[i].zBasis.angleTo(b[i].zBasis);
    }
    
    return delta;
    
}

fingers_position to_position(Leap::Hand const& hand)
{
    fingers_position fp;
    
    auto const normalized_centered = normalized_hand_transform(hand) * Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -hand.palmPosition()};
    auto const& fingers = hand.fingers();
    
    for(int f = 0; f != 5; ++f)
    {
        auto const& finger = fingers[f];
        
        for(int b = Leap::Bone::TYPE_METACARPAL; b != Leap::Bone::TYPE_DISTAL; ++b)
        {
            auto const& bone = finger.bone((Leap::Bone::Type)b);
            
            if(b == Leap::Bone::TYPE_METACARPAL)
            {
                fp[f][b] = normalized_centered.transformPoint(bone.prevJoint());
                fp[f][b + 1] = normalized_centered.transformPoint(bone.nextJoint());
            }
            else
            {
                fp[f][b + 1] = normalized_centered.transformPoint(bone.nextJoint());
            }
        }
    }
    
    return fp;
}

bones_directions to_directions(Leap::Hand const& hand)
{
    bones_directions bd;
    
    auto const normalized_centered = normalized_hand_transform(hand) * Leap::Matrix{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, -hand.palmPosition()};
    auto const& fingers = hand.fingers();
    
    for(int f = 0; f != 5; ++f)
    {
        auto const& finger = fingers[f];
        
        for(int b = Leap::Bone::TYPE_METACARPAL; b != Leap::Bone::TYPE_DISTAL; ++b)
        {
            auto const& bone = finger.bone((Leap::Bone::Type)b);
            
            bd[f * 5 + b] = normalized_centered * bone.basis();
        }
    }
    
    return bd;
}
    
Leap::Matrix normalized_hand_transform(Leap::Hand const& hand)
{
    Leap::Matrix normalized;
    
    // Make it look like the right hand.
    if(hand.isLeft())
    {
        normalized *= {{-1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    }

    normalized *= {{0, 0, 1}, hand.palmNormal().roll()};    // Negate rotations.
    normalized *= {{0, -1, 0}, hand.direction().yaw()};
    normalized *= {{1, 0, 0}, hand.direction().pitch()};
    
    float const scale_factor = 100. / hand.palmWidth();
    normalized *= {{scale_factor, 0, 0}, {0, scale_factor, 0}, {0, 0, scale_factor}};   // Normalize scale to 10 cm. palm width.
    
    return normalized;
}

void draw_skeleton_hand(Leap::Hand const& hand, Leap::Matrix const& transformation, LeapUtilGL::GLVector4fv const& bone_color, LeapUtilGL::GLVector4fv const& joint_color)
{
    static float const joint_radius_scale = 0.75f;
    static float const bone_radius_scale = 0.5f;
    static float const palm_radius_scale = 1.15f;
    
    Leap::Vector const palm = hand.palmPosition();
    Leap::Vector const palm_direction = hand.direction();
    Leap::Vector const palm_normal = hand.palmNormal();
    Leap::Vector const palm_side = palm_direction.cross(palm_normal).normalized();
    
    float const thumb_distance = hand.fingers()[Leap::Finger::TYPE_THUMB].bone(Leap::Bone::TYPE_METACARPAL).prevJoint().distanceTo(hand.palmPosition());
    Leap::Vector const wrist = palm - thumb_distance * (palm_direction * 0.90f + (hand.isLeft() ? -1.0f : 1.0f) * palm_side * 0.38f);
    
    Leap::FingerList const& fingers = hand.fingers();
    
    float radius = 0.0f;
    Leap::Vector current_box_base;
    Leap::Vector last_box_base = wrist;
    
    for(int i = 0, ei = fingers.count(); i < ei; i++)
    {
        Leap::Finger const& finger = fingers[i];
        radius = finger.width() * 0.5f;
        
        // draw individual fingers
        for(int j = Leap::Bone::TYPE_METACARPAL; j <= Leap::Bone::TYPE_DISTAL; j++)
        {
            Leap::Bone const& bone = finger.bone(static_cast<Leap::Bone::Type>(j));
            
            // don't draw metacarpal, a box around the metacarpals is draw instead.
            if(j == Leap::Bone::TYPE_METACARPAL)
            {
                // cache the top of the metacarpal for the next step in metacarpal box
                current_box_base = bone.nextJoint();
            }
            else
            {
                glColor4fv(bone_color);
                drawCylinder(LeapUtilGL::kStyle_Solid, transformation.transformPoint(bone.prevJoint()), transformation.transformPoint(bone.nextJoint()), bone_radius_scale * radius);
                glColor4fv(joint_color);
                drawSphere(LeapUtilGL::kStyle_Solid, transformation.transformPoint(bone.nextJoint()), joint_radius_scale * radius);
            }
        }
        
        // draw segment of metacarpal box
        glColor4fv(bone_color);
        drawCylinder(LeapUtilGL::kStyle_Solid, transformation.transformPoint(current_box_base), transformation.transformPoint(last_box_base), bone_radius_scale * radius);
        
        glColor4fv(joint_color);
        drawSphere(LeapUtilGL::kStyle_Solid, transformation.transformPoint(current_box_base), joint_radius_scale*radius);
        
        last_box_base = current_box_base;
    }
    
    // close the metacarpal box
    radius = fingers[Leap::Finger::TYPE_THUMB].width() * 0.5f;
    current_box_base = wrist;
    glColor4fv(bone_color);
    drawCylinder(LeapUtilGL::kStyle_Solid, transformation.transformPoint(current_box_base), transformation.transformPoint(last_box_base), bone_radius_scale * radius);
    
    glColor4fv(joint_color);
    drawSphere(LeapUtilGL::kStyle_Solid, transformation.transformPoint(current_box_base), joint_radius_scale * radius);
    
    // draw palm position
    glColor4fv(joint_color);
    drawSphere(LeapUtilGL::kStyle_Solid, transformation.transformPoint(palm), palm_radius_scale * radius);
}

}

namespace std
{
    
std::ostream& operator<<(std::ostream& o, LeapAsl::fingers_position const& positions)
{
    for(auto const& finger : positions)
    {
        for(auto const& joint : finger)
        {
            o << joint.x << '\n' << joint.y << '\n' << joint.z << '\n';
        }
    }
    
    return o;
}

}