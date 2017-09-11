#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QDebug>
#include "console.h"

using namespace std;


SettingsDialog::SettingsDialog(Settings *settings, QWidget *parent) : QDialog(parent), ui(new Ui::SettingsDialog){
    ui->setupUi(this);

    this->fcDialog = NULL;
    this->bcDialog = NULL;

    if(settings == NULL)
        throw invalid_argument("Settings cannot be null");
    else
        this->settings = settings;

    //load allowed baudrates
    for(unsigned int i = USBasp::USBASP_BAUD_2400; i < USBasp::USBASP_BAUD_57600; i++){
        this->ui->defaultBaudRate->addItem(QString::number(USBasp::baudrate[i]), i);
    }

    connect(this->ui->buttonBox, SIGNAL(accepted()), this, SLOT(storeNewSettings()));
    connect(this->ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(this->ui->baudrateRemeberButton, SIGNAL(clicked(bool)), this, SLOT(disableDefaultBaudrateSettings(bool)));
    connect(this->ui->baudrateDefaultButton, SIGNAL(clicked(bool)), this, SLOT(enableDefaultBaudrateSettings(bool)));

    connect(this->ui->selectFontColorButton, SIGNAL(clicked()), this, SLOT(showFontColorDialog()));
    connect(this->ui->selectBackgroundCOlorButton, SIGNAL(clicked()), this, SLOT(showBackgroundColorDialog()));

    this->loadSettings();
}

SettingsDialog::~SettingsDialog(){
    if(this->fcDialog != NULL)
        this->fcDialog->deleteLater();

    if(this->bcDialog != NULL)
        this->bcDialog->deleteLater();

    delete ui;
}

bool SettingsDialog::loadSettings(){
    //window
    this->ui->restGeometry->setCheckState(this->settings->getRestoreGeometry() == false ? Qt::Unchecked : Qt::Checked);

    //baudrate
    if(Settings::BRT_MANUAL == this->settings->getBaudRateType()){
        this->enableDefaultBaudrateSettings(true);
        this->ui->baudrateDefaultButton->setChecked(true);
    }
    this->ui->defaultBaudRate->setCurrentIndex(this->settings->getBaudRate());

    //font
    this->ui->fontSize->setValue(this->settings->getFontSize());
    this->ui->fontName->setCurrentFont(this->settings->getFontFamily());
    this->fColor = this->settings->getFontColor();
    this->ui->selectFontColorButton->setStyleSheet("background-color: " + this->fColor.name());
    this->ui->fontWeight->setCheckState(this->settings->getFontWeight() == false ? Qt::Unchecked : Qt::Checked);

    //console
    this->ui->noOfLines->setValue(this->settings->getNoOfRows());
    this->bColor = this->settings->getConsoleBackgroundColor();
    this->ui->selectBackgroundCOlorButton->setStyleSheet("background-color: " + this->bColor.name());

    return true;
}


//=== FONT AND CONSOLE SETTINGS ==================================


void SettingsDialog::showFontColorDialog(){
    if(this->fcDialog != NULL){
        this->fcDialog->deleteLater();
        //this->fcDialog = NULL;
    }

    this->fcDialog = new QColorDialog(this);
    this->fcDialog->setCurrentColor(this->fColor);

    connect(this->fcDialog, SIGNAL(accepted()), this, SLOT(fontColorAccepted()));

    this->fcDialog->show();
}

void SettingsDialog::showBackgroundColorDialog(){
    if(this->bcDialog != NULL){
        this->bcDialog->deleteLater();
    }

    this->bcDialog = new QColorDialog(this);
    this->bcDialog->setCurrentColor(this->bColor);

    connect(this->bcDialog, SIGNAL(accepted()), this, SLOT(consoleColorAccepted()));

    this->bcDialog->show();
}

void SettingsDialog::fontColorAccepted(){
    this->fColor = this->fcDialog->currentColor();
    this->ui->selectFontColorButton->setStyleSheet("background-color: " + this->fColor.name());
    this->ui->selectFontColorButton->clearFocus();
}

void SettingsDialog::consoleColorAccepted(){
    this->bColor = this->bcDialog->currentColor();
    this->ui->selectBackgroundCOlorButton->setStyleSheet("background-color: " + this->bColor.name());
    this->ui->selectBackgroundCOlorButton->clearFocus();
}


void SettingsDialog::disableDefaultBaudrateSettings(bool disabled){
    Q_UNUSED(disabled);

    this->setDisableDefaultBaudrateSettings(true);
}

void SettingsDialog::enableDefaultBaudrateSettings(bool disabled){
    Q_UNUSED(disabled);

    this->setDisableDefaultBaudrateSettings(false);
}

void SettingsDialog::setDisableDefaultBaudrateSettings(bool disabled){
    //qDebug() << "set switch";
    this->ui->defaultBaudRate->setDisabled(disabled);
    this->ui->defaultBaudrateLabel->setDisabled(disabled);
}



void SettingsDialog::storeNewSettings(){

    //qDebug() << "store" << (NULL == this->settings);

    try {
        //window
        this->settings->setRestoreGeometry((int)(this->ui->restGeometry->checkState() == Qt::Checked));

        //baudrate
        this->settings->setBaudRate(this->ui->defaultBaudRate->currentIndex());
        this->settings->setBaudRateType(this->ui->baudrateDefaultButton->isChecked() ? 0 : 1);

        //font
        this->settings->setFontFamily(this->ui->fontName->currentFont());
        this->settings->setFontSize(this->ui->fontSize->value());
        this->settings->setFontColor(this->fColor);
        this->settings->setFontWeight((int)this->ui->fontWeight->checkState() == Qt::Checked);

        //console
        this->settings->setConsoleBackgroundColor(this->bColor);
        this->settings->setNoOfRows(this->ui->noOfLines->value());

        this->settings->sync();
        this->accept();
    }
    catch(std::exception &e){
        qDebug() << e.what();
    }
}
