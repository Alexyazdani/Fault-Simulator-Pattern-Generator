## Build and Run
    mkdir build && cd build
    cmake ..
    make
    
    ./simulator <int seed>
    
    If no seed is provided, the seed will default to 658.

## TPG Command
    The TPG command is the top-level function to run fault simulation and pattern generation
    To use our baseline:
    TPG <DALG/PODEM> <PFS/DFS> my_patterns.tp


## RTPG
There are four different versions of RTPG:

V1:
    TPG  <DALG/PODEM> <PFS/DFS> <outfile> -rtp v1 <FC_switch>

V2:
    TPG  <DALG/PODEM> <PFS/DFS> <outfile> -rtp v2 <delta_FC>

V3:
    TPG  <DALG/PODEM> <PFS/DFS> <outfile> -rtp v3 <delta_FC>

V4:
    TPG  <DALG/PODEM> <PFS/DFS> <outfile> -rtp v4 <delta_FC>

FC_switch is the FC value where TPG switches from RTPG to ATPG
delta_FC is the change in FC that when reached, will make TPG exit


## D-FRONTIER HEURISTICS:

    TPG  <DALG/PODEM> <PFS/DFS> -df <nl/nh/lh/cc> 

    nl - prioritize lowest node number
    nh - prioritize highest node number
    lh - prioritize highest level node
    cc - prioritize lowest controllability


## J-FRONTIER HEURISTICS:

    TPG  <DALG/PODEM> <PFS/DFS> -jf v0


## FAULT ORDER HEURISTICS:

    TPG  <DALG/PODEM> <PFS/DFS> <outfile> -fl rfl
    TPG  <DALG/PODEM> <PFS/DFS> <outfile> -fl scoap EZ
    TPG  <DALG/PODEM> <PFS/DFS> <outfile> -fl scoap HD

    rfl - reduced fault list
    scoap EZ - lowest CC + CO
    scoap HD - highest CC + CO


## TEST VOLUME COMPRESSION:

    TPG  <DALG/PODEM> <PFS/DFS> <outfile> -tvc

    Takes every 'X' in generated pattern, 
    checks if '1' or '0' will result in more faults detected


## DTPFC:
    This command takes a fault list as input, and reports the FC every step for a given pattern frequency
    DTPFC <pattern_file> <FREQ> <outfile>


## Sample commands:

// BASELINE:
// READ ../auto-tests-phase3/ckts/c17.ckt
// TPG PODEM DFS my_patterns.tp
// TPG DALG DFS my_patterns.tp

// RTPG:
// READ ../ckts_all/c1908.ckt
// TPG DALG PFS my_patterns.tp
// TPG PODEM PFS my_patterns.tp
// TPG PODEM PFS my_patterns.tp -rtp v1 80
// TPG PODEM PFS my_patterns.tp -rtp v2 0.02
// TPG PODEM PFS my_patterns.tp -rtp v3 0.02
// TPG PODEM PFS my_patterns.tp -rtp v4 0.1

// D-FRONTIER:
// READ ../ckts_all/c1355.ckt
// TPG PODEM PFS my_patterns.tp -df nl
// TPG PODEM PFS my_patterns.tp -df nh
// TPG PODEM PFS my_patterns.tp -df lh
// TPG PODEM PFS my_patterns.tp -df cc

// J-FRONTIER:
// READ ../ckts_all/c432.ckt
// TPG PODEM PFS my_patterns.tp -jf v0
// TPG PODEM PFS my_patterns.tp

// FAULT ORDER
// READ ../ckts_all/c6288.ckt
// TPG PODEM PFS my_patterns.tp -fl rfl
// TPG PODEM PFS my_patterns.tp -fl scoap EZ
// TPG PODEM PFS my_patterns.tp -fl scoap HD

// TEST VOLUME COMPRESSION:
// READ ../ckts_all/c6288.ckt
// TPG PODEM PFS my_patterns.tp -tvc

// DTPFC:
// READ ../ckts_all/c1355.ckt
// TPG PODEM PFS my_patterns.tp
// DTPFC my_patterns.tp 1 dalg_report_1908.out
// DTPFC my_patterns.tp 1 podem_report_1908.out
// DTPFC my_patterns.tp 1 podem_report_880.out
// DTPFC my_patterns.tp 1 podem_report_1908_rtpgv1.out
// DTPFC my_patterns.tp 1 podem_report_1908_rtpgv2.out
// DTPFC my_patterns.tp 1 podem_report_1908_rtpgv3.out
// DTPFC my_patterns.tp 1 podem_report_1908_rtpgv4.out
// DTPFC my_patterns.tp 1 podem_report_1908_jf.out
// DTPFC my_patterns.tp 1 podem_report_1908_fl_rfl.out
// DTPFC my_patterns.tp 1 podem_report_1908_fl_scoap_ez.out
// DTPFC my_patterns.tp 1 podem_report_1908_fl_scoap_hd.out
// DTPFC my_patterns.tp 1 podem_report_1908_tvc.out
// DTPFC my_patterns.tp 1 podem_report_880_tvc.out
// DTPFC my_patterns.tp 1 podem_report_1908_DFS.out
// DTPFC my_patterns.tp 1 podem_report_6288_DFS.out
// DTPFC my_patterns.tp 1 podem_report_6288_PFS.out

// RANDOM SEED:
// ./simulator 1234
