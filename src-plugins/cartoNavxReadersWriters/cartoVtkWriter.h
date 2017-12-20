#pragma once

#include <dtkCore/dtkAbstractDataWriter.h>
#include <cartoNavxReadersWritersPluginExport.h>
#include <medDataIndex.h>

class vtkPolyData;

class CARTONAVXREADERSWRITERSPLUGIN_EXPORT cartoVtkWriter : public dtkAbstractDataWriter
{
    Q_OBJECT

public:
    cartoVtkWriter();
    virtual ~cartoVtkWriter();

    virtual QString identifier() const;
    virtual QString description() const;
    virtual QStringList handled() const;

    virtual QStringList supportedFileExtensions() const;

    static bool registered();
    static dtkAbstractDataWriter * create();

public slots:
    virtual bool canWrite(const QString &file);
    virtual bool write(const QString &file);

private:
    void rescaleScalarsToLUTRange(vtkPolyData * mesh);
};
