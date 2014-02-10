/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkExtractFilter.h"
#include "itkExtractFilterPlugin.h"
#include "itkExtractFilterToolBox.h"

#include <dtkLog/dtkLog.h>

// /////////////////////////////////////////////////////////////////
// itkExtractFilterPluginPrivate
// /////////////////////////////////////////////////////////////////

class itkExtractFilterPluginPrivate
{
public:
    // Class variables go here.
    static const char *s_Name;
};

const char * itkExtractFilterPluginPrivate::s_Name = "itkExtractFilter";

// /////////////////////////////////////////////////////////////////
// itkExtractFilterPlugin
// /////////////////////////////////////////////////////////////////

itkExtractFilterPlugin::itkExtractFilterPlugin(QObject *parent) : dtkPlugin(parent), d(new itkExtractFilterPluginPrivate)
{
    
}

itkExtractFilterPlugin::~itkExtractFilterPlugin()
{
    delete d;
    
    d = NULL;
}

bool itkExtractFilterPlugin::initialize()
{
    if(!itkExtractFilter::registered())
        dtkWarn() << "Unable to register itkExtractFilter type";
    
    if ( !itkExtractFilterToolBox::registered() )
        dtkWarn() << "Unable to register itkExtractFilter toolbox";
    
    return true;
}

bool itkExtractFilterPlugin::uninitialize()
{
    return true;
}

QString itkExtractFilterPlugin::name() const
{
    return "itkExtractFilterPlugin";
}

QString itkExtractFilterPlugin::description() const
{
    return tr("");
}

QString itkExtractFilterPlugin::version() const
{
    return ITKEXTRACTFILTERPLUGIN_VERSION;
}

QString itkExtractFilterPlugin::contact() const
{
    return "";
}

QStringList itkExtractFilterPlugin::authors() const
{
    QStringList list;
    return list;
}

QStringList itkExtractFilterPlugin::contributors() const
{
    QStringList list;
    return list;
}

QString itkExtractFilterPlugin::identifier() const
{
    return itkExtractFilterPluginPrivate::s_Name;
}


QStringList itkExtractFilterPlugin::tags() const
{
    return QStringList();
}

QStringList itkExtractFilterPlugin::types() const
{
    return QStringList() << "itkExtractFilter";
}
QStringList itkExtractFilterPlugin::dependencies() const
{
    return QStringList();
}
Q_EXPORT_PLUGIN2(itkExtractFilterPlugin, itkExtractFilterPlugin)
