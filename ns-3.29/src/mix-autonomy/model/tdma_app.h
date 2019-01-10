/**
 * @file tdma_app.h
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

#include "ns3/sim-config.h"

namespace ns3 
{

class TDMAApplication: public Application
{
public:
  static TypeId GetTypeId  (void);
  TDMAApplication ();
  virtual ~TDMAApplication ();

  void SetStartTime (Time start);
  void SetStopTime (Time stop);

  /**
   * @brief 时隙单元定义
   */
  struct TDMASlot {
    Time start; /**< 时隙开始时间，注意是相对时间 */
    Time duration; /**< 时隙长度 */
    uint64_t id; /**< 时隙ID，这个成员可以是Optional的，在本类中不会使用，可以供子类使用*/
  };

protected:
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  virtual void DoDispose ();
  virtual void DoInitialize ();

  /** 时隙ID */
  uint32_t slotId; 
  /** 发送socket */
  Ptr<Socket> socket;
  /** 接收socket */
  Ptr<Socket> sink;
  /** 用于创建socket的type id*/
  TypeId socketTid;
  /** 数据率 */
  DataRate dataRate;
  /** 调度发送的句柄 */
  EventId txEvent;
  EventId slotEndEvt;
  EventId slotStartEvt;
  /** 是否正在发送 */
  bool isBusySending;
  bool isAtOwnSlot;

  /** 当前时隙 */
  TDMASlot curSlot;
  /** slotCount, 本节点经过的时隙计数  */
  uint64_t slotCnt = 0;

  /** 发送的trace */
  TracedCallback<Ptr<const Packet>, const Address & > m_txTrace;
  /** 接收的trace */
  TracedCallback<Ptr<const Packet>, Ptr<const Application>, const Address & > m_rxTrace;

  /** 全局仿真配置 */
  SimulationConfig &config;

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
   * @brief 时隙已经结束（注意这个函数不是用来结束时隙，而是告知时隙已经结束。下面的SlotStarted含义类似
   */
  void SlotEnded (void);

  /**
   * @brief 时隙开始
   */
  void SlotStarted (void);
  
  /**
   * @brief 取消所有调度事件
   * 
   */
  void CancelAllEvents (void);

protected:

  /**
   * 对接收到的数据包进行处理，子类必须实现
   */
  virtual void ReceivePacket (Ptr<const Packet> pkt, const Address & srcAddr) = 0;

  /**
   * @brief 获取下一个时隙的间隔，TDMAApplication会按照这个函数返回的间隔来设定定时器
   * 以在对应的时间唤醒发送队列
   * 
   * @return TDMASlot 下一个时隙
   */
  virtual TDMASlot GetNextSlotInterval (void) = 0;

  /**
   * @brief 获取起始时隙
   * 
   * @return TDMASlot   起始时隙
   */
  virtual TDMASlot GetInitalSlot (void) = 0;

  /**
   * @brief 时隙开始的钩子函数
   */
  virtual void SlotDidStart (void) {};

  /**
   * @brief 时隙结束的钩子函数
   * 
   */
  virtual void SlotDidEnd (void) {};
};

}

#endif