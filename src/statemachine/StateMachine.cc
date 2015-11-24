#include "StateMachine.h"

SMFactory &SMFactory::getInstance()
{
    static SMFactory smFactory_;
    return smFactory_;
}

bool SMFactory::registerSmType(uint32_t machineType, CreateFunc *pf)
{
    smTypeMap_.insert(std::pair<uint32_t, CreateFunc*>(machineType, pf));
    return true;
}

StateMachineBase *SMFactory::createSM(uint32_t type)
{
    if (smTypeMap_.find(type) == smTypeMap_.end())
        return NULL;
    return smTypeMap_[type]();
}
