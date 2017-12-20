#include <cartoNavxReadersWritersPlugin.h>
#include <cartoVtkWriter.h>
#include <navxDifReader.h>
#include <navxDifWriter.h>

#include <dtkLog/dtkLog.h>

cartoNavxReadersWritersPlugin::cartoNavxReadersWritersPlugin(QObject *parent) : dtkPlugin(parent)
{
}

cartoNavxReadersWritersPlugin::~cartoNavxReadersWritersPlugin(void)
{
}

bool cartoNavxReadersWritersPlugin::initialize(void)
{
    if( ! cartoVtkWriter::registered()) {
        dtkWarn() << "Unable to register cartoVtkWriter type";
    }
    if( ! navxDifWriter::registered()) {
        dtkWarn() << "Unable to register navxDifWriter type";
    }
    if( ! navxDifReader::registered()) {
        dtkWarn() << "Unable to register navxDifReader type";
    }
    return true;
}

bool cartoNavxReadersWritersPlugin::uninitialize(void)
{
    return true;
}

QString cartoNavxReadersWritersPlugin::name(void) const
{
    return "cartoNavxReadersWritersPlugin";
}

QString cartoNavxReadersWritersPlugin::description(void) const
{
    return "";
}

QStringList cartoNavxReadersWritersPlugin::authors(void) const
{
    return QStringList() << "Florian Vichot";
}

QStringList cartoNavxReadersWritersPlugin::contributors() const
{
    QStringList list;
    list << "Mathilde Merle <mathilde.merle@ihu-liryc.fr>";
    return list;
}

QStringList cartoNavxReadersWritersPlugin::tags(void) const
{
    return QStringList() << "CARTO" << "Mesh";
}

QStringList cartoNavxReadersWritersPlugin::types(void) const
{
    return QStringList() << "cartoMeshReader" << "cartoVtkWriter";
}

Q_EXPORT_PLUGIN2(cartoNavxReadersWritersPlugin, cartoNavxReadersWritersPlugin)
