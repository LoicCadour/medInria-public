#pragma once

#include <dtkCore/dtkAbstractDataReader.h>
#include <cartoNavxReadersWritersPluginExport.h>

class CARTONAVXREADERSWRITERSPLUGIN_EXPORT navxDifReader : public dtkAbstractDataReader
{
    Q_OBJECT

public:
    navxDifReader();
    virtual ~navxDifReader();

    virtual QString identifier() const;
    virtual QString description() const;
    virtual QStringList handled() const;

    virtual QStringList supportedFileExtensions() const;

    static bool registered();
    static dtkAbstractDataReader * create();

public slots:
    virtual bool canRead(const QString &file);
    virtual bool read(const QString &file);
};
