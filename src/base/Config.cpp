#include "Config.h"
#include "Utils.h"

using namespace std;

namespace yux {
namespace base {

void Config::load()
{
    fstream fs(file_, std::fstream::in);
    string line;
    int epos, n = 0;
    while (getline(fs, line))
    {
        n++;
        if (line[0] == '#' || line.empty())
            continue;
        epos = line.find('=');
        if (epos == string::npos)
        {
            std::cout<<"Can't find = in line"<<n<<"when loading config file\n";
            return;
        }

        string key = trim(line.substr(0, epos));
        string value = trim(line.substr(epos+1));

        config_[key] = value;
    }
}

}} // namespace
