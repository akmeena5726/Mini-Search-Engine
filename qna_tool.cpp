#include <assert.h>
#include <sstream>
#include "qna_tool.h"

using namespace std;

// Edited by Girvar
long long tot_words = 0;
//

class paragraphNode
{
public:
    int book_code;
    int page;
    int paragraph;
    int sentence_no;
    int offset;
    int tot_count;
    long double score;
    paragraphNode *next;
    paragraphNode *prev;
    paragraphNode(int book_code, int page, int paragraph, int sentence_no, int offset, long double score)
    {
        this->book_code = book_code;
        this->page = page;
        this->paragraph = paragraph;
        this->sentence_no = sentence_no;
        this->offset = offset;
        this->score = score;
        this->next = nullptr;
        this->prev = nullptr;
    }
};


// Edited by Girvar
vector<vector<paragraphNode*>>paras(100000);
// paras.resize(100000); (not working)
//


QNA_tool::QNA_tool()
{
    // Implement your function here
}

QNA_tool::~QNA_tool()
{
    // Implement your function here
}

int powerr(int a, int b)
{
    int tmp = 1;
    while (b > 0)
    {
        tmp *= a;
        b--;
    }
    return tmp;
}


vector<string> my_tokenize(string sentence)
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

int hash_funcc(std::string id)
{
    int n = id.size();
    int tmp = 0;

    for (int i = 0; i < 5; i++)
    {
        int tmp1 = 0;
        for (int j = i; j < n; j += 5)
        {
            tmp1 += id[j];
        }
        tmp += (tmp1 % 10) * powerr(10, i);
    }
    return tmp % 100000;
}

void QNA_tool::insert_sentence(int book_code, int page, int paragraph, int sentence_no, string sentence)
{
    // Implement your function here
    dict.insert_sentence(book_code, page, paragraph, sentence_no, sentence);
    my_search.insert_sentence(book_code, page, paragraph, sentence_no, sentence);

    // Edited by Girvar
    vector<string> tokens = my_tokenize(sentence);
    tot_words += tokens.size();

    string code = to_string(book_code) + to_string(page) + to_string(paragraph);
    int hash_value = hash_funcc(code);
    for (int i = 0 ; i<paras[hash_value].size() ; i++){
        if (paras[hash_value][i]->book_code == book_code && paras[hash_value][i]->page == page && paras[hash_value][i]->paragraph == paragraph){
            paras[hash_value][i]->tot_count += tokens.size();
            return;
        }
    }
    paragraphNode* new_node = new paragraphNode(book_code, page, paragraph, sentence_no, 0, tokens.size());
    paras[hash_value].push_back(new_node);
    //

    return;
}

void merge(vector<pair<long double, paragraphNode*>> &scores, int s, int m, int e)
{
    vector<pair<long double, paragraphNode*>> temp;

    int i, j;
    i = s;
    j = m + 1;

    while( i <= m && j <= e )
    {
        if (scores[i].first > scores[j].first)
        {
            temp.push_back(scores[i]);
            i++;
        }
        else
        {
            temp.push_back(scores[j]);
            j++;
        }
    }

    while (i <= m)
    {
        temp.push_back(scores[i]);
        i++;
    }

    while (j <= e)
    {
        temp.push_back(scores[j]);
        j++;
    }

    for (int i = s; i <= e; i++)
    {
        scores[i] = temp[i - s];
    }
}

void merge_sort_para_score(vector<pair<long double, paragraphNode*>> &scores, int s, int e)
{
    if (s >= e)
    {
        return;
    }

    int m = (s + e) / 2;

    merge_sort_para_score(scores, s, m);
    merge_sort_para_score(scores, m + 1, e);

    merge(scores, s, m, e);
}

