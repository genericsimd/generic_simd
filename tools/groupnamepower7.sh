#/bin/ksh
g=$1
shift
C5="PM_RUN_INST_CMPL  : Run instructions completed";
C6="PM_RUN_CYC  : Run cycles";
case $g in
 0) 
C1="PM_CYC [shared core] : Processor cycles";
C2="PM_RUN_CYC  : Run cycles";
C3="PM_INST_DISP  : Instructions dispatched";
C4="PM_INST_CMPL  : Instructions completed";;
 1)
C1="PM_BR_PRED_CCACHE  : Branch count cache prediction";
C2="PM_BR_PRED_LSTACK  : A conditional branch was predicted, link stack";
C3="PM_BR_MPRED_CCACHE  : Branch misprediction due to count cache prediction";
C4="PM_BR_MPRED_TA  : Branch mispredictions due to target address";;
 2)
C1="PM_BR_PRED  : A conditional branch was predicted";
C2="PM_BR_PRED_CR  : A conditional branch was predicted, CR prediction";
C3="PM_BR_PRED_CCACHE  : Branch count cache prediction";
C4="PM_BR_PRED_LSTACK  : A conditional branch was predicted, link stack";;
 3)
C1="PM_BRU_FIN  : BRU produced a result";
C2="PM_BR_TAKEN  : Branches taken";
C3="PM_BR_PRED  : A conditional branch was predicted";
C4="PM_BR_MPRED  : Branches incorrectly predicted";;
 4)
C1="PM_BR_MPRED_CR  : Branch mispredictions due to CR bit setting";
C2="PM_BR_UNCOND  : Unconditional branch";
C3="PM_BR_MPRED_TA  : Branch mispredictions due to target address";
C4="PM_BR_MPRED_CCACHE  : Branch misprediction due to count cache prediction";;
 5)
C1="PM_BR_PRED_CR_TA  : A conditional branch was predicted, CR and target prediction";
C2="PM_BR_MPRED_CR_TA  : Branch mispredict - taken/not taken and target";
C3="PM_BR_PRED  : A conditional branch was predicted";
C4="PM_BR_PRED_CR  : A conditional branch was predicted, CR prediction";;
 6)
C1="PM_BR_PRED_CCACHE  : Branch count cache prediction";
C2="PM_BR_PRED_LSTACK  : A conditional branch was predicted, link stack";
C3="PM_BR_PRED_CR  : A conditional branch was predicted, CR prediction";
C4="PM_BR_PRED_TA  : A conditional branch was predicted, target prediction";;
 7)
C1="PM_BR_MPRED_CR  : Branch mispredictions due to CR bit setting";
C2="PM_BR_PRED_CR  : A conditional branch was predicted, CR prediction";
C3="PM_BR_PRED_CCACHE  : Branch count cache prediction";
C4="PM_BR_PRED_LSTACK  : A conditional branch was predicted, link stack";;
 8)
C1="PM_BR_MPRED_TA  : Branch mispredictions due to target address";
C2="PM_BR_PRED_CR  : A conditional branch was predicted, CR prediction";
C3="PM_BR_PRED_CCACHE  : Branch count cache prediction";
C4="PM_BR_PRED_LSTACK  : A conditional branch was predicted, link stack";;
 9)
C1="PM_BR_MPRED_CCACHE  : Branch misprediction due to count cache prediction";
C2="PM_BR_PRED_CR  : A conditional branch was predicted, CR prediction";
C3="PM_BR_PRED_CCACHE  : Branch count cache prediction";
C4="PM_BR_PRED_LSTACK  : A conditional branch was predicted, link stack";;
 10)
C1="PM_IERAT_MISS  : IERAT miss count";
C2="PM_DSLB_MISS  : Data SLB misses";
C3="PM_ISLB_MISS  : Instruction SLB misses";
C4="PM_SLB_MISS  : SLB misses";;
 11)
C1="PM_BTAC_MISS  : BTAC Misses";
C2="PM_TLB_MISS  : TLB misses";
C3="PM_DTLB_MISS  : Data TLB misses";
C4="PM_ITLB_MISS  : Instruction TLB misses";;
 12)
C1="PM_DTLB_MISS_16G  : Data TLB miss for 16G page";
C2="PM_DTLB_MISS_4K  : Data TLB miss for 4K page";
C3="PM_DTLB_MISS_64K  : Data TLB miss for 64K page";
C4="PM_DTLB_MISS_16M  : Data TLB miss for 16M page";;
 13)
C1="PM_DERAT_MISS_4K  : DERAT misses for 4K page";
C2="PM_DERAT_MISS_64K  : DERAT misses for 64K page";
C3="PM_DERAT_MISS_16M  : DERAT misses for 16M page";
C4="PM_DERAT_MISS_16G  : DERAT misses for 16G page";;
 14)
C1="PM_INST_CMPL  : Instructions completed";
C2="PM_DERAT_MISS_64K  : DERAT misses for 64K page";
C3="PM_DERAT_MISS_16M  : DERAT misses for 16M page";
C4="PM_DERAT_MISS_16G  : DERAT misses for 16G page";;
 15)
C1="PM_DSLB_MISS  : Data SLB misses";
C2="PM_DATA_FROM_L2MISS  : Data loaded missed L2";
C3="PM_LSU_DERAT_MISS  : DERAT misses";
C4="PM_LD_MISS_L1  : L1 D cache load misses";;
 16)
C1="PM_CYC [shared core] : Processor cycles";
C2="PM_PTEG_FROM_L3MISS  : PTEG loaded from L3 miss";
C3="PM_LSU_DERAT_MISS  : DERAT misses";
C4="PM_RUN_INST_CMPL  : Run instructions completed";;
 17)
C1="PM_CYC [shared core] : Processor cycles";
C2="PM_PTEG_FROM_L3MISS  : PTEG loaded from L3 miss";
C3="PM_LSU_DERAT_MISS  : DERAT misses";
C4="PM_PTEG_FROM_L2MISS  : PTEG loaded from L2 miss";;
 18)
C1="PM_DSLB_MISS  : Data SLB misses";
C2="PM_INST_FROM_L3MISS  : Instruction fetched missed L3";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_RUN_INST_CMPL  : Run instructions completed";;
 19)
C1="PM_IERAT_MISS  : IERAT miss count";
C2="PM_DSLB_MISS  : Data SLB misses";
C3="PM_ISLB_MISS  : Instruction SLB misses";
C4="PM_INST_CMPL  : Instructions completed";;
 20)
C1="PM_PTEG_FROM_L2  : PTEG loaded from L2";
C2="PM_INST_PTEG_FROM_L3  : Instruction PTEG loaded from L3";
C3="PM_PTEG_FROM_L21_MOD  : PTEG loaded from another L2 on same chip modified";
C4="PM_INST_PTEG_FROM_DL2L3_MOD  : Instruction PTEG loaded from distant L2 or L3 modified";;
 21)
C1="PM_INST_PTEG_FROM_L2  : Instruction PTEG loaded from L2";
C2="PM_INST_PTEG_FROM_RL2L3_SHR  : Instruction PTEG loaded from remote L2 or L3 shared";
C3="PM_INST_PTEG_FROM_DL2L3_SHR  : Instruction PTEG loaded from remote L2 or L3 shared";
C4="PM_PTEG_FROM_DL2L3_MOD  : PTEG loaded from distant L2 or L3 modified";;
 22)
C1="PM_PTEG_FROM_L31_MOD  : PTEG loaded from another L3 on same chip modified";
C2="PM_PTEG_FROM_L3MISS  : PTEG loaded from L3 miss";
C3="PM_INST_PTEG_FROM_RMEM  : Instruction PTEG loaded from remote memory";
C4="PM_PTEG_FROM_LMEM  : PTEG loaded from local memory";;
 23)
C1="PM_INST_PTEG_FROM_RL2L3_MOD  : Instruction PTEG loaded from remote L2 or L3 modified";
C2="PM_PTEG_FROM_DMEM  : PTEG loaded from distant memory";
C3="PM_PTEG_FROM_RMEM  : PTEG loaded from remote memory";
C4="PM_PTEG_FROM_LMEM  : PTEG loaded from local memory";;
 24)
C1="PM_PTEG_FROM_RL2L3_MOD  : PTEG loaded from remote L2 or L3 modified";
C2="PM_PTEG_FROM_L31_SHR  : PTEG loaded from another L3 on same chip shared";
C3="PM_PTEG_FROM_DL2L3_SHR  : PTEG loaded from distant L2 or L3 shared";
C4="PM_PTEG_FROM_L21_SHR  : PTEG loaded from another L2 on same chip shared";;
 25)
C1="PM_INST_PTEG_FROM_L31_MOD  : Instruction PTEG loaded from another L3 on same chip modified";
C2="PM_INST_PTEG_FROM_DMEM  : Instruction PTEG loaded from distant memory";
C3="PM_INST_PTEG_FROM_L21_MOD  : Instruction PTEG loaded from another L2 on same chip modified";
C4="PM_INST_PTEG_FROM_LMEM  : Instruction PTEG loaded from local memory";;
 26)
C1="PM_INST_PTEG_FROM_L31_MOD  : Instruction PTEG loaded from another L3 on same chip modified";
C2="PM_INST_PTEG_FROM_L31_SHR  : Instruction PTEG loaded from another L3 on same chip shared";
C3="PM_INST_PTEG_FROM_L21_MOD  : Instruction PTEG loaded from another L2 on same chip modified";
C4="PM_INST_PTEG_FROM_L21_SHR  : Instruction PTEG loaded from another L2 on same chip shared";;
 27)
C1="PM_INST_PTEG_FROM_L2  : Instruction PTEG loaded from L2";
C2="PM_INST_PTEG_FROM_L3MISS  : Instruction PTEG loaded from L3 miss";
C3="PM_INST_PTEG_FROM_RMEM  : Instruction PTEG loaded from remote memory";
C4="PM_INST_PTEG_FROM_L2MISS  : Instruction PTEG loaded from L2 miss";;
 28)
C1="PM_PTEG_FROM_L2  : PTEG loaded from L2";
C2="PM_PTEG_FROM_L3  : PTEG loaded from L3";
C3="PM_PTEG_FROM_RMEM  : PTEG loaded from remote memory";
C4="PM_PTEG_FROM_L2MISS  : PTEG loaded from L2 miss";;
 29)
C1="PM_PTEG_FROM_L2  : PTEG loaded from L2";
C2="PM_PTEG_FROM_L3  : PTEG loaded from L3";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_CYC [shared core] : Processor cycles";;
 30)
C1="PM_PTEG_FROM_RL2L3_MOD  : PTEG loaded from remote L2 or L3 modified";
C2="PM_PTEG_FROM_RL2L3_SHR  : PTEG loaded from remote L2 or L3 shared";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_PTEG_FROM_DL2L3_MOD  : PTEG loaded from distant L2 or L3 modified";;
 31)
C1="PM_INST_CMPL  : Instructions completed";
C2="PM_PTEG_FROM_DMEM  : PTEG loaded from distant memory";
C3="PM_PTEG_FROM_RMEM  : PTEG loaded from remote memory";
C4="PM_PTEG_FROM_LMEM  : PTEG loaded from local memory";;
 32)
