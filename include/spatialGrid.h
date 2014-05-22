#ifndef SPATIAL_GRID_H
#define SPATIAL_GRID_H

#include <UT/UT_Vector3.h>
#include <GU/GU_Detail.h>
#include <GEO/GEO_Point.h>
#include <GA/GA_AttributeRef.h>

#include <vector>
#include <thread>

#include "dataHandler.h"

typedef unsigned int uint;

class Volume
{
public:
    Volume(){}
    Volume(UT_Vector3 volume):_width(volume.x()), _height(volume.y()), _depth(volume.z()){}
    Volume(const float width, const float height, const float depth):
        _width(width), _height(height), _depth(depth){}

    float getWidth(){return _width;}
    float getHeight(){return _height;}
    float getDepth(){return _depth;}
    float getDimension(){return _width*_height*_depth;}

    void setWidth(const float width){_width = width;}
    void setHeight(const float height){_height = height;}
    void setDepth(const float depth){_depth = depth;}

private:
    float _width;
    float _height;
    float _depth;
};

struct Vertex
{
    Vertex(GEO_Vertex geo, uint index):geo(geo), index(index){}

    GEO_Vertex geo;
    uint index;
};

struct GridCell
{
public:
    GridCell(){}
    ~GridCell(){}

    Data<GEO_Point>& getPoints(){return _points;}
    void addPoint(GEO_Point point){_points.addDataValue(point);}
    bool isEmpty(){return (_points.getDataSize()==0);}
    void clear(){_points.clear();}
    uint getSize(){return _points.getDataSize();}

    int getNearestVertex(const uint pointIndex)
    {
        float minDelta = 1.0e3f;
        UT_Vector3 delta;

        int nearestVertexIndex = -1;
        for (uint i = 0; i < _vertices.getDataSize(); ++i)
        {
            delta = _points.getDataValue(pointIndex).getPos3() - _vertices.getDataValue(i).geo.getPos3();

            if (delta.length() < minDelta)
                nearestVertexIndex = _vertices.getDataValue(i).index;
        }

        return nearestVertexIndex;
    }

    Data<Vertex>& getVertices(){return _vertices;}
    void addVertex(Vertex vertex){_vertices.addDataValue(vertex);}

private:
    Data<GEO_Point> _points;
    Data<Vertex> _vertices;
};

typedef std::vector<GridCell> Cells;
//typedef std::vector<GA_RWAttributeRef> AttributeList;

struct Grid
{
public:
    Grid(){}
    ~Grid(){clear();}
    void initialize(uint dimension)
    {
        clear();
        for (uint i = 0; i < dimension; ++i)
            _cells.push_back(GridCell());
    }
    void clear(){_cells.clear();}

    //Data<GA_RWAttributeRef>& getAttributes(){return _attributes;}
    Cells& getCells(){return _cells;}
    GridCell& getCell(const uint index){return _cells.at(index);}
    uint getSize(){return _cells.size();}

private:
    //Data<GA_RWAttributeRef> _attributes;
    Cells _cells;
};

class SpatialGrid
{
public:
    SpatialGrid();
    SpatialGrid(Volume resolution, Volume domain);
    ~SpatialGrid();
    void initialize(Volume resolution, Volume domain);

    void addPoint(GEO_Point point);
    void addVertex(GEO_Vertex vertex, const uint index);
    GridCell& getCell(const uint cellIndex);

    uint getSize(){return _grid->getSize();}
    uint getPointSize(){return _pointSize;}

    Volume getResolution(){return _resolution;}
    Volume getDomain(){return _domain;}

    uint getCellIndex(const UT_Vector3 position);
    uint getCellIndex(const uint x, const uint y, const uint z);
    UT_Vector3 getCellPosition(const uint cellIndex);
    int getPointIndex(const uint cellIndex, const UT_Vector3 position);
    uint getUsedCellSize(){return _cellList.size();}
    std::vector<uint> getCellList(){return _cellList;}

    void applyNormalDisplacement(GU_Detail* outputVertices);

private:
    Volume _resolution;
    Volume _domain;
    uint _pointSize;
    uint _vertexSize;

    std::vector<uint> _cellList;
    std::shared_ptr<Grid> _grid;
};

#endif // SPATIAL_GRID_H
