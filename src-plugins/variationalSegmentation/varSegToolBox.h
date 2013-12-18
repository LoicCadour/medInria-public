#ifndef VarSegToolBox_H
#define VarSegToolBox_H

#include "medSegmentationAbstractToolBox.h"
#include <medAbstractView.h>
#include <vtkImageView2D.h>
#include <vtkImageView3D.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkSmartPointer.h>
#include <medWorkspace.h>

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
class VarSegToolBox : public medSegmentationAbstractToolBox
{
    Q_OBJECT
public:

    VarSegToolBox( QWidget *parent );
    virtual ~VarSegToolBox();

     //! Override dtkAbstractObject
    QString description() const { return s_description(); }
    QString identifier() const { return s_identifier(); }

    static medSegmentationAbstractToolBox * createInstance( QWidget *parent );

    static QString s_description();
    static QString s_identifier();
    static QString s_name();

public slots:
    virtual void update(dtkAbstractView * view);
    void updateLandmarksRenderer(QString key, QString value);
    void addBinaryImage();
    void applyMaskToImage();
    void startSegmentation();
    void endSegmentation();
    void segmentation(bool);
    void bringBackOriginalImage();
    void moveToMPRmode(bool);

private:
    vtkLandmarkSegmentationController* controller;
    medAbstractView * currentView;
    medAbstractView * originalView;
    QList<medAbstractView*> medViews;
    QList<vtkImageView2D*> * views2D;
    QList<vtkImageView3D*> * views3D;
    QPushButton * segButton;
    QPushButton * clearChanges;
    QPushButton * binaryImageButton;
    QPushButton * applyMaskButton;
    QPushButton * mprMode;
    dtkSmartPointer<dtkAbstractData> originalInput;
    dtkSmartPointer<dtkAbstractData>  output;
    int inputSize[3];
    bool segOn;
    bool mprOn;
    medWorkspace * workspace;
};

}

#endif // VarSegToolBox_H
