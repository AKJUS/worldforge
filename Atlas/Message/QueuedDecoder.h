// This file may be redistributed and modified only under the terms of
// the GNU Lesser General Public License (See COPYING for details).
// Copyright (C) 2000 Stefanus Du Toit

#ifndef ATLAS_MESSAGE_QUEUEDDECODER_H
#define ATLAS_MESSAGE_QUEUEDDECODER_H

#include <queue>
#include "Object.h"
#include "DecoderBase.h"

namespace Atlas { namespace Message {

/** Decoder that presents a queue of Atlas::Message::Element.
 *
 * This message decoder puts arrived objects into a queue and allows the
 * application to pop them off the front of the queue, peek at the front of 
 * the queue, as well as checking to see how many objects are currently in the
 * queue.
 *
 * @see DecoderBase
 * @see Element
 * @author Stefanus Du Toit <sdt@gmx.net>
 *
 */
    
class QueuedDecoder : public DecoderBase
{
public:

    QueuedDecoder();

    /// Retrieve the current size of the message queue.    
    size_t queueSize();
    /// Pop an object from the front of the message queue.
    Element::MapType pop();
    /// Peek at the object at the front of the queue.
    Element::MapType front();
    /// Clear the message queue.
    void clear();
    
protected:

    /// This adds a message to the queue.
    void objectArrived(const Element::MapType& obj);
    
private:
    
    std::queue<Element::MapType> m_objectQueue;
};

} } // namespace Atlas::Message

#endif
