#include "levhenstein_distance.h"

#include "LeapAsl/LeapAsl.h"

#include <chrono>
#include <fstream>
#include <string>
#include <thread>

using namespace std;

int main()
{
    string last_top_sentence;
    size_t cumulative_distance = 0;
    auto const on_gesture = [&](vector<pair<double, char>> const&, string const& top_sentence)
    {
        cumulative_distance += levenshtein_distance(last_top_sentence, top_sentence);
        last_top_sentence = top_sentence;
    };
    
    LeapAsl::Analyzer analyzer("aspell_en_expanded", "romeo_and_juliet_corpus.mmap", on_gesture);

    ifstream lexicon_data_istream("lexicon", ios::binary);
    if(!lexicon_data_istream)
    {
        lexicon_data_istream.open("lexicon.sample", ios::binary);
        if(!lexicon_data_istream)
        {
            return EXIT_FAILURE;
        }
    }
    
    LeapAsl::RecordPlayer record_player(cin);
    
    LeapAsl::Recognizer<LeapAsl::Predictors::Lexicon> recognizer(forward<ifstream>(lexicon_data_istream), bind(&LeapAsl::Analyzer::on_recognition, ref(analyzer), placeholders::_1));
    record_player.add_listener(recognizer);
    
    record_player.read();
    
    cout << last_top_sentence << '\n' << cumulative_distance << '\n';
    
    return EXIT_SUCCESS;
}
