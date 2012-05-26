#include <QtGui>
#include <QtDebug>

#include "fb2save.h"
#include "fb2view.h"

//---------------------------------------------------------------------------
//  Fb2SaveWriter
//---------------------------------------------------------------------------

Fb2SaveWriter::Fb2SaveWriter(Fb2WebView &view, QByteArray *array, QList<int> *folds)
    : QXmlStreamWriter(array)
    , m_folds(folds)
    , m_view(view)
    , m_line(0)
{
    Init();
}

Fb2SaveWriter::Fb2SaveWriter(Fb2WebView &view, QIODevice *device)
    : QXmlStreamWriter(device)
    , m_folds(0)
    , m_view(view)
    , m_line(0)
{
    Init();
}

Fb2SaveWriter::Fb2SaveWriter(Fb2WebView &view, QString *string)
    : QXmlStreamWriter(string)
    , m_folds(0)
    , m_view(view)
    , m_line(0)
{
    Init();
}

void Fb2SaveWriter::Init()
{
    writeStartDocument();
}

Fb2SaveWriter::~Fb2SaveWriter()
{
    writeEndDocument();
}

void Fb2SaveWriter::writeStartElement(const QString &name, int level)
{
    if (level) writeLineEnd();
    for (int i = 1; i < level; i++) writeCharacters("  ");
    QXmlStreamWriter::writeStartElement(name);
}

void Fb2SaveWriter::writeEndElement(int level)
{
    if (level) writeLineEnd();
    for (int i = 1; i < level; i++) writeCharacters("  ");
    QXmlStreamWriter::writeEndElement();
}

void Fb2SaveWriter::writeLineEnd()
{
    writeCharacters("\n");
    m_line++;
}

QString Fb2SaveWriter::getFile(const QString &path)
{
    StringHash::const_iterator it = m_files.find(path);
    if (it == m_files.end()) {
        QString name = m_view.fileName(path);
        if (name.isEmpty()) return QString();
        m_files[name] = path;
        m_names << name;
        return name;
    }
    return it.value();
}

QString Fb2SaveWriter::getData(const QString &name)
{
    return m_view.fileData(name);
}

void Fb2SaveWriter::writeFiles()
{
    StringList::const_iterator it;
    for (it = m_names.constBegin(); it != m_names.constEnd(); it++) {
        QString name = *it;
        if (name.isEmpty()) continue;
        QString data = getData(name);
        if (data.isEmpty()) continue;
        writeStartElement("binary", 2);
        if (m_folds) m_folds->append(m_line);
        writeAttribute("id", name);
        writeLineEnd();
        int pos = 0;
        while (true) {
            QString text = data.mid(pos, 76);
            if (text.isEmpty()) break;
            writeCharacters(text);
            writeLineEnd();
            pos += 76;
        }
        writeCharacters("  ");
        QXmlStreamWriter::writeEndElement();
    }
}

//---------------------------------------------------------------------------
//  Fb2SaveHandler::BodyHandler
//---------------------------------------------------------------------------

FB2_BEGIN_KEYHASH(Fb2SaveHandler::BodyHandler)
    FB2_KEY( Section , "div"    );
    FB2_KEY( Anchor  , "a"      );
    FB2_KEY( Image   , "img"  );
    FB2_KEY( Table   , "table"  );
    FB2_KEY( Parag   , "p"      );
    FB2_KEY( Strong  , "b"      );
    FB2_KEY( Emphas  , "i"      );
    FB2_KEY( Strike  , "strike" );
    FB2_KEY( Sub     , "sub"    );
    FB2_KEY( Sup     , "sup"    );
    FB2_KEY( Code    , "tt"     );
FB2_END_KEYHASH

Fb2SaveHandler::BodyHandler::BodyHandler(Fb2SaveWriter &writer, const QString &name, const QXmlAttributes &atts, const QString &tag)
    : NodeHandler(name)
    , m_writer(writer)
    , m_tag(tag)
    , m_level(1)
    , m_hasChild(false)
{
    Init(atts);
}

Fb2SaveHandler::BodyHandler::BodyHandler(BodyHandler *parent, const QString &name, const QXmlAttributes &atts, const QString &tag)
    : NodeHandler(name)
    , m_writer(parent->m_writer)
    , m_tag(tag)
    , m_level(parent->nextLevel())
    , m_hasChild(false)
{
    Init(atts);
}

void Fb2SaveHandler::BodyHandler::Init(const QXmlAttributes &atts)
{
    if (m_tag.isEmpty()) return;
    m_writer.writeStartElement(m_tag, m_level);
    int count = atts.count();
    for (int i = 0; i < count; i++) {
        QString name = atts.qName(i);
        if (name == "id") {
            m_writer.writeAttribute(name, atts.value(i));
        } else if (name == "name") {
            m_writer.writeAttribute(name, atts.value(i));
        } else if (name.left(4) == "fb2:") {
            m_writer.writeAttribute(name.mid(4), atts.value(i));
        }
    }
}

