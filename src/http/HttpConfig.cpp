#include <fstream>
#include <sstream>
#include <vector>

#include "HttpConfig.h"
#include "base/Log.h"
#include "common/Utils.h"

using namespace std;
using namespace yux::base;
using namespace yux::common;

namespace yux {
namespace http {

void MimeConfig::load()
{
    fstream fs(file_, std::fstream::in);
    string line;
    int epos, n = 0;
    while (getline(fs, line))
    {
        n++;
        if (line == "")
        {
            continue;
        }

        if (line.back() != ';')
        {
            log_error("syntax error in %s ", file_.c_str());
        }

        line.pop_back();

        stringstream tokenStream(line);
        vector<string> tokens;
        string token;

        while (tokenStream>>token)
        {
            tokens.push_back(token);
        }

        if (tokens.size() < 2 )
        {
            log_error("syntax error in %s ", file_.c_str());
            return;
        }

        for (int i=1; i<tokens.size(); i++)
        {
            //log_trace("set mime map %s:%s", tokens[i].c_str(), tokens[0].c_str());
            config_[tokens[i]] = tokens[0];
        }
    }
}

}} // namespace
