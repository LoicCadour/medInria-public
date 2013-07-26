/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "medSegmentationAbstractToolBox.h"

#include "msegPluginExport.h"

#include "medProcessPaintSegm.h"

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medDataIndex.h>
#include <medViewEventFilter.h>
#include <medImageMaskAnnotationData.h>

#include <QVector3D>
#include <QTextEdit>

#include <vector>

class medAbstractData;
class medAbstractView;
class medAnnotationData;

class dtkAbstractProcessFactory;
class medSeedPointAnnotationData;

namespace mseg {
    class ClickAndMoveEventFilter;

struct PaintState {
    enum E{ None, Wand, Stroke, DeleteStroke };
};

    struct PaintState {
        enum E{ None, Wand, Stroke, DeleteStroke, BoundaryStroke };
    };

//! Segmentation toolbox to allow manual painting of pixels
class MEDVIEWSEGMENTATIONPLUGIN_EXPORT AlgorithmPaintToolbox : public medSegmentationAbstractToolBox
{
    Q_OBJECT;
public:

    typedef itk::Image<unsigned char, 3> MaskType;
    typedef QPair<MaskType::IndexType,unsigned char> pair;
    typedef QList<pair> list_pair;

    AlgorithmPaintToolbox( QWidget *parent );
    ~AlgorithmPaintToolbox();

     //! Override dtkAbstractObject
    QString description() const { return s_description(); }
    QString identifier() const { return s_identifier(); }

    static medSegmentationAbstractToolBox * createInstance( QWidget *parent );

    static QString s_description();

    /** Get name to use for this when registering with a factory.*/
    static QString s_identifier();

    //! Get a human readable name for this widget.
    /** \param trObj : Provide an object for the tr() function. If NULL qApp will be used. */
    static QString s_name(const QObject * trObj =  NULL);

    inline void forcePaintState(PaintState::E state){m_paintState = state;}
    inline PaintState::E paintState(){return m_paintState;}

    inline void setPaintState( PaintState::E value){m_paintState = value;}
    inline PaintState::E paintState(){return m_paintState;}
    void setCurrentView(medAbstractView * view);

    bool getSeedPlanted();
    void setSeedPlanted(bool,MaskType::IndexType,unsigned int,double);
    void setSeed(QVector3D);

public slots:
    void onStrokeToggled(bool);
    void onMagicWandToggled(bool);

    void onApplyButtonClicked();
    void onClearMaskClicked();

    void onLabelChanged(int newVal);
    void onSelectLabelColor();

    void synchronizeWandSpinBoxesAndSliders(void);
    
    void updateStroke(ClickAndMoveEventFilter * filter, medAbstractView * view);
    void updateWandRegion(medAbstractView * view, QVector3D &vec);
    void updateMouseInteraction();

    void onUndo();
    void onRedo();
    void addToStackIndex(medAbstractView * view);

    void wheelEvent(QWheelEvent * event);
    void onAcceptGrowth();
    void onRemoveSeed();

    void copySliceMask();
    void pasteSliceMask();

protected:
    friend class ClickAndMoveEventFilter;
    friend class ClickEventFilter;

    void addStroke( medAbstractView *view, const QVector3D &vec );
    void setData( dtkAbstractData *data );

    // update with seed point data.
    void updateTableRow(int row);

    void initializeMaskData( medAbstractData * imageData, medAbstractData * maskData );

    void updateFromGuiItems();

    void showButtons( bool value);

    void generateLabelColorMap(unsigned int numLabels);

    void updateButtons();
    void addBrushSize(int size);

    char computePlaneIndex(const QVector3D &,MaskType::IndexType & ,bool& isInside);

    void copySliceFromMask3D(itk::Image<unsigned char,2>::Pointer copy,const char planeIndex,const char * direction,const int slice);
    void pasteSliceToMask3D(itk::Image<unsigned char,2>::Pointer image2D,const char planeIndex,const char * direction,const int slice);

private:
    typedef dtkSmartPointer<medSeedPointAnnotationData> SeedPoint;

    QPushButton *m_strokeButton;
    QPushButton *m_labelColorWidget;
    QSpinBox *m_strokeLabelSpinBox;
    QPushButton * m_acceptGrowthButton;
    QPushButton * m_removeSeedButton;
    QShortcut *undo_shortcut, *redo_shortcut, *copy_shortcut, *paste_shortcut, *acceptGrowth_shortcut, *removeSeed_shortcut;
    
    QLabel *m_colorLabel;
    QLabel * m_wandInfo;

    QSlider *m_brushSizeSlider;
    QSpinBox *m_brushSizeSpinBox;
    QLabel *m_brushRadiusLabel;

    QFormLayout * magicWandLayout;
    QPushButton *m_magicWandButton;
    QSlider *m_wandUpperThresholdSlider, *m_wandLowerThresholdSlider;
    QDoubleSpinBox *m_wandUpperThresholdSpinBox , * m_wandLowerThresholdSpinBox;
    QCheckBox *m_wand3DCheckbox;

    bool seedPlanted;
    QVector3D m_seed;

    double m_MinValueImage;
    double m_MaxValueImage;

    QPushButton *m_applyButton;

    QPushButton *m_clearMaskButton;

    dtkSmartPointer< medViewEventFilter > m_viewFilter;

    dtkSmartPointer<medImageMaskAnnotationData> m_maskAnnotationData;

    dtkSmartPointer<medAbstractData> m_maskData;
    dtkSmartPointer<medAbstractData> m_imageData;
    
    medImageMaskAnnotationData::ColorMapType m_labelColorMap;
    
    MaskType::Pointer m_itkMask;
    QPair<itk::Image<unsigned char,2>::Pointer,char> m_copy;

    // undo_redo_feature's attributes
    list_pair * listIndexPixel;
    QHash<medAbstractView*,QStack<list_pair*>*> * undoStacks, * redoStacks;
    medAbstractView * currentView;

    template <typename IMAGE> void RunConnectedFilter (MaskType::IndexType &index, unsigned int planeIndex);
    template <typename IMAGE> void GenerateMinMaxValuesFromImage ();

    QVector3D m_lastVup;
    QVector3D m_lastVpn;
    double m_sampleSpacing[2];

    double m_wandRadius, m_wandUpperThreshold, m_wandLowerThreshold;
    double m_strokeRadius;
    unsigned int m_strokeLabel;

    PaintState::E m_paintState;
};

} // namespace mseg


