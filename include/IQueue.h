#ifndef IQUEUE_H
#define IQUEUE_H

/**
 * @file IMessageQueue.h
 * @brief  Message Queue interface.
 *        An abstract queue interface for messaging.
 * @author Guillermo M. Paris
 * @date 2020-01-08
 */


template<class E, class C>
class IQueue
{
public:
    virtual bool  isEmpty() = 0;
    virtual void  push(E) = 0;
    virtual void  setConsumer(C) = 0; // It is up to consumer class to call pop()
    virtual bool  start() = 0;
    virtual bool  stop() = 0;
};


#endif // IQUEUE_H

