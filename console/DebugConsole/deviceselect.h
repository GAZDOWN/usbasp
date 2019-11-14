#ifndef DEVICESELECT_H
#define DEVICESELECT_H

#include <QDialog>
#include <QVector>
#include "usbasp.h"

namespace Ui {
    class deviceselect;
}

class DeviceSelect : public QDialog {
    Q_OBJECT

    USBasp *programmer;
    int selectedDevice;

    public:
        /**
         * @brief DeviceSelect - constructor
         * @param USBasp * programmer   a USBasp device manager
         * @param QWidget * parent      an optional pointer to the parent
         */
        explicit DeviceSelect(USBasp *programmer, QWidget *parent = nullptr);
        ~DeviceSelect();

        int getSelectedDevice();

    private:
        Ui::deviceselect *ui;

        /**
         * @brief generateProgrammerList function creates a GUI list of
         * found devices
         */
        void generateProgrammerList();

        /**
         * @brief destroyProgrammerList function remove the list of found
         * devices
         */
        void destroyProgrammerList();

    protected slots:
        /**
         * @brief startPinging slot send ping request to a USBasp device
         */
        void startPinging();

        /**
         * @brief refreshList refresh list of found USBasp devices
         */
        void refreshList();

        void deviceSelected();
};

#endif // DEVICESELECT_H
