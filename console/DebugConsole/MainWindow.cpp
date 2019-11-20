#include "MainWindow.h"
#include <QMessageBox>
#include "ui_MainWindow.h"
#include "settingsdialog.h"
#include "deviceselect.h"
#include <QSettings>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->selectedProgrammer = -1;

    //SETTINGS
    this->settings = new Settings();

    this->fileDialog = NULL;
    this->outputFile = NULL;

    //menu actions
    connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(connectProgrammer()));
    connect(ui->actionSelectProgrammer, SIGNAL(triggered()), this, SLOT(selectProgrammer()));
    connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectProgrammer()));
    connect(ui->actionClearConsole, SIGNAL(triggered()), this->ui->consoleBox, SLOT(clear()));
    connect(ui->actionClearConsole, SIGNAL(triggered()), this->ui->consoleBox, SLOT(setFocus()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(showSettingsDialog()));


    //connect / disconnect button
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectProgrammer()));

    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectProgrammer()));
    ui->disconnectButton->setDisabled(true);

    //clear button
    connect(ui->clearTerm, SIGNAL(clicked()), ui->consoleBox, SLOT(clear()));
    connect(ui->clearTerm, SIGNAL(clicked()), ui->consoleBox, SLOT(setFocus()));

    //save to file
    connect(this->ui->saveToFile, SIGNAL(toggled(bool)), this, SLOT(saveToFileEnabled(bool)));

    //baudrates
    for(unsigned int i = USBasp::USBASP_BAUD_2400; i < USBasp::USBASP_BAUD_57600; i++){
        this->ui->baudRates->addItem(QString::number(USBasp::baudrate[i]), i);
    }

    //rx / tx data ready signal
    connect(&programmer, SIGNAL(rxDataReady()), this, SLOT(rxDataReady()));
    connect(ui->consoleBox, SIGNAL(keyPressed(char)), &programmer, SLOT(USBSendChar(char)));

    //add label into status bar
    this->statusBar()->addWidget(&(this->status));

    //set console focus
    ui->consoleBox->setFocus();

    this->loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;

    if(NULL != this->fileDialog)
        delete this->fileDialog;

    if(NULL != this->outputFile){
        this->outputFile->close();
        delete this->outputFile;
    }

    delete this->settings;
}

bool MainWindow::loadSettings(bool refresh){
    if(!refresh)
        this->restoreGeometry(this->settings->getRestoreGeometryValue());

    this->ui->baudRates->setCurrentIndex(this->settings->getBaudRate());

    qDebug() << this->settings->getFontColor();

    this->ui->consoleBox->setBackgroundColor(this->settings->getConsoleBackgroundColor());
    this->ui->consoleBox->setFontColor(this->settings->getFontColor());
    //this->ui->connectButton->setFont();

    QFont font = this->settings->getFontFamily();
    font.setPointSize(this->settings->getFontSize());
    font.setBold(this->settings->getFontWeight());
    qDebug() << font;

    this->ui->consoleBox->setFont(font);

    this->ui->consoleBox->update();

    return true;
}

void MainWindow::connectProgrammer(){
    try {
        //try to select programmer
        if(this->selectedProgrammer < 0 && !this->selectProgrammer()){
            return;
        }

        //this->programmer.findDevices();
        //programmer.connectDevice(0, (enum USBasp::baudrates)this->ui->baudRates->itemData(ui->baudRates->currentIndex()).toInt());
        programmer.connectDevice(this->selectedProgrammer, (enum USBasp::baudrates)this->ui->baudRates->itemData(ui->baudRates->currentIndex()).toInt());
        ui->disconnectButton->setDisabled(false);
        ui->connectButton->setText("Reco&nnect");
        ui->consoleBox->setFocus();

        const USBasp::TUSBaspProg *prog = this->programmer.getDevice(this->selectedProgrammer);

        this->status.setText(QString("Connected to USBAsp (DEV: %1, BUS %2) at " + this->ui->baudRates->itemText(ui->baudRates->currentIndex()) + " bauds").arg(prog->device).arg(prog->bus));
    }
    catch (USBException &e){
        QMessageBox(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok).exec();
        this->selectedProgrammer = -1;
    }
}

