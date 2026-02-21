#include "usb.h"

#include "common.h"
#include "fifo.h"

#include <libusb.h>

static libusb_context *s_ctx = nullptr;
static libusb_device **s_devs = nullptr;
static libusb_device_handle *s_handle = nullptr;
static FIFO<char, USB_TX_BUFFER_SIZE> s_usb_tx_buf;
static FIFO<char, USB_RX_BUFFER_SIZE> s_usb_rx_buf;

int usb_init()
{
    libusb_init_option options[] = {
        {LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING},
    };

    int res = libusb_init_context(&s_ctx, options, sizeof(options) / sizeof(options[0]));
    if (res < 0)
    {
        return res;
    }

    return 0;
}

int usb_deinit()
{
    if (s_ctx)
    {
        libusb_exit(s_ctx);
        s_ctx = nullptr;
    }

    return 0;
}

int usb_connect(int vendor_id, int product_id)
{
    int res = 0;

    // Get the list of USB devices
    ssize_t cnt = libusb_get_device_list(s_ctx, &s_devs);
    if (cnt < 0)
    {
        return (int)cnt;
    }

    int found = -1;

    // Search for a device matching the desired VID and PID
    for (ssize_t i = 0; i < cnt; i++)
    {
        libusb_device *dev = s_devs[i];
        libusb_device_descriptor desc;
        res = libusb_get_device_descriptor(dev, &desc);
        if (res < 0)
        {
            continue;
        }

        if (desc.idVendor == vendor_id && desc.idProduct == product_id)
        {
            found = i;
            break;
        }
    }

    if (found < 0)
    {
        TRACE_PRINTF("Device with VID:PID %04x:%04x not found\n", vendor_id, product_id);

        // Free the device list
        libusb_free_device_list(s_devs, 1);

        return -1;
    }
    TRACE_PRINTF("Device with VID:PID %04x:%04x found\n", vendor_id, product_id);

    // Check config descriptor
    libusb_config_descriptor *config = nullptr;
    libusb_get_config_descriptor(s_devs[found], 0, &config);
    if (config)
    {
        for (uint8_t i = 0; i < config->bNumInterfaces; i++)
        {
            const libusb_interface &interface = config->interface[i];
            for (int j = 0; j < interface.num_altsetting; j++)
            {
                const libusb_interface_descriptor &altsetting = interface.altsetting[j];
                for (uint8_t k = 0; k < altsetting.bNumEndpoints; k++)
                {
                    const libusb_endpoint_descriptor &endpoint = altsetting.endpoint[k];
                    TRACE_PRINTF("Interface: %d, Endpoint %d: address=0x%02x, attributes=0x%02x, max_packet_size=%d\n",
                                 i, k, endpoint.bEndpointAddress, endpoint.bmAttributes, endpoint.wMaxPacketSize);
                }
            }
        }
        libusb_free_config_descriptor(config);
    }

    // Open the device
    res = libusb_open(s_devs[found], &s_handle);

    // Free the device list
    libusb_free_device_list(s_devs, 1);

    if (res < 0 || s_handle == nullptr)
    {
        TRACE_PRINTF("Failed to open device with VID:PID %04x:%04x, error: %d\n", vendor_id, product_id, res);
        return -1;
    }

    // Claim the CDC interface (interface 0)
    res = libusb_claim_interface(s_handle, 0);
    res = libusb_claim_interface(s_handle, 1);
    if (res < 0)
    {
        TRACE_PRINTF("Failed to claim interface 0 on device with VID:PID %04x:%04x, error: %d\n", vendor_id, product_id, res);
        return res;
    }

    return 0;
}

int usb_disconnect()
{
    if (s_handle)
    {
        libusb_release_interface(s_handle, 1);
        libusb_release_interface(s_handle, 0);
        libusb_close(s_handle);
        s_handle = nullptr;
    }

    return 0;
}

void usb_task()
{
    int res;

    if (s_usb_tx_buf.num_items() > 0)
    {
        // Prepare a contiguous buffer for transmission
        unsigned char tx_buf[USB_TX_BUFFER_SIZE];
        int tx_len = s_usb_tx_buf.num_items();
        s_usb_tx_buf.pop((char *)tx_buf, tx_len);

        res = libusb_bulk_transfer(s_handle, EPNUM_CDC_0_OUT, tx_buf, tx_len, nullptr, 100);
        if (res < 0)
        {
            TRACE_PRINTF("Failed to send data, error: %d\n", res);
        }
    }

    if (!s_usb_tx_buf.is_full())
    {
        // Try to receive at most the remaining space in the buffer
        // Prepare a contiguous buffer for reception
        unsigned char rx_buf[USB_RX_BUFFER_SIZE];
        int rx_len = s_usb_rx_buf.space_left();

        int actual_length = 0;
        res = libusb_bulk_transfer(s_handle, EPNUM_CDC_0_IN, rx_buf, rx_len, &actual_length, 100);
        if (res < 0 && res != LIBUSB_ERROR_TIMEOUT)
        {
            TRACE_PRINTF("Failed to receive data, error: %d\n", res);
        }
        else if (actual_length > 0)
        {
            s_usb_rx_buf.push((char *)rx_buf, actual_length);
        }
    }
}

int usb_send(const char *data, int len)
{
    if (!data || len > s_usb_tx_buf.space_left())
    {
        return -1;
    }

    s_usb_tx_buf.push(data, len);

    return 0;
}

int usb_available()
{
    return s_usb_rx_buf.num_items();
}

int usb_receive(char *buf, int len)
{
    if (!buf)
    {
        return -1;
    }

    // Receive at most len bytes
    int to_receive = min(len, s_usb_rx_buf.num_items());
    if (to_receive > 0)
    {
        s_usb_rx_buf.pop(buf, to_receive);
    }

    return to_receive;
}