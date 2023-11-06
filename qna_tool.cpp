#include <assert.h>
#include <sstream>
#include "qna_tool.h"

using namespace std;

QNA_tool::QNA_tool()
{
    // Implement your function here
}

QNA_tool::~QNA_tool()
{
    // Implement your function here
}

class paragraphNode
{
public:
    int book_code;
    int page;
    int paragraph;
    int sentence_no;
    int offset;
    // int count;
    double score;
    paragraphNode *next;
    paragraphNode *prev;
    paragraphNode(int book_code, int page, int paragraph, int sentence_no, int offset, double score)
    {
        this->book_code = book_code;
        this->page = page;
        this->paragraph = paragraph;
        this->sentence_no = sentence_no;
        this->offset = offset;
        this->score = score;
        this->next = NULL;
        this->prev = NULL;
    }
};

void QNA_tool::insert_sentence(int book_code, int page, int paragraph, int sentence_no, string sentence)
{
    // Implement your function here
    dict.insert_sentence(book_code, page, paragraph, sentence_no, sentence);
    search.insert_sentence(book_code, page, paragraph, sentence_no, sentence);
    return;
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

// sort the paragraph nodes based on score
void merge_sort(vector<pair<int, paragraphNode *>> &para_score, int l, int r)
{
    if (l >= r)
    {
        return;
    }
    int mid = (l + r) / 2;
    merge_sort(para_score, l, mid);
    merge_sort(para_score, mid + 1, r);
    vector<pair<int, paragraphNode *>> tmp;
    int i = l;
    int j = mid + 1;
    while (i <= mid && j <= r)
    {
        if (para_score[i].first > para_score[j].first)
        {
            tmp.push_back(para_score[i]);
            i++;
        }
        else
        {
            tmp.push_back(para_score[j]);
            j++;
        }
    }
    while (i <= mid)
    {
        tmp.push_back(para_score[i]);
        i++;
    }
    while (j <= r)
    {
        tmp.push_back(para_score[j]);
        j++;
    }
    for (int k = l; k <= r; k++)
    {
        para_score[k] = tmp[k - l];
    }
}

Node *QNA_tool::get_top_k_para(string question, int k)
{
    // Implement your function here

    vector<string> tokens = my_tokenize(question);

    vector<paragraphNode *> para_nodes;

    fstream file;
    file.open("unigram_freq.csv", ios::in);
    string line;
    vector<pair<string, long long>> unigram_freq;
    getline(file, line);
    while (getline(file, line))
    {
        cout << line << endl;
        stringstream ss(line);
        string word;
        getline(ss, word, ',');
        long long freq = stoi(word);
        getline(ss, word, ',');
        unigram_freq.push_back(make_pair(word, freq));
    }

    for (string token : tokens)
    {
        int matches = 0;
        Node *head = search.search(token, matches);
        double score = 0;
        int spec_freq = dict.get_word_count(token);
        int gen_freq = 0;
        for (int i = 0; i < unigram_freq.size(); i++)
        {
            if (unigram_freq[i].first == token)
            {
                gen_freq = unigram_freq[i].second;
                break;
            }
        }
        // will replace this later with double
        score = (spec_freq + 1) / (gen_freq + 1);

        while (head->right != NULL)
        {
            string para = get_paragraph(head->book_code, head->page, head->paragraph);
            vector<string> para_tokens = my_tokenize(para);
            int para_cnt = 0;
            for (string para_token : para_tokens)
            {
                if (para_token == token)
                {
                    para_cnt++;
                }
            }

            // can be implemented better with hash table
            bool flag = false;
            for (int i = 0; i < para_nodes.size(); i++)
            {
                if (
                    (para_nodes[i]->book_code == head->book_code) &&
                    (para_nodes[i]->page == head->page) &&
                    (para_nodes[i]->paragraph == head->paragraph))
                {
                    para_nodes[i]->score += (double)para_cnt * score;
                    flag = true;
                    break;
                }
            }

            if (!flag)
            {
                paragraphNode *new_node = new paragraphNode(head->book_code, head->page, head->paragraph, head->sentence_no, head->offset, score * (double)para_cnt);
                para_nodes.push_back(new_node);
                head = head->right;
            }
        }
    }

    vector<pair<int, paragraphNode *>> para_score;
    for (int i = 0; i < para_nodes.size(); i++)
    {
        para_score.push_back(make_pair(para_nodes[i]->score, para_nodes[i]));
    }

    merge_sort(para_score, 0, para_score.size() - 1); // sort the paragraph nodes based on score (it might be wrong)

    Node *head = new Node();
    Node *tail = new Node();
    head->right = tail;
    tail->left = head;

    int i = para_score.size() - 1;
    int temp = k;

    while (k > 0 && i >= 0)
    {
        Node *new_node = new Node(para_score[i].second->book_code, para_score[i].second->page, para_score[i].second->paragraph, para_score[i].second->sentence_no, para_score[i].second->offset);
        tail->left->right = new_node;
        new_node->left = tail->left;
        new_node->right = tail;
        tail->left = new_node;
        i--;
        k--;
    }

    tail->left->right = NULL;
    tail->left = NULL;
    return head->right;
}

void QNA_tool::query(string question, string filename)
{
    // Implement your function here
    std::cout << "Q: " << question << std::endl;
    std::cout << "A: "
              << "Studying COL106 :)" << std::endl;
    return;
}

std::string QNA_tool::get_paragraph(int book_code, int page, int paragraph)
{

    cout << "Book_code: " << book_code << " Page: " << page << " Paragraph: " << paragraph << endl;

    std::string filename = "mahatma-gandhi-collected-works-volume-";
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