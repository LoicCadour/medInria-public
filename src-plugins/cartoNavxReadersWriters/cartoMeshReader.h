#pragma once

#include <dtkCore/dtkAbstractDataReader.h>

#include <cartoNavxReadersWritersPluginExport.h>

class cartoMeshReaderPrivate;

class CARTONAVXREADERSWRITERSPLUGIN_EXPORT cartoMeshReader : public dtkAbstractDataReader
{
    Q_OBJECT

public:
             cartoMeshReader(void);
    virtual ~cartoMeshReader(void);

    virtual QString identifier() const;
    virtual QString description(void) const;    
    virtual QStringList handled(void) const;

    static bool registered(void);
    static dtkAbstractDataReader * create(void);

public slots:
    virtual bool canRead(const QString& file);
    virtual bool read(const QString& file);

private:
    bool canReadVersion(const QByteArray & line, int & version);
    bool canReadSection(const QByteArray & line, QString & title);
    bool canReadKeyValues(const QByteArray & line, QString & key, QStringList & values);
    bool canReadMoreValues(const QByteArray & line, QStringList & values);
    bool canSkipLine(const QByteArray & line);

    bool processGeneralAttributes(const QString & key, const QStringList & values);
    bool processVertice(const QString & key, const QStringList & values);
    bool processTriangles(const QString & key, const QStringList & values);
    bool processVerticesColors(const QString & key, const QStringList & values);

    cartoMeshReaderPrivate * d;
};
