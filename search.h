// Do NOT add any other includes
#include <string>
#include <vector>
#include <iostream>
#include "Node.h"
using namespace std;

class SearchEngine
{
private:
    // You can add attributes/helper functions here
    class sentenceNode
    {
    public:
        int book_code;
        int page;
        int paragraph;
        int sentence_no;
        string sentence;
        sentenceNode(int b_code, int pg, int para, int s_no, string sen)
        {
            book_code = b_code;
            page = pg;
            paragraph = para;
            sentence_no = s_no;
            sentence = sen;
        }
    };

    vector<sentenceNode> sentences;

    int hash_function1(string word)
    {

        int hash = 0;
        int s = word.length();
        for (int i = 0; i < s; i++)
        {
            hash += word[i];
        }
        return hash;
    }

    int hash_function2(string word)
    {
        int hash = 0;
        int s = word.length();
        for (int i = 0; i < s; i++)
        {
            hash += word[i] * (2 * i + 1);
        }
        return hash;
    }

public:
    /* Please do not touch the attributes and
    functions within the guard lines placed below  */
    /* ------------------------------------------- */
    SearchEngine();

    ~SearchEngine();

    void insert_sentence(int book_code, int page, int paragraph, int sentence_no, string sentence);

    Node *search(string pattern, int &n_matches);

    /* -----------------------------------------*/
};