C1="PM_POWER_EVENT1  : Power Management 1";
C2="PM_DPU_HELD_POWER  : DISP unit held due to Power Management";
C3="PM_FREQ_DOWN  : Frequency is being slewed down due to Power Management";
C4="PM_FREQ_UP  : Frequency is being slewed up due to Power Management";;
 33)
C1="PM_POWER_EVENT1  : Power Management 1";
C2="PM_DPU_HELD_POWER  : DISP unit held due to Power Management";
C3="PM_DISP_HELD_THERMAL  : Dispatch Held due to Thermal";
C4="PM_FREQ_UP  : Frequency is being slewed up due to Power Management";;
 34)
C1="PM_LD_REF_L1  : L1 D cache load references";
C2="PM_LD_REF_L1_LSU0  : LSU0 L1 D cache load references";
C3="PM_LD_REF_L1_LSU1  : LSU1 L1 D cache load references";
C4="PM_LSU_TWO_TABLEWALK_CYC  : Cycles when two tablewalks pending on this thread";;
 35)
C1="PM_FLUSH_DISP_SYNC  : Dispatch Flush: Sync";
C2="PM_FLUSH_DISP_TLBIE  : Dispatch Flush: TLBIE";
C3="PM_FLUSH_DISP_SB  : Dispatch Flush: Scoreboard";
C4="PM_FLUSH  : Flushes";;
 36)
C1="PM_FLUSH_PARTIAL  : Partial flush";
C2="PM_FLUSH_DISP  : Dispatch flush";
C3="PM_LSU_FLUSH  : Flush initiated by LSU";
C4="PM_LSU_PARTIAL_CDF  : A partial cacheline was returned from the L3";;
 37)
C1="PM_FLUSH_DISP  : Dispatch flush";
C2="PM_CYC [shared core] : Processor cycles";
C3="PM_FLUSH_COMPLETION  : Completion Flush";
C4="PM_FLUSH  : Flushes";;
 38)
C1="PM_LSU_FLUSH_ULD  : LRQ unaligned load flushes";
C2="PM_LSU_FLUSH_UST  : SRQ unaligned store flushes";
C3="PM_LSU_FLUSH_LRQ  : LRQ flushes";
C4="PM_LSU_FLUSH_SRQ  : SRQ flushes";;
 39)
C1="PM_LSU_FLUSH_ULD  : LRQ unaligned load flushes";
C2="PM_LSU0_FLUSH_ULD  : LSU0 unaligned load flushes";
C3="PM_LSU1_FLUSH_ULD  : LSU1 unaligned load flushes";
C4="PM_FLUSH  : Flushes";;
 40)
C1="PM_LSU_FLUSH_UST  : SRQ unaligned store flushes";
C2="PM_LSU0_FLUSH_UST  : LSU0 unaligned store flushes";
C3="PM_LSU1_FLUSH_UST  : LSU1 unaligned store flushes";
C4="PM_FLUSH  : Flushes";;
 41)
C1="PM_LSU_FLUSH_LRQ  : LRQ flushes";
C2="PM_LSU0_FLUSH_LRQ  : LSU0 LRQ flushes";
C3="PM_LSU1_FLUSH_LRQ  : LSU1 LRQ flushes";
C4="PM_FLUSH  : Flushes";;
 42)
C1="PM_LSU_FLUSH_SRQ  : SRQ flushes";
C2="PM_LSU0_FLUSH_SRQ  : LSU0 SRQ lhs flushes";
C3="PM_LSU1_FLUSH_SRQ  : LSU1 SRQ lhs flushes";
C4="PM_FLUSH  : Flushes";;
 43)
C1="PM_IC_DEMAND_CYC  : Cycles when a demand ifetch was pending";
C2="PM_IC_PREF_REQ  : Instruction prefetch requests";
C3="PM_IC_RELOAD_SHR  : I cache line reloading to be shared by threads";
C4="PM_IC_PREF_WRITE  : Instruction prefetch written into I cache";;
 44)
C1="PM_ANY_THRD_RUN_CYC  : One of threads in run_cycles";
C2="PM_THRD_ALL_RUN_CYC [shared chip] : All Threads in run_cycles";
C3="PM_THRD_CONC_RUN_INST [shared chip] : Concurrent Run Instructions";
C4="PM_THRD_4_RUN_CYC [shared chip] : 4 thread in Run Cycles";;
 45)
C1="PM_THRD_GRP_CMPL_BOTH_CYC [shared chip] : Cycles  completed by both threads";
C2="PM_THRD_ALL_RUN_CYC [shared chip] : All Threads in run_cycles";
C3="PM_THRD_CONC_RUN_INST [shared chip] : Concurrent Run Instructions";
C4="PM_THRD_PRIO_0_1_CYC  :  Cycles thread running at priority level 0 or 1";;
 46)
C1="PM_THRD_3_CONC_RUN_INST [shared chip] : 3 thread Concurrent Run Instructions";
C2="PM_THRD_2_RUN_CYC [shared chip] : 2 thread in Run Cycles";
C3="PM_THRD_3_RUN_CYC [shared chip] : 3 thread in Run Cycles";
C4="PM_THRD_PRIO_4_5_CYC  :  Cycles thread running at priority level 4 or 5";;
 47)
C1="PM_THRD_PRIO_2_3_CYC  :  Cycles thread running at priority level 2 or 3";
C2="PM_THRD_4_CONC_RUN_INST [shared chip] : 4 thread Concurrent Run Instructions";
C3="PM_1THRD_CON_RUN_INSTR  : 1 thread Concurrent Run Instructions";
C4="PM_THRD_2_CONC_RUN_INSTR [shared chip] : 2 thread Concurrent Run Instructions";;
 48)
C1="PM_THRD_PRIO_0_1_CYC  :  Cycles thread running at priority level 0 or 1";
C2="PM_THRD_PRIO_2_3_CYC  :  Cycles thread running at priority level 2 or 3";
C3="PM_THRD_PRIO_4_5_CYC  :  Cycles thread running at priority level 4 or 5";
C4="PM_THRD_PRIO_6_7_CYC  :  Cycles thread running at priority level 6 or 7";;
 49)
C1="PM_THRD_1_RUN_CYC [shared chip] : 1 thread in Run Cycles";
C2="PM_THRD_ALL_RUN_CYC [shared chip] : All Threads in run_cycles";
C3="PM_THRD_CONC_RUN_INST [shared chip] : Concurrent Run Instructions";
C4="PM_THRD_4_RUN_CYC [shared chip] : 4 thread in Run Cycles";;
 50)
C1="PM_FXU_IDLE  : FXU idle";
C2="PM_FXU_BUSY  : FXU busy";
C3="PM_FXU0_BUSY_FXU1_IDLE  : FXU0 busy FXU1 idle";
C4="PM_FXU1_BUSY_FXU0_IDLE  : FXU1 busy FXU0 idle";;
 51)
C1="PM_FXU0_FIN  : FXU0 produced a result";
C2="PM_RUN_CYC  : Run cycles";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_FXU1_FIN  : FXU1 produced a result";;
 52)
C1="PM_CYC [shared core] : Processor cycles";
C2="PM_FXU_BUSY  : FXU busy";
C3="PM_FXU0_BUSY_FXU1_IDLE  : FXU0 busy FXU1 idle";
C4="PM_FXU1_BUSY_FXU0_IDLE  : FXU1 busy FXU0 idle";;
 53)
C1="PM_FXU_IDLE  : FXU idle";
C2="PM_FXU_BUSY  : FXU busy";
C3="PM_CYC [shared core] : Processor cycles";
C4="PM_INST_CMPL  : Instructions completed";;
 54)
C1="PM_L2_RCLD_DISP [shared core] :  L2  RC load dispatch attempt";
C2="PM_L2_RCLD_DISP_FAIL_OTHER [shared core] :  L2  RC load dispatch attempt failed due to other reasons";
C3="PM_L2_RCST_DISP [shared core] :  L2  RC store dispatch attempt";
C4="PM_L2_RCLD_BUSY_RC_FULL [shared core] :  L2  activated Busy to the core for loads due to all RC full";;
 55)
C1="PM_L2_CO_FAIL_BUSY [shared core] : L2  RC Cast Out dispatch attempt failed due to all CO machines busy";
C2="PM_CYC [shared core] : Processor cycles";
C3="PM_L2_RC_ST_DONE [shared core] : RC did st to line that was Tx or Sx";
C4="PM_INST_CMPL  : Instructions completed";;
 56)
C1="PM_L2_RCLD_DISP [shared core] :  L2  RC load dispatch attempt";
C2="PM_L2_RCLD_DISP_FAIL_OTHER [shared core] :  L2  RC load dispatch attempt failed due to other reasons";
C3="PM_L2_RCST_DISP_FAIL_ADDR [shared core] :  L2  RC store dispatch attempt failed due to address collision with RC/CO/SN/SQ";
C4="PM_L2_RCST_DISP_FAIL_OTHER [shared core] :  L2  RC store dispatch attempt failed due to other reasons";;
 57)
C1="PM_L2_ST  : Data Store Count";
C2="PM_L2_LD_MISS  : Data Load Miss";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_CYC [shared core] : Processor cycles";;
 58)
C1="PM_INST_CMPL  : Instructions completed";
C2="PM_CYC [shared core] : Processor cycles";
C3="PM_L2_LD_HIT [shared core] : All successful load dispatches that were L2 hits";
C4="PM_L2_ST_HIT [shared core] : All successful store dispatches that were L2Hits";;
 59)
C1="PM_INST_CMPL  : Instructions completed";
C2="PM_CYC [shared core] : Processor cycles";
C3="PM_L2_LD_DISP [shared core] : All successful load dispatches";
C4="PM_L2_ST_DISP [shared core] : All successful store dispatches";;
 60)
C1="PM_L2_RCLD_DISP_FAIL_ADDR [shared core] :  L2  RC load dispatch attempt failed due to address collision with RC/CO/SN/SQ";
C2="PM_L2_RCST_BUSY_RC_FULL [shared core] :  L2  activated Busy to the core for stores due to all RC full";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_CYC [shared core] : Processor cycles";;
 61)
C1="PM_PB_NODE_PUMP [shared chip] :  Nest events (MC0/MC1/PB/GX), Pair0 Bit0";
C2="PM_PB_SYS_PUMP [shared chip] :  Nest events (MC0/MC1/PB/GX), Pair1 Bit0";
C3="PM_PB_RETRY_NODE_PUMP [shared chip] :  Nest events (MC0/MC1/PB/GX), Pair2 Bit0";
C4="PM_PB_RETRY_SYS_PUMP [shared chip] :  Nest events (MC0/MC1/PB/GX), Pair3 Bit0";;
 62)
C1="PM_MEM0_RQ_DISP [shared chip] :  Nest events (MC0/MC1/PB/GX), Pair0 Bit1";
C2="PM_MEM0_PREFETCH_DISP [shared chip] :  Nest events (MC0/MC1/PB/GX), Pair1 Bit1";
C3="PM_MEM0_RD_CANCEL_TOTAL [shared chip] :  Nest events (MC0/MC1/PB/GX), Pair2 Bit1";
C4="PM_MEM0_WQ_DISP [shared chip] :  Nest events (MC0/MC1/PB/GX), Pair3 Bit1";;
 63)
