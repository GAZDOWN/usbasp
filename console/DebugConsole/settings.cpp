#include "settings.h"
#include "console.h"
#include "usbasp.h"
#include <QDebug>

const QString Settings::SET_PATH = ".gusbaspdebugconsole";

const QString Settings::SET_RESTORE_GEOMETRY          = "RestoreGeometry";
const QString Settings::SET_GEOMETRY_VALUE            = "GeometryValue";
const QString Settings::SET_DEFAULT_BAUDRATE          = "DafaultBaudrate";
const QString Settings::SET_DEFAULT_BAUDRATE_TYPE     = "BaudrateType";
const QString Settings::SET_FONT_NAME                 = "FontName";
const QString Settings::SET_FONT_SIZE                 = "FontSize";
const QString Settings::SET_FONT_COLOR                = "FontColor";
const QString Settings::SET_CONSOLE_BACKGROUND_COLOR  = "ConsoleBackgroundColor";
const QString Settings::SET_CONSOLE_ROWS              = "ConsoleNoOfRows";
const QString Settings::SET_FONT_WEIGHT               = "FontWeight";


Settings::Settings(){
    this->settings = new QSettings(Settings::SET_PATH, QSettings::NativeFormat);
}

Settings::~Settings(){
    delete this->settings;
}

void Settings::sync(){
    this->settings->sync();
}


//=== DATA VALIDATORS ===================================================
int Settings::validateGeometry(const int value){
    if(value == 0 || value == 1)
        return value;
    else
        return 1;
}

int Settings::validateNoOfRows(const unsigned int value){
    if(value >= Console::MIN_ROWS && value <= Console::MAX_ROWS)
        return value;
    else
        return Console::DEF_MAX_ROWS;
}

int Settings::validateFontSize(const unsigned int value){
    if(value >= Console::MIN_FONT_SIZE && value <= Console::MAX_FONT_SIZE)
        return value;
    else
        return Console::DEF_FONT_SIZE;
}

int Settings::validateBaudRate(const int value){
    if(value >= USBasp::USBASP_BAUD_2400 && value < USBasp::USBASP_BAUD_57600)
        return value;
    else
        return USBasp::USBASP_BAUD_9600;
}

int Settings::validateBaudRateType(const int value){
    if(value == Settings::BRT_MANUAL || value == Settings::BRT_AUTO)
        return value;
    else
        return Settings::BRT_MANUAL;
}

int Settings::validateFontWeight(const int value){
    if(value == Settings::FONT_NORMAL || value == Settings::FONT_BOLD)
        return value;
    else
        return Settings::FONT_NORMAL;
}

QFont Settings::validateFontFamily(const QFont font){
    QFontDatabase db = QFontDatabase();

    if(db.families().indexOf(font.family()) >= 0 && db.isFixedPitch(font.family())){
        return font;
    }
    else {
        return QFontDatabase::systemFont(QFontDatabase::FixedFont);
    }
}

QColor Settings::validateColor(const QColor color, const QColor defaultColor){
    if(color.isValid())
        return color;
    else
        return defaultColor;
}

//=== DATA LOADERS ======================================================
QString Settings::loadStringValue(const QString &name, const QString &defaultValue){
    if(this->settings->contains(name)){
        return this->settings->value(name).toString();
    }
    else
        return defaultValue;
}

int Settings::loadIntValue(const QString &name, const int defaultValue){
    if(this->settings->contains(name)){
        return this->settings->value(name).toInt();
    }
    else
        return defaultValue;
}

//=== GETTERS ==========================================================

int Settings::getRestoreGeometry(){
    return Settings::validateGeometry(this->loadIntValue(Settings::SET_RESTORE_GEOMETRY));
}

int Settings::getNoOfRows(){
    return Settings::validateNoOfRows(this->loadIntValue(Settings::SET_CONSOLE_ROWS));
}

int Settings::getFontSize(){
    return Settings::validateFontSize(this->loadIntValue(Settings::SET_FONT_SIZE, Console::DEF_FONT_SIZE));
}

int Settings::getBaudRateType(){
    return Settings::validateBaudRateType(this->loadIntValue(Settings::SET_DEFAULT_BAUDRATE_TYPE));
}

int Settings::getBaudRate(){
    return Settings::validateBaudRate(this->loadIntValue(Settings::SET_DEFAULT_BAUDRATE, USBasp::USBASP_BAUD_9600));
}

int Settings::getFontWeight(){
    return Settings::validateFontWeight(this->settings->value(Settings::SET_FONT_WEIGHT).toInt());
}

QFont Settings::getFontFamily(){
    return Settings::validateFontFamily(QFont(this->loadStringValue(Settings::SET_FONT_NAME)));
}

QByteArray Settings::getRestoreGeometryValue(){
    return this->settings->value(Settings::SET_GEOMETRY_VALUE).toByteArray();
}

QColor Settings::getFontColor(){
    return Settings::validateColor(QColor(this->loadStringValue(Settings::SET_FONT_COLOR)), Console::DEF_FONT_COLOR);
}

QColor Settings::getConsoleBackgroundColor(){
    return Settings::validateColor(QColor(this->loadStringValue(Settings::SET_CONSOLE_BACKGROUND_COLOR)), Console::DEF_BCKG_COLOR);
}

//=== SETTERS =============================================================

void Settings::setRestoreGeometry(const int restore){
    this->settings->setValue(Settings::SET_RESTORE_GEOMETRY, Settings::validateGeometry(restore));
}

void Settings::setNoOfRows(const int rows){
    this->settings->setValue(Settings::SET_CONSOLE_ROWS, Settings::validateNoOfRows(rows));
}

void Settings::setFontSize(const int size){
    this->settings->setValue(Settings::SET_FONT_SIZE, Settings::validateFontSize(size));
}

void Settings::setBaudRateType(const int type){
    this->settings->setValue(Settings::SET_DEFAULT_BAUDRATE_TYPE, Settings::validateBaudRateType(type));
}

void Settings::setBaudRate(const int baudrate){
    this->settings->setValue(Settings::SET_DEFAULT_BAUDRATE, Settings::validateBaudRate(baudrate));
}

void Settings::setFontFamily(const QFont &family){
    this->settings->setValue(Settings::SET_FONT_NAME, Settings::validateFontFamily(family).family());
}

void Settings::setRestoreGeometryValue(QByteArray geometry){
    this->settings->setValue(Settings::SET_GEOMETRY_VALUE, geometry);
}

void Settings::setFontColor(QColor color){
    this->settings->setValue(Settings::SET_FONT_COLOR, validateColor(color, Console::DEF_FONT_COLOR).name());
}

void Settings::setConsoleBackgroundColor(QColor color){
    this->settings->setValue(Settings::SET_CONSOLE_BACKGROUND_COLOR, validateColor(color, Console::DEF_BCKG_COLOR).name());
}

void Settings::setFontWeight(const int weight){
    this->settings->setValue(Settings::SET_FONT_WEIGHT, validateFontWeight(weight));
}
