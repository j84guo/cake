#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

using std::endl;
using std::cerr;
using std::cout;
using std::string;
using std::vector;
using std::getline;
using std::ostream;
using std::ifstream;
using std::unordered_map;

/**
 * Target is a DAG node; it has a vertex ID (name), some edges (adjacent) and
 * data (tasks).
 */
class Target
{
    
public:
    Target(const string& name): name(name) { };
    string name;
    vector<string> adjacent;
    vector<string> tasks;

    friend ostream& operator<<(ostream& out, const Target& t);
};

/** for convenience */
typedef unordered_map<string, Target> TargetMap;

ostream& operator<<(ostream& out, const vector<string>& v)
{
    out << "[";
    for (auto it=v.begin(); it!=v.end(); ++it) {
        out << *it;
        if (it + 1 != v.end())
            out << ", ";
    }
    out << "]";
    return out;
}

ostream& operator<<(ostream& out, const Target& t)
{
    out << t.name << ": " << t.tasks << ", " << t.adjacent;
    return out;
}

string trimmed(const string& str, const string& rem=" ")
{
    string::size_type begin = str.find_first_not_of(rem);
    if (begin == std::string::npos)
        return "";
    return str.substr(begin, str.find_last_not_of(rem) - begin + 1);
}

bool readFile(const string& path, vector<string>& lines)
{
    ifstream file(path);
    if (!file)
        return false;

    string line;
    while (getline(file, line))
        lines.push_back(trimmed(line));

    file.close();
    return !file.bad();
}

void errorOnLine(unsigned int line, const string& msg)
{
    cerr << "Error: " << msg << " [line " << line << "]\n";
}

void parseAdjacent(string adj, Target& tgt)
{
    string::size_type pos;
    while ((pos = adj.find(" ")) != string::npos) {
        string token = adj.substr(0, pos);
        adj.erase(0, pos + 1);

        if (token.size())
            tgt.adjacent.push_back(token);
    }
    if (adj.size())
        tgt.adjacent.push_back(adj);
}

/** returns whether to continue parsing current Target */
bool parseTask(const string& t, vector<string>& tasks)
{
    if (t.find_first_not_of(' ') == string::npos)
        return true;
    if (t.at(0) != '\t')
        return false;

    tasks.push_back(t.substr(1));
    return true;
}

bool parseTargets(TargetMap& nodes, vector<string>& lines)
{
    unsigned int lineNo = 1;
    auto it = lines.begin();

    while (it != lines.end()) {
        if (it->find_first_not_of(' ') == string::npos) {
            ++it; ++lineNo;
            continue;
        }

        auto pos = it->find(":");
        if ((pos == string::npos) || !pos) {
            errorOnLine(lineNo, "no target");
            return false;
        }

        string name = it->substr(0, pos);
        nodes.emplace(name, name);
        parseAdjacent(it->substr(pos + 1), nodes.at(name));

        while (++it != lines.end() && ++lineNo) {
            if (!parseTask(*it, nodes.at(name).tasks))
                break;
        }
    }

    return true;
}

/** topological sort */
void sortTargets(TargetMap& nodes, vector<string>& order)
{
    
}

/**
 * Todo: build graph, topological sort, execute tasks
 */
int main()
{
    vector<string> lines;
    if (!readFile("Cakefile", lines)) {
        perror("readFile");
        return 1;
    }

    TargetMap nodes;
    if (!parseTargets(nodes, lines))
        return 1;

    for (auto& p : nodes)
        cout << p.second << "\n";

    vector<string> order;
    sortTargets(nodes, order);
}