C1="PM_NEST_PAIR0_ADD [shared chip] : Nest events (MC0/MC1/PB/GX), Pair0 ADD";
C2="PM_NEST_PAIR1_ADD [shared chip] : Nest events (MC0/MC1/PB/GX), Pair1 ADD";
C3="PM_NEST_PAIR2_ADD [shared chip] : Nest events (MC0/MC1/PB/GX), Pair2 ADD";
C4="PM_NEST_PAIR3_ADD [shared chip] : Nest events (MC0/MC1/PB/GX), Pair3 ADD";;
 64)
C1="PM_NEST_PAIR0_AND [shared chip] : Nest events (MC0/MC1/PB/GX), Pair0 AND";
C2="PM_NEST_PAIR1_AND [shared chip] : Nest events (MC0/MC1/PB/GX), Pair1 AND";
C3="PM_NEST_PAIR2_AND [shared chip] : Nest events (MC0/MC1/PB/GX), Pair2 AND";
C4="PM_NEST_PAIR3_AND [shared chip] : Nest events (MC0/MC1/PB/GX), Pair3 AND";;
 65)
C1="PM_IC_DEMAND_L2_BHT_REDIRECT  : L2 I cache demand request due to BHT redirect";
C2="PM_IC_DEMAND_L2_BR_REDIRECT  : L2 I cache demand request due to branch redirect";
C3="PM_IC_DEMAND_REQ  : Demand Instruction fetch request";
C4="PM_IC_BANK_CONFLICT  : Read blocked due to interleave conflict";;
 66)
C1="PM_DATA_FROM_L2  : Data loaded from L2";
C2="PM_INST_DISP  : Instructions dispatched";
C3="PM_L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="PM_1PLUS_PPC_DISP  : Cycles at least one instruction dispatched";;
 67)
C1="PM_DATA_FROM_L3  : Data loaded from L3";
C2="PM_CYC [shared core] : Processor cycles";
C3="PM_L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="PM_INST_CMPL  : Instructions completed";;
 68)
C1="PM_DATA_FROM_RL2L3_MOD  : Data loaded from remote L2 or L3 modified";
C2="PM_DATA_FROM_RL2L3_SHR  : Data loaded from remote L2 or L3 shared";
C3="PM_L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="PM_INST_CMPL  : Instructions completed";;
 69)
C1="PM_LSU_REJECT  : LSU reject";
C2="PM_LSU0_REJECT_LHS  : LSU0 load hit store reject";
C3="PM_LSU1_REJECT_LHS  : LSU1 load hit store reject";
C4="PM_LSU_REJECT_LHS  : Load hit store reject";;
 70)
C1="PM_LSU_REJECT  : LSU reject";
C2="PM_LSU_REJECT_ERAT_MISS  : LSU reject due to ERAT miss";
C3="PM_LSU_REJECT_SET_MPRED  : LSU reject due to mispredicted set";
C4="PM_LSU_SRQ_EMPTY_CYC  : Cycles SRQ empty";;
 71)
C1="PM_LSU_REJECT_SET_MPRED  : LSU reject due to mispredicted set";
C2="PM_LSU_SET_MPRED  : Line already in cache at reload time";
C3="PM_CYC [shared core] : Processor cycles";
C4="PM_INST_CMPL  : Instructions completed";;
 72)
C1="PM_LSU_REJECT_LMQ_FULL  : LSU reject due to LMQ full or missed data coming";
C2="PM_LSU0_REJECT_LMQ_FULL  : LSU0 reject due to LMQ full or missed data coming";
C3="PM_LSU1_REJECT_LMQ_FULL  : LSU1 reject due to LMQ full or missed data coming";
C4="PM_INST_CMPL  : Instructions completed";;
 73)
C1="PM_LSU_NCLD  : Non-cacheable load";
C2="PM_LSU0_NCLD  : LSU0 non-cacheable loads";
C3="PM_LSU1_NCLD  : LSU1 non-cacheable loads";
C4="PM_INST_CMPL  : Instructions completed";;
 74)
C1="PM_GCT_NOSLOT_CYC  : Cycles no GCT slot allocated";
C2="PM_GCT_EMPTY_CYC [shared core] : Cycles GCT empty";
C3="PM_GCT_FULL_CYC [shared core] : Cycles GCT full";
C4="PM_CYC [shared core] : Processor cycles";;
 75)
C1="PM_GCT_UTIL_1_TO_2_SLOTS  : GCT Utilization 1-2 entries";
C2="PM_GCT_UTIL_3_TO_6_SLOTS  : GCT Utilization 3-6 entries";
C3="PM_GCT_UTIL_7_TO_10_SLOTS  : GCT Utilization 7-10 entries";
C4="PM_GCT_UTIL_11_PLUS_SLOTS  : GCT Utilization 11+ entries";;
 76)
C1="PM_L2_CASTOUT_MOD [shared core] : L2 castouts - Modified (M";
C2="PM_L2_DC_INV [shared core] : Dcache invalidates from L2";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_CYC [shared core] : Processor cycles";;
 77)
C1="PM_L2_CASTOUT_SHR [shared core] : L2 castouts - Shared (T";
C2="PM_L2_IC_INV [shared core] : Icache Invalidates from L2";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_CYC [shared core] : Processor cycles";;
 78)
C1="PM_DISP_HELD  : Dispatch Held";
C2="PM_DPU_HELD_POWER  : DISP unit held due to Power Management";
C3="PM_DISP_HELD_THERMAL  : Dispatch Held due to Thermal";
C4="PM_1PLUS_PPC_DISP  : Cycles at least one instruction dispatched";;
 79)
C1="PM_THERMAL_WARN [shared chip] : Processor in Thermal Warning";
C2="PM_DPU_HELD_POWER  : DISP unit held due to Power Management";
C3="PM_DISP_HELD_THERMAL  : Dispatch Held due to Thermal";
C4="PM_THERMAL_MAX [shared chip] : Processor in thermal MAX";;
 80)
C1="PM_DISP_CLB_HELD_BAL  : Dispatch/CLB Hold: Balance";
C2="PM_DISP_CLB_HELD_RES  : Dispatch/CLB Hold: Resource";
C3="PM_DISP_CLB_HELD_TLBIE  : Dispatch Hold: Due to TLBIE";
C4="PM_DISP_CLB_HELD_SYNC  : Dispatch/CLB Hold: Sync type instruction";;
 81)
C1="PM_POWER_EVENT1  : Power Management 1";
C2="PM_POWER_EVENT2  : Power Management 2";
C3="PM_POWER_EVENT3  : Power Management 3";
C4="PM_POWER_EVENT4  : Power Management 4";;
 82)
C1="PM_1PLUS_PPC_CMPL  : One or more PPC instruction completed";
C2="PM_INST_DISP  : Instructions dispatched";
C3="PM_GRP_DISP : Group dispatches";
C4="PM_1PLUS_PPC_DISP  : Cycles at least one instruction dispatched";;
 83)
C1="PM_1PLUS_PPC_CMPL  : One or more PPC instruction completed";
C2="PM_CYC [shared core] : Processor cycles";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_1PLUS_PPC_DISP  : Cycles at least one instruction dispatched";;
 84)
C1="PM_IC_REQ_ALL  : Icache requests, prefetch + demand";
C2="PM_IC_WRITE_ALL  : Icache sectors written, prefetch + demand";
C3="PM_IC_PREF_CANCEL_ALL  : Prefetch Canceled due to page boundary or icache hit";
C4="PM_IC_DEMAND_L2_BR_ALL  : L2 I cache demand request due to BHT or redirect";;
 85)
C1="PM_IC_PREF_CANCEL_PAGE  : Prefetch Canceled due to page boundary";
C2="PM_IC_PREF_CANCEL_HIT  : Prefetch Canceled due to icache hit";
C3="PM_IC_PREF_CANCEL_L2  : L2 Squashed request";
C4="PM_IC_PREF_CANCEL_ALL  : Prefetch Canceled due to page boundary or icache hit";;
 86)
C1="PM_IERAT_MISS  : IERAT miss count";
C2="PM_L1_ICACHE_MISS  : L1 I cache miss count";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_CYC [shared core] : Processor cycles";;
 87)
C1="PM_DATA_FROM_L2  : Data loaded from L2";
C2="PM_CMPLU_STALL_DCACHE_MISS  : Completion stall caused by D cache miss";
C3="PM_L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="PM_CMPLU_STALL_ERAT_MISS  : Completion stall caused by ERAT miss";;
 88)
C1="PM_FXU_IDLE  : FXU idle";
C2="PM_CMPLU_STALL_FXU  : Completion stall caused by FXU instruction";
C3="PM_GRP_CMPL  : Group completed";
C4="PM_CMPLU_STALL_DIV  : Completion stall caused by DIV instruction";;
 89)
C1="PM_TABLEWALK_CYC  : Cycles when a tablewalk (I or D) is active";
C2="PM_CMPLU_STALL_LSU  : Completion stall caused by LSU instruction";
C3="PM_DATA_TABLEWALK_CYC  : Cycles doing data tablewalks";
C4="PM_CMPLU_STALL_REJECT  : Completion stall caused by reject";;
 90)
C1="PM_FLOP  : Floating Point Operation Finished";
C2="PM_CMPLU_STALL_SCALAR_LONG  : Completion stall caused by long latency scalar instruction";
C3="PM_MRK_STALL_CMPLU_CYC [marked] : Marked Group Completion Stall cycles";
C4="PM_CMPLU_STALL_SCALAR  : Completion stall caused by FPU instruction";;
 91)
C1="PM_CMPLU_STALL_END_GCT_NOSLOT  : Count ended because GCT went empty";
C2="PM_CMPLU_STALL_VECTOR  : Completion stall caused by Vector instruction";
C3="PM_MRK_STALL_CMPLU_CYC_COUNT [marked] : Marked Group Completion Stall cycles";
C4="PM_CMPLU_STALL  : No s completed, GCT not empty";;
 92)
C1="PM_CMPLU_STALL_THRD : Completion Stalled due to thread conflict."
C2="PM_CMPLU_STALL_DFU  : Completion stall caused by Decimal Floating Point Unit";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_GCT_NOSLOT_BR_MPRED_IC_MISS  : GCT empty by branch  mispredict + IC miss";;
 93)
C1="PM_GCT_NOSLOT_CYC  : Cycles no GCT slot allocated";
C2="PM_GCT_NOSLOT_IC_MISS  : No slot in GCT caused by I cache miss";
C3="PM_IOPS_DISP  : IOPS dispatched";
C4="PM_GCT_NOSLOT_BR_MPRED  : No slot in GCT caused by branch mispredict";;
 94)
C1="PM_DATA_FROM_L2  : Data loaded from L2";
C2="PM_DATA_FROM_L3  : Data loaded from L3";
C3="PM_DATA_FROM_RMEM  : Data loaded from remote memory";
C4="PM_DATA_FROM_LMEM  : Data loaded from local memory";;
 95)
C1="PM_DATA_FROM_L3  : Data loaded from L3";
C2="PM_DATA_FROM_L31_SHR  : Data loaded from another L3 on same chip shared";
C3="PM_DATA_FROM_LMEM  : Data loaded from local memory";
C4="PM_DATA_FROM_L2MISS  : Data loaded missed L2";;
 96)
C1="PM_DATA_FROM_DMEM  : Data loaded from distant memory";
C2="PM_DATA_FROM_L3MISS  : Data loaded from private L3 miss";
C3="PM_DATA_FROM_L21_MOD  : Data loaded from another L2 on same chip modified";
C4="PM_DATA_FROM_L2MISS  : Data loaded missed L2";;
 97)
