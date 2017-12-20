#include <cartoMeshReader.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractData.h>
#include <medAbstractDataFactory.h>

#include <vtkMetaSurfaceMesh.h>
#include <vtkPolyData.h>
#include <vtkIdList.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>

// /////////////////////////////////////////////////////////////////
// cartoMeshReaderPrivate
// /////////////////////////////////////////////////////////////////

struct GeneralAttributes
{
    GeneralAttributes()
    {
        numVertex = numTriangles = 0;
        numVertexColors = 0;
        topologyStatus = -1;
        meshColor.setRgba(qRgba(0,1,0,1)); // default color per the spec is green
    }

    QString meshName;
    unsigned int numVertex;
    unsigned int numTriangles;

    unsigned int numVertexColors;
    QStringList colorsNames;
    QColor meshColor;
    int topologyStatus;
    QMatrix4x4 matrix;
};

class cartoMeshReaderPrivate
{
  public:
    cartoMeshReaderPrivate()
        : versionRE("#TriangulatedMeshVersion(\\d)\\.(\\d)")
        , sectionTitleRE("\\[([a-z]+)\\]", Qt::CaseInsensitive)
        , keyValueRE("([0-9a-z]+)\\s*=\\s*((?:\\S+\\s*)*)", Qt::CaseInsensitive)
        , valuesRE("(\\S+)")
        , skippableLineRE(";.*")
        , metaDataSet(NULL)
        , polyData(NULL)
        , points(NULL)
        , idList(NULL)
        , scalarsArrays(NULL)
    {
    }

    QRegExp versionRE;
    QRegExp sectionTitleRE;
    QRegExp keyValueRE;
    QRegExp valuesRE;
    QRegExp skippableLineRE;

    GeneralAttributes generalAttributes;
    vtkMetaDataSet * metaDataSet;
    vtkPolyData * polyData;
    vtkPoints * points;
    vtkIdList * idList;
    vtkDoubleArray ** scalarsArrays;
};

typedef bool (cartoMeshReader::*ProcessorMemberFunc)(const QString &, const QStringList &);


typedef enum {
    Not_Started,
    Read_Version,
    Read_Section,
    Read_KeyValues,
    Read_MoreValues
} IniState;

#define PARSE_ERROR(format, ...) do{qDebug("cartoMeshReader [at line %d]: " format, lineCount, ##__VA_ARGS__);return false;}while(0);
#define PARSE_FAILED PARSE_ERROR("Parsing '%s' failed", qPrintable(path))

// /////////////////////////////////////////////////////////////////
// cartoMeshReader
// /////////////////////////////////////////////////////////////////

cartoMeshReader::cartoMeshReader(void) : dtkAbstractDataReader(), d(new cartoMeshReaderPrivate)
{

}


cartoMeshReader::~cartoMeshReader(void)
{
    if (d)
    {
        if (d->metaDataSet)
            d->metaDataSet->Delete();
        d->metaDataSet = NULL;
        if (d->polyData)
            d->polyData->Delete();
        d->polyData = NULL;
        if (d->points)
            d->points->Delete();
        d->points = NULL;
        if (d->idList)
            d->idList->Delete();
        d->idList = NULL;
        if (d->scalarsArrays)
        {
            for(unsigned int i = 0; i < d->generalAttributes.numVertexColors; ++i)
            {
                if (d->scalarsArrays[i]) d->scalarsArrays[i]->Delete();
                d->scalarsArrays[i] = NULL;
            }
            delete[] d->scalarsArrays;
            d->scalarsArrays = NULL;
        }
    }
    delete d;
    d = NULL;
}


QString cartoMeshReader::identifier() const
{
    return QString("cartoMeshReader");
}


QString cartoMeshReader::description(void) const
{
    return QString("Reader for BioSense CARTO .mesh files");
}


QStringList cartoMeshReader::handled(void) const
{
    return QStringList() << "vtkDataMesh";
}


bool cartoMeshReader::registered(void)
{
    return medAbstractDataFactory::instance()->registerDataReaderType("cartoMeshReader", QStringList() << "vtkDataMesh", create);
}


bool cartoMeshReader::canRead(const QString & path)
{
    if (path.endsWith(".mesh"))
        return true;
    return false;
}


