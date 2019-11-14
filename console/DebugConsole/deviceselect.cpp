#include "deviceselect.h"
#include "ui_deviceselect.h"
#include <QLabel>
#include <QDebug>
#include "deviceitem.h"

DeviceSelect::DeviceSelect(USBasp *programmer, QWidget *parent) : QDialog(parent), ui(new Ui::deviceselect) {
    ui->setupUi(this);

    this->programmer = programmer;
    this->selectedDevice = -1;

    this->setModal(true);

    this->generateProgrammerList();

    connect(this->ui->refreshBtn, SIGNAL(clicked(bool)), this, SLOT(refreshList()));
}

DeviceSelect::~DeviceSelect() {
    this->destroyProgrammerList();

    delete ui;
}

int DeviceSelect::getSelectedDevice(){
    return this->selectedDevice;
}

void DeviceSelect::generateProgrammerList(){
    this->selectedDevice = -1;

    try {
        this->programmer->findDevices();
    } catch (USBException &e){
        //TODO: Add some meaningfull message for user
        return;
    }

    if(!this->programmer->getDeviceCount()){
        // No device
        QLabel *label = new QLabel("No device found");

        QHBoxLayout * hlayout = new QHBoxLayout();
        hlayout->addWidget(label);

        this->ui->progListArea->addLayout(hlayout);
    }
    else {
        // TODO: Make a widget out of this select processing only known data
        for(int i = 0; i < this->programmer->getDeviceCount(); i++){
            QRadioButton * radio = new QRadioButton(QString("Device %1 on bus %2").arg(this->programmer->getDevice(i)->device).arg(this->programmer->getDevice(i)->bus), this);
            if(i == 0){
                radio->setChecked(true);
                this->selectedDevice = 0;
            }
            connect(radio, SIGNAL(toggled(bool)), this, SLOT(deviceSelected()));

            QPushButton * button = new QPushButton(QString("ping"), this);
            button->setProperty("progid", QVariant(i));
            connect(button, SIGNAL(clicked(bool)), this, SLOT(startPinging()));

            QHBoxLayout * hlayout = new QHBoxLayout();
            hlayout->addWidget(radio);
            hlayout->addWidget(button);

            this->ui->progListArea->addLayout(hlayout);
        }
    }
}

void DeviceSelect::destroyProgrammerList(){
    for(int i = 0; i < this->ui->progListArea->count(); i++){
        QLayout * hlayout = this->ui->progListArea->itemAt(i)->layout();

        while(hlayout->count() > 0){
            QWidget *w = hlayout->itemAt(0)->widget();
            hlayout->removeWidget(w);
            delete w;
        }

        this->ui->progListArea->removeItem(hlayout);

        delete hlayout;
    }
}

void DeviceSelect::startPinging(){
    int progid = QObject::sender()->property("progid").toInt();
    this->programmer->pingDevice(progid);
}

void DeviceSelect::refreshList(){
    this->destroyProgrammerList();
    this->generateProgrammerList();
}

void DeviceSelect::deviceSelected(){
    this->selectedDevice = QObject::sender()->property("progid").toInt();
}