Node *QNA_tool::get_top_k_para(string question, int k)
{
    vector<string> tokens = my_tokenize(question);

    // hash table to store the paragraph nodes for fast access
    vector<vector<paragraphNode *>> para_found;
    para_found.resize(100000);


    // reading the unigram_freq.csv file which contains word count for general corpus and
    // storing it in a vector of pairs named unigram_freq
    fstream file;
    file.open("unigram_freq.csv", ios::in);
    string line;
    vector<pair<string, long long>> unigram_freq;
    getline(file, line);
    while (getline(file, line))
    {
        stringstream ss(line);
        string word;
        getline(ss, word, ',');
        string cnt;
        getline(ss, cnt, ',');
        long long freq = stoll(cnt);
        getline(ss, word, ',');
        unigram_freq.push_back(make_pair(word, freq));
    }

    // iterating over the tokens and calculating the scores to all valid paragraphs i.e.
    // paragraphs containing atleast one token
    for (string token : tokens)
    {
        // cout << "token: " << token << endl;
        int matches = 0;
        // will get a linked list of all occurences of the token in the corpus
        Node *head = my_search.search(token, matches);
        long double score = 0;
        // getting the frequency of the token in the our corpus
        long long spec_freq = dict.get_word_count(token);
        // cout << "spec_freq: " << spec_freq << endl;
        long long gen_freq = 0;
        // getting the frequency of the token in the general corpus
        for (int i = 0; i < unigram_freq.size(); i++)
        {
            if (unigram_freq[i].first == token)
            {
                gen_freq = unigram_freq[i].second;
                break;
            }
        }
        // cout << "gen_freq: " << gen_freq << endl;

        score = ((long double)spec_freq + (long double)1)/((long double)gen_freq + (long double)1);
        // cout << fixed << setprecision(8) << "score: " << score << endl;
        // iterating over all occurences of the token in the corpus and updating the scores
        while (head != NULL)
        {
            bool flag = false;

            string code = to_string(head->book_code) + to_string(head->page) + to_string(head->paragraph);
            int hash_value = hash_funcc(code);

            // checking if the paragraph is already present in the hash table
            for (int i = 0; i < para_found[hash_value].size(); i++)
            {
                if (para_found[hash_value][i]->book_code == head->book_code && para_found[hash_value][i]->page == head->page && para_found[hash_value][i]->paragraph == head->paragraph)
                {
                    para_found[hash_value][i]->score += score;
                    flag = true;
                    break;
                }
            }

            // if not present, then create a new node and push it in the hash table
            if (!flag)
            {
                paragraphNode *new_node = new paragraphNode(head->book_code, head->page, head->paragraph, head->sentence_no, head->offset, score);
                string code = to_string(head->book_code) + to_string(head->page) + to_string(head->paragraph);
                int hash_value = hash_funcc(code);
                para_found[hash_value].push_back(new_node);
            }
            head = head->right;
        }
    }

    // merging all the paragraphs into a single vector
    vector<pair<long double, paragraphNode *>> para_score;

    for (int i = 0; i < para_found.size(); i++)
    {
        for (int j = 0; j < para_found[i].size(); j++)
        {
            para_score.push_back(make_pair(para_found[i][j]->score, para_found[i][j]));
        }
    }

    // sorting the paragraphs on the basis of their scores in descending order
    merge_sort_para_score(para_score, 0 , para_score.size()-1);

    Node *head = new Node();
    Node *tail = new Node();
    head->right = tail;
    tail->left = head;

    int i = 0;
    // int temp = k;

    // creating a linked list of top k paragraphs
    while (k > 0 && i < para_score.size())
    {
        // cout << "score: " << para_score[i].first << endl;
        // cout << para_score[i].second->book_code << " " << para_score[i].second->page << " " << para_score[i].second->paragraph << endl;
        // cout << "score: " << para_score[i].first << endl;
        Node *new_node = new Node(para_score[i].second->book_code, para_score[i].second->page, para_score[i].second->paragraph, para_score[i].second->sentence_no, para_score[i].second->offset);
        tail->left->right = new_node;
        new_node->left = tail->left;
        new_node->right = tail;
        tail->left = new_node;
        i++;
        k--;
    }

    tail->left->right = nullptr;
    tail->left = nullptr;
    return head->right;
}