bool cartoMeshReader::read(const QString & path)
{
    qDebug() << "Can read with: " << identifier();

    QFile file(path);
    if( ! file.open(QIODevice::ReadOnly))
    {
        qWarning("cartoMeshReader: File could not be opened for reading");
        return false;
    }

    dtkSmartPointer<dtkAbstractData> meshData = medAbstractDataFactory::instance()->createSmartPointer("vtkDataMesh");
    d->metaDataSet = vtkMetaSurfaceMesh::New();
    d->polyData = vtkPolyData::New();
    d->points = vtkPoints::New();

    QByteArray line;
    int lineCount = -1;

    IniState currentState = Not_Started;
    int version = -1;
    QString currentSection;
    QString currentKey;
    QStringList currentValues;
    ProcessorMemberFunc currentProcessor = NULL;
    while( ! file.atEnd() )
    {
        currentValues.clear();

        line = file.readLine().trimmed();
        lineCount++;

        if (canSkipLine(line)) continue;

        if (currentState == Not_Started && canReadVersion(line, version))
        {
            if (version != 2 /*&& version != 3*/)
                PARSE_ERROR("unsupported version : %d", version);
            currentState = Read_Version;
        }
        else if( (currentState == Read_Version || currentState == Read_Section || currentState == Read_KeyValues ) && canReadSection(line, currentSection))
        {
            if(currentSection == "GeneralAttributes")
                currentProcessor = &cartoMeshReader::processGeneralAttributes;
            else if(currentSection == "VerticesSection")
            {
                if(d->generalAttributes.topologyStatus != 0)
                {
                    qDebug("cartoMeshReader: the TopologyStatus is not supported");
                    return false;
                }
                if (d->generalAttributes.numVertex == 0)
                {
                    qDebug("cartoMeshReader: no vertices in the mesh !");
                    return false;
                }
                d->points->SetNumberOfPoints(d->generalAttributes.numVertex);
                currentProcessor = &cartoMeshReader::processVertice;
            }
            else if(currentSection == "TrianglesSection")
            {
                if (d->generalAttributes.numTriangles == 0)
                {
                    qDebug("cartoMeshReader: NumTriangle == 0 !");
                    return false;
                }
                if (d->points->GetNumberOfPoints() == 0)
                {
                    qDebug("cartoMeshReader: no vertices in the mesh !");
                    return false;
                }
                d->polyData->SetPoints(d->points);
                d->polyData->Allocate(d->generalAttributes.numTriangles);
                d->idList = vtkIdList::New();
                d->idList->SetNumberOfIds(3);
                currentProcessor = &cartoMeshReader::processTriangles;
            }
            else if(currentSection == "VerticesColorsSection")
            {
                if (d->generalAttributes.numVertexColors == 0)
                {
                    qDebug("cartoMeshReader: VerticesColorsSection section with NumVertexColors == 0");
                    currentProcessor = NULL;
                }
                else
                {
                    d->scalarsArrays = new vtkDoubleArray*[d->generalAttributes.numVertexColors];
                    for(unsigned int i = 0; i < d->generalAttributes.numVertexColors; ++i)
                    {
                        d->scalarsArrays[i] = vtkDoubleArray::New();
                        d->scalarsArrays[i]->SetNumberOfComponents(1);
                        d->scalarsArrays[i]->SetNumberOfValues(d->generalAttributes.numVertex);
                        if ((unsigned int)d->generalAttributes.colorsNames.size() > i)
                            d->scalarsArrays[i]->SetName(d->generalAttributes.colorsNames.at(i).toUtf8().constData());
                        else
                            d->scalarsArrays[i]->SetName(QString("__attribute_%d").arg(i).toAscii().constData());
                    }
                    currentProcessor = &cartoMeshReader::processVerticesColors;
                }
            }

            currentState = Read_Section;
        }
        else if ((currentState == Read_Section || currentState == Read_KeyValues ) && canReadKeyValues(line, currentKey, currentValues))
        {
            if(currentProcessor)
                (this->*currentProcessor)(currentKey, currentValues);
            currentState = Read_KeyValues;
        }
        else PARSE_FAILED;

        this->setProgress(file.pos() / file.size() * 100);
    }

    d->polyData->Update();

    if (d->polyData->GetNumberOfPoints() == 0 || d->polyData->GetNumberOfCells() == 0)
        return false;

    d->metaDataSet->SetDataSet(d->polyData);
    for(unsigned int i = 0; i < d->generalAttributes.numVertexColors; ++i)
    {
        d->metaDataSet->GetDataSet()->GetPointData()->AddArray(d->scalarsArrays[i]);
        // Apparently this is the null value for scalar data, but it's not documented anywhere...
        //d->metaDataSet->SetScalarNullValue(d->scalarsArrays[i]->GetName(), -10000.0);
    }

    meshData->setData(d->metaDataSet);
    this->setData(meshData);

    return true;
}