void MainWindow::disconnectProgrammer(){
    try {

        this->status.clear();
        programmer.disconnetctDevice();
    }
    catch (USBException &e){
        QMessageBox(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok).exec();
        this->selectedProgrammer = -1;
    }
    ui->disconnectButton->setDisabled(true);
    ui->connectButton->setText("Connect");
    ui->consoleBox->setFocus();
    //ui->consoleBox->putString("<font style=\"color: red\">USBasp programmer successfully disconnected</font><br>");
}

int MainWindow::selectProgrammer(){
    this->selectedProgrammer = -1;

    if(this->programmer.isConnected())
        this->disconnectProgrammer();
    try {
        this->programmer.findDevices();

        if(this->programmer.getDeviceCount() == 0){
            QMessageBox(QMessageBox::Critical, "Error", "No USBasp device found", QMessageBox::Ok).exec();
        }
        else if(this->programmer.getDeviceCount() <= 1){
            // One device only, there is no need for additional user action
            this->selectedProgrammer = 0;

            return 1;
        }
        else {
            // Multiple devices, let the user choose
            DeviceSelect devsel(&(this->programmer), this);
            devsel.exec();

            if(devsel.result() == QDialog::Accepted){
                this->selectedProgrammer = devsel.getSelectedDevice();
                return 1;
            }
        }
    } catch (USBException &e){
        QMessageBox(QMessageBox::Critical, "Error", e.what(), QMessageBox::Ok).exec();
    }


    return 0;
}

void MainWindow::rxDataReady(){
    const USBasp::TBuffer *b = programmer.readRecievedData();

    QString line = QString(b->data).left(b->len);

    ui->consoleBox->putString(line);

    //save to file
    if(NULL != this->outputFile && this->outputFile->isOpen()){
        this->outputFile->write(line.toUtf8());
    }
}

void MainWindow::closeEvent(QCloseEvent *e){
    Q_UNUSED(e);
    try {
        programmer.disconnetctDevice();
    }
    catch (USBException &ex){
        Q_UNUSED(ex);
    }

    this->settings->setRestoreGeometryValue(this->saveGeometry());

    if(this->settings->getBaudRateType() == Settings::BRT_AUTO){
        this->settings->setBaudRate(this->ui->baudRates->currentIndex());
    }
}

void MainWindow::saveToFileEnabled(bool s){
    if(s){
        if(NULL != this->fileDialog){
            delete this->fileDialog;
        }
        
        this->fileDialog = new QFileDialog(this, tr("Select output file"));

        QStringList filters;
        filters << "Text files (*.txt)"
                << "Any files (*)";

        this->fileDialog->setNameFilters(filters);

        connect(this->fileDialog, SIGNAL(fileSelected(QString)), this, SLOT(fileSelected(QString)));
        connect(this->fileDialog, SIGNAL(rejected()), this, SLOT(fileRejected()));

        this->fileDialog->show();
    }
    else {
        if(NULL != this->outputFile){
            this->outputFile->close();
            delete this->outputFile;
            this->outputFile = NULL;
        }
    }
}

void MainWindow::fileSelected(QString s){
    if(s.size()){
        this->outputFile = new QFile(s);
        this->outputFile->open(QIODevice::Append);
    }
}

void MainWindow::fileRejected(){
    this->ui->saveToFile->setCheckState(Qt::Unchecked);
}

void MainWindow::showSettingsDialog(){
    this->setDialog = new SettingsDialog(this->settings, this);

    connect(this->setDialog, SIGNAL(accepted()), this, SLOT(settingsDialogAccepted()));
    connect(this->setDialog, SIGNAL(rejected()), this, SLOT(settingsDialogClosed()));

    this->setDialog->show();
}

void MainWindow::settingsDialogAccepted(){
    this->loadSettings(true);
    this->settingsDialogClosed();
}

void MainWindow::settingsDialogClosed(){
    disconnect(this->setDialog, SIGNAL(accepted()), this, SLOT(settingsDialogAccepted()));
    disconnect(this->setDialog, SIGNAL(rejected()), this, SLOT(settingsDialogClosed()));

    this->setDialog->deleteLater();
}
