#include "usbasp.h"
#include <QDebug>
#include <string.h>
#include <QThread>
#include <sstream>



#define USB_EN_DEBUG_MODE       17
#define USB_DIS_DEBUG_MODE      18
#define USB_RW_DEBUG            19

const unsigned int USBasp::baudrate[] = {2400, 4800, 9600, 19200, 28800, 38400, 57600};
//---------------------------------------------------------------------------------//

USBasp::USBasp(){
    this->usbContext = nullptr;
    this->openedDevice = nullptr;
    this->devList = nullptr;

    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(&pTimer, SIGNAL(timeout()), this, SLOT(pingTick()));

    if(libusb_init(&this->usbContext)){
        throw USBException("USB subsystem cannot be initialized");
    }
}

USBasp::~USBasp(){
    this->clearDeviceList();

    if(this->usbContext != nullptr)
        libusb_exit(this->usbContext);
}

void USBasp::connectDevice(const int device, const enum baudrates baudRate){
    this->canclePing();
    this->_connectDevice(device, baudRate);
}

void USBasp::_connectDevice(const int device, const enum baudrates baudRate){
    USBasp::TBuffer temp;
    //first byte of the message sets the baudrate
    temp.len = baudRate;

    //make sure there is at least one device
    if(this->getDeviceCount() == 0)
        throw USBException("No device found");


    //test if something went wrong (first: try to open device, next switch device into debug mode)
    int err;
    if(USB_E_OK != (err = this->openDevice(device)) || 0 > USBTransmitControl(RECEIVE_ENABLE, USB_EN_DEBUG_MODE, &temp, &temp)){
        throw USBException("Device cannot be connected");
    }


    //start timer
    if(!timer.isActive()){
        timer.setInterval(8);
        timer.start();
    }
}


void USBasp::disconnetctDevice(){
    USBasp::TBuffer temp;

    if(!this->isConnected()){
            throw USBException("Device not connected");
    }

    if(timer.isActive())
        timer.stop();

    //set programmer to idle state
    if(0 > USBTransmitControl(RECEIVE_ENABLE, USB_DIS_DEBUG_MODE, &temp, &temp)){
            this->closeDevice();
            throw USBException("Device can not be set into idle mode, is USBAsp programmer still conected?");
    }

    this->closeDevice();
}

int USBasp::findDevices(){
    libusb_device_descriptor desc;

    this->clearDeviceList();
    int usbdevs = libusb_get_device_list(this->usbContext, &(this->devList));

    unsigned char manString[128];
    unsigned char prodSting[128];
    libusb_device_handle *tmpHandle;

    for(int i = 0; i < usbdevs; i++){
        libusb_get_device_descriptor(devList[i], &desc);

        if(desc.idVendor == VENDOR_ID && desc.idProduct == PRODUCT_ID){
            int err = 0;
            if((err = libusb_open(devList[i], &tmpHandle)) != 0){
                qDebug() << err;
                switch (err){
                    case LIBUSB_ERROR_NO_MEM:
                        throw USBException("USBasp device could not be open: no memory");
                    case LIBUSB_ERROR_ACCESS:
                        throw USBException("USBasp device could not be open: insufficient permissions");
                    case LIBUSB_ERROR_NO_DEVICE:
                        throw USBException("USBasp device could not be open: device does not exist");
                    default:
                        throw USBException("USBasp device could not be open: unknown error");
                }
            }

            libusb_get_string_descriptor_ascii(tmpHandle, desc.iManufacturer, manString, 128);
            libusb_get_string_descriptor_ascii(tmpHandle, desc.iProduct, prodSting, 128);

            this->progList.push_back(new TUSBaspProg(libusb_get_bus_number(devList[i]), libusb_get_device_address(devList[i]), i));

            libusb_close(tmpHandle);
        }
    }

    return this->getDeviceCount();
}

void USBasp::pingDevice(const int device) {
    this->canclePing();
    this->pDevice = device;


    this->pTimer.setInterval(150);
    this->pTimer.start();
}

int USBasp::openDevice(const int device){
    if(nullptr != this->openedDevice){
        this->closeDevice();
    }
    else if(!this->getDeviceCount()){
        throw USBException("No USBasp device detected");
    }

    libusb_open(this->devList[this->progList.at(device)->busindex] , &(this->openedDevice));

    if(this->openedDevice != nullptr){
        /*if(NULL == (handler = usb_open(device))){
            return USB_E_NO_HANDLER;
        }*/
    } else {
        return USB_E_NO_DEV;
    }

    return USB_E_OK;
}

