/* this class is based on QT
 * examples
 */
#ifndef LATEXHANDLER_H
#define LATEXHANDLER_H

#include <QFont>
#include <QObject>
#include <QTextCursor>
#include <QUrl>
#include <QSettings>
#include <QTemporaryFile>
#include <QDir>
#include <QImageReader>
#include <QFile>
#include <QFileInfo>
#include <QFileSelector>
#include <QQmlFile>
#include <QQmlFileSelector>
#include <QQuickTextDocument>
#include <QTextCharFormat>
#include <QTextCodec>
#include <QTextDocument>
#include <QIcon>

#include <diff_match_patch.h>

#include "texfontsize.h"
#include "datatypes.h"
#include "httprequest.h"
#include "latexbasichighlighter.h"
#include "latexclient.h"

QT_BEGIN_NAMESPACE
class QTextDocument;
class QQuickTextDocument;
QT_END_NAMESPACE

class LatexHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged)
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY italicChanged)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline NOTIFY underlineChanged)

    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileUrlChanged)
    Q_PROPERTY(QString fileType READ fileType NOTIFY fileUrlChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY fileUrlChanged)

    Q_PROPERTY(int displaymode READ displaymode WRITE setDisplaymode NOTIFY displaymodeChanged)
    //Q_PROPERTY(latexClient *client READ client WRITE setClient NOTIFY clientChanged)
    //Q_PROPERTY(QSettings *setting READ setting WRITE setSetting NOTIFY settingChanged)

public:
    explicit LatexHandler(displayMode mode= DISPLAY_VIEW,
                          int fontsize=12,
                          QObject *parent= nullptr);
    explicit LatexHandler(latexClient *client,
                          QSettings *setting,
                          displayMode mode = DISPLAY_VIEW,
                          int fontsize=12,
                          QObject *parent = nullptr);
	virtual ~LatexHandler();

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    Q_INVOKABLE void initHighlighter();
    Q_INVOKABLE void init(QSettings *setting,latexClient* client);
    Q_INVOKABLE void bracket(QChar c);//TODO
    Q_INVOKABLE void textChanged();
    //Q_INVOKABLE void jump();

    QString output();

    int cursorPosition() const;
    void setCursorPosition(int position);

    int selectionStart() const;
    void setSelectionStart(int position);

    int selectionEnd() const;
    void setSelectionEnd(int position);

    QString fontFamily() const;
    void setFontFamily(const QString &family);

    QColor textColor() const;
    void setTextColor(const QColor &color);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    bool bold() const;
    void setBold(bool bold);

    bool italic() const;
    void setItalic(bool italic);

    bool underline() const;
    void setUnderline(bool underline);

    int fontSize() const;
    void setFontSize(int size);

    QString fileName() const;
    QString fileType() const;
    QUrl fileUrl() const;

    int displaymode() const;
    void setDisplaymode(int mode);


//    latexClient* client();
//    void setClient(latexClient *client);

//    QSettings* setting();
//    void setSetting(QSettings *setting);
signals:
    void documentChanged();
    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();

    void fontFamilyChanged();
    void textColorChanged();
    void alignmentChanged();

    void boldChanged();
    void italicChanged();
    void underlineChanged();

    void fontSizeChanged();

    void fileUrlChanged();

    void displaymodeChanged();
    void onDisplaymodeChanged(displayMode mode);

    void loaded(const QString &text);
    void error(const QString &message);

//    void clientChanged();
//    void settingChanged();

public slots:
    void load(const QUrl &fileUrl);
    void saveAs(const QUrl &fileUrl);
    void updateDoc(const QString doc);
    void resetDoc(const QString doc=nullptr);
private:
    void displaySvg(QByteArray svg);
    void displaySvg(int poz, QByteArray svg, QTemporaryFile* file, bool isinline);

    void mathDisplay();

    QString patchDoc(QString doc);

    void reset();
    QTextCursor textCursor() const;
    QTextDocument *textDocument() const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    QQuickTextDocument *m_latex;
    QString m_commit;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    displayMode m_mode;
    QFont m_font;
    int m_fontSize;
    QUrl m_fileUrl;

    QHash<QString, QTemporaryFile*> maths;
    QHash<QString, bool> indoc;

    //TODO when quit, clear cache
    diff_match_patch m_dmp;
    QSettings *m_setting=nullptr;//TODO handle setting change
    httpRequest* m_http=nullptr;
    latexBasicHighlighter* m_highlighter;
    latexClient *m_client=nullptr;
    texFontSize m_size;

    class svgDatum{
        public:
            int poz;
            QTemporaryFile* file;
            bool isinline;
    };
    QList<svgDatum> m_svgData;
    QList<int> m_svgCount;
};

#endif // LATEXHANDLER_H