C1="PM_DATA_FROM_L31_MOD  : Data loaded from another L3 on same chip modified";
C2="PM_DATA_FROM_RL2L3_SHR  : Data loaded from remote L2 or L3 shared";
C3="PM_DATA_FROM_DL2L3_MOD  : Data loaded from distant L2 or L3 modified";
C4="PM_DATA_FROM_DL2L3_MOD  : Data loaded from distant L2 or L3 modified";;
 98)
C1="PM_DATA_FROM_L31_SHR  : Data loaded from another L3 on same chip shared";
C2="PM_DATA_FROM_DMEM  : Data loaded from distant memory";
C3="PM_DATA_FROM_DL2L3_SHR  : Data loaded from distant L2 or L3 shared";
C4="PM_DATA_FROM_L21_SHR  : Data loaded from another L2 on same chip shared";;
 99)
C1="PM_DATA_FROM_RL2L3_MOD  : Data loaded from remote L2 or L3 modified";
C2="PM_DATA_FROM_RL2L3_SHR  : Data loaded from remote L2 or L3 shared";
C3="PM_DATA_FROM_L21_SHR  : Data loaded from another L2 on same chip shared";
C4="PM_DATA_FROM_L2MISS  : Data loaded missed L2";;
 100)
C1="PM_DATA_FROM_RL2L3_SHR  : Data loaded from remote L2 or L3 shared";
C2="PM_DATA_FROM_L3MISS  : Data loaded from private L3 miss";
C3="PM_DATA_FROM_DL2L3_MOD  : Data loaded from distant L2 or L3 modified";
C4="PM_DATA_FROM_DL2L3_MOD  : Data loaded from distant L2 or L3 modified";;
 101)
C1="PM_INST_CMPL  : Instructions completed";
C2="PM_DATA_FROM_L3  : Data loaded from L3";
C3="PM_DATA_FROM_L3MISS  : Data loaded from private L3 miss";
C4="PM_DATA_FROM_LMEM  : Data loaded from local memory";;
 102)
C1="PM_DATA_FROM_L2  : Data loaded from L2";
C2="PM_DATA_FROM_L2MISS  : Data loaded missed L2";
C3="PM_L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="PM_LD_MISS_L1  : L1 D cache load misses";;
 103)
C1="PM_DATA_FROM_RL2L3_MOD  : Data loaded from remote L2 or L3 modified";
C2="PM_DATA_FROM_RL2L3_SHR  : Data loaded from remote L2 or L3 shared";
C3="PM_DATA_FROM_DL2L3_SHR  : Data loaded from distant L2 or L3 shared";
C4="PM_DATA_FROM_DL2L3_MOD  : Data loaded from distant L2 or L3 modified";;
 104)
C1="PM_DATA_FROM_L2  : Data loaded from L2";
C2="PM_DATA_FROM_L2MISS  : Data loaded missed L2";
C3="PM_DATA_FROM_L3MISS  : Data loaded from private L3 miss";
C4="PM_RUN_INST_CMPL  : Run instructions completed";;
 105)
C1="PM_DATA_FROM_RL2L3_MOD  : Data loaded from remote L2 or L3 modified";
C2="PM_DATA_FROM_DMEM  : Data loaded from distant memory";
C3="PM_DATA_FROM_RMEM  : Data loaded from remote memory";
C4="PM_DATA_FROM_LMEM  : Data loaded from local memory";;
 106)
C1="PM_DERAT_MISS_4K  : DERAT misses for 4K page";
C2="PM_INST_CMPL  : Instructions completed";
C3="PM_DATA_FROM_DL2L3_SHR  : Data loaded from distant L2 or L3 shared";
C4="PM_DATA_FROM_DL2L3_MOD  : Data loaded from distant L2 or L3 modified";;
 107) 
C1="PM_DATA_FROM_DMEM  : Data loaded from distant memory";
C2="PM_INST_CMPL  : Instructions completed";
C3="PM_DATA_FROM_RMEM  : Data loaded from remote memory";
C4="PM_DATA_FROM_LMEM  : Data loaded from local memory";;
 108)
C1="PM_DATA_FROM_DMEM  : Data loaded from distant memory";
C2="PM_INST_CMPL  : Instructions completed";
C3="PM_L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="PM_DATA_FROM_LMEM  : Data loaded from local memory";;
 109)
C1="PM_INST_FROM_L2  : Instruction fetched from L2";
C2="PM_INST_FROM_L3  : Instruction fetched from L3";
C3="PM_INST_FROM_LMEM  : Instruction fetched from local memory";
C4="PM_INST_FROM_L2MISS  : Instruction fetched missed L2";;
 110)
C1="PM_INST_FROM_L3  : Instruction fetched from L3";
C2="PM_INST_FROM_DMEM  : Instruction fetched from distant memory";
C3="PM_INST_FROM_DL2L3_MOD  : Instruction fetched from distant L2 or L3 modified";
C4="PM_INST_FROM_LMEM  : Instruction fetched from local memory";;
 111)
C1="PM_INST_FROM_DMEM  : Instruction fetched from distant memory";
C2="PM_INST_FROM_L3MISS  : Instruction fetched missed L3";
C3="PM_INST_FROM_DL2L3_SHR  : Instruction fetched from distant L2 or L3 shared";
C4="PM_INST_FROM_DL2L3_MOD  : Instruction fetched from distant L2 or L3 modified";;
 112)
C1="PM_INST_FROM_L31_MOD  : Instruction fetched from another L3 on same chip modified";
C2="PM_INST_FROM_L31_SHR  : Instruction fetched from another L3 on same chip shared";
C3="PM_INST_FROM_L21_MOD  : Instruction fetched from another L2 on same chip modified";
C4="PM_INST_FROM_L21_SHR  : Instruction fetched from another L2 on same chip shared";;
 113)
C1="PM_INST_FROM_L31_SHR  : Instruction fetched from another L3 on same chip shared";
C2="PM_INST_FROM_RL2L3_SHR  : Instruction fetched from remote L2 or L3 shared";
C3="PM_INST_FROM_L21_SHR  : Instruction fetched from another L2 on same chip shared";
C4="PM_INST_FROM_L2MISS  : Instruction fetched missed L2";;
 114)
C1="PM_INST_FROM_PREF  : Instruction fetched from prefetch";
C2="PM_INST_FROM_L3MISS  : Instruction fetched missed L3";
C3="PM_INST_FROM_LMEM  : Instruction fetched from local memory";
C4="PM_INST_FROM_L2MISS  : Instruction fetched missed L2";;
 115)
C1="PM_INST_FROM_RL2L3_MOD  : Instruction fetched from remote L2 or L3 modified";
C2="PM_INST_FROM_RL2L3_SHR  : Instruction fetched from remote L2 or L3 shared";
C3="PM_INST_FROM_DL2L3_SHR  : Instruction fetched from distant L2 or L3 shared";
C4="PM_INST_FROM_DL2L3_MOD  : Instruction fetched from distant L2 or L3 modified";;
 116)
C1="PM_INST_FROM_RL2L3_SHR  : Instruction fetched from remote L2 or L3 shared";
C2="PM_INST_FROM_L3MISS  : Instruction fetched missed L3";
C3="PM_INST_FROM_LMEM  : Instruction fetched from local memory";
C4="PM_INST_FROM_L2MISS  : Instruction fetched missed L2";;
 117)
C1="PM_INST_FROM_PREF  : Instruction fetched from prefetch";
C2="PM_INST_FROM_DMEM  : Instruction fetched from distant memory";
C3="PM_INST_FROM_RMEM  : Instruction fetched from remote memory";
C4="PM_INST_FROM_LMEM  : Instruction fetched from local memory";;
 118)
C1="PM_INST_FROM_L2  : Instruction fetched from L2";
C2="PM_INST_FROM_L3  : Instruction fetched from L3";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_CYC [shared core] : Processor cycles";;
 119)
C1="PM_INST_FROM_RL2L3_MOD  : Instruction fetched from remote L2 or L3 modified";
C2="PM_INST_FROM_RL2L3_SHR  : Instruction fetched from remote L2 or L3 shared";
C3="PM_INST_FROM_LMEM  : Instruction fetched from local memory";
C4="PM_INST_CMPL  : Instructions completed";;
 120)
C1="PM_CYC [shared core] : Processor cycles";
C2="PM_INST_CMPL  : Instructions completed";
C3="PM_INST_FROM_DL2L3_SHR  : Instruction fetched from distant L2 or L3 shared";
C4="PM_INST_FROM_DL2L3_MOD  : Instruction fetched from distant L2 or L3 modified";;
 121)
C1="PM_INST_FROM_DMEM  : Instruction fetched from distant memory";
C2="PM_INST_CMPL  : Instructions completed";
C3="PM_INST_FROM_RMEM  : Instruction fetched from remote memory";
C4="PM_INST_FROM_LMEM  : Instruction fetched from local memory";;
 122)
C1="PM_LSU_DC_PREF_STREAM_ALLOC  : D cache new prefetch stream allocated";
C2="PM_L3_PREF_LDST  : L3 cache prefetches LD + ST";
C3="PM_LSU_DC_PREF_STREAM_CONFIRM  : Dcache new prefetch stream confirmed";
C4="PM_L1_PREF  : L1 cache data prefetches";;
 123)
C1="PM_LSU_DC_PREF_STRIDED_STREAM_CONFIRM  : Dcache Strided prefetch stream confirmed (software + hardware)";
C2="PM_LD_REF_L1  : L1 D cache load references";
C3="PM_LSU_FIN  : LSU Finished an instruction (up to 2 per cycle)";
C4="PM_LD_MISS_L1  : L1 D cache load misses";;
 124)
C1="PM_VSU0_1FLOP  : one flop (fadd, fmul, fsub, fcmp, fsel, fabs, fnabs, fres, fsqrte, fneg, xsadd, xsmul, xssub, xscmp, xssel, xsabs, xsnabs, xsre, xssqrte, xsneg) operation finished";
C2="PM_VSU1_1FLOP  : one flop (fadd, fmul, fsub, fcmp, fsel, fabs, fnabs, fres, fsqrte, fneg, xsadd, xsmul, xssub, xscmp, xssel, xsabs, xsnabs, xsre, xssqrte, xsneg) operation finished";
C3="PM_VSU0_2FLOP  : two flops operation (scalar fmadd, fnmadd, fmsub, fnmsub and DP vector versions of single flop instructions)";
C4="PM_VSU1_2FLOP  : two flops operation (scalar fmadd, fnmadd, fmsub, fnmsub and DP vector versions of single flop instructions)";;
 125)
C1="PM_VSU0_4FLOP  : four flops operation (scalar fdiv, fsqrt; DP vector version of fmadd, fnmadd, fmsub, fnmsub; SP vector versions of single flop instructions)";
C2="PM_VSU1_4FLOP  : four flops operation (scalar fdiv, fsqrt; DP vector version of fmadd, fnmadd, fmsub, fnmsub; SP vector versions of single flop instructions)";
C3="PM_VSU0_8FLOP  : eight flops operation (DP vector versions of fdiv,fsqrt and SP vector versions of fmadd,fnmadd,fmsub,fnmsub)";
C4="PM_VSU1_8FLOP  : eight flops operation (DP vector versions of fdiv,fsqrt and SP vector versions of fmadd,fnmadd,fmsub,fnmsub)";;
 126)
