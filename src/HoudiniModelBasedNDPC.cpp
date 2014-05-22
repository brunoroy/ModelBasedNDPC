#include <UT/UT_Interrupt.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Director.h>
#include <GA/GA_Range.h>

#include "HoudiniModelBasedNDPC.h"
#include "timer.h"

void newSopOperator(OP_OperatorTable *table)
{
     table->addOperator(new OP_Operator("ModelBasedNDPC",
                        "ModelBasedNDPC",
                        HoudiniModelBasedNDPC::sopConstructor,
                        HoudiniModelBasedNDPC::templateList,
                        2,
                        2,
                        0));
}

static PRM_Range resolutionRange(PRM_RANGE_RESTRICTED, 8, PRM_RANGE_RESTRICTED, 128);
static PRM_Range domainRange(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_RESTRICTED, 20);

static PRM_Name controlNames[] = {
    PRM_Name("gridResolution", "Grid resolution"),
    PRM_Name("domainDimension", "Domain dimension")
};

static PRM_Default controlDefaultValues[] = {
    PRM_Default(8),
    PRM_Default(8),
    PRM_Default(8),
    PRM_Default(3),
    PRM_Default(5),
    PRM_Default(3)
};

PRM_Template HoudiniModelBasedNDPC::templateList[] = {
    PRM_Template(PRM_INT_J, 3, &controlNames[0], &controlDefaultValues[0], 0, &resolutionRange),
    PRM_Template(PRM_INT_J, 3, &controlNames[1], &controlDefaultValues[3], 0, &domainRange),
    PRM_Template()
};

OP_Node* HoudiniModelBasedNDPC::sopConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new HoudiniModelBasedNDPC(net, name, op);
}

HoudiniModelBasedNDPC::HoudiniModelBasedNDPC(OP_Network *net, const char *name, OP_Operator *op) :
    SOP_Node(net, name, op)
{
    _spatialGrid.reset(new SpatialGrid());
}

HoudiniModelBasedNDPC::~HoudiniModelBasedNDPC()
{
}

OP_ERROR HoudiniModelBasedNDPC::cookMySop(OP_Context &context)
{
    fpreal now = context.getTime();

    if (lockInputs(context) >= UT_ERROR_ABORT)
        return error();

    _currentFrame = context.getFrame()-1;
    _gridResolution = evalIntVector(controlNames[0].getToken(), now);
    _domainDimension = evalIntVector(controlNames[1].getToken(), now);

    int inputError;
    GU_Detail* inputVertices = (GU_Detail*)inputGeo(0, context);
    GU_Detail* inputPoints = (GU_Detail*)inputGeo(1, context);
    if (inputPoints->points().entries() > 0)
        inputError = process(inputVertices, inputPoints);

    if (inputError == 0)
        return error();

    unlockInputs();
    return error();
}

const char* HoudiniModelBasedNDPC::inputLabel(unsigned int inputIndex) const
{
    switch (inputIndex)
    {
        case 0: return "Input mesh"; break;
        case 1: return "Input point cloud"; break;
    }
}

unsigned int HoudiniModelBasedNDPC::disableParms()
{
    unsigned int changed = 0;

    return changed;
}

UT_Vector3i HoudiniModelBasedNDPC::evalIntVector(const char *pn, fpreal t)
{
    int v[3];
    for (int i = 0; i < 3; ++i)
        v[i] = evalInt(pn, i, t);

    return UT_Vector3i(v[0], v[1], v[2]);
}

