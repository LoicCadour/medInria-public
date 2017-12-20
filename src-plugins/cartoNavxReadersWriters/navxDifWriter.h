#pragma once

#include <medAbstractDataWriter.h>
#include <cartoNavxReadersWritersPluginExport.h>
#include <medDataIndex.h>
#include <medAbstractData.h>

class CARTONAVXREADERSWRITERSPLUGIN_EXPORT navxDifWriter : public medAbstractDataWriter
{
    Q_OBJECT

public:
    navxDifWriter();

    virtual QString identifier() const;
    virtual QString description() const;
    virtual QStringList handled() const;

    virtual QStringList supportedFileExtensions() const;

    static bool registered();
    static dtkAbstractDataWriter * create();

    using medAbstractDataWriter::setData;

public slots:
    virtual bool canWrite(const QString &file);
    virtual bool write(const QString &file);
    void showWarning(QString warning);
    void showWarningPopUp(QString warning);

signals:
    void needWarning(QString warning);
    void needWarningPopUp(QString warning);

private:
    QString getMetaData(medAbstractData* medData, QString key);
    void displayWarning(QString warning);
    void displayWarningPopUp(QString warning);
    void askForConfirmationIfNavXLimitsExceeded();
    bool writeFile(const QString & path);

    bool validated;

    // NavX release from Feb 2016 has a limit of 100k vertices and 200k triangles
    static const int maxNumberVertices  = 100000;
    static const int maxNumberTriangles = 200000;
};
