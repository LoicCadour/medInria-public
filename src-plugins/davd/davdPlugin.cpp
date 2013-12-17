/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "davdWorkspace.h"
#include "davdPlugin.h"

#include <dtkLog/dtkLog.h>

// /////////////////////////////////////////////////////////////////
// davdPluginPrivate
// /////////////////////////////////////////////////////////////////

class davdPluginPrivate
{
public:
    // Class variables go here.
    static const char *s_Name;
};

const char * davdPluginPrivate::s_Name = "davd";

// /////////////////////////////////////////////////////////////////
// davdPlugin
// /////////////////////////////////////////////////////////////////

davdPlugin::davdPlugin(QObject *parent) : dtkPlugin(parent), d(new davdPluginPrivate)
{
    
}

davdPlugin::~davdPlugin()
{
    delete d;
    
    d = NULL;
}

bool davdPlugin::initialize()
{
    if(!davdWorkspace::registered())
        dtkWarn() << "Unable to register davd type";
    
    return true;
}

bool davdPlugin::uninitialize()
{
    return true;
}

QString davdPlugin::name() const
{
    return "davdPlugin";
}

QString davdPlugin::description() const
{
    return tr("");
}

QString davdPlugin::version() const
{
    return DAVDPLUGIN_VERSION;
}

QString davdPlugin::contact() const
{
    return "";
}

QStringList davdPlugin::authors() const
{
    QStringList list;
    return list;
}

QStringList davdPlugin::contributors() const
{
    QStringList list;
    return list;
}

QString davdPlugin::identifier() const
{
    return davdPluginPrivate::s_Name;
}


QStringList davdPlugin::tags() const
{
    return QStringList();
}

QStringList davdPlugin::types() const
{
    return QStringList() << "davd";
}
QStringList davdPlugin::dependencies() const
{
    return QStringList();
}
Q_EXPORT_PLUGIN2(davdPlugin, davdPlugin)
