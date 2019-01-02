#ifndef THREE_LAYER_SLOT_NODE_H
#define THREE_LAYER_SLOT_NODE_H

#include "ns3/core-module.h"

namespace ns3 
{

class MADynamicSlotHelper;

/**
 * 时隙节点，定义为接口形式
 */ 
class MASlotNode 
{
public:
    virtual ~MASlotNode () {};
    // 时隙开始
    virtual void SlotEnabled (uint32_t slotSize, MADynamicSlotHelper& slotHelper) = 0;
    // 时隙结束
    virtual void SlotDisabled (uint32_t slotSize, MADynamicSlotHelper& slotHelper) = 0;
    // 注册到SlotHelper
    virtual void RegisteredToSlotHelper (MADynamicSlotHelper& slotHelper) = 0;
    // 设置时隙id
    virtual void SetSlotId (uint32_t slotId) = 0;
    virtual uint32_t GetSlotId () = 0;
    // 这个函数返回的结果表明了此类型的节点最大允许的时隙数量
    virtual uint32_t MaxAllowedSlots () = 0;
};

}


#endif