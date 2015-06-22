/*
 *  event.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EVENT_H
#define EVENT_H

#include <cassert>

#include "nest.h"
#include "nest_time.h"
#include "lockptr.h"
#include "exceptions.h"
#include "name.h"

namespace nest
{

class Node;

/**
 * Encapsulates information which is sent between Nodes.
 *
 * For each type of information there has to be a specialized event
 * class.
 *
 * Events are used for two tasks. During connection, they are used as
 * polymorphic connect objects. During simulation they are used to
 * transport basic event information from one node to the other.
 *
 * A connection between two elements is physically established in two
 * steps: First, create an event with the two envolved elements.
 * Second, call the connect method of the event.
 *
 * An event object contains only administrative information which is
 * needed to successfully deliver the event. Thus, event objects
 * cannot direcly contain custom data: events are not messages. If a
 * node receives an event, arbitrary abounts of data may be exchanged
 * between the participating nodes.

 * With this restriction it is possible to implement a comparatively
 * efficient event handling scheme. 5-6 function calls per event may
 * seem a long time, but this is cheap if we consider that event
 * handling makes update and communication succeptible to parallel
 * execution.
 *
 * @see Node
 * @see SpikeEvent
 * @see RateEvent
 * @see CurrentEvent
 * @see CurrentEvent
 * @see ConductanceEvent
 * @ingroup event_interface
 */

class Event
{

public:
  Event();

  virtual ~Event()
  {
  }

  /**
   * Virtual copy constructor.
   */
  virtual Event* clone() const = 0;

  /**
   * Deliver the event to receiver.
   *
   * This operator calls the handler for the specific event type at
   * the receiver.
   */
  virtual void operator()() = 0;

  /**
   * Change pointer to receiving Node.
   */
  void set_receiver( Node& );

  /**
   * Return reference to receiving Node.
   */
  Node& get_receiver() const;

  /**
   * Return reference to sending Node.
   */
  Node& get_sender() const;

  /**
   * Change pointer to sending Node.
   */
  void set_sender( Node& );

  /**
   * Return GID of sending Node.
   */
  index get_sender_gid() const;

  /**
   * Change GID of sending Node.
   */
  void set_sender_gid( index );

  /**
   * Return time stamp of the event.
   * The stamp denotes the time when the event was created.
   * The resolution of Stamp is limited by the time base of the
   * simulation kernel (@see class nest::Time).
   * If this resolution is not fine enough, the creation time
   * can be corrected by using the time attribute.
   */
  Time const& get_stamp() const;

  /**
   * Set the transmission delay of the event.
   * The delay refers to the time until the event is
   * expected to arrive at the receiver.
   * @param t delay.
   */

  void set_delay( delay );

  /**
   * Return transmission delay of the event.
   * The delay refers to the time until the event is
   * expected to arrive at the receiver.
   */
  delay get_delay() const;

  /**
   * Relative spike delivery time in steps.
   * Returns the delivery time of the spike relative to a given
   * time in steps.  Causality commands that the result should
   * not be negative.
   *
   * @returns stamp + delay - 1 - t in steps
   * @param Time reference time
   *
   * @see NEST Time Memo, Rule 3
   */
  long_t get_rel_delivery_steps( const Time& t ) const;

  /**
   * Return the sender port number of the event.
   * This function returns the number of the port over which the
   * Event was sent.
   * @retval A negative return value indicates that no port number
   * was available.
   */
  port get_port() const;

  /**
   * Return the receiver port number of the event.
   * This function returns the number of the r-port over which the
   * Event was sent.
   * @note A return value of 0 indicates that the r-port is not used.
   */
  rport get_rport() const;

  /**
   * Set the port number.
   * Each event carries the number of the port over which the event
   * is sent. When a connection is established, it receives a unique
   * ID from the sender. This number has to be stored in each Event
   * object.
   * @param p Port number of the connection, or -1 if unknown.
   */
  void set_port( port p );

  /**
   * Set the receiver port number (r-port).
   * When a connection is established, the receiving Node may issue
   * a port number (r-port) to distinguish the incomin
   * connection. By the default, the r-port is not used and its port
   * number defaults to zero.
   * @param p Receiver port number of the connection, or 0 if unused.
   */
  void set_rport( rport p );

  /**
   * Return the creation time offset of the Event.
   * Each Event carries the exact time of creation. This
   * time need not coincide with an integral multiple of the
   * temporal resolution. Rather, Events may be created at any point
   * in time.
   */
  double_t get_offset() const;

