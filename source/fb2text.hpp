#ifndef FB2TEXT_H
#define FB2TEXT_H

#include <QFrame>
#include <QResizeEvent>
#include <QTimer>
#include <QThread>
#include <QWebElement>
#include <QWebView>

#include "fb2temp.hpp"

QT_BEGIN_NAMESPACE
class QDockWidget;
class QToolBar;
class QUndoCommand;
class QWebInspector;
QT_END_NAMESPACE

class FbNoteView;
class FbReadThread;
class FbTextElement;

class FbTextBase : public QWebView
{
    Q_OBJECT

public:
    FbTextBase(QWidget* parent = 0)
        : QWebView(parent)
    {
          m_timer.setInterval(100);
          m_timer.setSingleShot(true);
          connect(&m_timer, SIGNAL(timeout()), SLOT(doResize()));
    }

    void addTools(QToolBar *tool);

protected slots:
    void doResize() {
        QResizeEvent event(size(), m_size);
        QWebView::resizeEvent(&event);
        QWebView::update();
    }

protected:
     void resizeEvent(QResizeEvent* event) {
          if (!m_timer.isActive()) m_size = event->oldSize();
          m_timer.start();
     }

     void keyPressEvent(QKeyEvent *event) {
         if (event->key() == Qt::Key_Escape) return;
         QWebView::keyPressEvent(event);
     }

private:
    QTimer m_timer;
    QSize m_size;
};

class FbTextPage : public QWebPage
{
    Q_OBJECT

public:
    explicit FbTextPage(QObject *parent = 0);

    FbNetworkAccessManager *temp();
    void push(QUndoCommand * command, const QString &text = QString());
    FbTextElement element(const QString &location);
    FbTextElement current();
    QString location();
    QString status();

    FbTextElement body();
    FbTextElement doc();

    void appendSection(const FbTextElement &parent);

public slots:
    void insertBody();
    void insertTitle();
    void insertAnnot();
    void insertAuthor();
    void insertEpigraph();
    void insertSubtitle();
    void insertSection();
    void insertPoem();
    void insertStanza();
    void insertDate();

protected:
    virtual bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);

protected:
    static QString div(const QString &style, const QString &text);
    static QString p(const QString &text = "<br/>");
    void update();

private slots:
    void loadFinished();
    void fixContents();
};

class FbTextEdit : public FbTextBase
{
    Q_OBJECT

public:
    explicit FbTextEdit(QWidget *parent = 0);
    virtual ~FbTextEdit();

    FbTextPage *page();
    FbNetworkAccessManager *files();
    void load(const QString &filename, const QString &xml = QString());
    bool save(QIODevice *device, const QString &codec = QString());
    bool save(QByteArray *array);
    bool save(QString *string);

    bool UndoEnabled();
    bool RedoEnabled();
    bool CutEnabled();
    bool CopyEnabled();
    bool BoldChecked();
    bool ItalicChecked();
    bool StrikeChecked();
    bool SubChecked();
    bool SupChecked();

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

public slots:
    void html(QString html);
    void data(QString name, QByteArray data);
    void insertImage();
    void insertNote();
    void insertLink();
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void find();

private slots:
    void linkHovered(const QString &link, const QString &title, const QString &textContent);

private:
    void execCommand(const QString &cmd, const QString &arg);
    void exec(QUndoCommand *command);
    FbTemporaryFile * file(const QString &name);
    FbNoteView & noteView();
    QWebElement body();
    QWebElement doc();

private:
    FbNoteView *m_noteView;
    FbReadThread *m_thread;
    QPoint m_point;
};

class FbTextFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FbTextFrame(QWidget* parent = 0);
    ~FbTextFrame();
    void hideInspector();
    FbTextEdit view;

public slots:
    void showInspector();

private:
    QDockWidget * dock;
};

#endif // FB2TEXT_H
