#include "../inc/espacc.h"



void dma_read(word_t array[], unsigned dma_index, unsigned dma_length, dma_word_t *in1)
{
  for (unsigned i = 0; i < dma_length; i++) {
#pragma HLS loop_tripcount min=2 max=16 avg=8
    for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS loop_unroll
      array[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
      //      std::cout << "in[" << i * VALUES_PER_WORD + j << "]  = " << array[i * VALUES_PER_WORD + j] << std::endl; 
    }
  }
}


void dma_write(word_t array[], unsigned dma_index, unsigned dma_length, dma_word_t *out)
{
  for (unsigned i = 0; i < dma_length; i++) {
#pragma HLS loop_tripcount min=2 max=16 avg=8
    for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS loop_unroll
      out[dma_index + i].word[j] =  array[i * VALUES_PER_WORD + j];
      //      std::cout << "out[" << i * VALUES_PER_WORD + j << "]  = " << array[i * VALUES_PER_WORD + j] << std::endl;                                 
    }
  }
}



typedef struct DMA_index_load{

  unsigned kx;
  unsigned ky;
  unsigned kz;
  unsigned phiR;
  unsigned phiI;
  unsigned x;
  unsigned y;
  unsigned z;

} dma_index_load_t;


typedef struct DMA_length_load {

  unsigned kx;
  unsigned ky;
  unsigned kz;
  unsigned phiR;
  unsigned phiI;
  unsigned x;
  unsigned y;
  unsigned z;

} dma_length_load_t;

typedef struct DMA_index_store{

  unsigned Qr;
  unsigned Qi;
} dma_index_store_t;

typedef struct DMA_length_store{

  unsigned Qr;
  unsigned Qi;
} dma_length_store_t;
