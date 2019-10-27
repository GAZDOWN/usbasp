#include "console.h"
#include <QScrollBar>
#include <QDebug>
#include <QApplication>
#include <QStyleOption>
#include <QPainter>

const QColor Console::DEF_FONT_COLOR = QColor("#000000");
const QColor Console::DEF_BCKG_COLOR = QColor("#FFFFFF");

Console::Console(QWidget *parent) : QPlainTextEdit(parent){
    this->newLineCursorPosition = 0;
    this->rowCursorPosition = 0;

    this->setCursorWidth(5);
    this->setWordWrapMode(QTextOption::WrapAnywhere);
    this->setMaximumBlockCount(Console::DEF_MAX_ROWS);
    this->setOverwriteMode(true);

    //this->setTextInteractionFlags(this->textInteractionFlags() | Qt::TextSelectableByKeyboard);

    this->setTextInteractionFlags(Qt::TextSelectableByKeyboard);
}

void Console::setBackgroundColor(const QColor color){
    QPalette p = this->palette();
    p.setColor(QPalette::Base, color);
    this->setPalette(p);

    this->repaint();
}

void Console::setFontColor(const QColor color){
    QPalette p = this->palette();
    p.setColor(QPalette::Text, color);
    this->setPalette(p);

    this->repaint();
}

int Console::_putChar(char c){
    switch (c) {
        case 0x08:  if(this->newLineBuffer > 0){
                        this->newLineBuffer.chop(1);
                        this->newLineCursorPosition--;
                    }
                    break;

        case 0x1B:  this->escBuffer.append(ESC_2_BYTE);
                    break;

        case '\a':  QApplication::beep();
                    break;

        case '\n':  this->newLineBuffer.append(c);
                    break;

        case '\r':  this->newLineCursorPosition = 0;
                    break;


        default:    if(this->escBuffer.length() == 0){
                        if(this->newLineCursorPosition < this->newLineBuffer.length()){
                            this->newLineBuffer.replace(this->newLineCursorPosition, 1, c);
                        }
                        else {
                            this->newLineBuffer.append(c);
                        }
                        this->newLineCursorPosition++;
                    }
                    else { //escape codes
                        if(this->escBuffer.length() == 1 && c == 0x5B){
                            this->escBuffer.replace(0, 1, ESC_CSI);
                        }
                        else if(this->escBuffer.at(0) == ESC_2_BYTE && c >= 64 && c <= 95){
                            this->escBuffer.append(c);
                            //escape character handling
                            this->handleEscapeCharacter();
                        }
                        else if(this->escBuffer.at(0) == ESC_CSI && !(c >= 64 && c <= 126)){
                            this->escBuffer.append(c);

                        }
                        else if(this->escBuffer.at(0) == ESC_CSI && c >= 64 && c <= 126){
                            this->escBuffer.append(c);

                            //escape character handling
                            this->handleEscapeCharacter();
                        }
                        else {
                            QApplication::beep();
                            this->escBuffer.clear();
                        }
                    }
                    break;
    }

    return c;
}

void Console::handleEscapeCharacter(){
    if(this->escBuffer.at(0) == ESC_2_BYTE){
        if(this->escBuffer.at(1) == 'L'){
            this->clear();
        }
        else {
            QApplication::beep();
        }
    }
    else {
        if(this->escBuffer.length() == 3 && this->escBuffer.at(1) == '2' && this->escBuffer.at(2) == 'J'){
            this->clear();
        }
        else {
            QApplication::beep();
        }
    }


    this->escBuffer.clear();
}

char Console::putChar(char c){
    this->putString(QString(c));

    return c;
}

int Console::putString(QString str){
    if(str.length() == 0)
        return 0;


    int refresh = 1;

    for(int i = 0; i < str.length(); i++){
        this->_putChar(str.at(i).toLatin1());

        if(str.at(i) == '\n'){
            this->setPlainText(this->toPlainText().replace(this->rowCursorPosition, this->newLineBuffer.length(), this->newLineBuffer));
            this->moveCursor(QTextCursor::End);

            this->rowCursorPosition = this->toPlainText().lastIndexOf("\n") + 1;
            this->newLineBuffer.clear();

            refresh = 0;
        }
        else {
            refresh = 1;
        }
    }

    if(refresh){
        this->setPlainText(this->toPlainText().replace(this->rowCursorPosition, this->toPlainText().length() - this->rowCursorPosition, this->newLineBuffer));
        this->moveCursor(QTextCursor::End);
    }

    return str.length();
}

void Console::clear(){
    QPlainTextEdit::clear();

    this->newLineBuffer.clear();
    this->newLineCursorPosition = 0;
    this->rowCursorPosition = 0;
}

void Console::mousePressEvent(QMouseEvent *ev){
    Q_UNUSED(ev);
    this->setFocus();
}

void Console::paintEvent(QPaintEvent *e){
    QPlainTextEdit::paintEvent(e);
}



void Console::keyPressEvent(QKeyEvent *ev){
    if(ev->key() == Qt::Key_Backspace){
        QPlainTextEdit::keyPressEvent(ev);
        emit keyPressed(8);
    }
    else if(ev->key() == Qt::Key_Return && ev->modifiers() == Qt::NoModifier || ev->key() == Qt::Key_Enter)
        emit keyPressed('\n');
    else if(ev->key() == Qt::Key_Left){
        emit keyPressed(27);
        emit keyPressed('D');
    }
    else if(ev->key() == Qt::Key_Up){
        emit keyPressed(27);
        emit keyPressed('A');
    }
    else if(ev->key() == Qt::Key_Down){
        emit keyPressed(27);
        emit keyPressed('B');
    }
    else if(ev->key() == Qt::Key_Right){
        emit keyPressed(27);
        emit keyPressed('C');
    }
    else if(ev->key() == Qt::Key_U && ev->modifiers() == Qt::ControlModifier){
        //CTRL + U
        emit keyPressed(27);
        emit keyPressed('[');
        emit keyPressed('2');
        emit keyPressed('K');
    }
    else {
        if(ev->text().length() > 0 && 0 != ev->text().at(0).unicode())
            emit keyPressed(ev->text().at(0).unicode());
    }
}
