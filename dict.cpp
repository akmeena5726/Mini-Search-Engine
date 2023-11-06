// Do NOT add any other includes
#include "dict.h"

Dict::Dict()
{
    hash_table_size = 100000;
    hash_table.resize(hash_table_size);
}

Dict::~Dict()
{
    // Implement your function here
}

vector<string> tokenize(string sentence)
{
    vector<string> tokens;
    string separators = " ,.-:!\"\'()?[];@";
    string token = "";
    for (int i = 0; i < sentence.length(); i++)
    {
        if (separators.find(sentence[i]) != string::npos)
        {
            if (token.length() > 0)
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
        {
            char tmp = sentence[i];
            if (tmp >= 'A' && tmp <= 'Z')
            {
                tmp = tolower(tmp);
            }
            token += tmp;
        }
    }
    if (token.length() > 0)
    {
        tokens.push_back(token);
    }
    return tokens;
}

void Dict::insert_sentence(int book_code, int page, int paragraph, int sentence_no, string sentence)
{
    // Implement your function here
    vector<string> tokens = tokenize(sentence);
    for (int i = 0; i < tokens.size(); i++)
    {
        int hash_value = hash_function(tokens[i]);
        bool found = false;
        for (int j = 0; j < hash_table[hash_value].size(); j++)
        {
            if (hash_table[hash_value][j].first == tokens[i])
            {
                hash_table[hash_value][j].second++;
                found = true;
                break;
            }
        }
        if (!found)
        {
            hash_table[hash_value].push_back(make_pair(tokens[i], 1));
        }
    }
}

int Dict::get_word_count(string word)
{
    // Implement your function here
    string wrd = "";
    for (int i = 0; i < word.length(); i++)
    {
        char tmp = word[i];
        if (tmp >= 'A' && tmp <= 'Z')
        {
            tmp = tolower(tmp);
        }
        wrd += tmp;
    }
    
    int hash_value = hash_function(wrd);
    for (int i = 0; i < hash_table[hash_value].size(); i++)
    {
        if (hash_table[hash_value][i].first == wrd)
        {
            return hash_table[hash_value][i].second;
        }
    }
    return -1;
}

void Dict::dump_dictionary(string filename)
{
    // Implement your function here
    ofstream fout(filename);
    for (int i = 0; i < hash_table_size; i++)
    {
        for (int j = 0; j < hash_table[i].size(); j++)
        {
            fout << hash_table[i][j].first << ", " << hash_table[i][j].second << endl;
        }
    }
    fout.close();
}