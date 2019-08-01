#include "codeeditorwindow.h"

CodeEditorWindow::CodeEditorWindow(Game *g) :
    game(g),
    do_add_script(false)
{
    wrapmode = QTextOption::NoWrap;
    textedits.resize(2);
    textedits[0] = new QPlainTextEdit();

    wordwrap_checkbox = new QCheckBox();
    wordwrap_checkbox->setChecked(false);
    wordwrap_checkbox->setText("Word Wrap");
    connect(wordwrap_checkbox, SIGNAL(clicked(bool)), this, SLOT(SlotToggleWordWrap()));

    button_setroomcode = new QPushButton("Save");
    connect(button_setroomcode, SIGNAL(clicked(bool)), this, SLOT(SlotSaveChanges()));

    button_getroomcode = new QPushButton("Load");
    connect(button_getroomcode, SIGNAL(clicked(bool)), this, SLOT(SlotGetRoomCode()));

    tabwidget = new QTabWidget();
    tabwidget->setStyleSheet("QWidget {color: #FFFFFF; background: #2F363B;}"); //Hover: #3E4D54; Click: #1F2227
    connect(tabwidget, SIGNAL(currentChanged(int)), this, SLOT(UpdateTabContents(int)));

    QGridLayout * v = new QGridLayout();
    v->addWidget(wordwrap_checkbox,0,0);
    v->addWidget(button_getroomcode, 1, 0);
    v->addWidget(button_setroomcode, 1, 1);
    v->addWidget(tabwidget,2,0,1,2);

    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);

    error_log = new QPlainTextEdit();
    error_log->setFont(f);
    error_log->setWordWrapMode(QTextOption::WordWrap);

    error_log_clear_btn = new QPushButton("Clear");
    connect(error_log_clear_btn, SIGNAL(clicked(bool)), this, SLOT(ClearErrorLog()));

    v->addWidget(new QLabel("Log:"),3,0,1,1);
    v->addWidget(error_log_clear_btn,3,1,1,1);
    v->addWidget(error_log,4,0,1,2);

    v->setAlignment(Qt::AlignTop);
//    v->setSpacing(0);
//    v->setMargin(1);

    setLayout(v);
}

void CodeEditorWindow::ClearErrorLog()
{
    error_log->setPlainText("");
}

void CodeEditorWindow::AddError(const QString s)
{
    //59.6 - error log only contains last 2000 characters
    const unsigned int max_error_log_length = 2000;
    error_log->setPlainText(error_log->toPlainText().right(max_error_log_length) + s + "\n");
    error_log->verticalScrollBar()->setValue(error_log->verticalScrollBar()->maximum());
}

void CodeEditorWindow::UpdateTabContents(int i)
{
    if (i > 0 && i == tabwidget->count()-1) {
        do_add_script = true;
    }    
    cur_room.clear();
}

void CodeEditorWindow::Update()
{    
    QPointer <Room> r = game->GetEnvironment()->GetCurRoom();
    if (r.isNull()) {
        return;
    }   

    if (do_add_script) {
        //59.3 - create new script
        r->AddNewAssetScript();
        do_add_script = false;        
    }   

    //update if empty and visible, or we changed rooms
    if (r->GetLoaded() && cur_room != r) {
        //temporarily disconnect index changed signal
        const int last_index = tabwidget->currentIndex();
        disconnect(tabwidget, SIGNAL(currentChanged(int)), 0, 0);

        cur_room = r;

        //obtain list of AssetScripts
        QFont f("unexistent");
        f.setStyleHint(QFont::Monospace);

        QString room_code;
        QTextStream ofs(&room_code);
        r->SaveXML(ofs);

        QHash <QString, QPointer <AssetScript> > scripts = r->GetAssetScripts();

        //delete existing textedits, and resize vector
        for (int i=0; i<textedits.size(); ++i) {
            if (textedits[i]) {
                delete textedits[i];
            }
        }
        textedits.resize(scripts.size()+2);

        tabwidget->clear();
        textedits[0] = new QPlainTextEdit();
        textedits[0]->setFont(f);
        textedits[0]->setWordWrapMode(wrapmode);
        textedits[0]->setPlainText(room_code);
        tabwidget->addTab(textedits[0], "JML");

        int i = 0;
        for (QPointer <AssetScript> & a : scripts) {
            ++i;

            if (a) {
                textedits[i] = new QPlainTextEdit();
                textedits[i]->setFont(f);
                textedits[i]->setWordWrapMode(wrapmode);
                if (a->GetFinished() && !a->GetError()) {
                    textedits[i]->setPlainText(a->GetJSCode());
                }
                else if (a->GetError()) {
                    textedits[i]->setPlainText(QString("Error: ") + a->GetErrorString());
                }
                else {
                    textedits[i]->setPlainText("Loading...");
                }

                tabwidget->addTab(textedits[i], a->GetProperties()->GetSrc());
            }
        }

        //59.3 - add "plus" tab, to create new assetscripts
        textedits[textedits.size()-1] = new QPlainTextEdit();
        tabwidget->addTab(textedits[textedits.size()-1], "+");

        if (last_index < tabwidget->count()) {
            tabwidget->setCurrentIndex(last_index);
        }

        //reconnect index changed signal
        connect(tabwidget, SIGNAL(currentChanged(int)), this, SLOT(UpdateTabContents(int)));
    }

    //update error log
    if (game) {
//        qDebug() << "errors" << MathUtil::GetErrorLogTemp().size() << AssetScript::GetErrors().size();
        const QStringList accum_errs = MathUtil::GetErrorLogTemp();
        for (int i=0; i<accum_errs.size(); ++i) {
            AddError(accum_errs[i]);
        }
        MathUtil::ClearErrorLogTemp();
    }
}

