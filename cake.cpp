#include <string>
#include <vector>
#include <fstream>
#include <iostream>

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

class Target
{
    
public:
    Target(const string& name);
    string name;
    vector<string> tasks;

    friend ostream& operator<<(ostream& out, const Target& t);
};

Target::Target(const string& name):
    name(name)
{ }

ostream& operator<<(ostream& out, const Target& t)
{
    out << t.name << ": [";
    for (auto it=t.tasks.begin(); it!=t.tasks.end(); ++it) {
        out << *it;
        if (it + 1 != t.tasks.end())
            out << ", ";
    }
    out << "]";
}

bool readFile(const string& path, vector<string>& lines)
{
    ifstream file(path);
    if (!file)
        return false;

    string line;
    while (getline(file, line))
        lines.push_back(line);

    file.close();
    return !file.bad();
}

void errorOnLine(int line, const string& msg)
{
    cerr << "Error: " << msg << " [line " << line << "]\n";
}

bool parseTargets(vector<Target>& targets, vector<string>& lines)
{
    int lineNo=1;
    auto it = lines.begin();

    while (it != lines.end()) {
        if (!it->size())
            continue;
        auto pos = it->find(":");
        if ((pos == string::npos) || !pos) {
            errorOnLine(lineNo, "no target");
            return false;
        }

        targets.emplace_back(it->substr(0, pos));
        while (++it != lines.end()) {
            if (!it->size())
                continue;
            if (it->at(0) != '\t')
                break;
            targets.back()
                .tasks
                .push_back(it->substr(1));
            ++lineNo;
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

    vector<Target> targets;
    if (!parseTargets(targets, lines))
        return 1;

    for (auto& t : targets)
        cout << t << "\n";
}
