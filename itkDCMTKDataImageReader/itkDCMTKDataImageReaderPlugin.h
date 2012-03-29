// /////////////////////////////////////////////////////////////////
// Generated by dtkPluginGenerator
// /////////////////////////////////////////////////////////////////

#ifndef ITKDCMTKDATAIMAGEREADERPLUGIN_H
#define ITKDCMTKDATAIMAGEREADERPLUGIN_H

#include <dtkCore/dtkPlugin.h>

#include "itkDCMTKDataImageReaderPluginExport.h"

class ITKDCMTKDATAIMAGEREADERPLUGIN_EXPORT itkDCMTKDataImageReaderPluginPrivate;

class ITKDCMTKDATAIMAGEREADERPLUGIN_EXPORT itkDCMTKDataImageReaderPlugin : public dtkPlugin
{
    Q_OBJECT
    Q_INTERFACES(dtkPlugin)

public:
     itkDCMTKDataImageReaderPlugin(QObject *parent = 0);
    ~itkDCMTKDataImageReaderPlugin(void);

    virtual bool initialize(void);
    virtual bool uninitialize(void);

    virtual QString name(void) const;
    virtual QString description(void) const;

    virtual QString version(void) const;

    virtual QStringList authors(void) const;
    virtual QStringList contributors(void) const;
    virtual QStringList tags(void) const;
    virtual QStringList types(void) const;

private:
     itkDCMTKDataImageReaderPluginPrivate *d;
};

#endif
