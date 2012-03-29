// /////////////////////////////////////////////////////////////////
// Generated by dtkPluginGenerator
// /////////////////////////////////////////////////////////////////

//#include "itkDataImageWriter.h"
#include "itkDataImageWriterPlugin.h"
#include "itkMetaDataImageWriter.h"
#include "itkAnalyzeDataImageWriter.h"
#include "itkPNGDataImageWriter.h"
#include "itkBMPDataImageWriter.h"
#include "itkJPEGDataImageWriter.h"
#include "itkTIFFDataImageWriter.h"
#include "itkNiftiDataImageWriter.h"
#include "itkNrrdDataImageWriter.h"
#include "itkGiplDataImageWriter.h"
#include "itkVTKDataImageWriter.h"
#include "itkPhilipsRECDataImageWriter.h"

#include <dtkCore/dtkLog.h>
#include <dtkItkLogForwarder/itkLogForwarder.h>

// /////////////////////////////////////////////////////////////////
// itkDataImageWriterPluginPrivate
// /////////////////////////////////////////////////////////////////

class itkDataImageWriterPluginPrivate
{
public:
    itkDataImageWriterPluginPrivate()
    {
        forwarder = itk::LogForwarder::New();
    }
    ~itkDataImageWriterPluginPrivate()
    {

    }
    // Class variables go here.
    itk::LogForwarder::Pointer forwarder;
};

// /////////////////////////////////////////////////////////////////
// itkDataImageWriterPlugin
// /////////////////////////////////////////////////////////////////

itkDataImageWriterPlugin::itkDataImageWriterPlugin(QObject *parent) : dtkPlugin(parent), d(new itkDataImageWriterPluginPrivate)
{

}

itkDataImageWriterPlugin::~itkDataImageWriterPlugin(void)
{
    delete d;

    d = NULL;
}

bool itkDataImageWriterPlugin::initialize(void)
{
    /*
    if(!itkDataImageChar3Writer::registered()) dtkWarning() << "Unable to register itkDataImageChar3Writer type";
    if(!itkDataImageUChar3Writer::registered()) dtkWarning() << "Unable to register itkDataImageUChar3Writer type";
    if(!itkDataImageShort3Writer::registered()) dtkWarning() << "Unable to register itkDataImageShort3Writer type";
    if(!itkDataImageUShort3Writer::registered()) dtkWarning() << "Unable to register itkDataImageUShort3Writer type";
    if(!itkDataImageInt3Writer::registered()) dtkWarning() << "Unable to register itkDataImageInt3Writer type";
    if(!itkDataImageUInt3Writer::registered()) dtkWarning() << "Unable to register itkDataImageUInt3Writer type";
    if(!itkDataImageLong3Writer::registered()) dtkWarning() << "Unable to register itkDataImageLong3Writer type";
    if(!itkDataImageULong3Writer::registered()) dtkWarning() << "Unable to register itkDataImageULong3Writer type";
    if(!itkDataImageFloat3Writer::registered()) dtkWarning() << "Unable to register itkDataImageFloat3Writer type";
    if(!itkDataImageDouble3Writer::registered()) dtkWarning() << "Unable to register itkDataImageDouble3Writer type";
    if(!itkDataImageVector3Writer::registered()) dtkWarning() << "Unable to register itkDataImageVector3Writer type";
    if(!itkDataImageRGB3Writer::registered()) dtkWarning() << "Unable to register itkDataImageRGB3Writer type";
  */
    if(!itkMetaDataImageWriter::registered()) dtkWarning() << "Unable to register itkMetaDataImageWriter type";
    if(!itkNiftiDataImageWriter::registered()) dtkWarning() << "Unable to register itkNiftiDataImageWriter type";
    if(!itkAnalyzeDataImageWriter::registered()) dtkWarning() << "Unable to register itkAnalyzeDataImageWriter type";
    if(!itkNrrdDataImageWriter::registered()) dtkWarning() << "Unable to register itkNrrdDataImageWriter type";
    if(!itkGiplDataImageWriter::registered()) dtkWarning() << "Unable to register itkGiplDataImageWriter type";
    if(!itkVTKDataImageWriter::registered()) dtkWarning() << "Unable to register itkVTKDataImageWriter type";
    if(!itkPNGDataImageWriter::registered()) dtkWarning() << "Unable to register itkPNGDataImageWriter type";
    if(!itkBMPDataImageWriter::registered()) dtkWarning() << "Unable to register itkBMPDataImageWriter type";
    if(!itkJPEGDataImageWriter::registered()) dtkWarning() << "Unable to register itkJPEGDataImageWriter type";
    if(!itkTIFFDataImageWriter::registered()) dtkWarning() << "Unable to register itkTIFFDataImageWriter type";
    if(!itkPhilipsRECDataImageWriter::registered()) dtkWarning() << "Unable to register itkPhilipsRECDataImageWriter type";


    return true;
}

bool itkDataImageWriterPlugin::uninitialize(void)
{
    return true;
}

QString itkDataImageWriterPlugin::name(void) const
{
    return "itkDataImageWriterPlugin";
}

QString itkDataImageWriterPlugin::description(void) const
{
    return "Writers for any ITK image data type.";
}

QString itkDataImageWriterPlugin::version(void) const
{
    return ITKDATAIMAGEWRITERPLUGIN_VERSION;
}


QStringList itkDataImageWriterPlugin::tags(void) const
{
    return QStringList() << "itk" << "data" << "image" << "writer";
}

QStringList itkDataImageWriterPlugin::types(void) const
{
    return QStringList() << "itkMetaDataImageWriter"
            << "itkNiftiDataImageWriter"
            << "itkAnalyzeDataImageWriter"
            << "itkNrrdDataImageWriter"
            << "itkGiplDataImageWriter"
            << "itkVTKDataImageWriter"
            << "itkPNGDataImageWriter"
            << "itkBMPDataImageWriter"
            << "itkJPEGDataImageWriter"
            << "itkTIFFDataImageWriter";
}

Q_EXPORT_PLUGIN2(itkDataImageWriterPlugin, itkDataImageWriterPlugin)
