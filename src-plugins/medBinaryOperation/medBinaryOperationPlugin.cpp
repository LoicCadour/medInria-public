/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medBinaryOperation.h"
#include "medBinaryOperationPlugin.h"
#include "medBinaryOperationToolBox.h"

#include "itkAndOperator.h"
#include "itkOrOperator.h"

#include <dtkLog/dtkLog.h>

// /////////////////////////////////////////////////////////////////
// medBinaryOperationPluginPrivate
// /////////////////////////////////////////////////////////////////

class medBinaryOperationPluginPrivate
{
public:
    // Class variables go here.
    static const char *s_Name;
};

const char * medBinaryOperationPluginPrivate::s_Name = "medBinaryOperation";

// /////////////////////////////////////////////////////////////////
// medBinaryOperationPlugin
// /////////////////////////////////////////////////////////////////

medBinaryOperationPlugin::medBinaryOperationPlugin(QObject *parent) : dtkPlugin(parent), d(new medBinaryOperationPluginPrivate)
{
    
}

medBinaryOperationPlugin::~medBinaryOperationPlugin()
{
    delete d;
    
    d = NULL;
}

bool medBinaryOperationPlugin::initialize()
{
    if(!medBinaryOperation::registered())
        dtkWarn() << "Unable to register medBinaryOperation type";

    if(!itkAndOperator::registered())
        dtkWarn() << "Unable to register itkAndOperator type";

    if(!itkOrOperator::registered())
        dtkWarn() << "Unable to register itkOrOperator type";
    
    if ( !medBinaryOperationToolBox::registered() )
        dtkWarn() << "Unable to register medBinaryOperation toolbox";
    
    return true;
}

bool medBinaryOperationPlugin::uninitialize()
{
    return true;
}

QString medBinaryOperationPlugin::name() const
{
    return "medBinaryOperationPlugin";
}

QString medBinaryOperationPlugin::description() const
{
    return tr("");
}

QString medBinaryOperationPlugin::version() const
{
    return MEDBINARYOPERATIONPLUGIN_VERSION;
}

QString medBinaryOperationPlugin::contact() const
{
    return "";
}

QStringList medBinaryOperationPlugin::authors() const
{
    QStringList list;
    return list;
}

QStringList medBinaryOperationPlugin::contributors() const
{
    QStringList list;
    return list;
}

QString medBinaryOperationPlugin::identifier() const
{
    return medBinaryOperationPluginPrivate::s_Name;
}


QStringList medBinaryOperationPlugin::tags() const
{
    return QStringList();
}

QStringList medBinaryOperationPlugin::types() const
{
    return QStringList() << "medBinaryOperation";
}
QStringList medBinaryOperationPlugin::dependencies() const
{
    return QStringList();
}
Q_EXPORT_PLUGIN2(medBinaryOperationPlugin, medBinaryOperationPlugin)