  /**
   * Set the creation time of the Event.
   * Each Event carries the exact time of creation in realtime. This
   * time need not coincide with an integral multiple of the
   * temporal resolution. Rather, Events may be created at any point
   * in time.
   * @param t Creation time in realtime. t has to be in [0, h).
   */
  void set_offset( double_t t );

  /**
   * Return the weight.
   */
  weight get_weight() const;

  /**
   * Set weight of the event.
   */
  void set_weight( weight t );

  /**
   * Check integrity of the event.
   * This function returns true, if all data, in particular sender
   * and receiver pointers are correctly set.
   */
  bool is_valid() const;

  /**
   * Set the time stamp of the event.
   * The time stamp refers to the time when the event
   * was created.
   */
  void set_stamp( Time const& );

protected:
  index sender_gid_; //!< GID of sender or -1.
                     /*
                      * The original formulation used references to Nodes as
                      * members, however, in order to avoid the reference of reference
                      * problem, we store sender and receiver as pointers and use
                      * references in the interface.
                      * Thus, we can still ensure that the pointers are never NULL.
                      */
  Node* sender_;     //!< Pointer to sender or NULL.
  Node* receiver_;   //!< Pointer to receiver or NULL.


  /**
   * Sender port number.
   * The sender port is used as a unique identifier for the
   * connection.  The receiver of an event can use the port number
   * to obtain data from the sender.  The sender uses this number to
   * locate target-specific information.  @note A negative port
   * number indicates an unknown port.
   */
  port p_;

  /**
   * Receiver port number (r-port).
   * The receiver port (r-port) can be used by the receiving Node to
   * distinguish incoming connections. E.g. the r-port number can be
   * used by Events to access specific parts of a Node. In most
   * cases, however, this port will no be used.
   * @note The use of this port number is optional.
   * @note An r-port number of 0 indicates that the port is not used.
   */
  rport rp_;

  /**
   * Transmission delay.
   * Number of simulations steps that pass before the event is
   * delivered at the receiver.
   * The delay must be at least 1.
   */
  delay d_;

  /**
   * Time stamp.
   * The time stamp specifies the absolute time
   * when the event shall arrive at the target.
   */
  Time stamp_;

  /**
   * Offset for precise spike times.
   * offset_ specifies a correction to the creation time.
   * If the resolution of stamp is not sufficiently precise,
   * this attribute can be used to correct the creation time.
   * offset_ has to be in [0, h).
   */
  double offset_;

  /**
   * Weight of the connection.
   */
  weight w_;
};


// Built-in event types

/**
 * Event for spike information.
 * Used to send a spike from one node to the next.
 */
class SpikeEvent : public Event
{
public:
  SpikeEvent();
  void operator()();
  SpikeEvent* clone() const;

  void set_multiplicity( int_t );
  int_t get_multiplicity() const;

protected:
  int_t multiplicity_;
};

inline SpikeEvent::SpikeEvent()
  : multiplicity_( 1 )
{
}

inline SpikeEvent*
SpikeEvent::clone() const
{
  return new SpikeEvent( *this );
}

inline void
SpikeEvent::set_multiplicity( int_t multiplicity )
{
  multiplicity_ = multiplicity;
}

inline int_t
SpikeEvent::get_multiplicity() const
{
  return multiplicity_;
}


/**
 * "Callback request event" for use in Device.
 *
 * Some Nodes want to perform a function on an event for each
 * of their targets. An example is the poisson_generator which
 * needs to draw a random number for each target. The DSSpikeEvent,
 * DirectSendingSpikeEvent, calls sender->event_hook(*this)
 * in its operator() function instead of calling receiver->handle().
 * The default implementation of Node::event_hook() just calls
 * target->handle(DSSpikeEvent&). Any reimplementation must also
 * execute this call. Otherwise the event will not be delivered.
 * If needed, target->handle(DSSpikeEvent&) may be called more than
 * once.
 *
 * @note Callback events must only be sent via static_synapse
 */
class DSSpikeEvent : public SpikeEvent
{
public:
  void operator()();
};

/**
 * Event for firing rate information.
 * Used to send firing rate from one node to the next.
 * The rate information is not contained in the event
 * object. Rather, the receiver has to poll this information
 * from the sender.
 */
class RateEvent : public Event
{
  double_t r_;

public:
  void operator()();
  RateEvent* clone() const;

