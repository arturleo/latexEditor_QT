#include <QDebug>

#include "latexhandler.h"

LatexHandler::LatexHandler(displayMode mode,
                           int fontsize,
                           QObject *parent)
    : QObject(parent)
    , m_latex(nullptr)
    , m_cursorPosition(-1)
    , m_selectionStart(0)
    , m_selectionEnd(0)
    , m_mode(mode)
    , m_fontSize(fontsize)
{
    //TODO
    //m_http = new httpRequest(m_setting->value("mathJaxUrl").toString());
//    connect(this, &LatexHandler::documentChanged,
//            this, &LatexHandler::mathDisplay);

    //connect(m_client,&latexClient::updateDoc,
            //this,&LatexHandler::updateDoc);
}

LatexHandler::LatexHandler(latexClient *client,
                           QSettings *setting,
                           displayMode mode,
                           int fontsize,
                           QObject *parent)
    : QObject(parent)
    , m_latex(nullptr)
    , m_cursorPosition(-1)
    , m_selectionStart(0)
    , m_selectionEnd(0)
    , m_fontSize(fontsize)
{
    setDisplaymode(mode);
    m_setting=setting;
    m_client=client;
	//TODO
    m_http = new httpRequest(m_setting->value("mathJaxUrl").toString());
//    connect(this, &LatexHandler::documentChanged,
//            this, &LatexHandler::mathDisplay);

    connect(m_client,&latexClient::updateDoc,
            this,&LatexHandler::updateDoc
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
    connect(m_http, &httpRequest::displaySvg,
            this, qOverload<QByteArray>(&LatexHandler::displaySvg)
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));

}

LatexHandler::~LatexHandler()
{
    m_http->disconnect();

    QHash<QString, QTemporaryFile*>::iterator itr=maths.begin();
    for(;itr!=maths.end();itr++)
    {
        itr.value()->close();
    }
    //m_client->disconnect();
}