bool cartoMeshReader::canReadVersion(const QByteArray & line, int & version)
{
    if(d->versionRE.exactMatch(line))
    {
        bool ok;
        version = d->versionRE.cap(1).toInt(&ok);
        return ok;
    }
    return false;
}


bool cartoMeshReader::canReadSection(const QByteArray & line, QString & title)
{
    if(d->sectionTitleRE.exactMatch(line))
    {
        title = d->sectionTitleRE.cap(1);
        qDebug() << "Title:" << title;
        return true;
    }
    return false;
}


bool cartoMeshReader::canReadKeyValues(const QByteArray & line, QString & key, QStringList & values)
{
    if(d->keyValueRE.exactMatch(line))
    {
        key = d->keyValueRE.cap(1);
        int pos = 0;
        while( (pos = d->valuesRE.indexIn(d->keyValueRE.cap(2), pos)) != -1 )
        {
            values.append(d->valuesRE.cap(1));
            pos += d->valuesRE.matchedLength();
        }
        return true;
    }
    return false;
}


bool cartoMeshReader::canSkipLine(const QByteArray & line)
{
    if (line.isEmpty() || d->skippableLineRE.exactMatch(line))
        return true;
    return false;
}


bool cartoMeshReader::processGeneralAttributes(const QString & key, const QStringList & values)
{
    if (values.isEmpty())
        return false;

    if(key == "MeshName")
        d->generalAttributes.meshName = values.first();

    else if (key == "NumVertex")
        d->generalAttributes.numVertex = values.first().toInt();

    else if (key == "NumTriangle")
        d->generalAttributes.numTriangles = values.first().toInt();

    else if (key == "NumVertexColors")
        d->generalAttributes.numVertexColors = values.first().toInt();

    else if (key == "ColorsNames")
        d->generalAttributes.colorsNames = values;

    else if (key == "TopologyStatus")
        d->generalAttributes.topologyStatus = values.first().toInt();

    else if (key == "MeshColor")
    {
        if (values.size() != 4)
        {
            qDebug("cartoMeshReader: invalid color for meshColor");
            return false;
        }
        d->generalAttributes.meshColor = QColor(values.at(0).toInt(), values.at(1).toInt(),
                                                values.at(2).toInt(), values.at(3).toInt());
    }
    else if (key == "Matrix")
    {
        if(values.size() != 16)
        {
            qDebug("cartoMeshReader: invalid matrix");
            return false;
        }
        int x = 0, y = 0;
        foreach(const QString & s, values)
        {
            d->generalAttributes.matrix(y, x) = s.toInt();
            x++;
            if (x % 4 == 0) {x = 0; y++;}
        }
    }
    return true;
}


bool cartoMeshReader::processVertice(const QString & key, const QStringList & values)
{
    if (values.size() < 3)
        return false;

    bool ok = true;
    double p[3];
    for(int i = 0; i < 3 && ok; i++)
        p[i] = values.at(i).toDouble(&ok);
    if (!ok)
        return false;

    unsigned int id = key.toUInt(&ok);
    if (!ok || id >= d->generalAttributes.numTriangles)
        return false;

    d->points->InsertPoint(id, p);
    return true;
}


bool cartoMeshReader::processTriangles(const QString & key, const QStringList & values)
{
    if (values.size() < 3)
        return false;

    bool ok = true;
    for(unsigned int i = 0; i < 3; i++)
    {
        unsigned int id = values.at(i).toUInt(&ok);
        if (!ok || id >= d->generalAttributes.numVertex)
            return false;
        d->idList->InsertId(i, id);
    }

    d->polyData->InsertNextCell(VTK_TRIANGLE, d->idList);
    return true;
}


bool cartoMeshReader::processVerticesColors(const QString & key, const QStringList & values)
{
    if ((unsigned int)values.size() < d->generalAttributes.numVertexColors)
        return false;

    bool ok = true;
    unsigned int id = key.toUInt(&ok);
    if (!ok || id >= d->generalAttributes.numVertex)
        return false;

    for(unsigned int i = 0; i < d->generalAttributes.numVertexColors; i++)
    {
        double val = values.at(i).toDouble(&ok);
        if (!ok)
            return false;

        d->scalarsArrays[i]->InsertValue(id, val);
    }
    return true;
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractDataReader * cartoMeshReader::create(void)
{
    return new cartoMeshReader;
}