C1="PM_VSU_2FLOP  : two flops operation (scalar fmadd, fnmadd, fmsub, fnmsub and DP vector versions of single flop instructions)";
C2="PM_VSU_2FLOP_DOUBLE  : DP vector version of fmul, fsub, fcmp, fsel, fabs, fnabs, fres ,fsqrte, fneg";
C3="PM_VSU0_2FLOP_DOUBLE  : two flop DP vector operation (xvadddp, xvmuldp, xvsubdp, xvcmpdp, xvseldp, xvabsdp, xvnabsdp, xvredp ,xvsqrtedp, vxnegdp)";
C4="PM_VSU1_2FLOP_DOUBLE  : two flop DP vector operation (xvadddp, xvmuldp, xvsubdp, xvcmpdp, xvseldp, xvabsdp, xvnabsdp, xvredp ,xvsqrtedp, vxnegdp)";;
 127)
C1="PM_VSU0_FMA  : two flops operation (fmadd, fnmadd, fmsub, fnmsub, xsmadd, xsnmadd, xsmsub, xsnmsub) Scalar instructions only!";
C2="PM_VSU1_FMA  : two flops operation (fmadd, fnmadd, fmsub, fnmsub, xsmadd, xsnmadd, xsmsub, xsnmsub) Scalar instructions only!";
C3="PM_VSU_FMA  : two flops operation (fmadd, fnmadd, fmsub, fnmsub) Scalar instructions only!";
C4="PM_INST_CMPL  : Instructions completed";;
 128)
C1="PM_VSU0_FMA_DOUBLE  : four flop DP vector operations (xvmadddp, xvnmadddp, xvmsubdp, xvmsubdp)";
C2="PM_VSU1_FMA_DOUBLE  : four flop DP vector operations (xvmadddp, xvnmadddp, xvmsubdp, xvmsubdp)";
C3="PM_VSU_FMA_DOUBLE  : DP vector version of fmadd,fnmadd,fmsub,fnmsub";
C4="PM_INST_CMPL  : Instructions completed";;
 129)
C1="PM_VSU_VECTOR_DOUBLE_ISSUED  : Double Precision vector instruction issued on Pipe0";
C2="PM_VSU0_VECT_DOUBLE_ISSUED  : Double Precision vector instruction issued on Pipe0";
C3="PM_VSU1_VECT_DOUBLE_ISSUED  : Double Precision vector instruction issued on Pipe1";
C4="PM_INST_CMPL  : Instructions completed";;
 130)
C1="PM_VSU_DENORM  : Vector or Scalar denorm operand";
C2="PM_VSU0_DENORM  : FPU denorm operand";
C3="PM_VSU1_DENORM  : FPU denorm operand";
C4="PM_INST_CMPL  : Instructions completed";;
 131)
C1="PM_VSU_FIN  : VSU0 Finished an instruction";
C2="PM_VSU0_FIN  : VSU0 Finished an instruction";
C3="PM_VSU1_FIN  : VSU1 Finished an instruction";
C4="PM_INST_CMPL  : Instructions completed";;
 132)
C1="PM_VSU_STF  : FPU store (SP or DP) issued on Pipe0";
C2="PM_VSU0_STF  : FPU store (SP or DP) issued on Pipe0";
C3="PM_VSU1_STF  : FPU store (SP or DP) issued on Pipe1";
C4="PM_INST_CMPL  : Instructions completed";;
 133)
C1="PM_VSU_SINGLE  : Vector or Scalar single precision";
C2="PM_VSU0_SINGLE  : FPU single precision";
C3="PM_VSU1_SINGLE  : FPU single precision";
C4="PM_VSU0_16FLOP  : Sixteen flops operation (SP vector versions of fdiv,fsqrt)";;
 134)
C1="PM_VSU_FSQRT_FDIV  : four flops operation (fdiv,fsqrt) Scalar Instructions only!";
C2="PM_VSU0_FSQRT_FDIV  : four flops operation (fdiv,fsqrt,xsdiv,xssqrt) Scalar Instructions only!";
C3="PM_VSU1_FSQRT_FDIV  : four flops operation (fdiv,fsqrt,xsdiv,xssqrt) Scalar Instructions only!";
C4="PM_INST_CMPL  : Instructions completed";;
 135)
C1="PM_VSU_FSQRT_FDIV_DOUBLE  : DP vector versions of fdiv,fsqrt";
C2="PM_VSU0_FSQRT_FDIV_DOUBLE  : eight flop DP vector operations (xvfdivdp, xvsqrtdp";
C3="PM_VSU1_FSQRT_FDIV_DOUBLE  : eight flop DP vector operations (xvfdivdp, xvsqrtdp";
C4="PM_INST_CMPL  : Instructions completed";;
 136)
C1="PM_VSU_SCALAR_DOUBLE_ISSUED  : Double Precision scalar instruction issued on Pipe0";
C2="PM_VSU0_SCAL_DOUBLE_ISSUED  : Double Precision scalar instruction issued on Pipe0";
C3="PM_VSU1_SCAL_DOUBLE_ISSUED  : Double Precision scalar instruction issued on Pipe1";
C4="PM_INST_CMPL  : Instructions completed";;
 137)
C1="PM_VSU_SCALAR_SINGLE_ISSUED  : Single Precision scalar instruction issued on Pipe0";
C2="PM_VSU0_SCAL_SINGLE_ISSUED  : Single Precision scalar instruction issued on Pipe0";
C3="PM_VSU1_SCAL_SINGLE_ISSUED  : Single Precision scalar instruction issued on Pipe1";
C4="PM_INST_CMPL  : Instructions completed";;
 138)
C1="PM_VSU_1FLOP  : one flop (fadd, fmul, fsub, fcmp, fsel, fabs, fnabs, fres, fsqrte, fneg) operation finished";
C2="PM_VSU_4FLOP  : four flops operation (scalar fdiv, fsqrt; DP vector version of fmadd, fnmadd, fmsub, fnmsub; SP vector versions of single flop instructions)";
C3="PM_VSU_8FLOP  : eight flops operation (DP vector versions of fdiv,fsqrt and SP vector versions of fmadd,fnmadd,fmsub,fnmsub)";
C4="PM_VSU_2FLOP  : two flops operation (scalar fmadd, fnmadd, fmsub, fnmsub and DP vector versions of single flop instructions)";;
 139)
C1="PM_VSU_VECTOR_SINGLE_ISSUED  : Single Precision vector instruction issued (executed)";
C2="PM_VSU0_VECTOR_SP_ISSUED  : Single Precision vector instruction issued (executed)";
C3="PM_VSU0_FPSCR  : Move to/from FPSCR type instruction issued on Pipe 0";
C4="PM_INST_CMPL  : Instructions completed";;
 140)
C1="PM_VSU_SIMPLE_ISSUED  : Simple VMX instruction issued";
C2="PM_VSU0_SIMPLE_ISSUED  : Simple VMX instruction issued";
C3="PM_VSU0_COMPLEX_ISSUED  : Complex VMX instruction issued";
C4="PM_VMX_RESULT_SAT_1  : VMX valid result with sat=1";;
 141)
C1="PM_VSU1_DD_ISSUED  : 64BIT Decimal Issued on Pipe1";
C2="PM_VSU1_DQ_ISSUED  : 128BIT Decimal Issued on Pipe1";
C3="PM_VSU1_PERMUTE_ISSUED  : Permute VMX Instruction Issued";
C4="PM_VSU1_SQ  : Store Vector Issued on Pipe1";;
 142)
C1="PM_VSU_FCONV  : Convert instruction executed";
C2="PM_VSU0_FCONV  : Convert instruction executed";
C3="PM_VSU1_FCONV  : Convert instruction executed";
C4="PM_INST_CMPL  : Instructions completed";;
 143)
C1="PM_VSU_FRSP  : Round to single precision instruction executed";
C2="PM_VSU0_FRSP  : Round to single precision instruction executed";
C3="PM_VSU1_FRSP  : Round to single precision instruction executed";
C4="PM_INST_CMPL  : Instructions completed";;
 144)
C1="PM_VSU_FEST  : Estimate instruction executed";
C2="PM_VSU0_FEST  : Estimate instruction executed";
C3="PM_VSU1_FEST  : Estimate instruction executed";
C4="PM_INST_CMPL  : Instructions completed";;
 145)
C1="PM_BRU_FIN  : BRU produced a result";
C2="PM_RUN_CYC  : Run cycles";
C3="PM_INST_CMPL  : Instructions completed";
C4="PM_VSU_FIN  : VSU0 Finished an instruction";;
 146)
C1="PM_LSU_LDF  : LSU executed Floating Point load instruction";
C2="PM_VSU_STF  : FPU store (SP or DP) issued on Pipe0";
C3="PM_VSU_FMA  : two flops operation (fmadd, fnmadd, fmsub, fnmsub) Scalar instructions only!";
C4="PM_VSU_1FLOP  : one flop (fadd, fmul, fsub, fcmp, fsel, fabs, fnabs, fres, fsqrte, fneg) operation finished";;
 147)
C1="PM_VSU_FSQRT_FDIV  : four flops operation (fdiv,fsqrt) Scalar Instructions only!";
C2="PM_VSU_FIN  : VSU0 Finished an instruction";
C3="PM_VSU_FMA  : two flops operation (fmadd, fnmadd, fmsub, fnmsub) Scalar instructions only!";
C4="PM_VSU_1FLOP  : one flop (fadd, fmul, fsub, fcmp, fsel, fabs, fnabs, fres, fsqrte, fneg) operation finished";;
 148)
C1="PM_FLOP  : Floating Point Operation Finished";
C2="PM_VSU_FIN  : VSU0 Finished an instruction";
C3="PM_VSU_FEST  : Estimate instruction executed";
C4="PM_VSU_1FLOP  : one flop (fadd, fmul, fsub, fcmp, fsel, fabs, fnabs, fres, fsqrte, fneg) operation finished";;
 149)
C1="PM_VSU_STF  : FPU store (SP or DP) issued on Pipe0";
C2="PM_VSU_FIN  : VSU0 Finished an instruction";
C3="PM_VSU_FRSP  : Round to single precision instruction executed";
C4="PM_VSU_FCONV  : Convert instruction executed";;
 150)
C1="PM_LSU_LMQ_FULL_CYC  : Cycles LMQ full";
C2="PM_LSU_LMQ_SRQ_EMPTY_CYC [shared chip] : Cycles LMQ and SRQ empty";
C3="PM_LSU_LMQ_SRQ_EMPTY_ALL_CYC [shared chip] : ALL threads lsu empty (lmq and srq empty)";
C4="PM_LSU_SRQ_EMPTY_CYC  : Cycles SRQ empty";;
 151)
C1="PM_LSU_FX_FIN  : LSU Finished a FX operation  (up to 2 per cycle)";
C2="PM_LSU_NCST  : Non-cacheable store";
C3="PM_LSU_FIN  : LSU Finished an instruction (up to 2 per cycle)";
C4="PM_LSU_FLUSH  : Flush initiated by LSU";;
 152)
