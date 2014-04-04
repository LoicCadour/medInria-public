/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkN4BiasCorrectionProcess.h"
#include "itkN4BiasCorrectionProcessPlugin.h"
#include "itkN4BiasCorrectionProcessToolBox.h"

#include <dtkLog/dtkLog.h>

// /////////////////////////////////////////////////////////////////
// itkN4BiasCorrectionProcessPluginPrivate
// /////////////////////////////////////////////////////////////////

class itkN4BiasCorrectionProcessPluginPrivate
{
public:
    // Class variables go here.
    static const char *s_Name;
};

const char * itkN4BiasCorrectionProcessPluginPrivate::s_Name = "itkN4BiasCorrectionProcess";

// /////////////////////////////////////////////////////////////////
// itkN4BiasCorrectionProcessPlugin
// /////////////////////////////////////////////////////////////////

itkN4BiasCorrectionProcessPlugin::itkN4BiasCorrectionProcessPlugin(QObject *parent) : dtkPlugin(parent), d(new itkN4BiasCorrectionProcessPluginPrivate)
{
    
}

itkN4BiasCorrectionProcessPlugin::~itkN4BiasCorrectionProcessPlugin()
{
    delete d;
    
    d = NULL;
}

bool itkN4BiasCorrectionProcessPlugin::initialize()
{
    if(!itkN4BiasCorrectionProcess::registered())
        dtkWarn() << "Unable to register itkN4BiasCorrectionProcess type";
    
    if ( !itkN4BiasCorrectionProcessToolBox::registered() )
        dtkWarn() << "Unable to register itkN4BiasCorrectionProcess toolbox";
    
    return true;
}

bool itkN4BiasCorrectionProcessPlugin::uninitialize()
{
    return true;
}

QString itkN4BiasCorrectionProcessPlugin::name() const
{
    return "itkN4BiasCorrectionProcessPlugin";
}

QString itkN4BiasCorrectionProcessPlugin::description() const
{
    return tr("");
}

QString itkN4BiasCorrectionProcessPlugin::version() const
{
    return ITKN4BIASCORRECTIONPROCESSPLUGIN_VERSION;
}

QString itkN4BiasCorrectionProcessPlugin::contact() const
{
    return "";
}

QStringList itkN4BiasCorrectionProcessPlugin::authors() const
{
    QStringList list;
    return list;
}

QStringList itkN4BiasCorrectionProcessPlugin::contributors() const
{
    QStringList list;
    return list;
}

QString itkN4BiasCorrectionProcessPlugin::identifier() const
{
    return itkN4BiasCorrectionProcessPluginPrivate::s_Name;
}


QStringList itkN4BiasCorrectionProcessPlugin::tags() const
{
    return QStringList();
}

QStringList itkN4BiasCorrectionProcessPlugin::types() const
{
    return QStringList() << "itkN4BiasCorrectionProcess";
}
QStringList itkN4BiasCorrectionProcessPlugin::dependencies() const
{
    return QStringList();
}
Q_EXPORT_PLUGIN2(itkN4BiasCorrectionProcessPlugin, itkN4BiasCorrectionProcessPlugin)
