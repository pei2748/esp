#!/bin/bash

DMA_SIZE="32 64"

ARCH="0 1"
PARAL="4 8 16"

for dma_size in $DMA_SIZE; do
    for arch in $ARCH; do
	for paral in $PARAL; do
	    # Get latency of each architecture
	    LATENCY_TRACE="${PWD}/bdw_work/trace/sim.BASIC_P${paral}_A${arch}_DMA${dma_size}_V.s.trace"
	    LATENCY_LOG="${PWD}/latency_P${paral}_A${arch}_DMA${dma_size}.log"
	    #	LATENCY_TRACE="${PWD}/bdw_work/trace/sim.BASIC_DMA${dma_size}_V.trace"
	    #	LATENCY_LOG="${PWD}/latency_DMA${dma_size}.log"
	    
	    if test ! -e $LATENCY_TRACE; then
		echo "--- $LATENCY_TRACE not found ---"
		continue
	    fi

	    LIST_INFO_DRIVER=$(cat $LATENCY_TRACE | grep "Info: testbench: @" | cut -d' ' -f 3)

	    ARR_INFO_DRIVER=($LIST_INFO_DRIVER)


	    TIME_BEG_SCIENCE=${ARR_INFO_DRIVER[0]//@}
	    TIME_BEG=$(printf "%.f" "$TIME_BEG_SCIENCE")


	    TIME_END_SCIENCE=${ARR_INFO_DRIVER[1]//@}
	    TIME_END=$(printf "%.f" "$TIME_END_SCIENCE")
	    TOT_TIME=$(echo "$TIME_END - $TIME_BEG" | bc)
	    
	    echo "$TOT_TIME" > $LATENCY_LOG
	done
     done
done
