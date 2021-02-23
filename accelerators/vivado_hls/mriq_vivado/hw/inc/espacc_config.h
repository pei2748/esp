// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef INC_ESPACC_CONFIG_H
#define INC_ESPACC_CONFIG_H

// User defined constants

// Data type

#define IS_TYPE_FIXED_POINT 1
#define FRAC_BITS 20
#define IS_TYPE_UINT 0
#define IS_TYPE_INT 1
#define IS_TYPE_FLOAT 0

// In/out arrays

//#define SIZE_IN_CHUNK_DATA 1311488

// size_in_k_data is 5*numK, numK = 3072
#define SIZE_IN_K_DATA 80
#define SIZE_IN_CHUNK_DATA 384 // 128*3
#define SIZE_OUT_CHUNK_DATA 256 // 128*2

#endif
