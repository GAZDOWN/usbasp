#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>

class DeviceItem : public QWidget{
        Q_OBJECT

        int device;

        QHBoxLayout     *hlayout;
        QPushButton     *pingButton;
        QRadioButton    *radio;

    public:
        explicit DeviceItem(const int device, const QString label, QWidget *parent = 0);
        ~DeviceItem();

        QHBoxLayout * getItem();

    signals:
        void pingClicked(int);

    protected slots:
        void emitPingClicked();
};

#endif // DEVICEITEM_H