const USBasp::TUSBaspProg *USBasp::getDevice(const int device){
    if(device >= this->getDeviceCount() && device < 0){
        std::stringstream ss;
        ss << "Device #" << device << " of " << this->getDeviceCount() << " cannot be opened.";
        throw new USBException(ss.str());
    }
    else {
        return (const TUSBaspProg *)this->progList.at(device);
    }
}

int USBasp::getDeviceCount(){
    qDebug() << progList.size();
    return this->progList.size();
}

int USBasp::isConnected(){
    return this->openedDevice != nullptr;
}

void USBasp::closeDevice(){
    if(nullptr != this->openedDevice){
        libusb_close(this->openedDevice);
        this->openedDevice = nullptr;
    }
}

void USBasp::clearDeviceList(){
    if(nullptr != this->devList){
        this->closeDevice();
        libusb_free_device_list(this->devList, 1);

        for(unsigned int i = 0; i < this->getDeviceCount(); i++){
            delete this->progList.at(i);
        }

        this->progList.clear();
    }
}

void USBasp::canclePing(){
    if(this->pTimer.isActive()){
        this->pTimer.stop();
        if(this->isConnected())
            this->disconnetctDevice();
    }
    this->pDevice = -1;
    this->pState = 0;
}


int USBasp::USBTransmitData(enum receive receive, unsigned char functionid, USBasp::TBuffer *txBuffer, USBasp::TBuffer *rxBuffer){
    int nbytes = 0, cycle = 0;
    USBasp::TBuffer tmpRxBuffer, tmpTxBuffer;

    // tmpfix
    rxBuffer->len = 0;

    do {
        //Copy data to tmp tx buffer
        for(int i = 0; i < 3; i++){
            if(i + 3 * cycle + 1 > txBuffer->len)
                break;

            tmpTxBuffer.data[tmpTxBuffer.len++] = txBuffer->data[i + 3 * cycle];
        }

        nbytes += USBTransmitControl(receive, functionid, &tmpTxBuffer, &tmpRxBuffer);
        tmpTxBuffer.len = 0;

        for(int i = 0; i < tmpRxBuffer.len; i++){
            rxBuffer->data[rxBuffer->len++] = tmpRxBuffer.data[i];
        }

        cycle++;
    } while(cycle * 3 < txBuffer->len);

    return nbytes;
}

int USBasp::USBTransmitControl(enum receive receive, unsigned char functionid, USBasp::TBuffer *txBuffer, USBasp::TBuffer *rxBuffer){
    if(nullptr == this->openedDevice)
        throw USBException("There is no USBasp device connected");

    int nbytes = libusb_control_transfer(
                this->openedDevice,
                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | (receive << 7),
                functionid,
                (txBuffer->data[0] << 8) | txBuffer->len,
                (txBuffer->data[2] << 8) | txBuffer->data[1],
                (unsigned char *)rxBuffer,
                rxBuffer->size,
                5000
    );

    return nbytes;
}

const USBasp::TBuffer * USBasp::readRecievedData(){
    return &rxBuffer;
}

char USBasp::USBSendChar(char c){
    if(txBuffer.len < DATA_LEN)
        txBuffer.data[txBuffer.len++] = c;

    return c;
}

void USBasp::timeout(){
    try {
        USBTransmitData(RECEIVE_ENABLE, USB_RW_DEBUG, &txBuffer, &rxBuffer);

        //reset buffer
        txBuffer.len = 0;

        //emit signal, when data are ready for read
        if(rxBuffer.len > 0)
            emit rxDataReady();
    }
    catch (USBException &e){
        Q_UNUSED(e)
    }
}

void USBasp::pingTick(){
    this->pState++;

    try {
        if(this->pState % 2){
            this->_connectDevice(this->pDevice, USBasp::USBASP_BAUD_2400);
        }
        else {
            this->disconnetctDevice();
        }
    } catch (USBException &e){
        Q_UNUSED(e)
        //qDebug() << e.what();
    }

    if(this->pState >= 20)
        this->canclePing();
}
