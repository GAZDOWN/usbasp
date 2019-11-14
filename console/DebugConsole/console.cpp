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

// TODO: rename this + refactor!!
bool Console::_putChar(char c){
    if(this->escBuffer.length() > 0){
        //escape codes
        /*if(this->escBuffer.length() == 1 && c == 0x5B){
            this->escBuffer.replace(0, 1, ESC_CSI);
        }
        else if(this->escBuffer.at(0) == ESC_2_BYTE && c >= 64 && c <= 95){

            // VT-100 ESC codes
            this->escBuffer.append(c);
            //escape character handling
            this->handleEscapeCharacter();
        }
        else if(this->escBuffer.at(0) == ESC_CSI && !(c >= 64 && c <= 126)){
            qDebug() << "VT-100: " << c;
            this->escBuffer.append(c);
        }
        else if(this->escBuffer.at(0) == ESC_CSI && c >= 64 && c <= 126){
            this->escBuffer.append(c);

            //escape character handling
            this->handleEscapeCharacter();
        }*/

        //VT-100
        if(this->escBuffer.length() == 1 && c == '[') {
            this->escBuffer.append(c);
        }
        // ESC[ + c (LEN 2)
        else if(this->escBuffer.length() == 2 && (c == '2' || c == '1')){
            this->escBuffer.append(c);
            //qDebug() << c;
        }
        // ESC[ + c + c (LEN 3)
        else if(this->escBuffer.length() == 3 && c == 'J'){
            // Clear console
            this->clear();
            this->escBuffer.clear();
        }
        else if(this->escBuffer.length() == 3 && c == 'D'){
            // Move cursor left
            for(int i = this->escBuffer.at(2).digitValue(); i > 0; i--){
                if(this->newLineCursorPosition > 0){
                    this->moveCursor(QTextCursor::Left);
                    this->newLineCursorPosition--;
                }
            }

            this->mode = Console::INSERT;

            this->escBuffer.clear();
        }
        else if(this->escBuffer.length() == 3 && c == 'C'){
            // Move cursor right
            for(int i = this->escBuffer.at(2).digitValue(); i > 0; i--){
                if(this->newLineBuffer.length() > this->newLineCursorPosition){
                    this->moveCursor(QTextCursor::Right);
                    this->newLineCursorPosition++;
                }
            }

            this->mode = Console::INSERT;

            this->escBuffer.clear();
        }
        // ERROR
        else {
            QApplication::beep();
            this->escBuffer.clear();
        }

        return false;
    }
    else {
        switch (c) {
            case '\b':  if(this->newLineCursorPosition > 0){ // backspace
                            this->newLineCursorPosition--;
                            this->newLineBuffer.remove(this->newLineCursorPosition, 1);
                        }
                        break;

            case 0x1B:  this->escBuffer.append(c);  // VT-52/100 Escape seq start - ESC = 27/0x1B
                        return false;

            case '\a':  QApplication::beep();
                        return false;

            case '\n':  this->newLineBuffer.append(c);
                        break;

            case '\r':  this->newLineCursorPosition = 0;
                        this->mode = Console::REPLACE;
                        return false;


            default:    //TODO: Copy & Insert at the same time
                        if(this->mode == Console::REPLACE){
                            this->newLineBuffer.replace(this->newLineCursorPosition, 1, c);
                        }
                        else if(this->mode == Console::INSERT) {
                            this->newLineBuffer.insert(this->newLineCursorPosition, c);
                        }
                        else {
                            this->newLineBuffer.append(c);
                        }

                        this->newLineCursorPosition++;

                        if(this->newLineCursorPosition == this->newLineBuffer.length()){
                            this->mode = Console::APPEND;
                        }

                        break;
        }
        return true;
    }
}

char Console::putChar(char c){
    this->putString(QString(c));

    return c;
}

