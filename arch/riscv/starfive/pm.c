#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/io.h>
#include <asm/cpuidle.h>
#include <asm/sbi.h>
#include <asm/suspend.h>

static int starfive_sbi_suspend_finisher(unsigned long suspend_type,
				unsigned long resume_addr,
				unsigned long opaque)
{
	struct sbiret ret;

	ret = sbi_ecall(SBI_EXT_HSM, SBI_EXT_HSM_HART_SUSPEND,
			suspend_type, resume_addr, opaque, 0, 0, 0);

	return (ret.error) ? sbi_err_map_linux_errno(ret.error) : 0;
}

static int starfive_sbi_suspend(u32 state)
{
	if (state & SBI_HSM_SUSP_NON_RET_BIT)
		return cpu_suspend(state, starfive_sbi_suspend_finisher);
	else
		return starfive_sbi_suspend_finisher(state, 0, 0);
}


static int starfive_suspend_enter(suspend_state_t state)
{
	if (!IS_ENABLED(CONFIG_PM))
		return 0;

	switch (state) {
	case PM_SUSPEND_MEM:
		printk("enter the starfive_suspend_enter\n");
		starfive_sbi_suspend(0x90000001);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct platform_suspend_ops starfive_suspend_ops = {
	.enter = starfive_suspend_enter,
	.valid = suspend_valid_only_mem,
};

int __init starfive_pm_init(void)
{
	suspend_set_ops(&starfive_suspend_ops);
	return 0;
}
arch_initcall(starfive_pm_init);