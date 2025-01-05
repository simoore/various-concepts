#include <bits/stdc++.h>

using namespace std;

string ltrim(const string &);
string rtrim(const string &);
vector<string> split(const string &);

class Node {
public:
    ~Node() {
        for (auto [c, child] : children) {
            delete child;
        }
    }
    
    Node *getChild(char c) const {
        auto it = children.find(c);
        if (it != children.end()) {
            return it->second;
        } 
        return nullptr;
    }
    
    Node *addChild(char c) {
        auto it = children.find(c);
        if (it == children.end()) {
            auto child = new Node();
            children[c] = child;
            return child;
        }
        return it->second;
    }
        
    void addWord(const std::string &gene, int pos, int score, int idx) {
        char key = gene[pos];
        auto child = addChild(key);
        if (gene.size() - pos == 1) {
            child->health.emplace_back(idx, score);
        } else {
            child->addWord(gene, pos + 1, score, idx);
        }
    }

    int64_t getScore(int first, int last) const {
        int64_t intScore = 0;
        for (auto [idx, score] : health) {
            if (idx >= first && idx <= last) {
                intScore += score;
            }
            if (idx == last) {
                break;
            }
        }
        return intScore;
    }
    
    int64_t getScore(int first, int last, const std::string d) const {
        int64_t score = 0;
        for (size_t i = 0; i < d.size(); i++) {
            const Node *node = this;
            size_t j = i;
            while (node != nullptr && j < d.size()) {
                node = node->getChild(d[j]);
                if (node) {
                    score += node->getScore(first, last);
                }
                j++;
            }
        } 
        return score;
    }
    
    static Node createNode(const std::vector<std::string> &genes, const std::vector<int> &health) {
        Node root;
        for (int i = 0; i < genes.size(); i++) {
            root.addWord(genes[i], 0, health[i], i);
        }
        return root;
    }
    
    void printNode(std::string indent, char c = 0) {
        std::cout << indent << c << " ";
        for (auto [idx, score] : health) {
            std::cout << "(" << idx << ", " << score << ")";
        } 
        std::cout << std::endl;
        for (auto [c, child] : children) {
            child->printNode(indent + "    ", c);
        }
    }
    
private:
    std::map<char, Node *> children;
    std::vector<std::pair<int, int>> health;
};

int main()
{
    string n_temp;
    getline(cin, n_temp);

    int n = stoi(ltrim(rtrim(n_temp)));

    string genes_temp_temp;
    getline(cin, genes_temp_temp);

    vector<string> genes_temp = split(rtrim(genes_temp_temp));

    vector<string> genes(n);

    for (int i = 0; i < n; i++) {
        string genes_item = genes_temp[i];
        genes[i] = genes_item;
    }

    string health_temp_temp;
    getline(cin, health_temp_temp);

    vector<string> health_temp = split(rtrim(health_temp_temp));

    vector<int> health(n);
    for (int i = 0; i < n; i++) {
        int health_item = stoi(health_temp[i]);
        health[i] = health_item;
    }

    auto root = Node::createNode(genes, health);

    string s_temp;
    getline(cin, s_temp);

    int s = stoi(ltrim(rtrim(s_temp)));

    int64_t max = 0;
    int64_t min = 0;
    for (int s_itr = 0; s_itr < s; s_itr++) {
        string first_multiple_input_temp;
        getline(cin, first_multiple_input_temp);

        vector<string> first_multiple_input = split(rtrim(first_multiple_input_temp));

        int first = stoi(first_multiple_input[0]);

        int last = stoi(first_multiple_input[1]);

        string d = first_multiple_input[2];
        
        int64_t score = root.getScore(first, last, d);
        
        if (score > max || s_itr == 0) {
            max = score;
        }
        if (score < min || s_itr == 0) {
            min = score;
        }
    }
    
    std::cout << min << " " << max;

    return 0;
}

string ltrim(const string &str) {
    string s(str);

    s.erase(
        s.begin(),
        find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace)))
    );

    return s;
}

string rtrim(const string &str) {
    string s(str);

    s.erase(
        find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(),
        s.end()
    );

    return s;
}

vector<string> split(const string &str) {
    vector<string> tokens;

    string::size_type start = 0;
    string::size_type end = 0;

    while ((end = str.find(" ", start)) != string::npos) {
        tokens.push_back(str.substr(start, end - start));

        start = end + 1;
    }

    tokens.push_back(str.substr(start));

    return tokens;
}
