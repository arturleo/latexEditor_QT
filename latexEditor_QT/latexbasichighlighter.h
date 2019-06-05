/*
 * originate from markdown highter
 */
#ifndef LATEXBASICHIGHLIGHTER_H
#define LATEXBASICHIGHLIGHTER_H

#include <QObject>
#include <QTextCharFormat>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTimer>
#include <QTextDocument>

#include "datatypes.h"

//TODO do twice to get full highlight
class latexBasicHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    // we use some predefined numbers here to be compatible with
    // the peg-markdown parser
    enum HighlighterState {//pre: math, code, img//tab process --=2=-- ==-2-==
        NoState =-1,
        Default = 0,
        //special
        MaskedSyntax,
        CurrentLineBackgroundColor,
        BrokenLink,
        //none
        NonePersis,
        None,

        //title
        Title,
        Subtitle,
        Author,
        //section
        Part,
        Chapter,
        Section,
        Subsection,
        Subsubsection,
        Paragraph,
        Subparagraph,
        Abstract,
        AbstractBegin,
        //Align
        AlignCenter,
        AlignLeft,
        AlignRight,
        //font size
        tiny,
        scriptsize,
        footnotesize,
        small,
        normalsize,
        large,
        Large,
        LARGE,
        huge,
        HUGE,
        //font family
        Italic,
        Emph,
        Bold,
        Underline,
        Color,
        //special
        Link,
        Image,
        Comment,
        //Math
        MathInline,
        MathEq,
    };

    struct HighlightingRule
    {
        QRegularExpression pattern;
        HighlighterState state;
        HighlighterState maskStyle;
        QHash<QString, qint8> attrs;
        QList<int> capturingGroup;
        QList<int> maskedGroup;
        bool disableIfCurrentStateIsSet;
        bool blockStyle;
    };

    latexBasicHighlighter(QTextDocument *parent = nullptr
            ,int fontsize=10
            , displayMode mode = DISPLAY_VIEW
            );

    void setTextFormats(QHash<HighlighterState, QTextCharFormat> formats);
    void setTextFormat(HighlighterState state, QTextCharFormat format);
    void setDisplayMode(displayMode mode);

	void setFontsize(double size);
	double getFontsize();

    displayMode Mode();

signals:
    void highlightingFinished();
    void newmath();
    void resetDoc(QString doc=nullptr);
protected:
    QList<QString> keyword={
        "emph",
        "textit",
        "textbf",
        "tiny",
        "scriptsize",
        "footnotesize",
        "small",
        "normalsize",
        "large",
        "Large",
        "LARGE",
        "huge",
        "Huge",
        "textcolor",
        "begin{document}",
        "end{document}",

    };

    void initHighlightingRules();
    void highlightBlock(const QString &text) override;
    void initTextFormats(double defaultFontSize = 12);
    void highlightLatex(QString text);
    void setdefault(QString text);
    void highlightAdditionalRules(QVector<HighlightingRule> &rules,
                                  QString text);

    QVector<HighlightingRule> _highlightingRulesPre;
    QVector<HighlightingRule> _highlightingRulesAfter;

    QHash<HighlighterState, QTextCharFormat> _formats;
    QHash<HighlighterState, QTextCharFormat> _formatsLine;
    QHash<HighlighterState, QTextBlockFormat> _formatsBlock;

    bool _highlightingFinished;

    QTextDocument* doc;
    double m_fontsize;
    displayMode currentMode;

};
#endif // LATEXBASICHIGHLIGHTER_H
