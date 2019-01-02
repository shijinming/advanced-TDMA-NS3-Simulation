/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef MIDDLE_LAYER_H
#define MIDDLE_LAYER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3
{

class HumanApplication : public Application
{
public:
    static TypeId GetTypeId (void);
    HumanApplication ();
    virtual ~HumanApplication ();

    enum Status {Middle, Outter};

    /**
     * 获取一个节点当前的状态：中间层还是外围层。
     * 如果没有安装HumanApplication，返回-1
     * 
     * 轮询node绑定的app，然后找出HumanApplication
     */
    static int GetHumanStatus (Ptr<Node> node);

    Status GetStatus ();
    void SetTDMAEnable (bool val);

protected:
    virtual void DoDispose ();

private:
    virtual void StartApplication ();
    virtual void StopApplication ();

    void StartSending ();  //开始发送数据包，实际上就是调用了SendPacket函数

    /**
     * 发送数据包。能够发送的条件是：
     * 1. 如果是中间层节点，则要求m_tdmaEnabled为true，且配置允许中间层发送
     * 2. 如果是外围层节点，则只需要外围层允许中间层发送即可
     * 
     * 发送完成后调用ScheduleNextTx，这也是一个PingPong循环过程
     */
    void SendPacket ();

    /**
     * 以一定的间隔进行beacon，beacon的间隔引入了随机因素，间隔为：
     * m_beaconInterval * x
     * 其中x是一个均匀分布于0.9到1.1之间的随机变量
     */
    void ScheduleNextTx ();

    /**
     * 开始调度第一个数据包。调度有一个初始的延时，由m_startDelay来控制
     * 调度执行的函数是StartSending
     */
    void ScheduleFirstTx ();
    void CancelAllEvents ();

    void TryCreateSockets ();  // 尝试创建m_socket和m_sinkSocket
    void CreateSendSocket ();
    void CreateRecvSink ();

    /**
     * 周期性的检查状态，主要内容是检查当前时间到m_lastTimeFromCore（即
     * 上一次收到来自内核层的消息的时间）的间隔，如果超过了阈值(m_setOutInterval)，
     * 则将状态设置为外围层
     */
    void PeriodicCheckStatus ();

    /**
     * 接收到来自内核层的数据包
     */
    void RecvFromCoreLayer (Ptr<Packet> packet, Ptr<Node> node);
    void RecvFromOtherHuman (Ptr<Packet> packet, Ptr<Node> node);

    bool                m_verbose;
    Status              m_status;
    uint32_t            m_beaconInterval;  //beacon的间隔（均值）
    uint32_t            m_startDelay;  //开始第一个beacon的延时
    uint32_t            m_pktSize;
    DataRate            m_dataRate;
    Ptr<Socket>         m_socket;
    Ptr<Socket>         m_sinkSocket;
    TypeId              m_tid;
    Address             m_peer;

    /**
     * 上一次收到来自内核层的消息的时间
     */
    Time                m_lastTimeFromCore;
    /**
     * pingpong调度发送beacon的事件
     */
    EventId             m_sendEvent;
    /**
     * 调度第一个数据包的事件
     */
    EventId             m_scheduleEvent;
    /**
     * 周期性检查状态的事件
     */
    EventId             m_statusCheckEvent;
    uint32_t            m_checkPeriod;  // in ms, check status
    uint32_t            m_setOutInterval;   // Set as outter nodes if no core packet
                                            // received within this time, in ms
    uint64_t            m_stream;

    /**
     * 这个变量用来记录中间层的状态，反应了中间层节点距离内核层的距离（跳跃数）
     */
    uint32_t            m_middleHops;
    /**
     * 只对中间层节点有效
     * 用于TDMA系统，设置为true时意味着此节点可以在tdma时隙发送数据包。false反之
     */
    bool                m_tdmaEnabled;

    /**
     * 这里trace的两个参数分别为
     * 1. 涉及的数据包
     * 2. 发送此数据包的节点的地址
     */
    TracedCallback<Ptr<const Packet>, const Address & > m_txTrace;
    /**
     * 这里trace的三个参数分别为：
     * 1. 涉及的数据包
     * 2. 收到此数据包的app
     * 3. 发送此数据包的节点的地址
     */
    TracedCallback<Ptr<const Packet>, Ptr<const Application>, const Address & > m_rxTrace;

    /**
     * m_socketSink的接收响应
     * 判断数据包是否来自内核层，并选择调用RecvFromCoreLayer还是RecvFromOtherHuman
     * 
     * @param socket收到数据包的socket
     */
    void ReceivePacket (Ptr<Socket> socket);

    MyGlobalConfig & config;
};


}

#endif