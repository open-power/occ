#if !defined(_P9_MISC_SCOM_ADDRESSES_H_)
#define _P9_MISC_SCOM_ADDRESSES_H_

#define PU_GPIO_INPUT       0x000B0050
#define PU_GPIO_OUTPUT      0x000B0051
#define PU_GPIO_OUTPUT_OR   0x000B0052
#define PU_GPIO_OUTPUT_CLR  0x000B0053
#define PU_GPIO_OUTPUT_EN   0x000B0054


// Found in Cumulus nest "MC Fault Isolation Register"
#define MCS_0_MCFIR         0x05010800
#define MCS_1_MCFIR         0x05010880
#define MCS_2_MCFIR         0x03010800
#define MCS_3_MCFIR         0x03010880

// found in Cumulus nest "MC Mode0 Register"
#define MCS_0_MCMODE0       0x05010811
#define MCS_1_MCMODE0       0x05010891
#define MCS_2_MCMODE0       0x03010811
#define MCS_3_MCMODE0       0x03010891

// found in Cumulus nest "MC Primary Memory Configuration Register"
#define    MCS_0_MCRSVDE    0x0501080E
#define    MCS_0_MCRSVDF    0x0501080F
#define    MCS_1_MCRSVDE    0x0501088E
#define    MCS_1_MCRSVDF    0x0501088F
#define    MCS_2_MCRSVDE    0x0301080E
#define    MCS_2_MCRSVDF    0x0301080F
#define    MCS_3_MCRSVDE    0x0301088E
#define    MCS_3_MCRSVDF    0x0301088F

#define    MCS_0_MCSYNC     0x05010815
#define    MCS_1_MCSYNC     0x05010895
#define    MCS_2_MCSYNC     0x03010815
#define    MCS_3_MCSYNC     0x03010895

// MC Memory Configuration Register FIR/CFG
#define    MCP_CHAN0_CHI_FIR        0x07010900
#define    MCP_CHAN1_CHI_FIR        0x07010940
#define    MCP_CHAN2_CHI_FIR        0x07010980
#define    MCP_CHAN3_CHI_FIR        0x070109C0
#define    MCP_CHAN4_CHI_FIR        0x08010900
#define    MCP_CHAN5_CHI_FIR        0x08010940
#define    MCP_CHAN6_CHI_FIR        0x08010980
#define    MCP_CHAN7_CHI_FIR        0x080109C0

#define    MCP_CHAN0_CHI_MCICFG1Q   0x0701090E
#define    MCP_CHAN1_CHI_MCICFG1Q   0x0701094E
#define    MCP_CHAN2_CHI_MCICFG1Q   0x0701098E
#define    MCP_CHAN3_CHI_MCICFG1Q   0x070109CE
#define    MCP_CHAN4_CHI_MCICFG1Q   0x0801090E
#define    MCP_CHAN5_CHI_MCICFG1Q   0x0801094E
#define    MCP_CHAN6_CHI_MCICFG1Q   0x0801098E
#define    MCP_CHAN7_CHI_MCICFG1Q   0x080109CE

#endif