Fb2XmlHandler::NodeHandler * Fb2SaveHandler::BodyHandler::NewTag(const QString &name, const QXmlAttributes &atts)
{
    m_hasChild = true;
    QString tag = QString();
    switch (toKeyword(name)) {
        case Section   : tag = atts.value("class") ; break;
        case Anchor    : return new AnchorHandler(this, name, atts);
        case Image     : return new ImageHandler(this, name, atts);
        case Parag     : return new ParagHandler(this, name, atts);
        case Strong    : tag = "strong"        ; break;
        case Emphas    : tag = "emphasis"      ; break;
        case Strike    : tag = "strikethrough" ; break;
        case Code      : tag = "code"          ; break;
        case Sub       : tag = "sub"           ; break;
        case Sup       : tag = "sup"           ; break;
        default: ;
    }
    return new BodyHandler(this, name, atts, tag);
}

void Fb2SaveHandler::BodyHandler::TxtTag(const QString &text)
{
    m_writer.writeCharacters(text);
}

void Fb2SaveHandler::BodyHandler::EndTag(const QString &name)
{
    Q_UNUSED(name);
    if (m_tag.isEmpty()) return;
    m_writer.writeEndElement(m_hasChild ? m_level : 0);
}

int Fb2SaveHandler::BodyHandler::nextLevel() const
{
    return m_level ? m_level + 1 : 0;
}

//---------------------------------------------------------------------------
//  Fb2SaveHandler::RootHandler
//---------------------------------------------------------------------------

Fb2SaveHandler::RootHandler::RootHandler(Fb2SaveWriter &writer, const QString &name, const QXmlAttributes &atts)
    : BodyHandler(writer, name, atts, "FictionBook")
{
    m_writer.writeAttribute("xmlns", "http://www.gribuser.ru/xml/fictionbook/2.0");
    m_writer.writeAttribute("xmlns:l", "http://www.w3.org/1999/xlink");
}

void Fb2SaveHandler::RootHandler::EndTag(const QString &name)
{
    m_writer.writeFiles();
    BodyHandler::EndTag(name);
}

//---------------------------------------------------------------------------
//  Fb2SaveHandler::AnchorHandler
//---------------------------------------------------------------------------

Fb2SaveHandler::AnchorHandler::AnchorHandler(BodyHandler *parent, const QString &name, const QXmlAttributes &atts)
    : BodyHandler(parent, name, atts, "a")
{
    QString href = Value(atts, "href");
    m_writer.writeAttribute("l:href", href);
}

//---------------------------------------------------------------------------
//  Fb2SaveHandler::ImageHandler
//---------------------------------------------------------------------------

Fb2SaveHandler::ImageHandler::ImageHandler(BodyHandler *parent, const QString &name, const QXmlAttributes &atts)
    : BodyHandler(parent, name, atts, "image")
{
    QString href = Value(atts, "src");
    QString file = m_writer.getFile(href);
    file.prepend('#');
    m_writer.writeAttribute("l:href", file);
    m_writer.writeEndElement(0);
}

//---------------------------------------------------------------------------
//  Fb2SaveHandler::ParagHandler
//---------------------------------------------------------------------------

Fb2SaveHandler::ParagHandler::ParagHandler(BodyHandler *parent, const QString &name, const QXmlAttributes &atts)
    : BodyHandler(parent, name, atts, "")
    , m_parent(parent->tag())
    , m_empty(true)
{
}

Fb2XmlHandler::NodeHandler * Fb2SaveHandler::ParagHandler::NewTag(const QString &name, const QXmlAttributes &atts)
{
    if (m_empty) start();
    return BodyHandler::NewTag(name, atts);
}

void Fb2SaveHandler::ParagHandler::TxtTag(const QString &text)
{
    if (m_empty) {
        if (isWhiteSpace(text)) return;
        start();
    }
    BodyHandler::TxtTag(text);
}

void Fb2SaveHandler::ParagHandler::EndTag(const QString &name)
{
    Q_UNUSED(name);
    if (m_empty) m_writer.writeStartElement("empty-line", m_level);
    m_writer.writeEndElement(0);
}

void Fb2SaveHandler::ParagHandler::start()
{
    if (!m_empty) return;
    QString tag = m_parent == "stanza" ? "v" : "p";
    m_writer.writeStartElement(tag, m_level);
    m_empty = false;
}

//---------------------------------------------------------------------------
//  Fb2SaveHandler
//---------------------------------------------------------------------------

Fb2SaveHandler::Fb2SaveHandler(Fb2WebView &view, QIODevice *device)
    : Fb2XmlHandler()
    , m_writer(view, device)
{
}

Fb2SaveHandler::Fb2SaveHandler(Fb2WebView &view, QByteArray *array, QList<int> *folds)
    : Fb2XmlHandler()
    , m_writer(view, array, folds)
{
}

Fb2XmlHandler::NodeHandler * Fb2SaveHandler::CreateRoot(const QString &name, const QXmlAttributes &atts)
{
    if (name == "body") return new RootHandler(m_writer, name, atts);
    m_error = QObject::tr("The tag <body> was not found.");
    return 0;
}

