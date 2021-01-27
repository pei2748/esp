// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __MRIQ_CONF_INFO_HPP__
#define __MRIQ_CONF_INFO_HPP__

#include <systemc.h>

//
// Configuration parameters for the accelerator.
//
class conf_info_t
{
public:

    //
    // constructors
    //
    conf_info_t()
    {
        /* <<--ctor-->> */
      this->numX = 262144;
      this->numK = 2048;
      this->batch_size_x = 1024;

#if(ARCH==0)
      this->batch_size_k = 2048;
#else
      this->batch_size_k = 2048;
#endif

      this->num_batch_x = 256;
      this->num_batch_k = 1;
    }

  conf_info_t(
	      /* <<--ctor-args-->> */
	      int32_t numX,
	      int32_t numK,
	      int32_t batch_size_x,
	      int32_t batch_size_k,
	      int32_t num_batch_x,
	      int32_t num_batch_k
	      )
  {
    /* <<--ctor-custom-->> */
    this->numX = numX;
    this->numK = numK;
    this->batch_size_x = batch_size_x;
    this->batch_size_k = batch_size_k;
    this->num_batch_x = num_batch_x;
    this->num_batch_k = num_batch_k;
  }
  // equals operator
  inline bool operator==(const conf_info_t &rhs) const
  {
    /* <<--eq-->> */
    if (numX != rhs.numX) return false;
    if (numK != rhs.numK) return false;
    if (batch_size_x != rhs.batch_size_x) return false;
    if (batch_size_k != rhs.batch_size_k) return false;
    if (num_batch_x != rhs.num_batch_x) return false;
    if (num_batch_k != rhs.num_batch_k) return false;
    return true;
  }

  // assignment operator
  inline conf_info_t& operator=(const conf_info_t& other)
  {
    /* <<--assign-->> */
    numX = other.numX;
    numK = other.numK;
    batch_size_x = other.batch_size_x;
    batch_size_k = other.batch_size_k;
    num_batch_x = other.num_batch_x;
    num_batch_k = other.num_batch_k;
    return *this;
  }

  // VCD dumping function
  friend void sc_trace(sc_trace_file *tf, const conf_info_t &v, const std::string &NAME)
  {}

  // redirection operator
  friend ostream& operator << (ostream& os, conf_info_t const &conf_info)
  {
    os << "{";
    /* <<--print-->> */
    os << "numX = " << conf_info.numX << ", ";
    os << "numK = " << conf_info.numK << "";
    os << "batch_size_x = " << conf_info.batch_size_x << "";
    os << "batch_size_k = " << conf_info.batch_size_k << "";
    os << "num_batch_x = " << conf_info.num_batch_x << "";
    os << "num_batch_k = " << conf_info.num_batch_k << "";
    os << "}";
    return os;
  }

  /* <<--params-->> */
  int32_t numX;
  int32_t numK;
  int32_t batch_size_x;
  int32_t batch_size_k;
  int32_t num_batch_x;
  int32_t num_batch_k;
};


#endif // __MRIQ_CONF_INFO_HPP__
