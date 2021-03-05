# Copyright (c) 2011-2021 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# User-defined configuration ports
# <<--directives-param-->>
set_directive_interface -mode ap_none "top" conf_info_numX
set_directive_interface -mode ap_none "top" conf_info_numK

# Insert here any custom directive

# load_k_ctrl is for kx,ky,kz,phiR,phiI
# load_ctrl is for x,y,z


# suppose in1 is the input for x,y,z
# suppose in2 is the input for kx,ky,kz,phiR,phiI



#set_directive_dataflow "top/batching"
# suppose _inbuff is for x,y,z
# suppose _inbuff_x is for kx,ky,kz,phiR,phiI

#set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _inbuff_k





set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _inbuff_x
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _inbuff_y
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _inbuff_z

#set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _outbuff_Qr
#set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" _outbuff_Qi
