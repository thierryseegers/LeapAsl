#pragma once

#include <lm/model.hh>

#include <atomic>
#include <cmath>
#include <fstream>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace LeapAsl
{

namespace detail
{
// Minimal trie.
// We use it as a special spell-checker that can indicate whether a string is a valid word, an incomplete valid word or neither.
class trie
{
public:
    trie()
    {}
    
    trie(std::string const& dictionary_path)
    {
        // We insert a '\0' to indicate valid words. e.g.:
        // b->\0
        //  ->e->\0
        //     ->a
        //       ->n->\0
        // That way, we know that "be" is a valid word and that "bea" leads to a valid word.
        std::ifstream dictionary_stream{dictionary_path};
        
        std::string word;
        while(std::getline(dictionary_stream, word))
        {
            insert(word.begin(), word.end())->insert('\0');
        }
    }
    
    trie* insert(char const c)
    {
        return &children_[c];
    }
    
	trie* insert(std::string::const_iterator begin, std::string::const_iterator const end)
    {
        trie *p = this;
        
        while(begin != end)
        {
            p = p->insert(*begin);
            ++begin;
        }
        
        return p;
    }

	const trie* find(char const c) const
	{
		auto i = children_.find(c);

		return i == children_.end() ? nullptr : &i->second;
	}

	bool is_word() const
	{
		return children_.find('\0') != children_.end();
	}
    
private:
    std::map<char, trie> children_;
};

}

class Analyzer
{
public:
    using on_gesture_f = std::function<void (std::vector<std::pair<double, char>> const&, std::string const&)>;
    
    Analyzer(std::string const& dictionary_path, std::string const& language_model_path, on_gesture_f&& on_gesture);
    
    void reset();
    
    void on_recognition(std::multimap<double, char> const& matches);
    
private:
    static std::string const dropped_symbol;
    
    detail::trie dictionary_;
    lm::ngram::Model model_;
    on_gesture_f on_gesture_;
    
    std::atomic<bool> reset_;
    
    struct combined_score
    {
        double complete_words, incomplete_word, gesture;
        
        bool operator<(combined_score const& other) const
        {
            return (std::abs(complete_words) + std::abs(incomplete_word) + gesture) < (std::abs(other.complete_words) + std::abs(other.incomplete_word) + other.gesture);
        }
    };

	struct sentence_state
	{
		std::string partial;
		detail::trie const* trie;
		lm::ngram::State state;
	};

    using sentences_t = std::multimap<combined_score, sentence_state>;
    sentences_t sentences_;
    
    std::list<std::string::size_type> dropped_char_indices_;
};

}