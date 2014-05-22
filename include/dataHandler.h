#ifndef DATA_HANDLER_H
#define DATA_HANDLER_H

#include <vector>

template <class DataType>
class Data
{
public:
    Data() {clear();}
    ~Data() {clear();}

    void clear() {_dataValues.clear();}
    void addDataValue(DataType data) {_dataValues.push_back(data);}
    DataType& getDataValue(const unsigned int index) {return _dataValues.at(index);}
    unsigned int getDataSize() {return _dataValues.size();}

private:
    std::vector<DataType> _dataValues;
};

#endif // DATA_H
