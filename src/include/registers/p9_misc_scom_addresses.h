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

// MC Primary Memory Configuration Register (MCFGP)
#define    MCS_0_MCFGP      0x0501080A
#define    MCS_1_MCFGP      0x0501088A
#define    MCS_2_MCFGP      0x0301080A
#define    MCS_3_MCFGP      0x0301088A
#endif
