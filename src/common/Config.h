#ifndef __YUX_CONFIG_H__
#define __YUX_CONFIG_H__

#include <unordered_map>
#include "base/Singleton.h"

namespace yux {
namespace common {

// Config define
class Config : public yux::base::Singleton<Config>
{
    friend class yux::base::Singleton<Config>;
    public:
        void loadConfigFile(const std::string& file) { file_ = file; load(); }
        bool exist(const std::string& key) { return config_.find(key) != config_.end(); }
        std::string get(const std::string& key, const std::string& defaultValue) { return exist(key) ? config_[key] : defaultValue; }
    private:
        Config() {}
        void load();
        std::string file_;
        std::unordered_map<std::string, std::string> config_;
};

}} // namespace
#endif
