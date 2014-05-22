#include "spatialGrid.h"

SpatialGrid::SpatialGrid()
{
}

SpatialGrid::SpatialGrid(Volume resolution, Volume domain)
{
    initialize(resolution, domain);
}

SpatialGrid::~SpatialGrid()
{
    _grid->clear();
}

void SpatialGrid::initialize(Volume resolution, Volume domain)
{
    uint dimension = static_cast<uint>(resolution.getDimension());

    _cellList.clear();
    _grid.reset(new Grid());
    _grid->initialize(dimension);

    _resolution = resolution;
    _domain = domain;
    _pointSize = 0;
    _vertexSize = 0;
}

void SpatialGrid::addPoint(GEO_Point point)
{    
    uint cellIndex = getCellIndex(point.getPos3());

    bool cellInList = false;
    for (uint i = 0; i < _cellList.size(); ++i)
    {
        if (_cellList.at(i) == cellIndex)
        {
            cellInList = true;
            break;
        }
    }

    if (!cellInList)
        _cellList.push_back(cellIndex);

    _grid->getCell(cellIndex).addPoint(point);
    _pointSize++;

    //std::clog << _pointSize << " points added." << std::endl;
}

void SpatialGrid::addVertex(GEO_Vertex vertex, const uint index)
{
    uint cellIndex = getCellIndex(vertex.getPos3());

    _grid->getCell(cellIndex).addVertex(Vertex(vertex, index));
    _vertexSize++;
    //std::clog << _pointSize << " points added." << std::endl;
}

GridCell& SpatialGrid::getCell(const uint cellIndex)
{
    return _grid->getCell(cellIndex);
}

uint SpatialGrid::getCellIndex(const UT_Vector3 position)
{
    Volume cellVolume;
    cellVolume.setWidth(_domain.getWidth()/_resolution.getWidth());
    cellVolume.setHeight(_domain.getHeight()/_resolution.getHeight());
    cellVolume.setDepth(_domain.getDepth()/_resolution.getDepth());

    uint gridWidth = static_cast<uint>(_resolution.getWidth());
    uint gridHeight = static_cast<uint>(_resolution.getHeight());
    uint gridDepth = static_cast<uint>(_resolution.getDepth());

    uint xCellIndex = static_cast<uint>((position.x()+(_domain.getWidth()/2.0f))/cellVolume.getWidth());
    uint yCellIndex = static_cast<uint>(position.y()/cellVolume.getHeight());
    uint zCellIndex = static_cast<uint>((position.z()+(_domain.getDepth()/2.0f))/cellVolume.getDepth());

    if (xCellIndex == gridWidth)
        xCellIndex = gridWidth-1;
    if (yCellIndex == gridHeight)
        yCellIndex = gridHeight-1;
    if (zCellIndex == gridDepth)
        zCellIndex = gridDepth-1;

    uint cellIndex = xCellIndex + yCellIndex * (gridWidth*gridDepth) + (zCellIndex*gridDepth);

    return cellIndex;
}

uint SpatialGrid::getCellIndex(const uint x, const uint y, const uint z)
{
    uint gridWidth = static_cast<uint>(_resolution.getWidth());
    uint gridDepth = static_cast<uint>(_resolution.getDepth());

    return x + y * (gridWidth*gridDepth) + (z*gridDepth);
}

UT_Vector3 SpatialGrid::getCellPosition(const uint cellIndex)
{
    uint dimXZ = static_cast<uint>(_resolution.getWidth()*_resolution.getDepth());

    uint cellY = static_cast<uint>(std::floor((float)cellIndex / (_resolution.getWidth()*_resolution.getDepth())));
    uint cellX = (cellIndex-(cellY*dimXZ)) % static_cast<uint>(_resolution.getWidth());
    uint cellZ = static_cast<uint>(std::floor(static_cast<float>(cellIndex-(cellY*dimXZ)) / _resolution.getWidth()));

    UT_Vector3 cellPosition(static_cast<float>(cellX), static_cast<float>(cellY), static_cast<float>(cellZ));

    return cellPosition;
}

int SpatialGrid::getPointIndex(const uint cellIndex, const UT_Vector3 position)
{
    for (uint i = 0; i < _grid->getCell(cellIndex).getPoints().getDataSize(); ++i)
    {
        GEO_Point point = _grid->getCell(cellIndex).getPoints().getDataValue(i);
        if (point.getPos3() == position)
            return i;
    }

    return -1;
}

void SpatialGrid::applyNormalDisplacement(GU_Detail* outputVertices)
{
    //outputPoints->clearAndDestroy();

    for (uint c = 0; c < getUsedCellSize(); ++c)
    {
        GridCell& cell = _grid->getCell(_cellList.at(c));

        /*if (!cell.isEmpty())
        {*/
            Data<GEO_Point>& points = cell.getPoints();
            //Data<GEO_Vertex>& vertices = cell.getVertices();

            for (uint i = 0; i < points.getDataSize(); ++i)
            {
                int nearestVertex = cell.getNearestVertex(i);
                if (nearestVertex != -1)
                {
                    GEO_Point point = points.getDataValue(i);
                    //GEO_Vertex vertex = vertices.getDataValue(nearestVertex);
                    GEO_Vertex vertex(outputVertices->getVertexMap(), outputVertices->vertexOffset(nearestVertex));

                    float delta = (vertex.getPos3() - point.getPos3()).length();
                    //std::clog << "delta: " << delta << std::endl;
                    if (delta > 0.02f)
                    {
                        vertex.setPos3(point.getPos3());
                        //GEO_Vertex outputVertex(outputVertices->getVertexMap(), outputVertices->vertexOffset())
                        //GEO_Point* newPoint = outputPoints->appendPoint();
                        //newPoint->setPos3(point.getPos3());
                    }
                }

                //std::clog << "displacement: " << (point.getPos3() - vertex.getPos3()) << std::endl;
            }
        //}
        //GEO_Point point = _grid->getCell(c).getPoints().getDataValue(c);
    }
}
