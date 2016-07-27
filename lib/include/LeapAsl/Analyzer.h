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
template<typename T>
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
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
            insert(word.begin(), word.end())->insert('\0');
        }
    }
    
    trie<T>* insert(T const& t)
    {
        return &children_[t];
    }
    
    template<typename I>
    trie<T>* insert(I begin, I end)
    {
        trie<T> *p = this;
        
        while(begin != end)
        {
            p = p->insert(*begin);
            ++begin;
        }
        
        return p;
    }
    
    const trie<T>* find(T const& t) const
    {
        auto i = children_.find(t);
        
        return i == children_.end() ? nullptr : &i->second;
    }
    
    size_t size() const
    {
        return children_.size();
    }
    
    size_t count(T const& t) const
    {
        return children_.count(t);
    }
    
    enum validity
    {
        invalid = 0x0,
        valid = 0x1,
        correct = 0x3
    };
    
    template<typename I>
    validity validate(I begin, I end) const
    {
        const trie<T> *p = this;
        
        while(p && begin != end)
        {
            p = p->find(*begin);
            ++begin;
        }
        
        if(p)
        {
            if(!p->count('\0'))
            {
                return valid;
            }
            else if(p->size() > 1)
            {
                return (validity)(valid | correct);
            }
            else
            {
                return correct;
            }
        }
        
        return invalid;
    }
    
    template<typename R>
    validity validate(R const& r) const
    {
        return validate(std::begin(r), std::end(r));
    }
    
private:
    std::map<T, trie<T>> children_;
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
    
    detail::trie<char> dictionary_;
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
    
    using sentences_t = std::multimap<combined_score, std::pair<std::string, lm::ngram::State>>;
    sentences_t sentences_;
    
    std::list<std::string::size_type> dropped_char_indices_;
};

}