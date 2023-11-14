// Do NOT add any other includes
#include "search.h"

SearchEngine::SearchEngine()
{
    // Implement your function here
}

SearchEngine::~SearchEngine()
{
    // Implement your function here
}

void SearchEngine::insert_sentence(int book_code, int page, int paragraph, int sentence_no, string sentence)
{
    // Implement your function here
    string temp = "";
    for (int i = 0; i < sentence.length(); i++)
    {
        char t = sentence[i];
        if (t >= 'A' && t <= 'Z')
        {
            t = tolower(t);
        }
        temp += t;
    }
    sentenceNode new_sentence(book_code, page, paragraph, sentence_no, temp);
    sentences.push_back(new_sentence);
}

Node *SearchEngine::search(string pattern, int &n_matches)
{
    string pat = "";
    for (int i = 0; i < pattern.length(); i++)
    {
        char t = pattern[i];
        if (t >= 'A' && t <= 'Z')
        {
            t = tolower(t);
        }
        pat += t;
    }
    int hash_pat1 = hash_function1(pat);
    int hash_pat2 = hash_function2(pat);
    int m = pat.length();

    // Node h = Node();
    // Node t = Node();
    Node *head = new Node();
    Node *tail = new Node();
    head->right = tail;
    tail->left = head;
    head->left = NULL;
    tail->right = NULL;

    string separators = " ,.-:!\"\'()?[];@";

    if (m == 0)
    {
        return head;
    }

    // yaha pe sentances ko direct tokenise karke compare karke linked list mein add kar denge

    for (int j = 0; j < sentences.size(); j++)
    {
        string sentence = sentences[j].sentence;
        int n = sentence.length();
        int hs1 = hash_function1(sentence.substr(0, m));
        int hs2 = hash_function2(sentence.substr(0, m));
        for (int i = 0; i < n - m + 1; i++)
        {
            if (hs1 == hash_pat1 && hs2 == hash_pat2)
            {
                for (int k = 0; k < m; k++)
                {
                    if (k == 0 && separators.find(sentence[i - 1]) == string::npos)
                    {
                        break;
                    }
                    if (sentence[i + k] != pat[k])
                    {
                        break;
                    }
                    if (k == m - 1)
                    {
                        if (separators.find(sentence[i + k + 1]) != string::npos || i + k + 1 == n)
                        {
                            n_matches++;
                            // Node tbi(sentences[j].book_code, sentences[j].page, sentences[j].paragraph, sentences[j].sentence_no, i);
                            Node *new_node = new Node(sentences[j].book_code, sentences[j].page, sentences[j].paragraph, sentences[j].sentence_no, i);
                            tail->left->right = new_node;
                            new_node->left = tail->left;
                            new_node->right = tail;
                            tail->left = new_node;
                        }
                    }
                }
            }
            if (i < n - m)
            {
                hs2 = hs2 + sentence[i] - 2 * hs1 + sentence[i + m] * (2 * m - 1);
                hs1 = hs1 - sentence[i] + sentence[i + m];
            }
        }
    }
    tail->left->right = NULL;
    tail->left = NULL;
    return head->right;
}


// See line no. 63