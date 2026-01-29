#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <libinput.h>
#include <libudev.h>

static int
open_restricted(const char *path, int flags, void *user_data)
{
	int fd = open(path, flags);
	if (fd < 0)
		perror(path);
	return fd;
}

static void
close_restricted(int fd, void *user_data)
{
	close(fd);
}

static const struct libinput_interface libinput_iface = {
	.open_restricted = open_restricted,
	.close_restricted = close_restricted,
};

static void
libopeninput_log_handler(struct libinput *libinput,
                     enum libinput_log_priority priority,
                     const char *format,
                     va_list args) {
    const char *p;

    switch (priority) {
    case LIBINPUT_LOG_PRIORITY_DEBUG: p = "DEBUG"; break;
    case LIBINPUT_LOG_PRIORITY_INFO:  p = "INFO";  break;
    case LIBINPUT_LOG_PRIORITY_ERROR: p = "ERROR"; break;
    default:                          p = "?";     break;
    }

    fprintf(stderr, "[libinput %s] ", p);
    vfprintf(stderr, format, args);
}

int
main(void)
{
	struct udev *udev;
	struct libinput *li;
	struct pollfd fds;
	struct libinput_event *event;

	printf("::: The libopeninput test running…\n");

	udev = udev_new();
	if (!udev) {
		fprintf(stderr, ">>> udev_new failed\n");
		return 1;
	}

	li = libinput_udev_create_context(&libinput_iface,
	                                  NULL, udev);
	if (!li) {
		fprintf(stderr, ">>> libinput_udev_create_context failed\n");
		return 1;
	} else {
		libinput_log_set_handler(li, libopeninput_log_handler);
		libinput_log_set_priority(li, LIBINPUT_LOG_PRIORITY_DEBUG);
	}

	if (libinput_udev_assign_seat(li, "seat0") != 0) {
		fprintf(stderr, ">>> assign_seat failed\n");
		return 1;
	}

	printf("::: Initial devices:\n");

	libinput_dispatch(li);

	while ((event = libinput_get_event(li)) != NULL) {
		if (libinput_event_get_type(event) == LIBINPUT_EVENT_DEVICE_ADDED) {
			struct libinput_device *dev = libinput_event_get_device(event);
			printf("   *** %s\n", libinput_device_get_sysname(dev));
		}
		libinput_event_destroy(event);
	}
	printf("\n");

	fds.fd = libinput_get_fd(li);
	fds.events = POLLIN;

	printf("::: Check plug/unplug devices to see events\n\n");

	while (1) {
		poll(&fds, 1, -1);

		libinput_dispatch(li);

		while ((event = libinput_get_event(li)) != NULL) {
			enum libinput_event_type type =
			    libinput_event_get_type(event);

			switch (type) {

			case LIBINPUT_EVENT_DEVICE_ADDED: {
				struct libinput_device *dev = libinput_event_get_device(event);
				printf("   *** DEVICE ADDED: %s\n", libinput_device_get_sysname(dev));
				break;
			}

			case LIBINPUT_EVENT_DEVICE_REMOVED: {
				struct libinput_device *dev = libinput_event_get_device(event);
				printf("   *** DEVICE REMOVED: %s\n", libinput_device_get_sysname(dev));
				break;
			}

			default:
				break;
			}

			libinput_event_destroy(event);
		}
	}

	printf("::: The libopeninput test finished\n");

	libinput_unref(li);
	udev_unref(udev);
	return 0;
}
