#include <sys/types.h>
#include <sys/module.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_types.h>
#include <sys/conf.h>



#define DEV_NAME "FOO_DEV"



static d_open_t foo_open;
static d_write_t foo_write;



static struct cdev *foo_cdev;
static struct cdevsw foo_cdevsw = {
	.d_version = D_VERSION,
	.d_open = foo_open,
	.d_write = foo_write,
	.d_name = DEV_NAME,
};



static int
foo_open(struct cdev *dev __unused, int oflags __unused,
    int devtype __unused, struct thread *td __unused)
{

	return (0);
}



static int
foo_write(struct cdev *dev __unused, struct uio *uio,
    int ioflag __unused)
{

	printf("ifunit_ref() inside a write()\n");
	CURVNET_SET(TD_TO_VNET(uio->uio_td));
	ifunit_ref("FOO");
	CURVNET_RESTORE();
	return (0);
}



static int
foo_loader(struct module *m, int what, void *arg)
{
	int err = 0;

	switch (what) {
	case MOD_LOAD:
		printf("ifunit_ref() inside loader function, "
		    "before make_dev_p()\n");
		ifunit_ref("FOO");
		err = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
		    &foo_cdev,
		    &foo_cdevsw,
		    0,
		    UID_ROOT,
		    GID_WHEEL,
		    0600,
		    DEV_NAME);
		if (err) {
			printf("Failed to register %s\n", DEV_NAME);
			break;
		}

		printf("ifunit_ref() inside loader function, "
		    "after make_dev_p()\n");
		ifunit_ref("FOO");
		printf("foo_module loaded\n");
		break;
	case MOD_UNLOAD:
		destroy_dev(foo_cdev);
		printf("foo_module unloaded\n");
		break;
	default:
		err = EOPNOTSUPP;
		break;
	}

	return (err);
}



DEV_MODULE(foo_module, foo_loader, NULL);
