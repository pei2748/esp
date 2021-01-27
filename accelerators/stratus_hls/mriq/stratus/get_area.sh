#!/bin/bash

DMA_SIZE="32 64"
ARCH="LESS SMALL FAST"

for dma_size in $DMA_SIZE; do
    for arch in $ARCH; do
	# Get area of each architecture
	AREA_TRACE="${PWD}/bdw_work/trace/hls.mriq.BASIC_${arch}_DMA${dma_size}.s.trace"
	AREA_LOG="${PWD}/area_${arch}_DMA${dma_size}.log"

	if test ! -e $AREA_TRACE; then
            echo "--- trace log of $arc not found ---"
            continue
	fi


	LIST_LUTS=$(cat $AREA_TRACE | grep "Total LUTs/Mults" |  sed 's/\s\+/ /g'  | cut -d " " -f 6)
	LIST_MULTS=$(cat $AREA_TRACE | grep "Total LUTs/Mults" |  sed 's/\s\+/ /g'  | cut -d " " -f 7)
	LIST_BRAM=$(cat $AREA_TRACE | grep "00402" | grep "plm_block" | sed 's/\s\+/ /g' | cut -d " " -f 8)
	
	ARR_LUTS=($LIST_LUTS)
	ARR_MULTS=($LIST_MULTS)
	
	# use the last item of the arrays LUTS/Mults
	A_BRAMS=$(( ${LIST_BRAM//$'\n'/+} ))
	TEMP1=${ARR_LUTS[-1]}
	A_LUTS=$(echo "$TEMP1 - $A_BRAMS" | bc)
	A_MULTS=${ARR_MULTS[-1]}
	
	# area formula: BRAM*0.00142 + LUTs*0.00000434 + Mults*0.000578704
	AREA_BRAMS=$(echo "$A_BRAMS*0.00142" | bc -l)
	AREA_LUTS=$(echo "$A_LUTS*0.00000434" | bc -l)
	AREA_MULTS=$(echo "$A_MULTS*0.000578704" | bc -l)
	
	TOT_AREA=$(echo "$AREA_BRAMS + $AREA_LUTS + $AREA_MULTS" | bc -l)
	AVG_AREA=$(echo "$TOT_AREA / 3" | bc -l)
	
	echo "0$AVG_AREA" > $AREA_LOG
    done
done
