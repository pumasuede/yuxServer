#ifndef __YUX_CONFIG_H__
#define __YUX_CONFIG_H__

#include <fstream>
#include <unordered_map>
#include "Singleton.h"

namespace yux {
namespace base {

// Config define
// LogFile

class Config : public Singleton<Config>
{
    friend class Singleton<Config>;
    public:
        void loadConfigFile(const std::string& file) { file_ = file; load(); }
        bool exist(const std::string& key) { return config_.find(key) != config_.end(); }
        std::string get(const std::string& key, const std::string& defaultValue) { return exist(key) ? config_[key] : defaultValue; }
    private:
        Config() {};
        void load();
        std::string file_;
        std::unordered_map<std::string, std::string> config_;
};

}} // namespace
#endif
