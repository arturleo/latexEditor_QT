#ifndef TEXFONTSIZE_H
#define TEXFONTSIZE_H

#include <QObject>
/*
 *  format.setFontPointSize(defaultFontSize * 0.5);0
    _formats[HighlighterState::tiny] = format;
    format.setFontPointSize(defaultFontSize * 0.7);1
    _formats[HighlighterState::scriptsize] = format;
    format.setFontPointSize(defaultFontSize * 0.8);2
    _formats[HighlighterState::footnotesize] = format;
    format.setFontPointSize(defaultFontSize * 0.9);3
    _formats[HighlighterState::small] = format;
    format.setFontPointSize(defaultFontSize);4
    _formats[HighlighterState::normalsize] = format;
    format.setFontPointSize(defaultFontSize * 1.2);5
    _formats[HighlighterState::large] = format;
    format.setFontPointSize(defaultFontSize * 1.44);6
    _formats[HighlighterState::Large] = format;
    format.setFontPointSize(defaultFontSize * 1.728);7
    _formats[HighlighterState::LARGE] = format;
    format.setFontPointSize(defaultFontSize * 2.074);8
    _formats[HighlighterState::huge] = format;
    format.setFontPointSize(defaultFontSize * 2.488);9
    _formats[HighlighterState::HUGE] = format;
 */
class texFontSize : public QObject
{
    Q_OBJECT
public:
    const QList<double> rule={
        0.5,0.7,0.8,0.9,1,1.2
        ,1.44,1.728,2.074,2.488};
    const QList<QString> rules={
        "tiny","scriptsize","footnotesize","small",
        "normalsize","large","Large","LARGE","huge"
        ,"HUGE"
    };
    explicit texFontSize(double defaultsize=12);
    QString getname(double size);
    double bigger(double size);
    double smaller(double size);

signals:

public slots:

private:
    int getIndex(double size);
    double m_size;
};

#endif // TEXFONTSIZE_H

