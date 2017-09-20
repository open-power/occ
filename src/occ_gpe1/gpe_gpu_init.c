#include "pk.h"
#include "ppe42_scom.h"
#include "gpu_structs.h"
#include "ipc_async_cmd.h"
#include "gpe_err.h"
#include "gpe_util.h"
#include "p9_misc_scom_addresses.h"

gpu_i2c_info_t G_gpu_i2c __attribute__((section(".sbss.G_gpu_i2c")));

void gpe_gpu_init(ipc_msg_t* cmd, void* arg)
{
    int rc = 0;
    int i;
    ipc_async_cmd_t *async_cmd = (ipc_async_cmd_t*)cmd;
    gpu_init_args_t *args = (gpu_init_args_t*)async_cmd->cmd_data;

    args->error.ffdc = 0;

    // Prevent MCK attention on scom failes (PK kernel fix?)
    mtmsr((mfmsr() & ~(MSR_SIBRC | MSR_SIBRCA)) | MSR_SEM);

    // According to Jordan Keuseman, Setting PV_CP0_P_PRV_GPIO0 pin on the
    // processor chip to low enables HW to automatically apply GPU power brake.
    // GPIO1 (GPU_PWR_BRAKE_FORCE_N) will not be controlled by FW, so needs to
    // be configured as input.
    uint64_t data64 = 0x8000000000000000ull;
    rc = putscom_abs(PU_GPIO_OUTPUT_CLR,data64);
    if(rc)
    {
        PK_TRACE("gpe_gpu_init: PU_GPIO0_OUTPUT failed. rc:0x%08x",rc);
        gpe_set_ffdc(&(args->error), 0, GPE_RC_GPU_INIT_FAILED, rc);
    }

    rc = getscom_abs(PU_GPIO_OUTPUT_EN, &data64);
    if(rc)
    {
        PK_TRACE("gpe_gpu_init: Read PU_GPIO0_OUTPUT_EN failed. rc:0x%08x",rc);
        gpe_set_ffdc(&(args->error), 0, GPE_RC_GPU_INIT_FAILED, rc);
    }

    // pin0 as output, pin1 as input, pin3 unchanged
    data64 &= 0xBfffffffffffffffull;
    data64 |= 0x8000000000000000ull;

    rc = putscom_abs(PU_GPIO_OUTPUT_EN, data64);

    if(rc)
    {
        PK_TRACE("gpe_gpu_init: PU_GPIO0_OUTPUT_EN failed. rc:0x%08x",rc);
        gpe_set_ffdc(&(args->error), 0, GPE_RC_GPU_INIT_FAILED, rc);
    }

    // Get i2c data
    G_gpu_i2c.pib_master = args->gpu_i2c.pib_master;
    G_gpu_i2c.bus_voltage = args->gpu_i2c.bus_voltage;
    for(i = 0; i < MAX_GPUS; ++i)
    {
        G_gpu_i2c.port[i] = args->gpu_i2c.port[i];
        G_gpu_i2c.addr[i] = args->gpu_i2c.addr[i];
    }

    rc = ipc_send_rsp(cmd, IPC_RC_SUCCESS);
    if(rc)
    {
      PK_TRACE("E>gpu_init: Failed to send response back. Halting GPE1", rc);
      pk_halt();
    }
}
