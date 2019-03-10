#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
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
using std::unordered_set;

class Target
{
public:
    Target(const string& name): name(name) { };
    string name;
    vector<string> adjacent;
    vector<string> tasks;

    friend ostream& operator<<(ostream& out, const Target& t);
};

typedef unordered_set<string> StringSet;
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
    if (begin == string::npos)
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
    return !file.bad();
}

void lineError(unsigned int line, const string& msg)
{
    cerr << "Error: " << msg << " [line " << line << "]\n";
}

void targetError(const string& target)
{
    cerr << "Error: processing target [" << target << "]\n";
}

void taskError(const string& task)
{
    cerr << "Error: processing task [" << task << "]\n";
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
            lineError(lineNo, "no target");
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

void topologicalSort
(
    TargetMap& nodes,
    const string& id,
    StringSet& visited,
    vector<string>& order
)
{
    if (visited.find(id) != visited.end())
        return;

    visited.emplace(id);
    for (const string& adj : nodes.at(id).adjacent)
        topologicalSort(nodes, adj, visited, order);
    order.push_back(id);
}

void sortTargets(TargetMap& nodes, vector<string>& order)
{
    StringSet visited;
    for (auto& p : nodes)
        topologicalSort(nodes, p.first, visited, order);
}

bool doTask(string task)
{
    cout << "@" << task << endl;
    char * const argv[] = {
        (char *) "/bin/bash",
        (char *) "-c",
        (char *) task.c_str(),
        NULL
    };

    int status;
    pid_t child = fork();

    if (child == -1) {
        perror("fork");
        return false;
    } else if (!child) {
        execvp("bash", argv);
        perror("execvp");
        exit(1);
    }

    if (wait(&status) == -1) {
        perror("wait");
        return false;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

bool processTarget(Target& tgt)
{
    for (const string& task : tgt.tasks) {
        if (!doTask(task)) {
            taskError(task);
            return false;
        }
    }
    return true;
}

bool processTargets(TargetMap& nodes, vector<string>& order)
{
    for (const string& name : order) {
        if (!processTarget(nodes.at(name))) {
            targetError(name);
            return false;
        }
    }
    return true;
}

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

    vector<string> order;
    sortTargets(nodes, order);

    cout << "[...Target Order...]\n";
    for (auto& name : order)
        cout << name << "\n";

    cout << "[...Processing...]\n";
    return processTargets(nodes, order) ? 0 : 1;
}