int Console::putString(QString str){
    if(str.length() == 0)
        return 0;

    bool append = false, refresh = false;

    for(int i = 0; i < str.length(); i++){
        append = this->_putChar(str.at(i).toLatin1());

        if(str.at(i) == '\n'){
            this->setPlainText(this->toPlainText().replace(this->rowCursorPosition, this->newLineBuffer.length(), this->newLineBuffer));
            this->moveCursor(QTextCursor::End);

            this->rowCursorPosition = this->toPlainText().lastIndexOf("\n") + 1;
            this->newLineBuffer.clear();

            refresh = false;
        }
        else if(append) {
            refresh = true;
        }
    }

    if(refresh){
        QTextCursor cursor = this->textCursor();

        this->setPlainText(this->toPlainText().replace(this->rowCursorPosition, this->toPlainText().length() - this->rowCursorPosition, this->newLineBuffer));

        //reset cursor position
        cursor.setPosition(this->rowCursorPosition + this->newLineCursorPosition);
        this->setTextCursor(cursor);
    }

    return str.length();
}

//char Console::putChar(char c){
//    this->putString(QString(c));

//    return c;
//}

//int Console::putString(QString str){
//    if(str.length() == 0)
//        return 0;

//    bool append = false, refresh = false;

//    for(int i = 0; i < str.length(); i++){
//        append = this->_putChar(str.at(i).toLatin1());

//        if(str.at(i) == '\n'){
//            this->setPlainText(this->toPlainText().replace(this->rowCursorPosition, this->newLineBuffer.length(), this->newLineBuffer));
//            this->moveCursor(QTextCursor::End);

//            this->rowCursorPosition = this->toPlainText().lastIndexOf("\n") + 1;
//            this->newLineBuffer.clear();

//            refresh = false;
//        }
//        else if(append) {
//            refresh = true;
//        }
//    }

//    if(refresh){
//        /*QTextCursor tmp = this->textCursor();
//        tmp.movePosition(QTextCursor::End);

//        if(this->textCursor().position() != tmp.position()){
//            qDebug() << "middle";
//        }
//        else {*/
//            // Append at the end of line
//            this->setPlainText(this->toPlainText().replace(this->rowCursorPosition, this->toPlainText().length() - this->rowCursorPosition, this->newLineBuffer));
//            this->moveCursor(QTextCursor::End);
//        //}
//    }

//    return str.length();
//}

void Console::clear(){
    QPlainTextEdit::clear();

    this->newLineBuffer.clear();
    this->newLineCursorPosition = 0;
    this->rowCursorPosition = 0;
}

void Console::mousePressEvent(QMouseEvent *ev){
    Q_UNUSED(ev)
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
    else if((ev->key() == Qt::Key_Return && ev->modifiers() == Qt::NoModifier) || ev->key() == Qt::Key_Enter)
        emit keyPressed('\n');
    else if(ev->key() == Qt::Key_Left){
        emit keyPressed(27);
        emit keyPressed('[');
        emit keyPressed('1');
        emit keyPressed('D');
    }
    else if(ev->key() == Qt::Key_Up){
        emit keyPressed(27);
        emit keyPressed('[');
        emit keyPressed('1');
        emit keyPressed('A');
    }
    else if(ev->key() == Qt::Key_Down){
        emit keyPressed(27);
        emit keyPressed('[');
        emit keyPressed('1');
        emit keyPressed('B');
    }
    else if(ev->key() == Qt::Key_Right){
        emit keyPressed(27);
        emit keyPressed('[');
        emit keyPressed('1');
        emit keyPressed('C');
    }
    else if(ev->key() == Qt::Key_U && ev->modifiers() == Qt::ControlModifier){
        //CTRL + U
        emit keyPressed(27);
        emit keyPressed('[');
        emit keyPressed('2');
        emit keyPressed('K');
    }
    else if(ev->key() == Qt::Key_U && ev->modifiers() == Qt::ControlModifier){
        //CTRL + C
        emit keyPressed(0x03);
    }
    else {
        if(ev->text().length() > 0 && 0 != ev->text().at(0).unicode())
            emit keyPressed(ev->text().at(0).unicode());
    }
}