uint HoudiniModelBasedNDPC::process(GU_Detail* inputVertices, GU_Detail* inputPoints)
{
    Timer spatialGridTimer(true);
    _spatialGrid->initialize(Volume(_gridResolution), Volume(_domainDimension));

    GEO_Point point = GEO_Point(inputPoints->getPointMap(), GA_INVALID_OFFSET);
    for (uint i = 0; i < inputPoints->points().entries(); ++i)
    {
        point = GEO_Point(inputPoints->getPointMap(), inputPoints->pointOffset(i));
        _spatialGrid->addPoint(point);
    }

    //GEO_Point vertex = GEO_Point(inputVertices->getPointMap(), GA_INVALID_OFFSET);
    //inputVertices->get
    GEO_Vertex vertex = GEO_Vertex(inputVertices->getVertexMap(), GA_INVALID_OFFSET);
    for (uint i = 0; i < inputVertices->points().entries(); ++i)
    {
        vertex = GEO_Vertex(inputVertices->getVertexMap(), inputVertices->vertexOffset(i));
        _spatialGrid->addVertex(vertex, i);
    }

    auto elapsed = spatialGridTimer.elapsed();
    std::cout << "spatial grid: " << std::fixed << elapsed.count() << " ms." << std::endl;

    Timer displacementTimer(true);
    gdp->copy(*inputVertices);
    _spatialGrid->applyNormalDisplacement(gdp);
    elapsed = displacementTimer.elapsed();
    std::cout << "displacement: " << std::fixed << elapsed.count() << " ms." << std::endl;


    /*Timer spatialGridTimer(true);
    AttributeList attributeList;
    const GA_AttributeDict& inputAttributes = inputPoints->getAttributeDict(GA_ATTRIB_POINT);
    for (GA_AttributeDict::iterator it = inputAttributes.begin(GA_SCOPE_PUBLIC); !it.atEnd(); ++it)
    {
        GA_Attribute *attribute = it.attrib();
        attributeList.push_back(gdp->addFloatTuple(GA_ATTRIB_POINT, attribute->getName(), attribute->getTupleSize()));
    }
    _spatialGrid->initialize(Volume(_gridResolution), Volume(_domainDimension), _patchResolution, attributeList);

    GEO_Point point = GEO_Point(inputPoints->getPointMap(), GA_INVALID_OFFSET);
    for (int i = 0; i < inputPoints->points().entries(); ++i)
    {
        point = GEO_Point(inputPoints->getPointMap(), inputPoints->pointOffset(i));
        _spatialGrid->addPoint(point);
    }
    auto elapsed = spatialGridTimer.elapsed();
    std::cout << "spatial grid: " << std::fixed << elapsed.count() << " ms." << std::endl;

    Timer initializePatchesTimer(true);
    _spatialGrid->computePatchSamples();
    elapsed = initializePatchesTimer.elapsed();
    std::cout << "patches: " << std::fixed << elapsed.count() << " ms." << std::endl;

    Timer computeGeometryTimer(true);
    uint computeError = computeGeometry();
    elapsed = computeGeometryTimer.elapsed();
    std::cout << "compute geometry: " << std::fixed << elapsed.count() << " ms." << std::endl;

    return computeError;*/

    //std::cout << "used cells: " << _spatialGrid->getUsedCellSize() << std::endl;

    //_spatialGrid->getCellCenter(0);

    /*std::clog << "points added: " << _spatialGrid->getPointSize() << std::endl;

    for (int i = 0; i < _spatialGrid->getSize(); ++i)
    {
        if (!_spatialGrid->getCell(i).isEmpty())
            std::clog << "cell[" << i << "]: " << _spatialGrid->getCell(i).getSize() << std::endl;
    }*/
}

uint HoudiniModelBasedNDPC::computeGeometry()
{
    /*for (int i = 0; i < _spatialGrid->getCellList().size(); ++i)
    {
        uint cellIndex = _spatialGrid->getCellList().at(i);
        GridCell& cell = _spatialGrid->getCell(cellIndex);
        Data<uint>& markerIds = cell.getMarkerIds();

        //std::clog << "markerIds: " << markerIds.getDataSize() << std::endl;

        for (int j = 0; j < markerIds.getDataSize(); ++j)
        {
            uint markerId = markerIds.getDataValue(j);
            //std::clog << "pointIndex: " << markerId << std::endl;
            GEO_Point marker = cell.getPoints().getDataValue(markerId);
            GEO_Point* point = gdp->appendPoint();
            point->setPos3(marker.getPos3());
        }
    }*/

    return gdp->points().entries();
}
