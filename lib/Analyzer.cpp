#include "LeapAsl/Analyzer.h"

#include <lm/model.hh>

#include <atomic>
#include <functional>
#include <numeric>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace LeapAsl
{

using namespace std;

Analyzer::Analyzer(ifstream&& dictionary, string const& language_model_path, on_gesture_f&& on_gesture)
    : dictionary_(move(dictionary))
    , model_(language_model_path.c_str())
    , on_gesture_(on_gesture)
    , reset_(false)
{}
    
void Analyzer::reset()
{
    reset_ = true;
}

void Analyzer::on_recognition(multimap<double, char> const& matches)
{
    // Keep the top N matches and give ourselves a vector of indices.
    size_t const n_top_matches = min((size_t)5, matches.size());
    
    vector<pair<double, char>> top_matches;
    copy_n(matches.rbegin(), n_top_matches, back_inserter(top_matches));

    // Normalize the scores of the top matches where the worse one is 5.0. (Somewhat in line with language model score.)
    double const normalization_factor = 5. * top_matches.back().first;
    transform(top_matches.begin(), top_matches.end(), top_matches.begin(), [&](auto p){ p.first = normalization_factor / p.first; return p; });

    vector<size_t> indices(n_top_matches);
    iota(indices.begin(), indices.end(), 0);
    
    // Clear progress.
    if(reset_.exchange(false))
    {
        dropped_char_indices_.clear();
        sentences_.clear();
    }
    
    // Update sentences.
    if(sentences_.empty())
    {
        for(int i : indices)
        {
			char const c = top_matches[i].second;

            if(c != ' ' && c != '.') // Skip space and punctuation at the begininng of a sentence.
            {
				if(detail::trie const* t = dictionary_.find(c))
				{
					lm::ngram::State out_state;
					auto const score = model_.Score(model_.BeginSentenceState(), model_.GetVocabulary().Index(string(1, c)), out_state);

					sentences_.emplace(combined_score{0, score, (i + 1.) * top_matches[i].first}, sentence_state{string(1, toupper(c)), t, out_state});
				}

				if(detail::trie const* t = dictionary_.find(toupper(c)))
				{
					lm::ngram::State out_state;
					auto const score = model_.Score(model_.BeginSentenceState(), model_.GetVocabulary().Index(string(1, toupper(c))), out_state);

					sentences_.emplace(combined_score{0, score, (i + 1.) * top_matches[i].first}, sentence_state{string(1, toupper(c)), t, out_state});
				}
            }
        }
    }
    else
    {
        sentences_t sentences;
        
        for(auto const& sentence : sentences_)
        {
            auto const last_space_ix = sentence.second.partial.rfind(" ");
            auto const last_word = (last_space_ix == string::npos ? sentence.second.partial : sentence.second.partial.substr(last_space_ix + 1));
            
            for(int i : indices)
            {
                char const c = top_matches[i].second;
                
                if(c == ' ')
                {
                    if(!last_word.empty())  // Do nothing on double spaces.
                    {
                        if(sentence.second.trie->is_word())
                        {
                            lm::ngram::State out_state;
                            auto const score = model_.Score(sentence.second.state, model_.GetVocabulary().Index(last_word), out_state);
                            
                            sentences.emplace(combined_score{sentence.first.complete_words + score, 0., sentence.first.gesture + (i + 1) * top_matches[i].first},
											  sentence_state{sentence.second.partial + c, &dictionary_, out_state});
                        }
                    }
                }
                else if(c == '.')
                {
                    if(!last_word.empty())  // Do nothing on double spaces.
                    {
                        if(sentence.second.trie->is_word())
                        {
                            lm::ngram::State out_state;
                            auto const score = model_.Score(sentence.second.state, model_.GetVocabulary().Index("</s>"), out_state);
                            
                            sentences.emplace(combined_score{sentence.first.complete_words + score, 0., sentence.first.gesture + (i + 1) * top_matches[i].first},
											  sentence_state{sentence.second.partial + c, &dictionary_, out_state});
                        }
                    }
                }
                else
                {
					if(detail::trie const* t = sentence.second.trie->find(c))
					{
						auto const word = last_word + c;

						lm::ngram::State out_state;
						auto const score = model_.Score(sentence.second.state, model_.GetVocabulary().Index(word), out_state);

						sentences.emplace(combined_score{sentence.first.complete_words, score, sentence.first.gesture + (i + 1) * top_matches[i].first},
										  sentence_state{sentence.second.partial + c, t, sentence.second.state});
					}

					if(detail::trie const* t = sentence.second.trie->find(toupper(c)))
					{
						auto const word = last_word + (char)toupper(c);

						lm::ngram::State out_state;
						auto const score = model_.Score(sentence.second.state, model_.GetVocabulary().Index(word), out_state);

						sentences.emplace(combined_score{sentence.first.complete_words, score, sentence.first.gesture + (i + 1) * top_matches[i].first},
										  sentence_state{sentence.second.partial + (char)toupper(c), t, sentence.second.state});
					}
                }
            }
        }
        
        if(sentences.size() == 0)
        {
            // This means none of the top matches lead to valid words. Record this spot as a dropped character.
            dropped_char_indices_.push_front(sentences_.begin()->second.partial.size());
        }
        else
        {
            // Keep the top N results.
            if(sentences.size() > 100)
            {
                sentences.erase(next(sentences.begin(), 100), sentences.end());
            }
            swap(sentences, sentences_);
        }
    }
	
    auto top_sentence = *sentences_.begin();
    for(auto const i : dropped_char_indices_)
    {
        top_sentence.second.partial.insert(i, Analyzer::dropped_symbol);
    }

    on_gesture_(top_matches, top_sentence.second.partial);
}

string const Analyzer::dropped_symbol = "?";

}


