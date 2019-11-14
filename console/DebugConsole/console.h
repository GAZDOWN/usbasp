#ifndef CONSOLE_H
#define CONSOLE_H

#include <QTextBrowser>
#include <QPlainTextEdit>
#include <QKeyEvent>

class Console : public QPlainTextEdit {
    Q_OBJECT
public:
    /** default number of visible rows **/
    static const unsigned int DEF_MAX_ROWS = 100;
    /** default size of font */
    static const unsigned int DEF_FONT_SIZE = 9;

    /** minimal size of font in points **/
    static const unsigned int MIN_FONT_SIZE = 8;
    /** maximal size of font in points **/
    static const unsigned int MAX_FONT_SIZE = 20;

    /** the minimal amount of visible rows **/
    static const unsigned int MIN_ROWS = 50;
    /** the maximal amount of visible rows **/
    static const unsigned int MAX_ROWS = 500;

    /** defaul font color **/
    static const QColor DEF_FONT_COLOR;
    /** default backgroun color **/
    static const QColor DEF_BCKG_COLOR;

    explicit Console(QWidget *parent = 0);

    /**
     * @brief setBackgroundColor set the background color of console widget
     * @param QColor color
     */
    void setBackgroundColor(const QColor color);

    /**
     * @brief setFontColor set the color of the font used in the console widget
     * @param QColor color
     */
    void setFontColor(const QColor color);


protected:
    typedef enum __insert_mode__ {
        APPEND = 0,
        INSERT,
        REPLACE
    } TInsertMode;

    typedef enum __esc_mode__ {
        VT52 = '0',
        VT100
    } TESCMode;


    //static const unsigned int   ESC_2_BYTE = '0';
    //static const unsigned int   ESC_CSI = '1';

    /** @var mode holds current mode of inserting a character into the console */
    TInsertMode mode = Console::APPEND;

    /** @var escBuffer holds escape sequence */
    QString escBuffer;

    /** @var rowCursorPosition holds position of statr of current line */
    int rowCursorPosition;

    /** @var newLineBuffer holds current line bufferred */
    QString newLineBuffer;

    /** @var newLineCursorPosition holds position of cursor in current line */
    int newLineCursorPosition;


    void keyPressEvent(QKeyEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *e);


    bool _putChar(char c);

public slots:
    /**
     * @brief putChar put char to console and send it to device
     * @param char c    character
     * @return char c
     */
    char putChar(char c);

    /**
     * @brief putString put string to console (info text etc)
     * @param String str
     * @return int num of printed characters
     */
    int putString(QString str);

    /**
     * @brief clear Clear the console and ingoing buffers.
     */
    void clear();


signals:
    void keyPressed(char c);
};

#endif // CONSOLE_H