void LatexHandler::initHighlighter()
{
    m_highlighter = new latexBasicHighlighter(textDocument(), m_fontSize);
    connect(m_highlighter, &latexBasicHighlighter::newmath,
            this, &LatexHandler::mathDisplay
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
    connect(this, &LatexHandler::onDisplaymodeChanged,
            m_highlighter, &latexBasicHighlighter::setDisplayMode
            ,(Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
    connect(m_highlighter, &latexBasicHighlighter::resetDoc,
            this, &LatexHandler::resetDoc
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
}

void LatexHandler::init(QSettings *setting, latexClient* client)
{
    m_setting=setting;
    m_client=client;
    m_http = new httpRequest(m_setting->value("mathJaxUrl").toString());
//    connect(this, &LatexHandler::documentChanged,
//            this, &LatexHandler::mathDisplay);

    connect(m_client,&latexClient::updateDoc,
            this,&LatexHandler::updateDoc
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
    connect(m_http, &httpRequest::displaySvg,
            this, qOverload<QByteArray>(&LatexHandler::displaySvg)
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
}

//TODO remove svg tag
QString LatexHandleroutput()
{
    return nullptr;
}

void LatexHandler::bracket(QChar c)
{
    QTextCursor cursor = textCursor();
    qDebug() << c;
    switch(c.unicode()){
        case '{':
        {
            cursor.insertText("{}");
            setCursorPosition(cursorPosition()-1);
//            cursor.movePosition(QTextCursor::PreviousCharacter);
//            textDocument()->cursorPositionChanged(cursor);
            break;
        }
        case '[':
        {
            cursor.insertText("[]");
            setCursorPosition(cursorPosition()-1);
//            cursor.movePosition(QTextCursor::PreviousCharacter);
//            textDocument()->cursorPositionChanged(cursor);
            break;
        }
        case '(':
        {
            cursor.insertText("()");
            setCursorPosition(cursorPosition()-1);
//            cursor.movePosition(QTextCursor::PreviousCharacter);
//            textDocument()->cursorPositionChanged(cursor);
            break;
        }
    }
    return;
}

QQuickTextDocument *LatexHandler::document() const
{
    return m_latex;
}

void LatexHandler::setDocument(QQuickTextDocument *document)
{
    if (document == m_latex)
        return;

    m_latex = document;

    emit documentChanged();
}

void LatexHandler::textChanged()
{
    if(m_client
            && m_client->isValid()
            && m_client->isLogin()
            && m_client->isConnected())
    {
        m_client->sendDoc(textDocument()->toPlainText());
    }

}

//void LatexHandler::jump()
//{
//    QTextCursor cursor = textCursor();
//    if (cursor.isNull())
//        return;
//    QTextCharFormat format = cursor.charFormat();
//    qDebug()<<format.fontPointSize();
//    while(0<format.fontPointSize()&&format.fontPointSize()<1)
//    {
//        setCursorPosition(cursorPosition()+1);
//    }
//}

int LatexHandler::cursorPosition() const
{
    return m_cursorPosition;
}

void LatexHandler::setCursorPosition(int position)
{
    if (position == m_cursorPosition)
        return;

    m_cursorPosition = position;
    reset();
    emit cursorPositionChanged();
}

int LatexHandler::selectionStart() const
{
    return m_selectionStart;
}

void LatexHandler::setSelectionStart(int position)
{
    if (position == m_selectionStart)
        return;

    m_selectionStart = position;
    emit selectionStartChanged();
}

int LatexHandler::selectionEnd() const
{
    return m_selectionEnd;
}

void LatexHandler::setSelectionEnd(int position)
{
    if (position == m_selectionEnd)
        return;

    m_selectionEnd = position;
    emit selectionEndChanged();
}

QString LatexHandler::fontFamily() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QString();
    QTextCharFormat format = cursor.charFormat();
    return format.font().family();
}

//TODO not works well
void LatexHandler::setFontFamily(const QString &family)
{
    QTextCharFormat format;
    format.setFontFamily(family);
    mergeFormatOnWordOrSelection(format);
	QTextCursor cursor = textCursor();
	cursor.insertText("\\fontfamily{" + family + "}\n");
	//setCursorPosition(cursorPosition() - 1);
    emit fontFamilyChanged();
}

QColor LatexHandler::textColor() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QColor(Qt::black);
    QTextCharFormat format = cursor.charFormat();
    return format.foreground().color();
}

//there are chances that color not compatible
void LatexHandler::setTextColor(const QColor &color)
{
	QTextCursor cursor = textCursor();
    if (cursor.isNull()||cursor.position()<=0)
		return;

	if (!cursor.hasSelection())
	{
		cursor.insertText("\\textcolor{" + color.name() + "}{}");
		setCursorPosition(cursorPosition() - 1);
	}
	else {
		cursor.setPosition(selectionStart());
		cursor.insertText("\\textcolor{" + color.name() + "}{");
		cursor.setPosition(selectionEnd());
		cursor.insertText("}");
		//setCursorPosition(cursorPosition() - 1);
	}

	//TODO delete it
    //QTextCharFormat format;
    //format.setForeground(QBrush(color));
    //mergeFormatOnWordOrSelection(format);
    emit textColorChanged();
}

Qt::Alignment LatexHandler::alignment() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return Qt::AlignLeft;
    return textCursor().blockFormat().alignment();
}

/*
 *AlignLeft = 0x0001,
        AlignLeading = AlignLeft,
        AlignRight = 0x0002,
        AlignTrailing = AlignRight,
        AlignHCenter = 0x0004,
        AlignJustify = 0x0008,
        AlignAbsolute = 0x0010,
        AlignHorizontal_Mask = AlignLeft | AlignRight | AlignHCenter | AlignJustify | AlignAbsolute,

        AlignTop = 0x0020,
        AlignBottom = 0x0040,
        AlignVCenter = 0x0080,
        AlignBaseline = 0x0100,
        // Note that 0x100 will clash with Qt::TextSingleLine = 0x100 due to what the comment above
        // this enum declaration states. However, since Qt::AlignBaseline is only used by layouts,
        // it doesn't make sense to pass Qt::AlignBaseline to QPainter::drawText(), so there
        // shouldn't really be any ambiguity between the two overlapping enum values.
        AlignVertical_Mask = AlignTop | AlignBottom | AlignVCenter | AlignBaseline,

        AlignCenter = AlignVCenter | AlignHCenter
 */
void LatexHandler::setAlignment(Qt::Alignment alignment)
{
    QString align;
    QTextCursor cursor = textCursor();
    if(alignment==Qt::AlignJustify){
        align="justify";
        if (!cursor.hasSelection())
        {
            cursor.insertText("\\"+align+"\n");
        }
        else {
            cursor.setPosition(selectionStart());
            cursor.insertText("\\"+align+"\n");
        }
    }
    else{
        if(alignment==Qt::AlignLeft)
            align="raggedleft";
        else if(alignment==Qt::AlignHCenter)
            align="centering";
        else if(alignment==Qt::AlignRight)
            align="raggedright";
        else return;

        if (!cursor.hasSelection())
        {
            cursor.insertText("\\"+align+"{}");
            setCursorPosition(cursorPosition() - 1);
        }
        else {
            cursor.setPosition(selectionStart());
            cursor.insertText("\\"+align+"{");
            cursor.setPosition(selectionEnd());
            cursor.insertText("}");
            //setCursorPosition(cursorPosition() - 1);
        }
    }

//    QTextBlockFormat format;
//    format.setAlignment(alignment);
//    QTextCursor cursor = textCursor();
//    cursor.mergeBlockFormat(format);
    emit alignmentChanged();
}

bool LatexHandler::bold() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontWeight() == QFont::Bold;
}

void LatexHandler::setBold(bool bold)
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return;

    if (!cursor.hasSelection())
    {
        cursor.insertText("\\textbf{}");
        setCursorPosition(cursorPosition() - 1);
    }
    else {
        cursor.setPosition(selectionStart());
        cursor.insertText("\\textbf{");
        cursor.setPosition(selectionEnd());
        cursor.insertText("}");
        //setCursorPosition(cursorPosition() - 1);
    }
//    QTextCharFormat format;
//    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
//    mergeFormatOnWordOrSelection(format);
    emit boldChanged();
}

bool LatexHandler::italic() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontItalic();
}

