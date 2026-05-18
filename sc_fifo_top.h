/*! 
******************************************************************************
 @file   sc_fifo_top.h

 @brief  

 @author Imagination Technologies

 @date   08/11/2010
 
         <b>Copyright 2010 by Imagination Technologies Limited.</b>
         All rights reserved.  No part of this software, either
         material or conceptual may be copied or distributed,
         transmitted, transcribed, stored in a retrieval system
         or translated into any human or computer language in any
         form by any means, electronic, mechanical, manual or
         other-wise, or disclosed to third parties without the
         express written permission of Imagination Technologies
         Limited, Unit 8, HomePark Industrial Estate,
         King's Langley, Hertfordshire, WD4 8LZ, U.K.

******************************************************************************/
#pragma once

#include <vector>
#include "systemc.h"

// fifo with access to the top item.
template <class T, unsigned int SIZE = 16>
class sc_fifo_top : public sc_fifo<T>
{
public:

    // constructors

    explicit sc_fifo_top( int size_ = SIZE)
	: sc_fifo<T>( size_ )
	{ }

    explicit sc_fifo_top( const char* name_, int size_ = SIZE)
	: sc_fifo<T>( name_, size_ )
	  
	{ }

	const T& top() const
	{
		return this->m_buf[this->m_ri];	
	}
	T& top()
	{
		return this->m_buf[this->m_ri];	
	}
	virtual ~sc_fifo_top()
	{
	}
};



template<class T, unsigned int SIZE >
class sc_fifo_top_array
	: public sc_vector<sc_fifo_top<T>> 
{
public: 
	sc_fifo_top_array(std::string prefix)
		: sc_vector<sc_fifo_top<T>>(prefix.c_str(), SIZE)
	{
	}

	void disable() 
	{
		for(unsigned int i = 0; i < SIZE; i++)
		{
			(*this)[i].disable();
		}
	}
}; 

template<class IF, unsigned int SIZE>
class sc_port_array
	: public sc_vector<sc_port<IF> >
{
public:
	sc_port_array(std::string prefix)
		: sc_vector<sc_port<IF> >(prefix.c_str(), SIZE)
	{
	}

	void disable()
	{
		for(unsigned int i = 0; i < SIZE; i++)
		{
			(*this)[i].disable();
		}
	}

};