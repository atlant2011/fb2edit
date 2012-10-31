#include "fb2dock.hpp"
#include "fb2code.hpp"
#include "fb2head.hpp"
#include "fb2text.hpp"

#include <QLayout>
#include <QtDebug>

//---------------------------------------------------------------------------
//  FbModeAction
//---------------------------------------------------------------------------

FbModeAction::FbModeAction(FbMainDock *parent, Fb::Mode mode, const QString &text)
    : QAction(text, parent)
    , m_dock(parent)
    , m_mode(mode)
{
    setCheckable(true);
    connect(this, SIGNAL(triggered()), SLOT(switchMode()));
}

void FbModeAction::switchMode()
{
    m_dock->setMode(m_mode);
}

//---------------------------------------------------------------------------
//  FbMainDock
//---------------------------------------------------------------------------

FbMainDock::FbMainDock(QWidget *parent)
    : QStackedWidget(parent)
    , isSwitched(false)
{
    textFrame = new FbTextFrame(this);
    m_text = new FbTextEdit(textFrame, parent);
    textFrame->layout()->addWidget(m_text);

    m_head = new FbHeadEdit(this, m_text);

    m_code = new FbCodeEdit(this);

    addWidget(textFrame);
    addWidget(m_head);
    addWidget(m_code);
}

void FbMainDock::setMode(Fb::Mode mode)
{
    if (mode == m_mode) return;
    switch (m_mode = mode) {
        case Fb::Text: setModeText(); return;
        case Fb::Head: setModeHead(); return;
        case Fb::Code: setModeCode(); return;
        case Fb::Html: setModeHtml(); return;
    }
}

void FbMainDock::setModeText()
{
    m_mode = Fb::Text;
    setCurrentWidget(textFrame);
    m_head->disconnectActions();
    m_code->disconnectActions();
    m_text->connectActions(m_tool);
}

void FbMainDock::setModeHead()
{
    m_mode = Fb::Head;
    setCurrentWidget(m_head);
    m_text->disconnectActions();
    m_code->disconnectActions();
    m_head->connectActions(m_tool);
}

void FbMainDock::setModeCode()
{
    m_mode = Fb::Code;
    setCurrentWidget(m_code);
    m_head->disconnectActions();
    m_code->disconnectActions();
    m_code->connectActions(m_tool);
}

void FbMainDock::setModeHtml()
{
    m_mode = Fb::Html;
    setCurrentWidget(m_code);
    m_head->disconnectActions();
    m_code->disconnectActions();
    m_code->connectActions(m_tool);
}

bool FbMainDock::load(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCritical() << QObject::tr("Cannot read file %1: %2.").arg(filename).arg(file.errorString());
        return false;
    }

    if (currentWidget() == m_code) {
        m_code->clear();
        return m_code->read(&file);
    }

    return false;
}

bool FbMainDock::save(QIODevice *device, const QString &codec)
{
    if (currentWidget() == m_code) {
        QTextStream out(device);
        out << m_code->toPlainText();
    } else {
        isSwitched = false;
        m_text->save(device, codec);
    }
    return true;
}

bool FbMainDock::isModified() const
{
    if (isSwitched) return true;
    QWidget * current = currentWidget();
    if (current == textFrame) return m_text->isModified();
    if (current == m_head) return m_text->isModified();
    if (current == m_code) return m_code->isModified();
    return false;
}