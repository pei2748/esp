# Copyright (c) 2011-2021 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# User-defined configuration ports
set_directive_interface -mode ap_none "top" conf_info_nbursts

# Insert here any custom directive

set_directive_loop_tripcount -min 4 -max 4 -avg 4 "top/go"                                                                    
#set_directive_dataflow "top/go"                                                                          

#set_directive_unroll -factor ${unroll_factor} "store/store_label1"
#set_directive_unroll -factor ${unroll_factor} "load/load_label0"