void QNA_tool::query(string question, string filename)
{
    // Implement your function here
    std::cout << "Q: " << question << std::endl;
    std::cout << "A: "
              << "Studying COL106 :)" << std::endl;

    // Edited by Girvar
    // cout<<"yes1"<<endl;
    vector<string> tokens = my_tokenize(question);
    vector<pair<long double, string>> tot_scores;       // will contains total scores of each word
    // cout<<"yes2"<<endl;
    for (int i = 0 ; i<tokens.size() ; i++){
        string token = tokens[i];
        int tot_count = dict.get_word_count(token);
        long double temp_score = (long double)tot_count/(long double)tot_words;
        tot_scores.push_back(make_pair(temp_score, token));
    }
    // cout<<"yes3"<<endl;
    vector<vector<paragraphNode *>> para_found;
    para_found.resize(100000);

    for (string token : tokens)
    {
        // cout<<"yes4"<<endl;
        // cout << "token: " << token << endl;
        int matches = 0;
        // will get a linked list of all occurences of the token in the corpus
        Node *head = my_search.search(token, matches);
        long double score = 0;
        // cout<<"yes5"<<endl;
        // getting the frequency of the token in the our corpus
        long long spec_freq = dict.get_word_count(token);
        // cout << "spec_freq: " << spec_freq << endl;
        long long gen_freq = 0;
        
        // cout << fixed << setprecision(8) << "score: " << score << endl;
        // iterating over all occurences of the token in the corpus and updating the scores
        while (head != NULL)
        {
            bool flag = false;

            string code = to_string(head->book_code) + to_string(head->page) + to_string(head->paragraph);
            int hash_value = hash_funcc(code);

            for (int i = 0 ; i<paras[hash_value].size() ; i++){
                if (paras[hash_value][i]->book_code == head->book_code && paras[hash_value][i]->page == head->page && paras[hash_value][i]->paragraph == head->paragraph){
                    gen_freq = paras[hash_value][i]->tot_count;
                    break;
                }
            }

            int temp_total_count = dict.get_word_count(token);

            score = ((long double)(1))/(((long double)(temp_total_count)));

            // checking if the paragraph is already present in the hash table
            for (int i = 0; i < para_found[hash_value].size(); i++)
            {
                if (para_found[hash_value][i]->book_code == head->book_code && para_found[hash_value][i]->page == head->page && para_found[hash_value][i]->paragraph == head->paragraph)
                {
                    para_found[hash_value][i]->score += score;
                    flag = true;
                    break;
                }
            }

            // if not present, then create a new node and push it in the hash table
            if (!flag)
            {
                paragraphNode *new_node = new paragraphNode(head->book_code, head->page, head->paragraph, head->sentence_no, head->offset, score);
                new_node->tot_count = gen_freq;
                string code = to_string(head->book_code) + to_string(head->page) + to_string(head->paragraph);
                int hash_value = hash_funcc(code);
                para_found[hash_value].push_back(new_node);
            }
            head = head->right;
        }
    }

    // cout<<"yes6"<<endl;
    vector<pair<long double, paragraphNode *>> para_score;

    for (int i = 0; i < para_found.size(); i++)
    {
        for (int j = 0; j < para_found[i].size(); j++)
        {
            para_score.push_back(make_pair(para_found[i][j]->score, para_found[i][j]));
        }
    }
    // cout<<"yes7"<<endl;
    // sorting the paragraphs on the basis of their scores in descending order
    merge_sort_para_score(para_score, 0 , para_score.size()-1);
    int ind = para_score.size()-1;
    int total_words = 0;
    Node* head = new Node();
    Node* tail = new Node();
    head->right = tail;
    tail->left = head;
    int k = 0;
    // cout<<"yes8"<<endl;
    while (ind > 0){
        if ((total_words + para_score[ind].second->tot_count) > 500){
            break;
        }
        else {
            total_words += para_score[ind].second->tot_count;
            cout<<total_words<<endl;
            Node* new_node = new Node(para_score[ind].second->book_code, para_score[ind].second->page, para_score[ind].second->paragraph, para_score[ind].second->sentence_no, para_score[ind].second->offset);
            tail->left->right = new_node;
            new_node->left = tail->left;
            new_node->right = tail;
            tail->left = new_node;
            k++;
            ind--;
        }
    }
    tail->left->right = NULL;
    tail->left = NULL;
    query_llm(filename, head->right, k, "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", question);

    //
    return;
}

