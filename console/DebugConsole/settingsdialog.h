#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <QSettings>
#include <usbasp.h>
#include "settings.h"

typedef struct __loaded_settings__ {

} LoadedSettings_t;

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

    QColorDialog    *fcDialog;
    QColorDialog    *bcDialog;
    Settings        *settings;

    QColor fColor;
    QColor bColor;

public:

    /**
     * @brief SettingsDialog
     * @param settings  a pointer to opened QSettings object
     * @param parent    parent object of created object
     * @throws std::invalid_argument
     */
    explicit SettingsDialog(Settings *settings, QWidget *parent = 0);
    ~SettingsDialog();

protected:
    /**
     * @brief loadSettings loads setting from file and prepares internal variables
     *        in the SettingsDialog object.
     * @return true it there was no error
     */
    bool loadSettings();

private:
    Ui::SettingsDialog *ui;

private slots:
    void storeNewSettings();
    void disableDefaultBaudrateSettings(bool disabled);
    void enableDefaultBaudrateSettings(bool disabled);
    void setDisableDefaultBaudrateSettings(bool disabled);
    void showBackgroundColorDialog();
    void showFontColorDialog();
    void fontColorAccepted();
    void consoleColorAccepted();
};

#endif // SETTINGSDIALOG_H
