#include "LeapAsl/Recognizer.h"

#include "LeapAsl/Predictors.h"
#include "LeapAsl/RecordPlayer.h"

#include <LeapSDK/Leap.h>

#include <functional>
#include <map>
#include <memory>

namespace LeapAsl
{

using namespace std;

Recognizer::Recognizer(Predictors::Predictor const& predictor, on_recognition_f&& on_recognition, duration const& hold_duration, duration const& rest_duration)
	: predictor_(predictor)
	, on_recognition_(on_recognition)
	, hold_duration_(hold_duration)
	, rest_duration_(rest_duration)
	, next_sample_()
	, n_frames_analyzed_(0)
{}

Recognizer::~Recognizer()
{}

void Recognizer::onFrame(Leap::Controller const& controller)
{
	analyze(controller.frame(), chrono::high_resolution_clock::now());
}

reference_wrapper<Predictors::Predictor const>& Recognizer::predictor()
{
	return predictor_;
}

void Recognizer::on_frame(RecordPlayer const& record_player)
{
	auto const frame = record_player.frame();
	analyze(frame, time_point{chrono::microseconds(frame.timestamp())});
}

void Recognizer::analyze(Leap::Frame const& frame, time_point const& now)
{
	// If we are not scheduled to take a sample yet, return immediately.
	if(now < next_sample_)
	{
		return;
	}

	// Reject and reset if there is more than one hand in the frame. (A (temporary?) restriction.)
	if(frame.hands().count() != 1)
	{
		anchor_ = fingers_position();
		scores_.clear();
		n_frames_analyzed_ = 0;

		return;
	}

	auto const& hand = frame.hands()[0];

	// Reject and reset if low-confidence.
	if(hand.confidence() < 0.2)
	{
		anchor_ = fingers_position();
		scores_.clear();
		n_frames_analyzed_ = 0;

		return;
	}

	fingers_position fp = to_position(hand);

	// If we have no anchor, the current position becomes the anchor by which we will compare future positions.
	// Else if the current position differs too much from the anchor, it becomes the new anchor.
	if(anchor_ == fingers_position())
	{
		anchor_ = fp;
		anchor_sample_ = now;
	}
	else if(difference(anchor_, fp) > 1000)
	{
		anchor_ = fp;
		anchor_sample_ = now;

		scores_.clear();
		n_frames_analyzed_ = 0;
	}

	// Analyze the one hand present.
	for(auto const& score : predictor_.get().predict(hand))
	{
		double& average = scores_[score.second];
		average = (average * n_frames_analyzed_ + score.first) / (n_frames_analyzed_ + 1);
	}

	// If we have past our \ref hold_duration, report what we have, reset the information and schedule the next sample analysis for \ref rest_duration_ later.
	// Else, just set the next sample analysis for as soon as possible.
	if(now - anchor_sample_ >= hold_duration_)
	{
		// Order gesture names by their scores.
		multimap<double, char> matches;
		for(auto const& score : scores_)
		{
			matches.emplace(score.second, score.first);
		}
		on_recognition_(matches);

		anchor_ = fingers_position();
		scores_.clear();
		n_frames_analyzed_ = 0;

		next_sample_ = now + rest_duration_;
	}
	else
	{
		n_frames_analyzed_ += 1;
		next_sample_ = now;
	}
}

}
