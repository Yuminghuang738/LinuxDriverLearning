// SPDX-License-Identifier: GPL-2.0
/*
 * dt_overlay - 运行时加载设备树插件（dtbo）
 *
 * 背景（实测结论，2026-10-18）：
 *   这颗 RK3506 的 6.1.99 内核把 drivers/of/configfs.c 裁掉了，
 *   全树没有 OF_CONFIGFS 符号，所以
 *       /sys/kernel/config/device-tree/overlays/
 *   永远不会出现。
 *
 *   但 CONFIG_OF_OVERLAY=y 是开的，而且 drivers/of/overlay.c:1057
 *   有 EXPORT_SYMBOL_GPL(of_overlay_fdt_apply)，所以可以用模块来加载 dtbo。
 *
 * 用法:
 *   insmod dt_overlay.ko                        # 默认读 /userdata/overlay.dtbo
 *   insmod dt_overlay.ko path=/userdata/my.dtbo # 指定路径
 *   rmmod dt_overlay                            # 卸载 overlay
 *
 * 验证:
 *   ls /proc/device-tree/            # overlay 里的节点会出现在这里
 *   dmesg | tail                     # 看 dt_overlay 的打印
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/err.h>

#define DT_OVERLAY_MAX_SIZE	(256 * 1024)

static char *path = "/userdata/overlay.dtbo";
module_param(path, charp, 0444);
MODULE_PARM_DESC(path, "dtbo 文件路径（默认 /userdata/overlay.dtbo）");

/* of_overlay_fdt_apply() 返回的 changeset id，rmmod 时用它移除 */
static int ovcs_id = -1;

static const unsigned char fdt_magic[4] = { 0xd0, 0x0d, 0xfe, 0xed };

static int read_dtbo(const char *p, void **buf_out, size_t *len_out)
{
	struct file *filp;
	void *buf;
	loff_t pos = 0;
	ssize_t n;
	int ret = 0;

	filp = filp_open(p, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		ret = PTR_ERR(filp);
		pr_err("dt_overlay: 打不开 %s (ret=%d)\n", p, ret);
		return ret;
	}

	buf = kmalloc(DT_OVERLAY_MAX_SIZE, GFP_KERNEL);
	if (!buf) {
		ret = -ENOMEM;
		goto out_close;
	}

	/* dtbo 很小，一次读完；kernel_read 返回实际读到的字节数 */
	n = kernel_read(filp, buf, DT_OVERLAY_MAX_SIZE, &pos);
	if (n < 0) {
		ret = n;
		pr_err("dt_overlay: 读 %s 失败 (ret=%d)\n", p, ret);
		goto out_free;
	}
	if (n < 8) {
		ret = -EINVAL;
		pr_err("dt_overlay: %s 太小 (%zd 字节)，不像 dtbo\n", p, n);
		goto out_free;
	}
	if (memcmp(buf, fdt_magic, sizeof(fdt_magic))) {
		ret = -EINVAL;
		pr_err("dt_overlay: %s 不是 FDT/DTB 格式（magic 不对）\n", p);
		goto out_free;
	}

	*buf_out = buf;
	*len_out = (size_t)n;
	pr_info("dt_overlay: 读到 %s，%zd 字节\n", p, n);
	filp_close(filp, NULL);
	return 0;

out_free:
	kfree(buf);
out_close:
	filp_close(filp, NULL);
	return ret;
}

static int __init dt_overlay_init(void)
{
	void *buf = NULL;
	size_t len = 0;
	int ret;

	pr_info("dt_overlay: 开始加载 %s\n", path);

	ret = read_dtbo(path, &buf, &len);
	if (ret)
		return ret;

	ret = of_overlay_fdt_apply(buf, (u32)len, &ovcs_id);
	/*
	 * 内核内部会把整个 FDT memcpy 一份（overlay.c: new_fdt = kmalloc + memcpy），
	 * 所以这里可以立刻释放自己的缓冲。
	 */
	kfree(buf);

	if (ret) {
		pr_err("dt_overlay: of_overlay_fdt_apply 失败 (ret=%d)\n", ret);
		ovcs_id = -1;
		return ret;
	}

	pr_info("dt_overlay: 应用成功, ovcs_id=%d\n", ovcs_id);
	return 0;
}

static void __exit dt_overlay_exit(void)
{
	int ret;

	if (ovcs_id <= 0)
		return;

	ret = of_overlay_remove(&ovcs_id);
	if (ret)
		pr_err("dt_overlay: 移除 overlay 失败 (ret=%d)\n", ret);
	else
		pr_info("dt_overlay: overlay 已移除\n");

	ovcs_id = -1;
}

module_init(dt_overlay_init);
module_exit(dt_overlay_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yuminghuang738 <Yuminghuang738@gmail.com>");
MODULE_DESCRIPTION("Runtime device tree overlay loader via of_overlay_fdt_apply()");
