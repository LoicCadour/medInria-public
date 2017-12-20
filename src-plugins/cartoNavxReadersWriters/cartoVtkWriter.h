#pragma once

#include <cartoNavxReadersWritersPluginExport.h>
#include <medAbstractDataWriter.h>

class vtkPolyData;

class CARTONAVXREADERSWRITERSPLUGIN_EXPORT cartoVtkWriter : public medAbstractDataWriter
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
    void showPatientInfoDialog();

signals:
    void needMoreParameters();

private:
    void rescaleScalarsToLUTRange(vtkPolyData * mesh);
    QString patientLastName, patientFirstName, patientID;
    QList<QString> ptAttributes, ptValues;
    bool isDialogOK;
};