std::string QNA_tool::get_paragraph(int book_code, int page, int paragraph)
{

    cout << "Book_code: " << book_code << " Page: " << page << " Paragraph: " << paragraph << endl;

    std::string filename = "txtfiles/mahatma-gandhi-collected-works-volume-";
    filename += to_string(book_code);
    filename += ".txt";

    std::ifstream inputFile(filename);

    std::string tuple;
    std::string sentence;

    if (!inputFile.is_open())
    {
        std::cerr << "Error: Unable to open the input file " << filename << "." << std::endl;
        exit(1);
    }

    std::string res = "";

    while (std::getline(inputFile, tuple, ')') && std::getline(inputFile, sentence))
    {
        // Get a line in the sentence
        tuple += ')';

        int metadata[5];
        std::istringstream iss(tuple);

        // Temporary variables for parsing
        std::string token;

        // Ignore the first character (the opening parenthesis)
        iss.ignore(1);

        // Parse and convert the elements to integers
        int idx = 0;
        while (std::getline(iss, token, ','))
        {
            // Trim leading and trailing white spaces
            size_t start = token.find_first_not_of(" ");
            size_t end = token.find_last_not_of(" ");
            if (start != std::string::npos && end != std::string::npos)
            {
                token = token.substr(start, end - start + 1);
            }

            // Check if the element is a number or a string
            if (token[0] == '\'')
            {
                // Remove the single quotes and convert to integer
                int num = std::stoi(token.substr(1, token.length() - 2));
                metadata[idx] = num;
            }
            else
            {
                // Convert the element to integer
                int num = std::stoi(token);
                metadata[idx] = num;
            }
            idx++;
        }

        if (
            (metadata[0] == book_code) &&
            (metadata[1] == page) &&
            (metadata[2] == paragraph))
        {
            res += sentence;
        }
    }

    inputFile.close();
    return res;
}

void QNA_tool::query_llm(string filename, Node *root, int k, string API_KEY, string question)
{

    // first write the k paragraphs into different files

    Node *traverse = root;
    int num_paragraph = 0;

    while (num_paragraph < k)
    {
        assert(traverse != nullptr);
        string p_file = "paragraph_";
        p_file += to_string(num_paragraph);
        p_file += ".txt";
        // delete the file if it exists
        remove(p_file.c_str());
        ofstream outfile(p_file);
        string paragraph = get_paragraph(traverse->book_code, traverse->page, traverse->paragraph);
        assert(paragraph != "$I$N$V$A$L$I$D$");
        outfile << paragraph;
        outfile.close();
        traverse = traverse->right;
        num_paragraph++;
    }

    // write the query to query.txt
    ofstream outfile("query.txt");
    outfile << "These are the excerpts from Mahatma Gandhi's books.\nOn the basis of this, ";
    outfile << question;
    // You can add anything here - show all your creativity and skills of using ChatGPT
    outfile.close();

    // you do not need to necessarily provide k paragraphs - can configure yourself

    // python3 <filename> API_KEY num_paragraphs query.txt
    string command = "python3 ";
    command += filename;
    command += " ";
    command += API_KEY;
    command += " ";
    command += to_string(k);
    command += " ";
    command += "query.txt";

    system(command.c_str());
    return;
}
