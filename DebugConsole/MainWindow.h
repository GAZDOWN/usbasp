#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QLabel>
#include <QFile>
#include "usbasp.h"
#include "settingsdialog.h"
//#include <QSettings>
#include "settings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    USBasp  programmer;

    QLabel  status;
    QFileDialog *fileDialog;
    QFile *outputFile;
    
    SettingsDialog *setDialog;

    Settings *settings;

    int selectedProgrammer;

public:    
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
    bool loadSettings(bool refresh = false);

private:
    Ui::MainWindow *ui;
    void closeEvent(QCloseEvent *e);

private slots:
    void connectProgrammer();
    void disconnectProgrammer();
    int selectProgrammer();

    void rxDataReady();
    //void txDataReady(char c);

    void saveToFileEnabled(bool s);
    void fileSelected(QString s);
    void fileRejected();

    void showSettingsDialog();
    void settingsDialogClosed();
    void settingsDialogAccepted();
};

#endif // MAINWINDOW_H
