#include <iostream>
#include <string>
#include <map>
#include <memory>

#include "common/Event.h"
#include "base/Socket.h"
#include "base/Singleton.h"

#ifndef STATTE_MACHINE_HH_
#define STATTE_MACHINE_HH_

using namespace yux::base;

class StateMachineBase
{
    public:
        StateMachineBase() { state_ = -1; }
        virtual ~StateMachineBase() { }

        virtual const std::string& Name() { return name_; }
        virtual uint32_t MachineType() = 0;
        void setName(const std::string& name) { name_ = name; }
        void setId(uint32_t id) { id_ = id; }
        void setType(uint16_t type) { type_=type; }
        virtual void Drive(const Event& event, SocketBase* sock) = 0;

    protected:
        std::string    name_;
        uint32_t       id_;
        uint16_t       type_;
        int32_t        state_;
    private:
        StateMachineBase(const StateMachineBase&);
        StateMachineBase& operator=(const StateMachineBase&);
};

template<class T>
class StateMachine : public StateMachineBase
{
    public :
        typedef int (T::*pStCb) (const Event &event, SocketBase* sock);
        typedef std::map<int32_t, pStCb> CbList;

        //register global state function
        static bool  registerCb(int32_t stateId, pStCb pf) { getCbList()[stateId] = pf; return true;}
        pStCb getCb(int32_t stateId) { return getCbList()[stateId]; }
        void Drive(const Event& event, SocketBase* sock);

    private:
        static CbList& getCbList()
        {
            static CbList cbList;
            return cbList;
        }
        //T& client;
        //pStCb pf_;
};

//template<class T>
//typename StateMachine<T>::CbList StateMachine<T>::cbList_;

#define SM_CB(STATEID, CLASS, CB)                                           \
    bool unused##CB = StateMachine<CLASS>::registerCb(STATEID, &CLASS::CB)

typedef std::map<uint32_t, StateMachineBase*> GlobalSMList;

class SMFactory : public Singleton<SMFactory>
{
    friend class Singleton<SMFactory>;
    typedef StateMachineBase* (CreateFunc)();
    typedef std::map<uint32_t, CreateFunc*> SMTypeMap;  // SM_TYPE to SM create function map

    public:
        // operation
        bool registerSmType(uint32_t machineType, CreateFunc *pf);
        bool isValidSmType(uint32_t machineType) { return smTypeMap_.find(machineType) != smTypeMap_.end(); }
        StateMachineBase *createSM(uint32_t MachineType); //crete a new statemachine instance.

        void addSM(uint32_t id, StateMachineBase *pSM) { pSM->setId(id); globalSMList_[id] = pSM; }
        void deleteSM(uint32_t id) { globalSMList_[id] = NULL ; }
        StateMachineBase* getSM(uint32_t id) { return globalSMList_[id]; }

    private:
        SMFactory() {}
        ~SMFactory() {}
        GlobalSMList    globalSMList_;
        SMTypeMap       smTypeMap_;
};

#define SM_REG(SM_TYPE, CLASS) \
    static StateMachineBase* CLASS##Instance() { StateMachineBase *pSM = new CLASS;  pSM->setType(SM_TYPE); return pSM ;} \
bool unused##SM_TYPE = SMFactory::getInstance()->registerSmType(SM_TYPE, CLASS##Instance);  \
template class StateMachine<CLASS>;                                           \

template<class T>
void StateMachine<T>::Drive(const Event& event, SocketBase* sock)
{
    std::cout<<__FILE__<<" Begin drive SM_"<<type_<<" id:"<<id_<<" state:"<<state_<<"\n";
    // find the state
    pStCb pf_ = getCb(state_);
    if ( !pf_ )
    {
        std::cout<<"Can't find state call back function\n";
        return;
    }

    // call state process fucntion
    if ( (((T*)this)->*pf_)(event, sock) != 0 ) //ERROR OR END
    {
        std::cout<<"call back return is not 0, SM ended\n";
        delete this;
        SMFactory::getInstance()->deleteSM(id_);
    }
}

#endif
