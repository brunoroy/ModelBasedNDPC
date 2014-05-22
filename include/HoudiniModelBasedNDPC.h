#ifndef HOUDINI_MODEL_BASED_NDPC_H
#define HOUDINI_MODEL_BASED_NDPC_H

#include <SOP/SOP_Node.h>

#include "spatialGrid.h"

class HoudiniModelBasedNDPC : public SOP_Node
{   
public:
    HoudiniModelBasedNDPC(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~HoudiniModelBasedNDPC();

    static PRM_Template templateList[];
    static OP_Node *sopConstructor(OP_Network *, const char *, OP_Operator *);
    unsigned int disableParms();

protected:
    virtual const char* inputLabel(unsigned int inputIndex) const;
    UT_Vector3i evalIntVector(const char *pn, fpreal t);
    virtual OP_ERROR cookMySop(OP_Context &context);

private:
    long _currentFrame;
    UT_Vector3i _gridResolution;
    UT_Vector3i _domainDimension;

    std::shared_ptr<SpatialGrid> _spatialGrid;

    uint process(GU_Detail* inputVertices, GU_Detail* inputPoints);
    uint computeGeometry();
};

#endif // HOUDINI_MODEL_BASED_NDPC_H
