/**
 * @file tdma-app.h
 * @author Woody Huang (woodyhuang1@gmail.com)
 * @brief 实现了TDMA功能的基础类，需要继承到子类使用
 * @version 0.1
 * @date 2019-01-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#ifndef TDMA_NODE_H
#define TDMA_NODE_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wave-module.h"

#include "ns3/sim-config.h"

#include "my-header.h"

namespace ns3 
{
/** 当前帧状态 */
enum Frame {CCH_apFrame, SCH_apFrame,CCH_hdvFrame,SCH_hdvFrame};

/**
 * @brief 时隙单元定义  时隙数后续调试可改
 */
struct TDMASlot {
  Time start; /**< 时隙开始时间，注意是相对时间 */
  Time duration; /**< 时隙长度 */
  uint64_t id; /**< 时隙ID，这个成员可以是Optional的，在本类中不会使用，可以供子类使用*/
  Frame curFrame;
  uint64_t frameId; /**< 处于当前帧内的第几个时隙 */
  uint64_t frameNum;  /**< 当前为仿真开始后的第几个帧（此帧为4帧的总和） */
  uint64_t CCHSlotNum; /**  控制帧时隙数 2*（apNum+1）*/
  uint64_t SCHSlotNum; /**  数据帧时隙数 2*（apNum+1）*/ 
  uint64_t apCCHSlotNum; //内核层控制帧时隙数，等于apNum+1
  uint64_t apSCHSlotNum; //内核层数据帧时隙数，等于apNum+1
  uint64_t hdvCCHSlotNum; //中间层控制帧时隙数，该层的控制帧位于内核层控制帧之后，等于apNum+1
  uint64_t hdvSCHSlotNum; //中间层数据帧时隙数，该层的数据帧位于内核层数据帧之后，apNum+1
};

/**
 * @brief 控制信道服务信道定义
 */
#define tdma_SCH1 172
#define tdma_SCH2 174
#define tdma_SCH3 176
#define tdma_CCH  178
#define tdma_SCH4 180
#define tdma_SCH5 182
#define tdma_SCH6 184

class TDMAApplication: public Application
{
public:
  static TypeId GetTypeId  (void);
  TDMAApplication ();
  virtual ~TDMAApplication ();

  void SetStartTime (Time start);
  void SetStopTime (Time stop);

protected:
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  virtual void DoDispose ();
  virtual void DoInitialize ();

  /** 时隙ID */
  int64_t slotId; 
  /** 发送socket */
  Ptr<Socket> socket;
  /** 接收socket */
  Ptr<Socket> sink;
  /** 用于创建socket的type id*/
  TypeId socketTid;
  /** 数据率 */
  DataRate dataRate;
  /** Mock包大小 */
  uint32_t mockPktSize;
  /** 调度发送的句柄 */
  EventId txEvent;
  EventId slotEndEvt;
  EventId slotStartEvt;
  /** 是否正在发送 */
  bool isAtOwnSlot;
  /** 是否启用mock packet发送 */
  bool enableMockTraffic;

  /** 当前时隙信息 */
  TDMASlot curSlot;
  /** slotCount, 本节点经过的时隙计数  */
  uint64_t slotCnt = 0;

  /** 自动驾驶车辆数量 */
  uint64_t apNum;

  Time minTxInterval = MicroSeconds (100);
  Time slotSize = MilliSeconds (20);

  /** 发送的trace */
  TracedCallback<Ptr<const Packet>, const Address & > txTrace;
  /** 接收的trace */
  TracedCallback<Ptr<const Packet>, Ptr<const Application>, const Address & > rxTrace;

  /** 全局仿真配置 */
  SimulationConfig &config;

  /** 
   * 发送队列 
   * 
   * 注意，发送具有实际意义的控制包的时候，不要直接使用socket，将包放入这个队列，可以直接使用SendPacket这个函数
   */
  std::queue<Ptr<Packet> > txqCCH;
  std::queue<Ptr<Packet> > txqSCH;
  virtual void SendPacket (void) = 0;

  /**
   * @brief 唤醒发送队列
   */
  void WakeUpTxQueue (void);

    /**
   * @brief 根据当前状态初始化发送包的帧头
   * 
   * @param hdr 
   */
  virtual void SetupHeader(PacketHeader &hdr) = 0;

private:

  //====================================
  // 函数定义
  //====================================

  /**
   * 创建Socket，包括socket和sink
   */
  void CreateSocket (void);

  /**
   * @brief m_sink接收到数据包后的回调
   * 
   * @param socket 出发回调的socket，这里总是sink
   */
  void OnReceivePacket (Ptr<Socket> socket);
  
  /**
   * @brief 取消所有调度事件
   * 
   */
  void CancelAllEvents (void);

  void DoSendPacket (Ptr<Packet> pkt);

  /**
   * @brief 信道切换
   * 
   */
  void SwitchToNextChannel (uint32_t curChannelNumber, uint32_t nextChannelNumber); 

  /**
   * @brief 周期性的在控制信道和服务信道中切换，从而发送控制帧和数据帧
   * 
   */
  void PeriodicSwitch (TDMASlot curSlot);

  /**
   * @brief 设置自动驾驶编队中车辆数
   * 
   */
  void SetapNum (uint32_t N);

protected:
  /**
   * @brief 时隙已经结束（注意这个函数不是用来结束时隙，而是告知时隙已经结束。下面的SlotStarted含义类似
   */
  void SlotEnded (void);

  /**
   * @brief 时隙开始
   */
  void SlotStarted (void);

  /**
   * 对接收到的数据包进行处理，子类必须实现
   */
  virtual void ReceivePacket (Ptr<Packet> pkt, Address & srcAddr) = 0;

  /**
   * @brief 获取下一个时隙的间隔，TDMAApplication会按照这个函数返回的间隔来设定定时器
   * 以在对应的时间唤醒发送队列
   * 
   * @return TDMASlot 下一个时隙
   */
  virtual struct TDMASlot GetNextSlotInterval (void) = 0;

  /**
   * @brief 获取起始时隙
   * 
   * @return TDMASlot   起始时隙
   */
  struct TDMASlot GetInitalSlot (void);

  /**
   * @brief 对curSlot结构体里的一些变量进行配置
   */
  void SetCurSlot(void);

  /**
   * @brief 时隙开始的钩子函数Ptr<Packet> pkt, Address & srcAddr
   */
  virtual void SlotWillStart (void) {};

  /**
   * @brief 时隙结束的钩子函数
   * 
   */
  virtual void SlotDidEnd (void) {};

  /**
   * @brief  将要发送Mock包，这个回调函数允许你再Mock包被发送前对改包进行修改.
   * 
   * @param pkt 
   */
  virtual void WillSendMockPacket (Ptr<Packet> pkt) {SendPacket();};

  virtual void SlotAllocation (void) {};

  virtual bool IsAPApplicationInstalled (Ptr<Node> node) = 0;
};

}

#endif