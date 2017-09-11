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
    this->usbContext = NULL;
    this->openedDevice = NULL;
    this->devList = NULL;
    this->progListSize = 0;

    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(&pTimer, SIGNAL(timeout()), this, SLOT(pingTick()));

    if(libusb_init(&this->usbContext)){
        throw USBException("USB subsystem cannot be initialized");
        //return USB_E_NO_DEV;
    }
}

USBasp::~USBasp(){
    this->clearDeviceList();

    if(this->usbContext != NULL)
        libusb_exit(this->usbContext);
}

void USBasp::connectDevice(const int device, const enum baudrates baudRate) throw (USBException) {
    this->canclePing();
    this->_connectDevice(device, baudRate);
}

void USBasp::_connectDevice(const int device, const enum baudrates baudRate) throw (USBException) {
    char temp[4] = {0};
    temp[0] = baudRate;

    //make sure there is at least one device
    if(this->getDeviceCount() == 0)
        throw USBException("No device found");


    //test if something went wrong (first: try to open device, next switch device into debug mode)
    int err;
    if(USB_E_OK != (err = this->openDevice(device)) || 0 > USBTransmit(1, USB_EN_DEBUG_MODE, temp, temp, sizeof(temp))){
        throw USBException("Device cannot be connected");
        //throw USBException(QString("Device can not be connected"));
    }


    //start timer
    if(!timer.isActive()){
        timer.setInterval(8);
        timer.start();
    }
}


void USBasp::disconnetctDevice() throw (USBException) {
    char temp[4];
    memset(temp, 0, sizeof(temp));


    if(!this->isConnected()){
            throw USBException("Device not connected");
    }

    if(timer.isActive())
        timer.stop();

    //set programmer to idle state
    if(0 > USBTransmit(1, USB_DIS_DEBUG_MODE, temp, temp, sizeof(temp))){
            this->closeDevice();
            throw USBException("Device can not be set into idle mode, is USBAsp programmer still conected?");
    }

    this->closeDevice();
}

int USBasp::findDevices(){
    libusb_device_descriptor desc;

    this->clearDeviceList();
    int usbdevs = libusb_get_device_list(this->usbContext, &(this->devList));

    for(int i = 0; i < usbdevs; i++){
        libusb_get_device_descriptor(devList[i], &desc);

        if(desc.idVendor == VENDOR_ID && desc.idProduct == PRODUCT_ID){
            this->progListSize++;

            unsigned char manString[128];
            unsigned char prodSting[128];

            libusb_device_handle *tmpHandle;

            if(libusb_open(devList[i], &tmpHandle)){
                //return USB_E_NO_HANDLER;
                continue;
            }

            libusb_get_string_descriptor_ascii(tmpHandle, desc.iManufacturer, manString, 128);
            libusb_get_string_descriptor_ascii(tmpHandle, desc.iProduct, prodSting, 128);

            /*qDebug()
                    << "device:" << libusb_get_device_address(devList[i])
                    << "bus:" << libusb_get_bus_number(devList[i])
                    << "man:" << QString((char *)manString)
                    << "prod:" << QString((char *)prodSting)
                    ;*/

            this->progList.push_back(new TUSBaspProg(libusb_get_bus_number(devList[i]), libusb_get_device_address(devList[i]), i));

            libusb_close(tmpHandle);
        }
    }

    return this->progListSize;
}

void USBasp::pingDevice(const int device) {
    this->canclePing();
    this->pDevice = device;


    this->pTimer.setInterval(150);
    this->pTimer.start();
}

int USBasp::openDevice(const int device){
    if(NULL != this->openedDevice){
        this->closeDevice();

    }

    libusb_open(this->devList[this->progList.at(device)->busindex] , &(this->openedDevice));

    if(this->openedDevice != NULL){
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
    return this->progListSize;
}

int USBasp::isConnected(){
    return this->openedDevice != NULL;
}

void USBasp::closeDevice(){
    if(NULL != this->openedDevice){
        libusb_close(this->openedDevice);
        this->openedDevice = NULL;
    }
}

void USBasp::clearDeviceList(){
    if(NULL != this->devList){
        this->closeDevice();
        libusb_free_device_list(this->devList, 1);
        this->progListSize = 0;

        for(unsigned int i = 0; i < this->progList.size(); i++){
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

int USBasp::USBTransmit(unsigned char receive, unsigned char functionid, char *send, char * buffer, int buffersize) throw (USBException) {
    //make sure receive is either 0 or 1
    receive &= 0x1;

    if(NULL == this->openedDevice)
        throw USBException("There is no USBasp device connected");

    int nbytes = libusb_control_transfer(
                this->openedDevice,
                LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | (receive << 7),
                functionid,
                (send[1] << 8) | send[0],
                (send[3] << 8) | send[2],
                (unsigned char *)buffer,
                buffersize,
                5000
    );

    /*if(nbytes < 0){
        throw USBException(QString("Error durring the transmission:  %1").arg(nbytes).arg(trUtf8(strerror(nbytes))));
    }*/

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
        USBTransmit(1, USB_RW_DEBUG, (char *)&(txBuffer), (char *)&rxBuffer, DATA_LEN);

        //reset buffer
        txBuffer.len = 0;

        //emit signal, when data are ready for read
        if(rxBuffer.len > 0)
            emit rxDataReady();
    }
    catch (USBException &e){}
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
        //qDebug() << e.what();
    }

    if(this->pState >= 20)
        this->canclePing();
}
