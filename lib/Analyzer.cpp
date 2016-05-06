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
    
Analyzer::Analyzer(string const& dictionary_path, string const& language_model_path, on_gesture_f&& on_gesture)
    : dictionary_(dictionary_path)
    , model_(language_model_path.c_str())
    , on_gesture_(on_gesture)
    , reset_(false)
{}
    
void Analyzer::reset()
{
    reset_ = true;
}

void Analyzer::on_gesture(multimap<double, string> const& matches)
{
    // Keep the top N matches and give ourselves a vector of indices.
    size_t const n_top_matches = min((size_t)5, matches.size());
    
    vector<pair<double, string>> top_matches;
    copy_n(matches.begin(), n_top_matches, back_inserter(top_matches));
    
    vector<size_t> indices(n_top_matches);
    iota(indices.begin(), indices.end(), 0);
    
    // Clear progress.
    if(reset_.exchange(false))
    {
        cout << "reset\n";
        
        dropped_char_indices_.clear();
        sentences_.clear();
    }
    
    // Update sentences.
    if(sentences_.empty())
    {
        for(int i : indices)
        {
            if(top_matches[i].second != " ") // Skip whitespace at the begininng of a sentence.
            {
                lm::ngram::State out_state;
                auto const score = model_.Score(model_.BeginSentenceState(), model_.GetVocabulary().Index(top_matches[i].second), out_state);
                
                sentences_.emplace(combined_score{0, score, i + 1.}, make_pair(top_matches[i].second, model_.BeginSentenceState()));
            }
        }
    }
    else
    {
        sentences_t sentences;
        
        for(auto const& sentence : sentences_)
        {
            auto const last_space_ix = sentence.second.first.rfind(" ");
            auto const last_word = (last_space_ix == string::npos ? sentence.second.first : sentence.second.first.substr(last_space_ix + 1));
            
            for(int i : indices)
            {
                string const s = top_matches[i].second;
                
                if(s == " ")
                {
                    if(!last_word.empty())  // Do nothing on double spaces.
                    {
                        auto const validity = dictionary_.validate(last_word.begin(), last_word.end());
                        if(validity == detail::trie<char>::correct)
                        {
                            lm::ngram::State out_state;
                            auto const score = model_.Score(sentence.second.second, model_.GetVocabulary().Index(last_word), out_state);
                            
                            sentences.emplace(combined_score{sentence.first.complete_words + score, 0., sentence.first.gesture + (i + 1) * top_matches[i].first},
                                              make_pair(sentence.second.first + s, out_state));
                        }
                    }
                }
                else if(s == ".")
                {
                    if(!last_word.empty())  // Do nothing on double spaces.
                    {
                        auto const validity = dictionary_.validate(last_word.begin(), last_word.end());
                        if(validity == detail::trie<char>::correct)
                        {
                            lm::ngram::State out_state;
                            auto const score = model_.Score(sentence.second.second, model_.GetVocabulary().Index("</s>"), out_state);
                            
                            sentences.emplace(combined_score{sentence.first.complete_words + score, 0., sentence.first.gesture + (i + 1) * top_matches[i].first},
                                              make_pair(sentence.second.first + s, out_state));
                        }
                    }
                }
                else
                {
                    auto const word = last_word + s;
                    
                    auto const validity = dictionary_.validate(word.begin(), word.end());
                    if(validity & detail::trie<char>::valid)
                    {
                        lm::ngram::State out_state;
                        auto const score = model_.Score(sentence.second.second, model_.GetVocabulary().Index(word), out_state);
                        
                        sentences.emplace(combined_score{sentence.first.complete_words, score, sentence.first.gesture + (i + 1) * top_matches[i].first},
                                          make_pair(sentence.second.first + s, sentence.second.second));
                    }
                }
            }
        }
        
        if(sentences.size() == 0)
        {
            // This means none of the top matches lead to valid words. Record this spot as a dropped character.
            dropped_char_indices_.push_front(sentences_.begin()->second.first.size());
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
        top_sentence.second.first.insert(i, Analyzer::dropped_symbol);
    }
    
    for(auto const sentence : sentences_)
    {
        cout << "[{" << sentence.first.complete_words << ", " << sentence.first.incomplete_word << ", " << sentence.first.gesture
        << "}, \"" << sentence.second.first << "\"]" << endl;
    }
    
    on_gesture_(top_matches, top_sentence.second.first);
}

string const Analyzer::dropped_symbol = "?";

}


