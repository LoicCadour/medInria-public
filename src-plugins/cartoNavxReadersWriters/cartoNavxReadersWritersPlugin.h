#pragma once

#include <dtkCore/dtkPlugin.h>
#include <cartoNavxReadersWritersPluginExport.h>

class CARTONAVXREADERSWRITERSPLUGIN_EXPORT cartoNavxReadersWritersPlugin : public dtkPlugin
{
    Q_OBJECT
    Q_INTERFACES(dtkPlugin)

public:
    cartoNavxReadersWritersPlugin(QObject *parent = 0);
    ~cartoNavxReadersWritersPlugin(void);

    virtual bool initialize(void);
    virtual bool uninitialize(void);

    virtual QString name(void) const;
    virtual QString description(void) const;

    virtual QStringList authors(void) const;
    virtual QStringList contributors() const;
    virtual QStringList tags(void) const;
    virtual QStringList types(void) const;
};