void LatexHandler::setItalic(bool italic)
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return;

    if (!cursor.hasSelection())
    {
        cursor.insertText("\\textit{}");
        setCursorPosition(cursorPosition() - 1);
    }
    else {
        cursor.setPosition(selectionStart());
        cursor.insertText("\\textit{");
        cursor.setPosition(selectionEnd());
        cursor.insertText("}");
        //setCursorPosition(cursorPosition() - 1);
    }
//    QTextCharFormat format;
//    format.setFontItalic(italic);
//    mergeFormatOnWordOrSelection(format);
    emit italicChanged();
}

bool LatexHandler::underline() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontUnderline();
}

void LatexHandler::setUnderline(bool underline)
{
//    QTextCharFormat format;
//    format.setFontUnderline(underline);
//    mergeFormatOnWordOrSelection(format);
    QTextCursor cursor = textCursor();
       if (cursor.isNull())
           return;

       if (!cursor.hasSelection())
       {
           cursor.insertText("\\underline{}");
           setCursorPosition(cursorPosition() - 1);
       }
       else {
           cursor.setPosition(selectionStart());
           cursor.insertText("\\underline{");
           cursor.setPosition(selectionEnd());
           cursor.insertText("}");
           //setCursorPosition(cursorPosition() - 1);
       }
    emit underlineChanged();
}

int LatexHandler::fontSize() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return 0;
    QTextCharFormat format = cursor.charFormat();
    return format.font().pointSize();
}

void LatexHandler::setFontSize(int size)
{
    if (size <= 0)
        return;

    QTextCursor cursor = textCursor();
    QString name=m_size.getname(size);
    if (cursor.isNull())
        return;
    if (!cursor.hasSelection())
    {
        cursor.insertText("\\"+name+"{}");
        setCursorPosition(cursorPosition() - 1);
    }
    else {
        cursor.setPosition(selectionStart());
        cursor.insertText("\\"+name+"{");
        cursor.setPosition(selectionEnd());
        cursor.insertText("}");
        //setCursorPosition(cursorPosition() - 1);
    }
//    if (!cursor.hasSelection())
//        cursor.select(QTextCursor::WordUnderCursor);

//    if (cursor.charFormat().property(QTextFormat::FontPointSize).toInt() == size)
//        return;

//    QTextCharFormat format;
//    format.setFontPointSize(size);
//    mergeFormatOnWordOrSelection(format);
    emit fontSizeChanged();
}