C1="PM_LSU0_LMQ_LHR_MERGE  : LS0  Load Merged with another cacheline request";
C2="PM_LSU1_LMQ_LHR_MERGE  : LS1 Load Merge with another cacheline request";
C3="PM_LSU_LMQ_S0_VALID  : LMQ slot 0 valid";
C4="PM_LSU_LMQ_FULL_CYC  : Cycles LMQ full";;
 153)
C1="PM_LSU_SRQ_STFWD  : SRQ store forwarded";
C2="PM_LSU0_SRQ_STFWD  : LSU0 SRQ store forwarded";
C3="PM_LSU1_SRQ_STFWD  : LSU1 SRQ store forwarded";
C4="PM_INST_CMPL  : Instructions completed";;
 154)
C1="LSU_SRQ_SYNC_CYC  : SRQ sync duration";
C2="LSU_SRQ_SYNC_COUNT  : SRQ sync count (edge of PM_LSU_SRQ_SYNC_CYC)";
C3="LSU_SRQ_S0_VALID  : SRQ slot 0 valid";
C4="INST_CMPL  : Instructions completed";;
 155)
C1="LSU_SRQ_S0_VALID  : SRQ slot 0 valid";
C2="LSU_LRQ_S0_VALID  : LRQ slot 0 valid";
C3="LSU_LMQ_S0_VALID  : LMQ slot 0 valid";
C4="INST_CMPL  : Instructions completed";;
 156)
C1="LSU_LMQ_S0_ALLOC  : LMQ slot 0 allocated";
C2="LSU_LRQ_S0_ALLOC  : LRQ slot 0 allocated";
C3="LSU_SRQ_S0_ALLOC  : SRQ slot 0 allocated";
C4="INST_CMPL  : Instructions completed";;
 157)
C1="L1_PREF  : L1 cache data prefetches";
C2="LSU0_L1_PREF  :  LS0 L1 cache data prefetches";
C3="LSU1_L1_PREF  :  LS1 L1 cache data prefetches";
C4="INST_CMPL  : Instructions completed";;
 158)
C1="L2_LOC_GUESS_CORRECT [shared core] : L2 guess loc and guess was correct (ie data local)";
C2="L2_LOC_GUESS_WRONG [shared core] : L2 guess loc and guess was not correct (ie data remote)";
C3="CYC [shared core] : Processor cycles";
C4="INST_CMPL  : Instructions completed";;
 159)
C1="L2_GLOB_GUESS_CORRECT [shared core] : L2 guess glb and guess was correct (ie data remote)";
C2="L2_GLOB_GUESS_WRONG [shared core] : L2 guess glb and guess was not correct (ie data local)";
C3="CYC [shared core] : Processor cycles";
C4="INST_CMPL  : Instructions completed";;
 160)
C1="INST_IMC_MATCH_CMPL  : IMC matched instructions completed";
C2="INST_FROM_L1  : Instruction fetched from L1";
C3="INST_IMC_MATCH_DISP  : IMC Matches dispatched";
C4="INST_CMPL  : Instructions completed";;
 161)
C1="EE_OFF_EXT_INT  : Cycles MSR(EE) bit off and external interrupt pending";
C2="EXT_INT  : External interrupts";
C3="TB_BIT_TRANS [shared chip] : Time Base bit transition";
C4="CYC [shared core] : Processor cycles";;
 162)
C1="1PLUS_PPC_CMPL  : One or more PPC instruction completed";
C2="HV_CYC [shared chip] : Hypervisor Cycles";
C3="INST_DISP  : Instructions dispatched";
C4="1PLUS_PPC_DISP  : Cycles at least one instruction dispatched";;
 163)
C1="PM_GRP_IC_MISS_NONSPEC  : Group experienced non-speculative I cache miss";
C2="GCT_NOSLOT_IC_MISS  : No slot in GCT caused by I cache miss";
C3="CYC [shared core] : Processor cycles";
C4="GCT_NOSLOT_BR_MPRED_IC_MISS  : GCT empty by branch  mispredict + IC miss";;
 164)
C1="PM_GRP_BR_MPRED_NONSPEC  : Group experienced non-speculative branch redirect";
C2="BR_MPRED_CR_TA  : Branch mispredict - taken/not taken and target";
C3="BR_MPRED_CCACHE  : Branch misprediction due to count cache prediction";
C4="BR_MPRED  : Branches incorrectly predicted";;
 165)
C1="L1_DEMAND_WRITE  : Instruction Demand sectors wriittent into IL1";
C2="IC_PREF_WRITE  : Instruction prefetch written into I cache";
C3="IC_WRITE_ALL  : Icache sectors written, prefetch + demand";
C4="INST_CMPL  : Instructions completed";;
 166)
C1="THRESH_TIMEO  : Threshold timeout";
C2="HV_CYC [shared chip] : Hypervisor Cycles";
C3="CYC [shared core] : Processor cycles";
C4="IFU_FIN  : IFU finished an instruction";;
 167)
C1="BR_MPRED_LSTACK  : Branch Mispredict due to Link Stack";
C2="EXT_INT  : External interrupts";
C3="L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="BR_MPRED  : Branches incorrectly predicted";;
 168)
C1="FLUSH_BR_MPRED  : Flush caused by branch mispredict";
C2="FLUSH_PARTIAL  : Partial flush";
C3="LSU_SET_MPRED  : Line already in cache at reload time";
C4="BR_MPRED  : Branches incorrectly predicted";;
 169)
C1="LSU_SRQ_FULL_CYC  : Cycles SRQ full";
C2="LSU_DC_PREF_STREAM_ALLOC  : D cache new prefetch stream allocated";
C3="L1_PREF  : L1 cache data prefetches";
C4="IBUF_FULL_CYC  : Cycles instruction buffer full";;
 170)
C1="FLOP  : Floating Point Operation Finished";
C2="CYC [shared core] : Processor cycles";
C3="PM_GRP_CMPL  : Group completed";
C4="INST_CMPL  : Instructions completed";;
 171)
C1="INST_CMPL  : Instructions completed";
C2="ST_FIN  : Store instructions finished";
C3="TB_BIT_TRANS [shared chip] : Time Base bit transition";
C4="FLUSH  : Flushes";;
 172)
C1="GCT_NOSLOT_CYC  : Cycles no GCT slot allocated";
C2="ST_FIN  : Store instructions finished";
C3="DTLB_MISS  : Data TLB misses";
C4="BR_MPRED  : Branches incorrectly predicted";;
 173)
C1="CYC [shared core] : Processor cycles";
C2="CYC [shared core] : Processor cycles";
C3="INST_CMPL  : Instructions completed";
C4="IFU_FIN  : IFU finished an instruction";;
 174)
C1="LSU_DCACHE_RELOAD_VALID  : count per sector of lines reloaded in L1 (demand + prefetch)"; 
C2="CMPLU_STALL_STORE  : Completion stall due to store instruction";
C3="L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="CMPLU_STALL_VECTOR_LONG  : completion stall due to long latency vector instruction";;
 175)
C1="CMPLU_STALL_END_GCT_NOSLOT  : Count ended because GCT went empty";
C2="LSU0_L1_SW_PREF  : LSU0 Software L1 Prefetches, including SW Transient Prefetches";
C3="LSU1_L1_SW_PREF  : LSU1 Software L1 Prefetches, including SW Transient Prefetches";
C4="CMPLU_STALL_IFU  : Completion stall due to IFU";; 
 176)
C1="BRU_FIN  : BRU produced a result";
C2="ST_FIN  : Store instructions finished";
C3="MRK_PTEG_FROM_DL2L3_SHR [marked] : Marked PTEG loaded from distant L2 or L3 shared";
C4="CMPLU_STALL_BRU  : Completion stall due to BRU";;
 177)
C1="SUSPENDED  : Suspended";
C2="CYC [shared core] : Processor cycles";
C3="LWSYNC  : Isync instruction completed";
C4="INST_CMPL  : Instructions completed";;
 178)
C1="IOPS_CMPL  : Internal operations completed";
C2="CYC [shared core] : Processor cycles";
C3="IOPS_DISP  : IOPS dispatched";
C4="INST_CMPL  : Instructions completed";;
 179)
C1="LWSYNC  : Isync instruction completed";
C2="CYC [shared core] : Processor cycles";
C3="LWSYNC_HELD  : LWSYNC held at dispatch";
C4="INST_CMPL  : Instructions completed";;
 180)
C1="CYC [shared core] : Processor cycles";
C2="SEG_EXCEPTION  : ISEG + DSEG Exception";
C3="ISEG  : ISEG Exception";
C4="DSEG  : DSEG Exception";;
 181)
C1="L3_HIT [shared core] : L3 cache hits";
C2="L3_LD_HIT [shared core] : L3 demand LD Hits";
C3="L3_PREF_HIT [shared core] : L3 Prefetch Directory Hit";
C4="L3_CO_L31 [shared core] : L3 Castouts to Memory";;
 182)
C1="SHL_DEALLOCATED  : SHL Table entry deallocated";
C2="SHL_CREATED  : SHL table entry Created";
C3="SHL_MERGED  : SHL table entry merged with existing";
C4="SHL_MATCH  : SHL Table Match";;
 183)
C1="L3_PREF_LD  : L3 cache LD prefetches";
C2="L3_PREF_ST  : L3 cache ST prefetches";
C3="L3_PREF_LDST  : L3 cache prefetches LD + ST";
C4="L1_PREF  : L1 cache data prefetches";;
 184)
C1="L3_MISS [shared core] : L3 cache misses";
C2="L3_LD_MISS [shared core] : L3 demand LD Miss";
C3="L3_PREF_MISS [shared core] : L3 Prefetch  Directory Miss";
C4="L3_CO_MEM [shared core] : L3 Castouts to L3.1";;
 185)
C1="CYC [shared core] : Processor cycles";
C2="LSU_DC_PREF_STREAM_CONFIRM  : Dcache new prefetch stream confirmed";
C3="LSU0_DC_PREF_STREAM_CONFIRM  : LS0 Dcache prefetch stream confirmed";
C4="LSU1_DC_PREF_STREAM_CONFIRM  : LS1 'Dcache prefetch stream confirmed";;
 186)
C1="CYC [shared core] : Processor cycles";
C2="LSU_DC_PREF_STRIDED_STREAM_CONFIRM  : Dcache Strided prefetch stream confirmed (software + hardware)";
C3="LSU0_DC_PREF_STREAM_CONFIRM_STRIDE  : LS0 Dcache Strided prefetch stream confirmed";
C4="LSU1_DC_PREF_STREAM_CONFIRM_STRIDE  : LS1  Dcache Strided prefetch stream confirmed";;
 187)
C1="DC_PREF_DST  : DST (Data Stream Touch) stream start";
C2="LSU_DC_PREF_STREAM_ALLOC  : D cache new prefetch stream allocated";
C3="LSU0_DC_PREF_STREAM_ALLOC  : LS0 D cache new prefetch stream allocated";
C4="LSU1_DC_PREF_STREAM_ALLOC  : LS 1 D cache new prefetch stream allocated";;
 188)
C1="LARX_LSU0  : Larx executed on LSU0";
C2="LARX_LSU1  : Larx executed on LSU1";
C3="CYC [shared core] : Processor cycles";
C4="LARX_LSU  : Larx Finished";;
 189)
C1="CYC [shared core] : Processor cycles";
C2="LSU_LDF  : LSU executed Floating Point load instruction";
C3="LSU0_LDF  : LSU0 executed Floating Point load instruction";
C4="LSU1_LDF  : LSU1 executed Floating Point load instruction";;
 190)
