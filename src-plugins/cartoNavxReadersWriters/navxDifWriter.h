#pragma once

#include <dtkCore/dtkAbstractDataWriter.h>
#include <cartoNavxReadersWritersPluginExport.h>
#include <medDataIndex.h>

class CARTONAVXREADERSWRITERSPLUGIN_EXPORT navxDifWriter : public dtkAbstractDataWriter
{
    Q_OBJECT

private:
    QList<medDataIndex> indexList;

public:
    navxDifWriter();
    virtual ~navxDifWriter();

    virtual QString identifier() const;
    virtual QString description() const;
    virtual QStringList handled() const;

    virtual QStringList supportedFileExtensions() const;

    static bool registered();
    static dtkAbstractDataWriter * create();

public slots:
    virtual bool canWrite(const QString &file);
    virtual bool write(const QString &file);
    void selectDataToSave();

signals:
    void needDataList();
};