QString LatexHandler::fileName() const
{
    const QString filePath = QQmlFile::urlToLocalFileOrQrc(m_fileUrl);
    const QString fileName = QFileInfo(filePath).fileName();
    if (fileName.isEmpty())
        return QStringLiteral("untitled.tex");
    return fileName;
}

QString LatexHandler::fileType() const
{
    return QFileInfo(fileName()).suffix();
}

QUrl LatexHandler::fileUrl() const
{
    return m_fileUrl;
}

int LatexHandler::displaymode() const
{   //qDebug()<< QString::number(1&(int)m_mode);
    return (int)m_mode;
}

void LatexHandler::setDisplaymode(int mode)
{
    //qDebug()<<mode;
    m_mode=(displayMode)mode;

    //resetDoc();
    emit onDisplaymodeChanged(m_mode);
    emit displaymodeChanged();
}
//void LatexHandler::setClient(latexClient *client)
//{
//    m_client = client;
//}

//latexClient* LatexHandler::client()
//{
//    return m_client;
//}

//void LatexHandler::setSetting(QSettings *setting)
//{
//    m_setting = setting;
//}

//QSettings* LatexHandler::setting()
//{
//    return m_setting;
//}

void LatexHandler::load(const QUrl &fileUrl)
{
    if (fileUrl == m_fileUrl)
        return;

    QQmlEngine *engine = qmlEngine(this);
    if (!engine) {
        qWarning() << "load() called before LatexHandler has QQmlEngine";
        return;
    }

    const QUrl path = QQmlFileSelector::get(engine)->selector()->select(fileUrl);
    const QString fileName = QQmlFile::urlToLocalFileOrQrc(path);
    if (QFile::exists(fileName)) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            //QTextCodec *codec = QTextCodec::codecForHtml(data);
            QTextCodec *codec = QTextCodec::codecForUtfText(data);
            if (QTextDocument *doc = textDocument())
                doc->setModified(false);

            emit loaded(codec->toUnicode(data));
            reset();
        }
    }

    m_fileUrl = fileUrl;
    emit fileUrlChanged();
}

void LatexHandler::saveAs(const QUrl &fileUrl)
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return;

    const QString filePath = fileUrl.toLocalFile();
    const bool isHtml = QFileInfo(filePath).suffix().contains(QLatin1String("htm"));
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Truncate | (isHtml ? QFile::NotOpen : QFile::Text))) {
        emit error(tr("Cannot save: ") + file.errorString());
        return;
    }
    file.write((isHtml ? doc->toHtml() : doc->toPlainText()).toUtf8());
    file.close();

    if (fileUrl == m_fileUrl)
        return;

    m_fileUrl = fileUrl;
    emit fileUrlChanged();
}

void LatexHandler::updateDoc(QString doc)
{

    QString res=patchDoc(doc);
    if(m_latex->textDocument()->toPlainText()==res)
        return;

    resetDoc(res);

}

void LatexHandler::resetDoc(QString doc)
{
    bool clearcache=false;//todo

    if(doc==nullptr)
        doc=m_latex->textDocument()->toPlainText();

    m_latex->textDocument()->clear();
    disconnect(m_highlighter, &latexBasicHighlighter::newmath,
            this, &LatexHandler::mathDisplay);
    m_latex->textDocument()->setPlainText(doc);

    for (QHash<QString,bool>::iterator i=indoc.begin();i!=indoc.end();++i) {
        i.value()=false;
    }

    if(clearcache)
        maths.clear();

    connect(m_highlighter, &latexBasicHighlighter::newmath,
            this, &LatexHandler::mathDisplay
            , (Qt::ConnectionType)(Qt::AutoConnection | Qt::UniqueConnection));
    reset();
}

