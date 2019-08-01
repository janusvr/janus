#ifndef CODEEDITORWINDOW_H
#define CODEEDITORWINDOW_H

#include <QtGui>
#include <QtWidgets>

#include "game.h"

class CodeEditorWindow : public QWidget
{
    Q_OBJECT

public:

    CodeEditorWindow(Game *  g);

    void Update();
    bool GetHasFocus();

public slots:

    void SlotSaveChanges();
    void SlotToggleWordWrap();    
    void SlotGetRoomCode();
    void UpdateTabContents(int i);

    void AddError(const QString s);
    void ClearErrorLog();

private:

    QPointer <Game> game;
    QPointer <Room> cur_room;

    QPointer <QPushButton> button_setroomcode;
    QPointer <QPushButton> button_getroomcode;
    QPointer <QTabWidget> tabwidget;
    QVector <QPointer <QPlainTextEdit> > textedits;
    QPointer <QCheckBox> wordwrap_checkbox;
    QTextOption::WrapMode wrapmode;
    bool do_add_script;
    QPointer <QPlainTextEdit> error_log;
    QPointer <QPushButton> error_log_clear_btn;
};

#endif // CODEEDITORWINDOW_H
