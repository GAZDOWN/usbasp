#ifndef SETTINGS
#define SETTINGS

#include <QSettings>
#include <QFont>

class Settings {
    QSettings *settings;

public:
    static const QString SET_PATH;
    static const QString SET_RESTORE_GEOMETRY;
    static const QString SET_GEOMETRY_VALUE;
    static const QString SET_DEFAULT_BAUDRATE;
    static const QString SET_DEFAULT_BAUDRATE_TYPE;
    static const QString SET_FONT_NAME;
    static const QString SET_FONT_SIZE;
    static const QString SET_FONT_COLOR;
    static const QString SET_CONSOLE_BACKGROUND_COLOR;
    static const QString SET_CONSOLE_ROWS;
    static const QString SET_FONT_WEIGHT;

    static const int BRT_MANUAL = 0;
    static const int BRT_AUTO = 1;

    static const int FONT_NORMAL = 0;
    static const int FONT_BOLD = 1;

    Settings();
    ~Settings();

    void sync();

    /**
     * @brief validateGeometry validate given value
     * @param value
     * @return
     */
    static int validateGeometry(const int value);
    static int validateNoOfRows(const unsigned int value);
    static int validateFontSize(const unsigned int value);
    static int validateBaudRate(const int value);
    static int validateBaudRateType(const int value);
    static int validateFontWeight(const int value);
    static QFont validateFontFamily(const QFont font);
    QColor validateColor(const QColor color, const QColor defaultColor);

    QString loadStringValue(const QString &name, const QString &defaultValue = QString());
    int loadIntValue(const QString &name, const int defaultValue = 0);

    int     getRestoreGeometry();
    int     getNoOfRows();
    int     getFontSize();
    int     getFontWeight();
    int     getBaudRateType();
    int     getBaudRate();
    QFont   getFontFamily();
    QByteArray getRestoreGeometryValue();
    QColor  getFontColor();
    QColor  getConsoleBackgroundColor();

    void setNoOfRows(const int rows);
    void setRestoreGeometry(const int restore);
    void setFontSize(const int size);
    void setBaudRateType(const int type);
    void setBaudRate(const int baudrate);
    void setFontFamily(const QFont &family);
    void setRestoreGeometryValue(QByteArray geometry);
    void setFontColor(QColor color);
    void setConsoleBackgroundColor(QColor color);
    void setFontWeight(const int weight);
};


#endif // SETTINGS