void LatexHandler::reset()
{
    emit fontFamilyChanged();
    emit alignmentChanged();
    emit boldChanged();
    emit italicChanged();
    emit underlineChanged();
    emit fontSizeChanged();
    emit textColorChanged();
}

QTextCursor LatexHandler::textCursor() const
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return QTextCursor();

    QTextCursor cursor = QTextCursor(doc);
    if (m_selectionStart != m_selectionEnd) {
        cursor.setPosition(m_selectionStart);
        cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(m_cursorPosition);
    }
    return cursor;
}

QTextDocument *LatexHandler::textDocument() const
{
    if (!m_latex)
        return nullptr;

    return m_latex->textDocument();
}

void LatexHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
}

//TODO multithreading
void LatexHandler::displaySvg(int poz,QByteArray svg,QTemporaryFile* file, bool isinline)
{
    m_svgCount.append(poz);
    std::sort(m_svgCount.begin(),m_svgCount.end());
    int z=m_svgCount.indexOf(poz);

    QString name;
    if (file->open()) {
        name=file->fileName(); //returns the unique file name
        if(!svg.isEmpty())
            file->write(svg);
        file->close();
    }
    else{
        qDebug() << "unable to open";
        return;
    }
    QString t;
    QTextCursor cur= textCursor();
    //TODO better position?
    cur.setPosition(poz+z);

    if(!isinline)
    {
        QTextBlockFormat format;
        format.setAlignment(Qt::AlignCenter);
        cur.mergeBlockFormat(format);
    }
    if(isinline)
        t="<img src=\"file:///"+name+"\" height=\""+
            QString::number(m_fontSize*1.8)+"\"/>";
    else
        t="<p><img src=\"file:///"+name+"\" height=\""+
            QString::number(m_fontSize*3)+"\"/></p>";

    cur.insertHtml(t);
    if(m_svgData.isEmpty())
        m_svgCount.clear();
}

void LatexHandler::displaySvg(QByteArray svg)
{
    if(m_svgData.isEmpty()){
        m_svgCount.clear();
        return;
    }

    int poz=m_svgData.begin()->poz;
    QTemporaryFile* file=m_svgData.begin()->file;
    bool isinline=m_svgData.begin()->isinline;

    m_svgCount<<poz;
    std::sort(m_svgCount.begin(),m_svgCount.end());
    int z=m_svgCount.indexOf(poz);

    QString name;
    if (file->open()) {
        name=file->fileName(); //returns the unique file name
        if(!svg.isEmpty())
            file->write(svg);
        file->close();
    }
    else{
        qDebug() << "unable to open";
        return;
    }
    QString t;
    QTextCursor cur= textCursor();
    //TODO better position?
    cur.setPosition(poz+z);

    //qDebug() << cur.WordLeft;

    if(isinline)
        t="<img src=\"file:///"+name+"\" height=\""+
            QString::number(m_fontSize*1.8)+"\"/>";
    else{
        cur.insertBlock();
        QTextBlockFormat format;
        format.setAlignment(Qt::AlignCenter);
        cur.mergeBlockFormat(format);
        t="<img src=\"file:///"+name+"\" height=\""+
            QString::number(m_fontSize*3)+"\"/>";
    }
    cur.insertHtml(t);

    m_svgData.pop_front();
    //m_svgCount++;
    if(m_svgData.isEmpty())
        m_svgCount.clear();
}