C1="CYC [shared core] : Processor cycles";
C2="LSU_LDX  : All Vector loads (vsx vector + vmx vector)";
C3="LSU0_LDX  : LS0 Vector Loads";
C4="LSU1_LDX  : LS1  Vector Loads";;
 191)
C1="L2_LD  : Data Load Count";
C2="L2_ST_MISS  : Data Store Miss";
C3="L3_PREF_HIT [shared core] : L3 Prefetch Directory Hit";
C4="CYC [shared core] : Processor cycles";;
 192)
C1="LARX_LSU  : Larx Finished";
C2="LSU_REJECT_LHS  : Load hit store reject";
C3="STCX_CMPL  : Conditional stores with reservation completed";
C4="STCX_FAIL  : STCX failed";;
 193)
C1="BTAC_HIT  : BTAC Correct Prediction";
C2="BTAC_MISS  : BTAC Misses";
C3="STCX_CMPL  : Conditional stores with reservation completed";
C4="STCX_FAIL  : STCX failed";;
 194)
C1="BC_PLUS_8_CONV  : BC+8 Converted";
C2="BC_PLUS_8_RSLV_TAKEN  : BC+8 Resolve outcome was Taken, resulting in the conditional instruction being canceled";
C3="CYC [shared core] : Processor cycles";
C4="INST_CMPL  : Instructions completed";;
 195)
C1="INST_IMC_MATCH_CMPL  : IMC matched instructions completed";
C2="INST_DISP  : Instructions dispatched";
C3="INST_IMC_MATCH_DISP  : IMC Matches dispatched";
C4="INST_CMPL  : Instructions completed";;
 196)
C1="L2_LDST [shared core] : Data Load+Store Count";
C2="L2_LDST_MISS [shared core] : Data Load+Store Miss";
C3="L2_INST_MISS  : Instruction Load Misses";
C4="L2_DISP_ALL [shared core] : All successful LD/ST dispatches for this thread(i+d)";;
 197)
C1="INST_CMPL  : Instructions completed";
C2="CYC [shared core] : Processor cycles";
C3="L2_INST  : Instruction Load Count";
C4="L2_DISP_ALL [shared core] : All successful LD/ST dispatches for this thread(i+d)";;
 198)
C1="INST_CMPL  : Instructions completed";
C2="CYC [shared core] : Processor cycles";
C3="L2_SYS_PUMP [shared core] : RC req that was a global (aka system) pump attempt";
C4="RUN_INST_CMPL  : Run instructions completed";;
 199)
C1="INST_CMPL  : Instructions completed";
C2="CYC [shared core] : Processor cycles";
C3="L2_SN_SX_I_DONE [shared core] : SNP dispatched and went from Sx or Tx to Ix";
C4="L2_SN_M_WR_DONE [shared core] : SNP dispatched for a write and was M";;
 200)
C1="INST_CMPL  : Instructions completed";
C2="CYC [shared core] : Processor cycles";
C3="L2_NODE_PUMP [shared core] : RC req that was a local (aka node) pump attempt";
C4="RUN_INST_CMPL  : Run instructions completed";;
 201)
C1="INST_CMPL  : Instructions completed";
C2="RUN_CYC  : Run cycles";
C3="CYC [shared core] : Processor cycles";
C4="L2_SN_M_RD_DONE [shared core] : SNP dispatched for a read and was M";;
 202)
C1="IERAT_MISS  : IERAT miss count";
C2="IERAT_XLATE_WR_16MPLUS  : large page 16M+";
C3="IERAT_WR_64K  : large page 64k";
C4="INST_CMPL  : Instructions completed";;
 203)
C1="DISP_CLB_HELD  : CLB Hold: Any Reason";
C2="DISP_CLB_HELD_SB  : Dispatch/CLB Hold: Scoreboard";
C3="CYC [shared core] : Processor cycles";
C4="INST_CMPL  : Instructions completed";;
 204)
C1="CYC [shared core] : Processor cycles";
C2="DPU_HELD_POWER  : DISP unit held due to Power Management";
C3="DISP_WT  : Dispatched Starved (not held, nothing to dispatch)";
C4="INST_CMPL  : Instructions completed";;
 205)
C1="RUN_SPURR [shared chip] : Run SPURR";
C2="RUN_CYC  : Run cycles";
C3="CYC [shared core] : Processor cycles";
C4="RUN_PURR [shared chip] : Run PURR Event";;
 206)
C1="PMC4_OVERFLOW  PMC4 Overflow";
C2="PMC1_OVERFLOW  PMC1 Overflow";
C3="PMC2_OVERFLOW  PMC2 Overflow";
C4="PMC3_OVERFLOW  PMC3 Overflow";;
 207)
C1="PMC5_OVERFLOW  PMC5 Overflow";
C2="PMC1_OVERFLOW  PMC1 Overflow";
C3="PMC6_OVERFLOW  PMC6 Overflow";
C4="PMC3_OVERFLOW  PMC3 Overflow";;
 208)
C1="PMC4_REWIND  PMC4 Rewind Event";
C2="RUN_CYC  : Run cycles";
C3="PMC2_REWIND  PMC2 Rewind Event";
C4="INST_CMPL  : Instructions completed";;
 209)
C1="PMC2_SAVED  PMC2 Rewind Value saved";
C2="RUN_CYC  : Run cycles";
C3="PMC4_SAVED  PMC4 Rewind Value saved";
C4="INST_CMPL  : Instructions completed";;
 210)
C1="FLUSH_DISP_TLBIE  : Dispatch Flush: TLBIE";
C2="DISP_CLB_HELD_TLBIE  : Dispatch Hold: Due to TLBIE";
C3="SNOOP_TLBIE [shared chip] : Snoop TLBIE";
C4="INST_CMPL  : Instructions completed";;
 211)
C1="IERAT_MISS  : IERAT miss count";
C2="L1_ICACHE_MISS  : L1 I cache miss count";
C3="ST_MISS_L1  : L1 D cache store misses";
C4="LD_MISS_L1  : L1 D cache load misses";;
 212)
C1="CYC [shared core] : Processor cycles";
C2="LSU_DERAT_MISS  : DERAT misses";
C3="DTLB_MISS  : Data TLB misses";
C4="ITLB_MISS  : Instruction TLB misses";;
 213)
C1="ANY_THRD_RUN_CYC  : One of threads in run_cycles";
C2="RUN_CYC  : Run cycles";
C3="CYC [shared core] : Processor cycles";
C4="RUN_PURR [shared chip] : Run PURR Event";;
 214)
C1="FLOP  : Floating Point Operation Finished";
C2="RUN_CYC  : Run cycles";
C3="CYC [shared core] : Processor cycles";
C4="RUN_INST_CMPL  : Run instructions completed";;
 215)
C1="1PLUS_PPC_CMPL  : One or more PPC instruction completed";
C2="RUN_CYC  : Run cycles";
C3="INST_DISP  : Instructions dispatched";
C4="1PLUS_PPC_DISP  : Cycles at least one instruction dispatched";;
 216)
C1="INST_CMPL  : Instructions completed";
C2="ST_FIN  : Store instructions finished";
C3="ST_MISS_L1  : L1 D cache store misses";
C4="LD_MISS_L1  : L1 D cache load misses";;
 217)
C1="INST_CMPL  : Instructions completed";
C2="DATA_FROM_L2MISS  : Data loaded missed L2";
C3="L1_DCACHE_RELOAD_VALID  : L1 reload data source valid";
C4="LD_MISS_L1  : L1 D cache load misses";;
 218)
C1="IERAT_MISS  : IERAT miss count";
C2="L1_ICACHE_MISS  : L1 I cache miss count";
C3="INST_CMPL  : Instructions completed";
C4="ITLB_MISS  : Instruction TLB misses";;
 219)
C1="SUSPENDED  : Suspended";
C2="SUSPENDED  : Suspended";
C3="SUSPENDED  : Suspended";
C4="SUSPENDED  : Suspended";;
 220)
C1="INST_CMPL  : Instructions completed";
C2="EXT_INT  : External interrupts";
C3="TB_BIT_TRANS [shared chip] : Time Base bit transition";
C4="CYC [shared core] : Processor cycles";;
 221)
C1="INST_IMC_MATCH_CMPL  : IMC matched instructions completed";
C2="INST_DISP  : Instructions dispatched";
C3="THRD_CONC_RUN_INST [shared chip] : Concurrent Run Instructions";
C4="FLUSH  : Flushes";;
 222)
C1="GCT_NOSLOT_CYC  : Cycles no GCT slot allocated";
C2="INST_DISP  : Instructions dispatched";
C3="CYC [shared core] : Processor cycles";
C4="BR_MPRED  : Branches incorrectly predicted";;
 223)
C1="MRK_BR_TAKEN [marked] : Marked branch taken";
C2="MRK_LD_MISS_L1 [marked] : Marked L1 D cache load misses";
C3="MRK_BR_MPRED [marked] : Marked branch mispredicted";
C4="INST_CMPL  : Instructions completed";;
 224)
C1="MRK_DATA_FROM_DMEM [marked] : Marked data loaded from distant memory";
C2="MRK_DATA_FROM_DMEM_CYC [marked] : Marked ld latency Data Source 1110 (Distant Memory)";
C3="INST_CMPL  : Instructions completed";
C4="MRK_DATA_FROM_L2MISS [marked] : Marked data loaded missed L2";;
 225)
C1="MRK_LD_MISS_EXPOSED_CYC [marked] : Marked Load exposed Miss";
C2="INST_CMPL  : Instructions completed";
C3="MRK_DATA_FROM_L21_MOD [marked] : Marked data loaded from another L2 on same chip modified";
C4="MRK_DATA_FROM_L21_MOD_CYC [marked] : Marked ld latency Data source 0101 (L2.1 M same chip)";;
 226)
C1="MRK_DATA_FROM_L3 [marked] : Marked data loaded from L3";
C2="MRK_DATA_FROM_L3MISS [marked] : Marked data loaded from L3 miss";
C3="INST_CMPL  : Instructions completed";
C4="MRK_DATA_FROM_L3_CYC [marked] : Marked load latency from L3";;
 227)
C1="INST_CMPL  : Instructions completed";
C2="MRK_DATA_FROM_LMEM_CYC [marked] : Marked load latency from local memory";
C3="MRK_DATA_FROM_LMEM [marked] : Marked data loaded from local memory";
C4="DATA_FROM_LMEM  : Data loaded from local memory";;
 228)
C1="MRK_DATA_FROM_L31_MOD [marked] : Marked data loaded from another L3 on same chip modified";
C2="INST_CMPL  : Instructions completed";
C3="MRK_INST_FIN [marked] : Marked instruction finished";
C4="MRK_DATA_FROM_L31_MOD_CYC [marked] : Marked ld latency Data source 0111  (L3.1 M same chip)";;
 229)
C1="MRK_LD_MISS_EXPOSED_CYC_COUNT [marked] : Marked Load exposed Miss (use edge detect to count #)";
C2="MRK_DATA_FROM_L21_SHR_CYC [marked] : Marked ld latency Data source 0100 (L2.1 S)";
C3="MRK_DATA_FROM_L21_SHR [marked] : Marked data loaded from another L2 on same chip shared";
C4="INST_CMPL  : Instructions completed";;
 230)