bool CodeEditorWindow::GetHasFocus()
{
    for (int i=0;i<textedits.size(); ++i) {
        if (textedits[i] && textedits[i]->hasFocus()) {
            return true;
        }
    }
    return false;
}

void CodeEditorWindow::SlotSaveChanges()
{
    const QString room_filename = game->GetEnvironment()->GetCurRoom()->GetSaveFilename();
    const bool visible = isVisible();
    if (!visible) { //not visible, just save as usual
        game->SaveRoom(room_filename);
    }
    else { //visible, save directly what's in the text, and update the room
        QFile file(room_filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "CodeEditorWindow::SlotSaveChanges(): File " << room_filename << " can't be saved";
            return;
        }

        //save out the data
        QTextStream ofs(&file);
        ofs << textedits[0]->toPlainText();
        //close file, report saving ok
        file.close();

        QPointer <Room> r = game->GetEnvironment()->GetCurRoom();
        QHash <QString, QPointer <AssetScript> > scripts = r->GetAssetScripts();

        int i = 0;
        for (QPointer <AssetScript> & a : scripts) {
            ++i;           

            if (a) {
                //59.4 - do not save if script had not finished loading, or Loading... message is shown
                QString js_code = textedits[i]->toPlainText();
                if (!a->GetFinished() || js_code == "Loading...") {
                    continue;
                }
                a->SetJSCode(textedits[i]->toPlainText());

                const QString js_filename = QUrl(a->GetProperties()->GetSrcURL()).toLocalFile();

    //            qDebug() << "a->GetFullURL();" << a->GetFullURL() << js_filename;
                QFile file(js_filename);
                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    qDebug() << "CodeEditorWindow::SlotSaveChanges(): JS File " << js_filename << " can't be saved";
                    return;
                }

                //save out the data
                QTextStream ofs(&file);
                ofs << textedits[i]->toPlainText();
                //close file, report saving ok
                file.close();
            }
        }

        if (!textedits.isEmpty()) {
            const QString code = textedits[0]->toPlainText();
            game->GetEnvironment()->UpdateRoomCode(code);
        }
    }
}

void CodeEditorWindow::SlotToggleWordWrap()
{
    if (wordwrap_checkbox->isChecked()) {
        wrapmode = QTextOption::WordWrap;
    }
    else {
        wrapmode = QTextOption::NoWrap;
    }

    for (int i=0;i<textedits.size(); ++i) {
        textedits[i]->setWordWrapMode(wrapmode);
    }
}

void CodeEditorWindow::SlotGetRoomCode()
{
    QString room_code;
    QTextStream ofs(&room_code);
    game->GetEnvironment()->GetCurRoom()->SaveXML(ofs);

    if (!textedits.isEmpty()) {
//        qDebug() << "CodeEditorWindow::SlotGetRoomCode()" << room_code;
       textedits[0]->setPlainText(room_code);
    }
}
