#ifndef VarSegToolBox_H
#define VarSegToolBox_H

#include "medToolBoxSegmentationCustom.h"

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkSmartPointer.h>

#include <QWidget>

class medAbstractData;
class medAbstractView;
class medAnnotationData;
class vtkLandmarkSegmentationController;

class dtkAbstractProcessFactory;
namespace mseg {
//    class ClickAndMoveEventFilter;
//    class ClickEventFilter;
//    class SelectDataEventFilter;

//! Segmentation toolbox to allow manual painting of pixels
class VarSegToolBox : public medToolBoxSegmentationCustom
{
    Q_OBJECT
public:

    VarSegToolBox( QWidget *parent );
    virtual ~VarSegToolBox();

     //! Override dtkAbstractObject
    QString description() const { return s_description(); }
    QString identifier() const { return s_identifier(); }

    static medToolBoxSegmentationCustom * createInstance( QWidget *parent );

    static QString s_description();
    static QString s_identifier();
    static QString s_name();

public slots:
    virtual void update(dtkAbstractView * view);
    void updateLandmarksRenderer(QString key, QString value);

private:
    vtkLandmarkSegmentationController* controller;

};

}

#endif // VarSegToolBox_H