//TODO handle dulpilcate, modify
//TODO multithreading is required here!!!!
void LatexHandler::mathDisplay(){
    //m_svgData=QList<svgDatum>();

    bool deletecache=false;
    //qDebug() << "math";
    QString text = textDocument()->toPlainText();

    QHash<QString, QTemporaryFile*> nmaths;
    QHash<QString, bool> nindoc=indoc;
    for (QHash<QString,bool>::iterator i=nindoc.begin();i!=nindoc.end();++i) {
        i.value()=false;
    }

    QRegularExpression regexlist,mathnotinline;
//    regexlist=QRegularExpression("\\\\\\[\\s*((?:[^\\]]|[^\\\\]\\])*?)\\s*\\\\\\]|\\$\\$\\s*((?:[^$]|[\\\\]\\$)*?)\\s*\\$\\$|\\\\begin{displaymath}\\s*([\\s\\S]*?)\\s*\\\\end{displaymath}|\\\\begin{equation}\\s*([\\s\\S]*?)\\s*\\\\end{equation}|\\\\\\(\\s*((?:[^\\)]|[^\\\\]\\))*?)\\s*\\\\\\)|\\$\\s*((?:[^$]|[\\\\]\\$)*?)\\s*\\$|\\\\begin{math}\\s*([\\s\\S]*?)\\s*\\\\end{math}"
//                                   ,QRegularExpression::DotMatchesEverythingOption|QRegularExpression::MultilineOption);
    //TODO \\\\b
    regexlist=QRegularExpression("(?:\\\\\\[\\s*((?:[^\\]]|[^\\\\]\\])*?)\\s*\\\\\\])|(\\$\\$\\s*((?:[^$]|[\\\\]\\$)*?)\\s*\\$\\$)|(?:\\\\begin{displaymath}\\s*([\\s\\S]*?)\\s*\\\\end{displaymath})|(\\\\begin{equation}\\s*([\\s\\S]*?)\\s*\\\\end{equation})|(?:\\\\\\(\\s*((?:[^\\)]|[^\\\\]\\))*?)\\s*\\\\\\))|(?:\\$\\s*((?:[^$]|[\\\\]\\$)*?)\\s*\\$)|(?:\\\\begin{math}\\s*([\\s\\S]*?)\\s*\\\\end{math})"
                                   ,QRegularExpression::DotMatchesEverythingOption|QRegularExpression::MultilineOption);


    mathnotinline=QRegularExpression("\\\\\\[\\s*((?:[^\\]]|[^\\\\]\\])*?)\\s*\\\\\\]|\\$\\$\\s*((?:[^$]|[\\\\]\\$)*?)\\s*\\$\\$|\\\\begin{displaymath}\\s*([\\s\\S]*?)\\s*\\\\end{displaymath}|\\\\begin{equation}\\s*([\\s\\S]*?)\\s*\\\\end{equation}"
                                    ,QRegularExpression::DotMatchesEverythingOption|QRegularExpression::MultilineOption);

    int i=0;
    QRegularExpression pattern=regexlist;
    //qDebug() << pattern.isValid();
    QRegularExpressionMatchIterator iterator = pattern.globalMatch(text);

    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString math=match.captured(match.lastCapturedIndex()),
                all=match.captured(0);
        qDebug()<<match;

        if(mathnotinline.globalMatch(all).hasNext())
            i=1;
        else i=0;

        QTemporaryFile* file;
        int poz = match.capturedStart(0);

        if(maths.contains(math))
        {
            file=nmaths[math]=maths[math];
            if(indoc.contains(math)&&indoc[math]){
                nindoc[math]=true;
                continue;
            }
            displaySvg(poz,QByteArray(),file,i==0?true:false);
            continue;
        }
        else{
            file = new QTemporaryFile();
            if(!QDir(QDir::tempPath()+"/latexQT/svg").exists())
            {
                QDir(QDir::tempPath()).mkdir("latexQT");
                QDir(QDir::tempPath()+"/latexQT").mkdir("svg");
            }
            file->setFileTemplate(QDir::tempPath()+"/latexQT/svg/mathDisplayXXXXXX.svg");
        }

        if (file->open()) {
            file->fileName(); //returns the unique file name
            file->setAutoRemove(false);
            file->close();
            file->setAutoRemove(false);
        }
        else{
            qDebug() << "unable to open";
            return;
        }
        nmaths[math] = maths[math] = file;
        nindoc[math]=true;
        m_svgData.push_back(svgDatum{poz,file,i==0?true:false});
        m_http->math2svg(math);

//        auto begin = std::chrono::system_clock::now();
//        while (std::chrono::system_clock::now() - begin > std::chrono::duration<double>(0.5))
//        {
//            QCoreApplication::processEvents();
//        }
//        QImage img = QImage();
//        img.fromData(svg,"svg");

//        //TODO override?
//        QUrl Uri ( QString ( "svginline://%1" ).arg ( mathi[math] ) );
//        textDocument()->addResource( QTextDocument::ImageResource, Uri, QVariant ( img ) );

//        QTextImageFormat imageFormat;
//        imageFormat.setWidth( img.width() );
//        imageFormat.setHeight( img.height() );
//        imageFormat.setName( Uri.toString() );
//        cur.insertImage(imageFormat);

       //qDebug() <<textDocument()->toPlainText()<<"\n"<<t;
    }
    if(deletecache){
    //delete temp
        foreach (QString mathin, maths.keys())
        {
            if(!nmaths.contains(mathin))
            {
                QTemporaryFile* f=maths.value(mathin);
                maths.remove(mathin);
                f->remove();
            }
        }
    }
        //mathi=nmathi;
    //}
    indoc=nindoc;