  void set_rate( double_t );
  double_t get_rate() const;
};

inline RateEvent*
RateEvent::clone() const
{
  return new RateEvent( *this );
}

inline void
RateEvent::set_rate( double_t r )
{
  r_ = r;
}

inline double_t
RateEvent::get_rate() const
{
  return r_;
}

/**
 * Event for electrical currents.
 * Used to send currents from one node to the next.
 */
class CurrentEvent : public Event
{
  double_t c_;

public:
  void operator()();
  CurrentEvent* clone() const;

  void set_current( double_t );
  double_t get_current() const;
};

inline CurrentEvent*
CurrentEvent::clone() const
{
  return new CurrentEvent( *this );
}

inline void
CurrentEvent::set_current( double_t c )
{
  c_ = c;
}

inline double_t
CurrentEvent::get_current() const
{
  return c_;
}

/**
 * "Callback request event" for use in Device.
 *
 * Some Nodes want to perform a function on an event for each
 * of their targets. An example is the noise_generator which
 * needs to draw a random number for each target. The DSCurrentEvent,
 * DirectSendingCurrentEvent, calls sender->event_hook(*this)
 * in its operator() function instead of calling receiver->handle().
 * The default implementation of Node::event_hook() just calls
 * target->handle(DSCurrentEvent&). Any reimplementation must also
 * execute this call. Otherwise the event will not be delivered.
 * If needed, target->handle(DSCurrentEvent&) may be called more than
 * once.
 *
 * @note Callback events must only be sent via static_synapse.
 */
class DSCurrentEvent : public CurrentEvent
{
public:
  void operator()();
};

/**
 * @defgroup DataLoggingEvents Event types for analog logging devices.
 *
 * Events for flexible data recording.
 *
 * @addtogroup Devices
 * @ingroup eventinterfaces
 */

/**
 * Request data to be logged/logged data to be sent.
 *
 * @see DataLoggingReply
 * @ingroup DataLoggingEvents
 */
class DataLoggingRequest : public Event
{
public:
  /** Create empty request for use during simulation. */
  DataLoggingRequest();

  /** Create event for given time stamp and vector of recordables. */
  DataLoggingRequest( const Time&, const std::vector< Name >& );

  DataLoggingRequest* clone() const;

  void operator()();

  /** Access to stored time interval.*/
  const Time& get_recording_interval() const;

  /** Access to vector of recordables. */
  const std::vector< Name >& record_from() const;

private:
  //! Interval between two recordings, first is step 1
  Time recording_interval_;

  /**
   * Names of properties to record from.
   * @note This pointer shall be NULL unless the event is sent by a connection routine.
   */
  std::vector< Name > const* const record_from_;
};

inline DataLoggingRequest::DataLoggingRequest()
  : Event()
  , recording_interval_( Time::neg_inf() )
  , record_from_( 0 )
{
}

inline DataLoggingRequest::DataLoggingRequest( const Time& rec_int,
  const std::vector< Name >& recs )
  : Event()
  , recording_interval_( rec_int )
  , record_from_( &recs )
{
}

inline DataLoggingRequest*
DataLoggingRequest::clone() const
{
  return new DataLoggingRequest( *this );
}

inline const Time&
DataLoggingRequest::get_recording_interval() const
{
  // During simulation, events are created without recording interval
  // information. On these, get_recording_interval() must not be called.
  assert( recording_interval_.is_finite() );

  return recording_interval_;
}

inline const std::vector< Name >&
DataLoggingRequest::record_from() const
{
  // During simulation, events are created without recordables
  // information. On these, record_from() must not be called.
  assert( record_from_ != 0 );

  return *record_from_;
}

/**
 * Provide logged data through request transmitting reference.
 * @see DataLoggingRequest
 * @ingroup DataLoggingEvents
 */
class DataLoggingReply : public Event
{
public:
  //! Data type data at single recording time
  typedef std::vector< double_t > DataItem;

  /** Data item with pertaining time stamp.
   * Items are initialized with time stamp -inf to mark them as invalid.
   * Data is initialized to <double_t>::max() as a highly implausible value.
   * Ideally, we should initialized to a NaN, but since the C++-standard does
   * not require NaN, that would result in unportable code. max() should draw
   * the users att
   */
  struct Item
  {
    Item( size_t n )
      : data( n, std::numeric_limits< double_t >::max() )
      , timestamp( Time::neg_inf() )
    {
    }
    DataItem data;
    Time timestamp;
  };

