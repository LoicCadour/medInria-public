/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "medToolBox.h"

#include "medGuiExport.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkMaskImageFilter.h"
#include "itkImageRegionIterator.h"
//#include "QuickView.h"

class dtkAbstractView;
class dtkAbstractData;
class medMaskApplicationToolBoxPrivate;
class medDataIndex;

typedef itk::Image<unsigned char, 2>  ImageType;

/**
  * @class medMaskApplicationToolBox
  * @author Pierre Fillard
  * @brief Toolbox to extract bundle from fibers and to compute statistics of those
  * This toolbox allows to perform fiber bundling. It must be used with subclasses
  * of @class medAbstractViewFiberInteractor. It allows to bundle fibers (i.e.,
  * declare groups of fibers as belonging to the same anatomical bundle), name
  * and color bundles, and compute and display FA, ADC and length statistics.
  */
class MEDGUI_EXPORT medMaskApplicationToolBox : public medToolBox
{
    Q_OBJECT
public:
     medMaskApplicationToolBox(QWidget *parent);
    ~medMaskApplicationToolBox();

    /**
      * Set input fibers as a dtkAbstractData object. Subclass should
      * inherit it and cass it into proper fiber data type.
      */
    virtual void setData(dtkAbstractData *data);

signals:
    /**
      * This signal is emitted when the user want to bundle fibers on screen.
      * @param name Name of the bundle
      * @param color Color of the bundle
      */
    void fiberSelectionValidated (const QString &name, const QColor &color);

    /**
      * This signal is emitted when the user wants to navigate into the
      * subset of visible fibers (usefull for recursive fiber bundling).
      */
    void fiberSelectionTagged();

    /**
      * This signal is emitted when the user wants to reset the fiber
      * navigation to the entire set of fibers.
      */
    void fiberSelectionReset();

    /**
      * This signal is emitted when the user wants to change the
      * boolean meaning of a ROI.
      * @param value Value of the ROI to be changed
      */
    void bundlingBoxBooleanOperatorChanged (int value);

    void CreateHalfMask(ImageType::Pointer image, ImageType::Pointer &mask);

protected slots:

    /**
      * When update() is called, this toolbox automatically searches for a subclass
      * of a medAbstractFiberViewInteractor and set its input to the interactor's
      * input. It adapts its GUI automatically (populate the list of bundles).
      */
    virtual void update (dtkAbstractView *view);

    /**
     * Clears the toolbox. Removes any bundle in the fiber bundle treeview,
     * any ROI previously loaded (if any), etc.
     */
    virtual void clear();

    /**
      * Add a bundle (identified by a name and a color) to the list of bundles.
      */
    virtual void addBundle (const QString &name, const QColor &color);

    /** Slot called when external ROI image finishes being imported. */
    virtual void onRoiImported(const medDataIndex &index);

    virtual void onImageImported(const medDataIndex &index);
    /**
     * Slot called when the @meDropSite is clicked.
     * Will open a @QFileDialog so the user can choose
     * and external ROI image to open.
     */
    virtual void onDropSiteClicked();

    /**
     * Sets the image passed as parameter as the @medDropSite image.
     */
    void setImage(const QImage& thumbnail);

    // internal method, doc to come
    virtual void onBundlingButtonVdtClicked();
    virtual void onBundlingButtonAndToggled (bool);
    virtual void onBundleBoxCheckBoxToggled (bool);
    virtual void onBundlingShowCheckBoxToggled (bool);

    virtual void onClearRoiButtonClicked();
    virtual void onRoiComboIndexChanged  (int value);

    virtual void onBundlingItemChanged (QStandardItem *item);

private:
    medMaskApplicationToolBoxPrivate *d;

};