//    pattern =
//    //qDebug() << pattern.isValid();
////    pattern = QRegularExpression("\n+|\\\\\\[.+?\\\\\\]",
////            QRegularExpression::DotMatchesEverythingOption|QRegularExpression::MultilineOption);
//    iterator = pattern.globalMatch(text);

//    while (iterator.hasNext()) {
//        QRegularExpressionMatch match = iterator.next();
//        QString math = match.captured(1);

//        if(maths.contains(math))
//        {
//            nmaths[math]=maths[math];
//            continue;
//        }

//        int poz = match.capturedStart(0);

//        QTemporaryFile* file=new QTemporaryFile();
//        QString name;

//        if(!QDir(QDir::tempPath()+"/latexQT/svg").exists())
//        {
//            QDir(QDir::tempPath()).mkdir("latexQT");
//            QDir(QDir::tempPath()+"/latexQT").mkdir("svg");
//        }
//        file->setFileTemplate(QDir::tempPath()+"/latexQT/svg/mathDisplayXXXXXX.svg");

//        if (file->open()) {
//            name=file->fileName(); //returns the unique file name
//            file->setAutoRemove(false);
//            file->close();
//            file->setAutoRemove(false);
//        }
//        else{
//            qDebug() << "unable to open";
//            return;
//        }
////        QTemporaryFile* t=&file;
////        qDebug()<<t;
////        t->open();
//        maths[math]= file;
//        nmaths[math]= file;
//        m_http->math2svg(math,poz,file,false);

//        //qDebug() << "returned :" <<svg;
//        //QImage img = QImage();
//        //img.fromData(svg,"svg");
//        //qDebug() << img.isNull();

//        //TODO override?
////        QUrl Uri ( QString ( "file://%1" ).arg ( name.right(9) ) );
////        QImage image = QIcon(name).pixmap(QSize(18,30)).toImage();
////        qDebug() <<image.bits();
////        if(image.isNull()){
////            qDebug() <<"read error";
////            return;
////        }
////        textDocument()->addResource( QTextDocument::ImageResource,
////                                     Uri, QVariant ( image ) );

////        QTextImageFormat imageFormat;
////        imageFormat.setWidth( image.width()/image.height()*18 );
////        //imageFormat.setHeight( image.height() );
////        imageFormat.setHeight( 18 );
////        //imageFormat.setHeight( m_fontSize );
////        imageFormat.setName( Uri.toString() );
////        qDebug() <<imageFormat.isValid();
////        cur.insertImage(imageFormat);

//    }
//    //dalete temp
//    foreach (QString mathin, maths.keys())
//    {
//        if(!nmaths.contains(mathin))
//        {
//            QTemporaryFile* f=maths.value(mathin);
//            maths.remove(mathin);
//            f->remove();
//        }
//    }
//    //maths=nmaths;
}

QString LatexHandler::patchDoc(QString doc)
{
    //QList<Patch> patch1=m_dmp.patch_make(m_commit,doc);
    //QList<Patch> patch2=m_dmp.patch_make(m_commit,m_latex->textDocument()->toPlainText());
	QList<Patch> patch1 = m_dmp.patch_fromText(doc);
    QPair<QString, QVector<bool> > out
        = m_dmp.patch_apply(patch1, m_latex->textDocument()->toPlainText());
    QString strResult = out.first;
    //error handling
    return strResult;
    //m_latex->textDocument()->setPlainText(strResult);
}