  //! Container for entries
  typedef std::vector< Item > Container;

  //! Construct with reference to data and time stamps to transmit
  DataLoggingReply( const Container& );

  void operator()();

  //! Access referenced data
  const Container&
  get_info() const
  {
    return info_;
  }

private:
  //! Prohibit copying
  DataLoggingReply( const DataLoggingReply& );

  //! Prohibit cloning
  DataLoggingReply*
  clone() const
  {
    assert( false );
    return 0;
  }

  //! data to be transmitted, with time stamps
  const Container& info_;
};

inline DataLoggingReply::DataLoggingReply( const Container& d )
  : Event()
  , info_( d )
{
}

/**
 * Event for electrical conductances.
 * Used to send conductance from one node to the next.
 * The conductance is contained in the event object.
 */
class ConductanceEvent : public Event
{
  double_t g_;

public:
  void operator()();
  ConductanceEvent* clone() const;

  void set_conductance( double_t );
  double_t get_conductance() const;
};

inline ConductanceEvent*
ConductanceEvent::clone() const
{
  return new ConductanceEvent( *this );
}

inline void
ConductanceEvent::set_conductance( double_t g )
{
  g_ = g;
}

inline double_t
ConductanceEvent::get_conductance() const
{
  return g_;
}


/**
 * Event for transmitting arbitrary data.
 * This event type may be used for transmitting arbitrary
 * data between events, e.g., images or their FFTs.
 * A lockptr to the data is transmitted.  The date type
 * is given as a template parameter.
 * @note: Data is passed via a lockptr.
 *        The receiver should copy the data at once, otherwise
 *        it may be modified by the sender.
 *        I hope this scheme is thread-safe, as long as the
 *        receiver copies the data at once.  HEP.
 * @note: This is a base class.  Actual event types had to
 *        be derived, since operator() cannot be instantiated
 *        otherwise.
 */
template < typename D >
class DataEvent : public Event
{
  lockPTR< D > data_;

public:
  void set_pointer( D& data );
  lockPTR< D > get_pointer() const;
};

template < typename D >
inline void
DataEvent< D >::set_pointer( D& data )
{
  data_ = data;
}

template < typename D >
inline lockPTR< D >
DataEvent< D >::get_pointer() const
{
  return data_;
}

class DoubleDataEvent : public DataEvent< double >
{
public:
  void operator()();
  DoubleDataEvent* clone() const;
};

inline DoubleDataEvent*
DoubleDataEvent::clone() const
{
  return new DoubleDataEvent( *this );
}

//*************************************************************
// Inline implementations.

inline bool
Event::is_valid() const
{
  return ( ( sender_ != NULL ) && ( receiver_ != NULL ) && ( d_ > 0 ) );
}

inline void
Event::set_receiver( Node& r )
{
  receiver_ = &r;
}

inline void
Event::set_sender( Node& s )
{
  sender_ = &s;
}

inline void
Event::set_sender_gid( index gid )
{
  sender_gid_ = gid;
}

inline Node&
Event::get_receiver( void ) const
{
  return *receiver_;
}

inline Node&
Event::get_sender( void ) const
{
  return *sender_;
}

inline index
Event::get_sender_gid( void ) const
{
  assert( sender_gid_ > 0 );
  return sender_gid_;
}

inline weight
Event::get_weight() const
{
  return w_;
}

inline void
Event::set_weight( weight w )
{
  w_ = w;
}

inline Time const&
Event::get_stamp() const
{
  return stamp_;
}

inline void
Event::set_stamp( Time const& s )
{
  stamp_ = s;
}

inline delay
Event::get_delay() const
{
  return d_;
}

inline long_t
Event::get_rel_delivery_steps( const Time& t ) const
{
  return stamp_.get_steps() + d_ - 1 - t.get_steps();
}

inline void
Event::set_delay( delay d )
{
  d_ = d;
}

inline double_t
Event::get_offset() const
{
  return offset_;
}

inline void
Event::set_offset( double_t t )
{
  offset_ = t;
}

inline port
Event::get_port() const
{
  return p_;
}

inline rport
Event::get_rport() const
{
  return rp_;
}

inline void
Event::set_port( port p )
{
  p_ = p;
}

inline void
Event::set_rport( rport rp )
{
  rp_ = rp;
}
}

#endif // EVENT_H