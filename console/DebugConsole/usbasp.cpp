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

    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(&pTimer, SIGNAL(timeout()), this, SLOT(pingTick()));

    if(libusb_init(&this->usbContext)){
        throw USBException("USB subsystem cannot be initialized");
    }
}

USBasp::~USBasp(){
    this->clearProgrammerList();

    if(this->usbContext != nullptr)
        libusb_exit(this->usbContext);
}

void USBasp::connectProgrammer(const int device, const enum baudrates baudRate){
    this->cancelPing();
    this->_connectDevice(device, baudRate);
}

void USBasp::_connectDevice(const int device, const enum baudrates baudRate){
    USBasp::TBuffer temp;
    //first byte of the message sets the baudrate
    temp.len = baudRate;

    //make sure there is at least one device
    if(this->getProgrammerCount() == 0)
        throw USBException("No device found");


    if(this->openDevice(this->progList.at(device)) || 0 > USBTransmitControl(RECEIVE_ENABLE, USB_EN_DEBUG_MODE, &temp, &temp)){
        throw USBException("Device cannot be connected");
    }


    //start timer
    if(!timer.isActive()){
        timer.setInterval(8);
        timer.start();
    }
}


void USBasp::disconnetctProgrammer(){
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

int USBasp::findProgrammers(){
    libusb_device_descriptor desc;

    this->clearProgrammerList();
    int usbdevs = libusb_get_device_list(this->usbContext, &(this->devList));

    for(int i = 0; i < usbdevs; i++){
        libusb_get_device_descriptor(this->devList[i], &desc);

        if(desc.idVendor == VENDOR_ID && desc.idProduct == PRODUCT_ID){
            // Check if device is accessible, if it is, push it into list of found programmers
            try {
                this->openDevice(this->devList[i]);
                this->closeDevice();
                this->progList.push_back(this->devList[i]);
            }  catch (USBException &e) {
                this->clearProgrammerList();
                throw e;
            }
        }
    }

    return this->getProgrammerCount();
}

void USBasp::pingProgrammer(const int device) {
    this->cancelPing();
    this->pDevice = device;


    this->pTimer.setInterval(150);
    this->pTimer.start();
}

int USBasp::openDevice(libusb_device * device){
    if(nullptr != this->openedDevice){
        this->closeDevice();
    }

    int err = 0;
    if((err = libusb_open(device, &(this->openedDevice))) != 0){
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

    return 0;
}

const USBasp::TUSBaspProgInfo USBasp::getProgrammerInfo(const int device){
    if(device >= this->getProgrammerCount() && device < 0){
        std::stringstream ss;
        ss << "Device #" << device << " of " << this->getProgrammerCount() << " cannot be opened.";
        throw new USBException(ss.str());
    }
    else {        
        return TUSBaspProgInfo(libusb_get_bus_number(this->progList.at(device)), libusb_get_device_address(this->progList.at(device)));
    }
}

int USBasp::getProgrammerCount(){
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

void USBasp::clearProgrammerList(){
    if(nullptr != this->devList){
        this->closeDevice();
        libusb_free_device_list(this->devList, 1);
        this->devList = nullptr;
        this->progList.clear();
    }
}

void USBasp::cancelPing(){
    if(this->pTimer.isActive()){
        this->pTimer.stop();
        if(this->isConnected())
            this->disconnetctProgrammer();
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
            this->disconnetctProgrammer();
        }
    } catch (USBException &e){
        Q_UNUSED(e)
        //qDebug() << e.what();
    }

    if(this->pState >= 20)
        this->cancelPing();
}
