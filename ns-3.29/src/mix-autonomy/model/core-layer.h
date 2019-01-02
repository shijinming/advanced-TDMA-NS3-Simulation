/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef CORE_LAYER_H
#define CORE_LAYER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3
{

class CoreLayerApplication : public Application 
{
public:
    static TypeId GetTypeId (void);
    CoreLayerApplication ();
    virtual ~CoreLayerApplication (); //虚析构函数
    /**
     * 判断指定节点是否安装了内核层应用
     * 
     * @param node  指定节点
     */
    static bool IsCoreLayerApplicationInstalled (Ptr<Node> node);

    /**
     * 以下三个virtual函数都是override的MASlotNode的纯虚函数
     * 这是的这个节点的时隙管理统一交给MADynamicSlotHelper来进行
     */ 
    virtual void SlotEnabled (uint32_t slotSize, MADynamicSlotHelper& slotHelper);
    virtual void SlotDisabled (uint32_t slotSize, MADynamicSlotHelper& slotHelper);
    virtual void RegisteredToSlotHelper (MADynamicSlotHelper& slotHelper);
    virtual void SetSlotId (uint32_t slotId);
    virtual uint32_t GetSlotId ();
    virtual uint32_t MaxAllowedSlots (); //为什么都定义的是virtual函数？
    
protected:
    virtual void DoDispose ();

private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    /**
     * 尝试创建Socket
     * - m_socket
     * - m_sinkSocket
     */ 
    void TryCreateSockets ();   //普通socket和汇聚节点的socket

    /**
     * 开始全速发送，发送速度由m_dataRate控制。
     * 函数会根据这个设定速度，计算发送完当前的数据包需要的时间，然后设置一个函数调度在发送完成后继续
     * 发送下一个包。直到调用StopSending结束当前的时隙
     * 
     * 这个函数在SlotEnabled(uint32_t, MADynamicSlotHelper&)中被调用。
     * 
     * CAUTION：
     * 1. 函数内尝试设置了底层mac的使能机制
     * 2. 第一个包的发送要延后一个时隙（这个函数直接调用了ScheduleNextTx而不是SendPacket)，这种
     * 设计机制可以减少冲突。
     */
    void StartSending ();
    /**
     * 创建并发送Packet，Packet的大小由m_pktSize指定
     * 同时会打上一个tag，tag标识了发送者的类别（即此包由内核层发送）
     * m_txTrace会被调用
     * 
     * 最后ScheduleNextTx这个函数会被调用
     */ 
    void SendPacket ();
    /**
     * 用来实现全速发送，如果当前m_enabled为true，则会调度下一次发送
     * 下一次发送的时间的计算方法为：用packet的大小，除以m_dataRate标明的速率
     * 调度下一次发送的方法是schedule SendPacket这个函数，这个函数内会再次调用ScheduleNextTx
     */ 
    void ScheduleNextTx ();
    /**
     * 停止发送过程
     * 主要原理是停止SendPacket和ScheduleNextTx的pingpong相互调用过程
     * 
     * 通过调用CancelEvents函数来实现
     */ 
    void StopSending ();
    /**
     * 取消m_sendEvent事件
     */
    void CancelEvents ();

    uint32_t    m_slotId;
    Ptr<Socket> m_socket;
    Ptr<Socket> m_sinkSocket;
    Address     m_peer;
    uint32_t    m_pktSize;
    DataRate    m_dataRate;
    TypeId      m_tid;
    /**
     * 在ScheduleNextTx中记录调度下一次SendPacket事件，以用于在StopSending中进行取消
     */ 
    EventId     m_sendEvent;

    bool        m_verbose; //冗余？？
    /**
     * 表示是否允许这个APP发送数据包
     * 这个变量被设置（为true）表明到了这个APP所占有的时隙
     */
    bool        m_enabled;

    TracedCallback<Ptr<const Packet>, const Address & > m_txTrace;  //又是& >这种符号结合
    TracedCallback<Ptr<const Packet>, Ptr<const Application>, const Address & > m_rxTrace;
    
    void PacketSentCallback (Ptr<Socket> socket, uint32_t bytesSent);

    /**
     * m_sinkSocket的接收响应函数，即这个socket接收到包后调用这个函数
     * 
     * 这个函数内同时会调用m_rxTrace
     */ 
    void ReceivePacket (Ptr<Socket> socket);

    MyGlobalConfig & config;
    MADynamicSlotHelper* m_slotHelper;
    /**
     * app是否已经被start
     */
    bool m_appStarted;
};

}

#endif