// /////////////////////////////////////////////////////////////////
// Generated by medPluginGenerator
// /////////////////////////////////////////////////////////////////

#ifndef VARIATIONALSEGMENTATION_H
#define VARIATIONALSEGMENTATION_H

#include <dtkCore/dtkAbstractProcess.h>

#include "variationalSegmentationPluginExport.h"

class variationalSegmentationPrivate;
    
class VARIATIONALSEGMENTATIONPLUGIN_EXPORT variationalSegmentation : public dtkAbstractProcess
{
    Q_OBJECT
    
public:
    variationalSegmentation(void);
    virtual ~variationalSegmentation(void);
    
    virtual QString description(void) const;
    
    static bool registered(void);
    
private:
    variationalSegmentationPrivate *d;
};

dtkAbstractProcess *createVariationalSegmentation(void);

#endif
