# Copyright (c) 2011-2021 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# User-defined configuration ports
# <<--directives-param-->>

set_directive_interface -mode ap_none "top" conf_info_num_batch_x


# Insert here any custom directive

set_directive_loop_tripcount -min 8 -max 8 -avg 8 "top/go"
set_directive_dataflow "top/go"


set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _inbuff_x
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _inbuff_y
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _inbuff_z
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _outbuff_Qr
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _outbuff_Qi



set_directive_array_partition -type cyclic -factor ${prl} -dim 1 "top" _inbuff_kx
set_directive_array_partition -type cyclic -factor ${prl} -dim 1 "top" _inbuff_ky
set_directive_array_partition -type cyclic -factor ${prl} -dim 1 "top" _inbuff_kz
set_directive_array_partition -type cyclic -factor ${prl} -dim 1 "top" _inbuff_phiR
set_directive_array_partition -type cyclic -factor ${prl} -dim 1 "top" _inbuff_phiI

config_array_partition -auto_partition_threshold 2
