#ifndef USBASP_H
#define USBASP_H

#include <libusb.h>
#include <QObject>
#include <stdexcept>
#include <QTimer>
#include <vector>
#include <iostream>

#define USB_E_OK            0
#define USB_E_NO_DEV        1
#define USB_E_NO_HANDLER    2

class USBException : public std::runtime_error {
    public:
        explicit USBException(std::string const & msg) : std::runtime_error(msg) {}
};

class USBasp : public QObject {
        Q_OBJECT

    public:
        //usb identification
        static const unsigned int VENDOR_ID  = 0x16C0;
        static const unsigned int PRODUCT_ID = 0x05DC;

        //maximal size of io buffers
        static const unsigned int DATA_LEN = 128;

        //allowed baudrates values
        enum baudrates {
            USBASP_BAUD_2400 = 0,
            USBASP_BAUD_4800,
            USBASP_BAUD_9600,
            USBASP_BAUD_19200,
            USBASP_BAUD_28800,
            USBASP_BAUD_38400,
            USBASP_BAUD_57600 //experimental, in most case will not work
        };

        //integer value of selected baudrate
        static const unsigned int baudrate[];

        //buffer structure
        //structure size is corresponding with data
        //received from device -> data array can be cast
        //to this structure
        typedef struct _data_buffer_ {
            unsigned char   len;
            char            data[DATA_LEN];
            unsigned char   size;

            _data_buffer_() : len(0), data{0}, size(DATA_LEN) {}
        } TBuffer;

        typedef struct _usbasp_prog_data_ {
            const int bus;
            const int device;

            _usbasp_prog_data_(const int _bus, const int _device) : bus(_bus), device(_device) {}
            _usbasp_prog_data_(const _usbasp_prog_data_ &b) : bus(b.bus), device(b.device) {}
        } TUSBaspProgInfo;

    private:
        enum receive {
            RECEIVE_DISABLE = 0,
            RECEIVE_ENABLE
        };

        libusb_context          *usbContext;
        libusb_device_handle    *openedDevice;
        libusb_device           **devList;

        std::vector<libusb_device *> progList;

        QTimer                  timer;

        int                     pState;
        int                     pDevice;
        QTimer                  pTimer;


    protected:
        TBuffer rxBuffer;
        TBuffer txBuffer;

    public:
        USBasp();
        ~USBasp();

        /**
         * Connect USBasp device
         * @param baudRate - enumeration, select supported bandwidth
         * @throw USBException if any error ocurs
         */
        void connectProgrammer(const int device, const enum baudrates baudRate);

        /**
         * Disconnect USBasp device
         * @throw USBException if any error ocurs
         */
        void disconnetctProgrammer();

        /**
         * Get recived data
         * @return TBuffer with data
         */
        const TBuffer * readRecievedData();

        /**
         * @brief findDevices function must be used before any other call as it is designed
         * to find all USBasp devices connected to the computer. This function initialize
         * internal list of USBasp devices and returns the number of found devices.
         * @return int
         * @throws USBException
         */
        int findProgrammers();

        /**
         * @brief pingDevice initialize ping request which will be sent to device selected by
         * *device* parameter. The reason for this function is an easy way to determine which
         * device is actually which, because pinging process is just blinking with the green
         * LED on the device.
         * @param int device    a number of found device
         */
        void pingProgrammer(const int device);

        /**
         * @brief getDevice function returns a structure with USBasp identifier.
         * @param device    a number of found device.
         * @return TUSBaspProg
         */
        const TUSBaspProgInfo getProgrammerInfo(const int device);

        /**
         * @brief getDeviceCount returns the amount of found USBasp devices.
         * @return int
         */
        int getProgrammerCount();

        /**
         * @brief isConnected function returns true in case there is any device actively connected
         * when this function is called.
         * @return int
         */
        int isConnected();
private:
        /**
         * Connect USBasp device
         * @param baudRate - enumeration, select supported bandwidth
         * @throw USBException if any error ocurs
         */
        void _connectDevice(const int device, const enum baudrates baudRate);

        /**
         * @brief USBOpenDevice
         * @param device    a usb device to open
         */
        int openDevice(libusb_device * device);


        /**
         * USBCloseDevice should be used to close current opened tty device
         */
        void closeDevice();

        /**
         * @brief clearDeviceList function clears the list of found USBasp devices
         * which were found by findDevices function.
         */
        void clearProgrammerList();

        void cancelPing();


        /**
         * send data to target device
         * (low level function)
         * @param unsigned char     receive     - enable receive (true / false)
         * @param unsigned char     functionid  - operation
         * @param char *            txBuffer    - tx buffer
         * @param char *            rxBuffer    - rx buffer
         *
         * @return int (look into libusb usb_control_msg documentation)
         * @throw USBException if any runtime error ocurs
         */
        int USBTransmitData(enum receive receive, unsigned char functionid, TBuffer *txBuffer, TBuffer *rxBuffer);

        /**
         * send CONTROL data to target device (max 4 bytes)
         * (low level function)
         * @param unsigned char     receive     - enable receive (true / false)
         * @param unsigned char     functionid  - operation
         * @param char *            txBuffer    - tx buffer
         * @param char *            rxBuffer    - rx buffer
         *
         * @return int (look into libusb usb_control_msg documentation)
         * @throw USBException if any runtime error ocurs
         */
        int USBTransmitControl(enum receive receive, unsigned char functionid, TBuffer *txBuffer, TBuffer *rxBuffer);

    public slots:
        /**
         * Send char to the target device
         * Character are send every 1ms as a chunk of data
         * @param c - character which will be sent
         * @return - sended character
         */
        char USBSendChar(char c);

    private slots:
        void timeout();
        void pingTick();

    signals:
        /**
         * this signal is emmited when PC has received data from target device
         */
        void rxDataReady();
};

#endif // USBASP_H