C1="MRK_DATA_FROM_L2 [marked] : Marked data loaded from L2";
C2="MRK_DATA_FROM_L2_CYC [marked] : Marked load latency from L2";
C3="INST_CMPL  : Instructions completed";
C4="MRK_DATA_FROM_L2MISS [marked] : Marked data loaded missed L2";;
 231)
C1="MRK_DATA_FROM_RL2L3_MOD [marked] : Marked data loaded from remote L2 or L3 modified";
C2="MRK_DATA_FROM_L3MISS [marked] : Marked data loaded from L3 miss";
C3="INST_CMPL  : Instructions completed";
C4="MRK_DATA_FROM_RL2L3_MOD_CYC [marked] : Marked ld latency Data source 1001 (L2.5/L3.5 M same 4 chip node)";;
 232)
C1="INST_CMPL  : Instructions completed";
C2="MRK_DATA_FROM_DL2L3_SHR_CYC [marked] : Marked ld latency Data Source 1010 (Distant L2.75/L3.75 S)";
C3="MRK_DATA_FROM_DL2L3_SHR [marked] : Marked data loaded from distant L2 or L3 shared";
C4="MRK_DATA_FROM_L2MISS [marked] : Marked data loaded missed L2";;
 233)
C1="MRK_DATA_FROM_RL2L3_SHR [marked] : Marked data loaded from remote L2 or L3 shared";
C2="MRK_DATA_FROM_RL2L3_SHR_CYC [marked] : Marked ld latency Data Source 1000 (Remote L2.5/L3.5 S)";
C3="DATA_FROM_RMEM  : Data loaded from remote memory";
C4="INST_CMPL  : Instructions completed";;
 234)
C1="MRK_LD_MISS_EXPOSED_CYC [marked] : Marked Load exposed Miss";
C2="INST_CMPL  : Instructions completed";
C3="MRK_DATA_FROM_RMEM [marked] : Marked data loaded from remote memory";
C4="MRK_DATA_FROM_RMEM_CYC [marked] : Marked load latency from remote memory";;
 235)
C1="MRK_DATA_FROM_L31_SHR [marked] : Marked data loaded from another L3 on same chip shared";
C2="MRK_DATA_FROM_L31_SHR_CYC [marked] : Marked ld latency Data source 0110 (L3.1 S)";
C3="MRK_INST_FIN [marked] : Marked instruction finished";
C4="INST_CMPL  : Instructions completed";;
 236)
C1="MRK_LD_MISS_EXPOSED_CYC_COUNT [marked] : Marked Load exposed Miss (use edge detect to count #)";
C2="INST_CMPL  : Instructions completed";
C3="MRK_DATA_FROM_DL2L3_MOD [marked] : Marked data loaded from distant L2 or L3 modified";
C4="MRK_DATA_FROM_DL2L3_MOD_CYC [marked] : Marked ld latency Data source 1011  (L2.75/L3.75 M different 4 chip node)";;
 237)
C1="MRK_LSU_FLUSH_ULD [marked] : Marked unaligned load flushes";
C2="MRK_LSU_FLUSH_UST [marked] : Marked unaligned store flushes";
C3="INST_CMPL  : Instructions completed";
C4="CYC [shared core] : Processor cycles";;
 238)
C1="INST_CMPL  : Instructions completed";
C2="CYC [shared core] : Processor cycles";
C3="MRK_LSU_FLUSH_LRQ [marked] : Marked LRQ flushes";
C4="MRK_LSU_FLUSH_SRQ [marked] : Marked SRQ lhs flushes";;
 239)
C1="MRK_LSU_REJECT_LHS [marked] : Marked load hit store reject";
C2="MRK_LSU_FLUSH [marked] : Flush: (marked) : All Cases";
C3="INST_CMPL  : Instructions completed";
C4="MRK_LSU_REJECT [marked] : LSU marked reject (up to 2 per cycle)";;
 240)
C1="MRK_INST_ISSUED [marked] : Marked instruction issued";
C2="MRK_INST_DISP [marked] : Marked instruction dispatched";
C3="MRK_INST_FIN [marked] : Marked instruction finished";
C4="INST_CMPL  : Instructions completed";;
 241)
C1="MRK_ST_CMPL [marked] : Marked store instruction completed";
C2="MRK_ST_NEST [marked] : marked store sent to Nest";
C3="MRK_ST_CMPL_INT [marked] : Marked store completed with intervention";
C4="INST_CMPL  : Instructions completed";;
 242)
C1="INST_CMPL  : Instructions completed";
C2="MRK_DTLB_MISS_4K [marked] : Marked Data TLB misses for 4K page";
C3="MRK_DTLB_MISS_64K [marked] : Marked Data TLB misses for 64K page";
C4="MRK_DTLB_MISS_16M [marked] : Marked Data TLB misses for 16M page";;
 243)
C1="MRK_DTLB_MISS_16G [marked] : Marked Data TLB misses for 16G page";
C2="MRK_DTLB_MISS_4K [marked] : Marked Data TLB misses for 4K page";
C3="MRK_DTLB_MISS_64K [marked] : Marked Data TLB misses for 64K page";
C4="INST_CMPL  : Instructions completed";;
 244)
C1="INST_CMPL  : Instructions completed";
C2="MRK_DERAT_MISS_64K [marked] : Marked DERAT misses for 64K page";
C3="MRK_DERAT_MISS_16M [marked] : Marked DERAT misses for 16M page";
C4="MRK_DERAT_MISS_16G [marked] : Marked DERAT misses for 16G page";;
 245)
C1="MRK_DERAT_MISS_4K [marked] : Marked DERAT misses for 4K page";
C2="MRK_DERAT_MISS_64K [marked] : Marked DERAT misses for 64K page";
C3="MRK_DERAT_MISS_16M [marked] : Marked DERAT misses for 16M page";
C4="INST_CMPL  : Instructions completed";;
 246)
C1="MRK_LD_MISS_EXPOSED_CYC [marked] : Marked Load exposed Miss";
C2="INST_CMPL  : Instructions completed";
C3="MRK_LSU_DERAT_MISS [marked] : Marked DERAT miss";
C4="MRK_LD_MISS_L1_CYC [marked] : L1 data load miss cycles";;
 247)
C1="INST_CMPL  : Instructions completed";
C2="MRK_PTEG_FROM_DMEM [marked] : Marked PTEG loaded from distant memory";
C3="MRK_PTEG_FROM_L21_MOD [marked] : Marked PTEG loaded from another L2 on same chip modified";
C4="MRK_PTEG_FROM_L21_SHR [marked] : Marked PTEG loaded from another L2 on same chip shared";;
 248)
C1="MRK_PTEG_FROM_L2 [marked] : Marked PTEG loaded from L2.5 modified";
C2="MRK_PTEG_FROM_RL2L3_SHR [marked] : Marked PTEG loaded from remote L2 or L3 shared";
C3="MRK_PTEG_FROM_RMEM [marked] : Marked PTEG loaded from remote memory";
C4="INST_CMPL  : Instructions completed";;
 249)
C1="INST_CMPL  : Instructions completed";
C2="MRK_PTEG_FROM_L31_SHR [marked] : Marked PTEG loaded from another L3 on same chip shared";
C3="MRK_PTEG_FROM_L21_MOD [marked] : Marked PTEG loaded from another L2 on same chip modified";
C4="MRK_PTEG_FROM_DL2L3_MOD [marked] : Marked PTEG loaded from distant L2 or L3 modified";;
 250)
C1="MRK_PTEG_FROM_L31_MOD [marked] : Marked PTEG loaded from another L3 on same chip modified";
C2="MRK_PTEG_FROM_L3 [marked] : Marked PTEG loaded from L3";
C3="INST_CMPL  : Instructions completed";
C4="MRK_PTEG_FROM_L2MISS [marked] : Marked PTEG loaded from L2 miss";;
 251)
C1="MRK_PTEG_FROM_RL2L3_MOD [marked] : Marked PTEG loaded from remote L2 or L3 modified";
C2="MRK_PTEG_FROM_L3MISS [marked] : Marked PTEG loaded from L3 miss";
C3="INST_CMPL  : Instructions completed";
C4="MRK_PTEG_FROM_LMEM [marked] : Marked PTEG loaded from local memory";;
 252)
C1="MRK_STCX_FAIL [marked] : Marked STCX failed";
C2="INST_CMPL  : Instructions completed";
C3="MRK_IFU_FIN [marked] : Marked instruction IFU processing finished";
C4="MRK_INST_TIMEO [marked] : marked Instruction finish timeout";;
 253)
C1="INST_CMPL  : Instructions completed";
C2="MRK_FXU_FIN [marked] : Marked instruction FXU processing finished";
C3="MRK_IFU_FIN [marked] : Marked instruction IFU processing finished";
C4="MRK_LSU_FIN [marked] : Marked instruction LSU processing finished";;
 254)
C1="INST_CMPL  : Instructions completed";
C2="MRK_BRU_FIN [marked] : Marked instruction BRU processing finished";
C3="MRK_LSU_PARTIAL_CDF [marked] : A partial cacheline was returned from the L3 for a marked load";
C4="MRK_LSU_FIN [marked] : Marked instruction LSU processing finished";;
 255)
C1="MRK_FIN_STALL_CYC [marked] : Marked instruction Finish Stall cycles (marked finish after NTC)"; 
C2="INST_CMPL  : Instructions completed";
C3="MRK_VSU_FIN [marked] : vsu (fpu) marked  instr finish";
C4="??";;
 256)
C1="MRK_FIN_STALL_CYC_COUNT [marked] : Marked instruction Finish Stall cycles (marked finish after NTC) (use edge detect to count #)";
C2="MRK_DFU_FIN [marked] : Decimal Unit marked instruction finish";
C3="??";
C4="INST_CMPL  : Instructions completed";;
 257)
C1="GRP_MRK_CYC [marked] : cycles IDU marked instruction before dispatch";
C2="RUN_CYC  : Run cycles";
C3="INST_CMPL  : Instructions completed";
C4="MRK_GRP_CMPL [marked] : Marked  completed";;
 258)
C1="MRK_LSU_REJECT_LHS [marked] : Marked load hit store reject";
C2="INST_CMPL  : Instructions completed";
C3="MRK_LSU_REJECT_ERAT_MISS [marked] : LSU marked reject due to ERAT (up to 2 per cycle)";
C4="MRK_LSU_REJECT [marked] : LSU marked reject (up to 2 per cycle)";;
 259)
C1="CYC [shared core] : Processor cycles";
C2="CYC [shared core] : Processor cycles";
C3="INST_CMPL  : Instructions completed";
C4="MRK_LSU_FIN [marked] : Marked instruction LSU processing finished";;
 260)
C1="MRK_DATA_FROM_L2 [marked] : Marked data loaded from L2";
C2="MRK_DATA_FROM_L2_CYC [marked] : Marked load latency from L2";
C3="LSU_DCACHE_RELOAD_VALID  : count per sector of lines reloaded in L1 (demand + prefetch)"; 
C4="CMPLU_STALL_COUNT  : EDGE cycles when no s completed and the GCT was not empty";;
esac

echo $C1 1>&2
echo $C2 1>&2
echo $C3 1>&2
echo $C4 1>&2
echo $C5 1>&2
echo $C6 1>&2

