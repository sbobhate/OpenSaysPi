#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x2ccf016, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0xb8eec98b, __VMLINUX_SYMBOL_STR(del_timer) },
	{ 0x8fdf772a, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0x563a8bba, __VMLINUX_SYMBOL_STR(gpiod_direction_input) },
	{ 0xc6aa79ee, __VMLINUX_SYMBOL_STR(gpiod_direction_output_raw) },
	{ 0x47229b5c, __VMLINUX_SYMBOL_STR(gpio_request) },
	{ 0x4156e4ab, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xfa5bcf35, __VMLINUX_SYMBOL_STR(mod_timer) },
	{ 0x50b84c8f, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0x80cdb172, __VMLINUX_SYMBOL_STR(gpiod_get_raw_value) },
	{ 0x4a2098ae, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x28cc25db, __VMLINUX_SYMBOL_STR(arm_copy_from_user) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "1EF23788D9ADD8FE1810FF8");
