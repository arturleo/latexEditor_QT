#include "latexbasichighlighter.h"
#include <QDebug>
//TODO math highlight in split mode and edit mode
/**
 * Latex syntax highlighting
 *
 * @param parent
 * @return
 */
latexBasicHighlighter::latexBasicHighlighter(
        QTextDocument *parent
        , int fontsize
        ,displayMode mode
        )
: QSyntaxHighlighter(parent),
  m_fontsize(fontsize),
  currentMode(mode),
  doc(parent)
{
    // initialize the highlighting rules
    initHighlightingRules();

    // initialize the text formats
    initTextFormats(m_fontsize);
}

void latexBasicHighlighter::setFontsize(double size)
{
	m_fontsize = size;
}

double latexBasicHighlighter::getFontsize()
{
	return m_fontsize;
}

//TODO resolve space
/**
 * Initializes the highlighting rules
 *
 */
void latexBasicHighlighter::initHighlightingRules() {
    HighlightingRule rule = HighlightingRule();

    QStringList keywordPatterns;
    keywordPatterns << "(\\\\\\w*?)(?:{\\w*?})" <<"(?<!\\\\)([{}])";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.state = HighlighterState::MaskedSyntax;//TODO
        rule.capturingGroup.append(1);
        _highlightingRulesPre.append(rule);
    };

    // [math]
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\\\((?:[^\\)]|[^\\\\]\\))*?\\\\\\)|\\$(?:[^$]|[\\\\]\\$)*?\\$|\\\\begin{math}[\\s\\S]*?\\\\end{math}");
    //qDebug()<<rule.pattern.isValid();
    rule.state = HighlighterState::MathInline;
    _highlightingRulesAfter.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\\\[(?:[^\\]]|[^\\\\]\\])*?\\\\\\]|\\$\\$(?:[^$]|[\\\\]\\$)*?\\$\\$|\\\\begin{displaymath}[\\s\\S]*?\\\\end{displaymath}|\\\\begin{equation}[\\s\\S]*?\\\\end{equation}");
    //rule.pattern = QRegularExpression("[\\S\\s]\n[\\s\\S]");
    //qDebug()<<rule.pattern.isValid();
    rule.state = HighlighterState::MathEq;
    _highlightingRulesAfter.append(rule);

    // [spaces]
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("[^\\s\\b](\r\n|\r|\n){2}[^\\s\\b]");
    rule.state = HighlighterState::NonePersis;
    rule.capturingGroup.append(1);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(?:\r\n|\r|\n){2}(\r\n|\r|\n)+");
    rule.state = HighlighterState::NonePersis;
    rule.capturingGroup.append(1);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    //three{}
    rule.pattern = QRegularExpression("[ \t]([ \t]+)");
    rule.state = HighlighterState::NonePersis;
    _highlightingRulesPre.append(rule);

    // [spaces]



    // [font style]
    //TODO Equivalent switch: {\normalfont ...}
    //https://en.wikibooks.org/wiki/LaTeX/Fonts
    //emph
    rule = HighlightingRule();
    //three{}
    rule.state = HighlighterState::Emph;
    rule.maskStyle=HighlighterState::None;
    rule.pattern = QRegularExpression("(\\\\emph{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    //Using emph{} inside of italic text removes the italics on the emphasized text.
    _highlightingRulesPre.append(rule);


    // italic
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\textit{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Italic;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    // underline
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\underline{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Underline;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    // bold
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\textbf{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Bold;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);
    // [!font style]


    // [font size]
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\tiny{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::tiny;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\scriptsize{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::scriptsize;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\footnotesize{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::footnotesize;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\small{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::small;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\normalsize{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::normalsize;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\large{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::large;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\Large{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Large;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\LARGE{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::LARGE;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\huge{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::huge;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\Huge{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::HUGE;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);
    // [!font size]

    // [color]
    //TODO done use Qcolor ,isValid
    //https://en.wikibooks.org/wiki/LaTeX/Colors
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\textcolor{([\\s|\\S]+?)}\\s*{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Color;
    rule.maskStyle = HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(4);
	rule.maskedGroup.append(2);
    rule.capturingGroup.append(3);
    rule.attrs.insert("color",2);
    _highlightingRulesPre.append(rule);
    // [!color]

    // [begin document]
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\begin{document}");
    rule.state = HighlighterState::None;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\end{document}");
    rule.state = HighlighterState::None;
    _highlightingRulesPre.append(rule);
    // [!begin document]

    // [title]
    //https://en.wikibooks.org/wiki/LaTeX/Title_Creation
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\title{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Title;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\author{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Author;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
        rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\subtitle{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Subtitle;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);
    // [!Title]

    // [section]
    //https://en.wikibooks.org/wiki/LaTeX/Document_Structure
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\part{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Part;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\chapter{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Chapter;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\section{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Section;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\subsection{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Subsection;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\subsubsection{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Subsubsection;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\paragraph{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Paragraph;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\subparagraph{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Subparagraph;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);
    // [!section]


    // [abstract]
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\begin{abstract})([\\S\\s]*?)(\\\\end{abstract})");
    rule.state = HighlighterState::Abstract;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\begin{(abstract)}[\\S\\s]*?\\\\end{abstract}");
    rule.state = HighlighterState::AbstractBegin;
    rule.capturingGroup.append(1);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);
    // [!abstract]

    // [Alignment]
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\centering{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::AlignCenter;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\raggedleft{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::AlignLeft;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\raggedright{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::AlignRight;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    rule.blockStyle = true;
    _highlightingRulesPre.append(rule);
    // [!Alignment]

    // [comment]
    //https://en.wikibooks.org/wiki/LaTeX/Paragraph_Formatting
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\begin{comment})([\\s\\S]*?)(\\\\end{comment})");
    rule.state = HighlighterState::Comment;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(%)([\\s\\S]*?)([\n|\r|\r\n])");
    rule.state = HighlighterState::Comment;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);
    // [!comment]

    // [font]
    //todo can read font?
    //https://en.wikibooks.org/wiki/LaTeX/Fonts
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\setmainfont([\\s|\\S]+?)\\s*{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::None;
    rule.attrs.insert("options",2);
    rule.attrs.insert("mainFont",3);
    _highlightingRulesPre.append(rule);
    // [!font]

    // [img]
    //not done
    //https://en.wikibooks.org/wiki/LaTeX/Hyperlinks
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\includegraphics\\[([\\s|\\S]+?)\\]\\s*{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Image;
    rule.maskStyle = HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(4);
    rule.capturingGroup.append(3);
    rule.attrs.insert("imgsize",2);
    _highlightingRulesPre.append(rule);
    // [!img]

    // [hyperlink]
    //https://en.wikibooks.org/wiki/LaTeX/Hyperlinks
    //TODO display file
    //TODO handle run:
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\hyperlink{([\\s|\\S]+?)}\\s*{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Link;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(4);
    rule.capturingGroup.append(3);
    rule.attrs.insert("label",2);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(\\\\url{)((?:(?:\\[{}]|[^{}])*(?:{(?:(?:\\[{}]|[^{}])*(?:{[\\s\\S]*})*(?:\\[{}]|[^{}])*)})*(?:\\[{}]|[^{}])*))(})");
    rule.state = HighlighterState::Link;
    rule.maskStyle=HighlighterState::None;
    rule.maskedGroup.append(1);
    rule.maskedGroup.append(3);
    rule.capturingGroup.append(2);
    _highlightingRulesPre.append(rule);
    // [!hyperlink]

    // [line]
    //https://en.wikibooks.org/wiki/LaTeX/Paragraph_Formatting
    //TODO so far no way to actually create new line
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\\\\\[\\s|\t|\r|\n]+?");
    rule.state = HighlighterState::None;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\(\\d+)[\\s|\t|\r|\n]+?");
    rule.state = HighlighterState::None;
    rule.attrs.insert("space",1);
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\linebreak\\d+[\\s|\t|\r|\n]+?");
    rule.state = HighlighterState::None;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\break[\\s|\t|\r|\n]+?");
    rule.state = HighlighterState::None;
    _highlightingRulesPre.append(rule);

    rule = HighlightingRule();
    rule.pattern = QRegularExpression("\\\\par[\\s|\t|\r|\n]+?");
    rule.state = HighlighterState::None;
    _highlightingRulesPre.append(rule);
    // [!line]
}

/**
 * Initializes the text formats
 *
 * @param defaultFontSize
 */
void latexBasicHighlighter::initTextFormats(double defaultFontSize) {

    //sequence: title => section=> align =>
    //  font( size family weight color) =>
    //  comment, img, link
    QTextCharFormat format;
    QTextBlockFormat formatb;

    format = QTextCharFormat();
    format.setFontPointSize(0.00001);
    _formats[HighlighterState::NonePersis] = format;

    format = QTextCharFormat();
    format.setForeground(QBrush(Qt::darkCyan));
    format.setFontItalic(true);
    _formats[HighlighterState::MaskedSyntax] = format;

    format = QTextCharFormat();
    format.setFontPointSize( defaultFontSize );
    _formats[HighlighterState::Default]=format;

    if(currentMode == DISPLAY_VIEW){
        format.setFontPointSize(0.00001);
        _formats[HighlighterState::None] =
                _formats[HighlighterState::NonePersis];
    }
    else if(currentMode == DISPLAY_EDIT
          ||currentMode == DISPLAY_EDIT)
    {
        _formats[HighlighterState::None] =
                _formats[HighlighterState::MaskedSyntax];
    }

    // set character formats for title
    format = QTextCharFormat();
    format.setFontPointSize(defaultFontSize * 2.074);
    format.setFontWeight(QFont::Bold);
    if(currentMode == DISPLAY_VIEW){
        formatb = QTextBlockFormat();
        formatb.setAlignment(Qt::AlignHCenter);
        _formatsBlock.insert(HighlighterState::Title,formatb);
        _formatsBlock.insert(HighlighterState::Subtitle,formatb);
        _formatsBlock.insert(HighlighterState::Author,formatb);
    }
    _formats[HighlighterState::Title]=format;
    //Subtitle
    format.setFontPointSize(defaultFontSize * 1.44);
    _formats[HighlighterState::Subtitle] = format;
    //author
    format.setFontWeight(QFont::SmallCaps);
    _formats[HighlighterState::Author] = format;


    // set character formats for section
    format = QTextCharFormat();
    format.setFontPointSize(defaultFontSize * 2.488);
    format.setFontWeight(QFont::Bold);
    if(currentMode == DISPLAY_VIEW){
        formatb = QTextBlockFormat();
        formatb.setAlignment(Qt::AlignHCenter);
        _formatsBlock.insert(HighlighterState::AbstractBegin,formatb);
    }
    _formats[HighlighterState::Part]=format;
    //Chapter
    format.setFontPointSize(defaultFontSize * 2.074);
    _formats[HighlighterState::Chapter] = format;
    //Section
    format.setFontPointSize(defaultFontSize * 1.728);
    _formats[HighlighterState::Section] = format;
    //Subsection
    format.setFontPointSize(defaultFontSize * 1.44);
    _formats[HighlighterState::Subsection] = format;
    //Subsubsection
    format.setFontPointSize(defaultFontSize * 1.2);
    _formats[HighlighterState::Subsubsection] = format;
    //Paragraph
    format.setFontPointSize(defaultFontSize);
    _formats[HighlighterState::Paragraph] = format;
    //Subparagraph
    format.setFontPointSize(defaultFontSize * 0.9);
    _formats[HighlighterState::Subparagraph] = format;
    //AbstractBegin
    format.setFontPointSize(defaultFontSize);
    _formats[HighlighterState::AbstractBegin] = format;

    // set character formats for align
    format = QTextCharFormat();
    if(currentMode == DISPLAY_VIEW){
        formatb = QTextBlockFormat();
        formatb.setAlignment(Qt::AlignHCenter);
        _formatsBlock.insert(HighlighterState::AlignCenter,formatb);
        formatb.setAlignment(Qt::AlignLeft);
        _formatsBlock.insert(HighlighterState::AlignLeft,formatb);
        formatb.setAlignment(Qt::AlignRight);
        _formatsBlock.insert(HighlighterState::AlignRight,formatb);
    }
    _formats[HighlighterState::AlignCenter]=format;
    _formats[HighlighterState::AlignLeft]=format;
    _formats[HighlighterState::AlignRight]=format;


    //font size
    format = QTextCharFormat();

    format.setFontPointSize(defaultFontSize * 0.5);
    _formats[HighlighterState::tiny] = format;
    format.setFontPointSize(defaultFontSize * 0.7);
    _formats[HighlighterState::scriptsize] = format;
    format.setFontPointSize(defaultFontSize * 0.8);
    _formats[HighlighterState::footnotesize] = format;
    format.setFontPointSize(defaultFontSize * 0.9);
    _formats[HighlighterState::small] = format;
    format.setFontPointSize(defaultFontSize);
    _formats[HighlighterState::normalsize] = format;
    format.setFontPointSize(defaultFontSize * 1.2);
    _formats[HighlighterState::large] = format;
    format.setFontPointSize(defaultFontSize * 1.44);
    _formats[HighlighterState::Large] = format;
    format.setFontPointSize(defaultFontSize * 1.728);
    _formats[HighlighterState::LARGE] = format;
    format.setFontPointSize(defaultFontSize * 2.074);
    _formats[HighlighterState::huge] = format;
    format.setFontPointSize(defaultFontSize * 2.488);
    _formats[HighlighterState::HUGE] = format;


    // font family
    format = QTextCharFormat();
    format.setFontItalic(true);
    _formats[HighlighterState::Italic] = format;
    _formats[HighlighterState::Emph] = format;

    format = QTextCharFormat();
    format.setFontWeight(QFont::Bold);
    _formats[HighlighterState::Bold]=format;

    format = QTextCharFormat();
    format.setFontUnderline(true);
    _formats[HighlighterState::Underline]=format;

    // font color
    format = QTextCharFormat();
    _formats[HighlighterState::Color] = format;

    //link
    format = QTextCharFormat();
    format.setForeground(QBrush(Qt::blue));
    format.setFontUnderline(true);
    _formats[HighlighterState::Link] = format;

    //img
    format = QTextCharFormat();
    format=_formats[HighlighterState::None];
    if(currentMode != DISPLAY_VIEW){
        format.setForeground(QBrush(Qt::darkYellow));
    }
    _formats[HighlighterState::Image] = format;

    //comment
    format = QTextCharFormat();
    format.setForeground(QBrush(Qt::darkGreen));
    format.setFontItalic(true);
    _formats[HighlighterState::Comment] = format;


    //math
    format = QTextCharFormat();
    if(currentMode == DISPLAY_VIEW)
    {
        format.setFontPointSize(0.001);
    }
    else {
        format.setFontItalic(true);
        format.setForeground(QBrush(Qt::darkYellow));
    }
    _formats[HighlighterState::MathInline] = format;
    _formats[HighlighterState::MathEq] = format;

    //format.setForeground(QBrush(QColorm(0, 66, 138)));

    //format.setForeground(QBrush(Qt::gray));

    //format.setForeground(QBrush("#cccccc"));

    //format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

}

/**
 * Sets the text formats
 *
 * @param formats
 */
void latexBasicHighlighter::setTextFormats(
        QHash<HighlighterState, QTextCharFormat> formats)
{
    _formats = formats;
}

/**
 * Sets a text format
 *
 * @param formats
 */
void latexBasicHighlighter::setTextFormat(HighlighterState state,
                                        QTextCharFormat format)
{
    _formats[state] = format;
}

/**
 * Does the latex highlighting
 *
 * @param text
 */
void latexBasicHighlighter::highlightBlock(const QString &text) {
    setCurrentBlockState(-1);//TODO -1
    currentBlock().setUserState(-1);
    //qDebug() << "highlightBlock";
    highlightLatex(text);
    _highlightingFinished = true;
}

void latexBasicHighlighter::highlightLatex(QString text)
{
    if (!text.isEmpty()) {
        setdefault(text);
        highlightAdditionalRules(_highlightingRulesPre, text);

//        // needs to be called after the horizontal ruler highlighting
//        highlightHeadline(text);

        highlightAdditionalRules(_highlightingRulesAfter, text);
    }

//    highlightCommentBlock(text);
//    highlightCodeBlock(text);
}

void latexBasicHighlighter::setdefault(QString text)
{
    QTextCharFormat format=QTextCharFormat();
    format.setFontPointSize(m_fontsize);
    format.setFontItalic(false);
    format.setFontUnderline(false);
    format.setForeground(QBrush(Qt::black));
    format.setFontWeight(QFont::Normal);

    QTextBlockFormat formatb=QTextBlockFormat();
    formatb.setAlignment(Qt::AlignLeft);

    setFormat(0,text.size(),format);

    QTextCursor cursor(doc);
    cursor.select(QTextCursor::Document);
    cursor.setBlockFormat(formatb);
}

/**
 * Highlights the rules from the _highlightingRules list
 *
 * @param text
 */
//TODO color
void latexBasicHighlighter::highlightAdditionalRules(
        QVector<HighlightingRule> &rules, QString text)
{
    QTextCharFormat &defaultFormat = _formats[HighlighterState::Default];

    foreach(const HighlightingRule &rule, rules) {
            // continue if an other current block state was already set if
            // disableIfCurrentStateIsSet is set
            if (rule.disableIfCurrentStateIsSet &&
                    (currentBlockState() != HighlighterState::NoState))
            {
                continue;
            }

            QRegularExpression expression(rule.pattern.pattern(),
                                QRegularExpression::MultilineOption|QRegularExpression::DotMatchesEverythingOption);
            //qDebug() << expression<< "is Valid " << rule.pattern.isValid();
            QRegularExpressionMatchIterator iterator = expression.globalMatch(text);
            QTextCharFormat format = _formats[rule.state];

            // store the current block state if useStateAsCurrentBlockState
            // is set
            if (iterator.hasNext() && rule.blockStyle) {
                setCurrentBlockState(rule.state);
            }

            // find and format all occurrences
            while (iterator.hasNext()) {
                QRegularExpressionMatch match = iterator.next();
                //qDebug() << expression << match;

                //qDebug() << match;
                if(rule.state==HighlighterState::MathEq||rule.state==HighlighterState::MathInline){
                    if(currentMode==displayMode::DISPLAY_VIEW)
                        emit newmath();
                }
                //Block style
                if (rule.blockStyle) {//TODO filed
                    QTextCursor cur = QTextCursor(document());
                    cur.setPosition(match.capturedStart(0)+1);
                    cur.mergeBlockFormat(_formatsBlock[rule.state]);
                }

                // if there is a capturingGroup set then first highlight
                // everything as maskStyle and highlight capturingGroup
                // with the real format
                if (rule.maskedGroup.size() > 0) {
                    QTextCharFormat &currentMaskedFormat = _formats[rule.maskStyle];

                    // set the font size from the current rule's font format
                    if(currentMode != DISPLAY_VIEW)
                        currentMaskedFormat.setFontPointSize(format.fontPointSize());

                    foreach(const int maskedGroup, rule.maskedGroup){
                        setFormat(match.capturedStart(maskedGroup),
                                  match.capturedLength(maskedGroup),
                                  currentMaskedFormat);
                    }
                }

                if(!rule.attrs.isEmpty())
                {
                    if(rule.attrs.contains("color"))
                    {
                        QColor color(match.captured(rule.attrs.value("color")));
                        if(color.isValid())
                            format.setForeground(QBrush(color));
                    }
                    else if(rule.attrs.contains("mainfont"))
                    {
                        QFont font(match.captured(rule.attrs.value("mainfont")));
                        format.setFont(font);
                    }
                    else if(rule.attrs.contains("label"))
                    {
                        QString label=match.captured(rule.attrs.value("label"));
                        if(!label.isEmpty())
                            format.setAnchorHref(label);
                    }
                }
                if (rule.capturingGroup.size() > 0) {
                    foreach(const int capturingGroup, rule.capturingGroup){
                        setFormat(match.capturedStart(capturingGroup),
                                  match.capturedLength(capturingGroup),
                                  format);
                    }
                }
                else
                {
                    setFormat(match.capturedStart(0),
                              match.capturedLength(0),
                              format);
                }
            }
        }
}

void latexBasicHighlighter::setDisplayMode(displayMode mode) {
    currentMode = mode;
    initHighlightingRules();
    initTextFormats(m_fontsize);
    resetDoc();
    //highlightBlock(doc->toPlainText());
}

