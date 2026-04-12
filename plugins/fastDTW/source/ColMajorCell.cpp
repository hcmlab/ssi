//
//  ColMajorCell.cpp
//  FastDTW-x
//
//  Created by Melo Yao on 12/4/13.
//  Copyright (c) 2013 melo.yao. All rights reserved.
//

#include "ColMajorCell.h"
FD_NS_START

ColMajorCell::ColMajorCell():_col(0),_row(0)
{
}

ColMajorCell::ColMajorCell(JInt col, JInt row):_col(col),_row(row)
{
    
}

ColMajorCell::ColMajorCell(const ColMajorCell& cell):_col(cell._col),_row(cell._row)
{
}

JInt ColMajorCell::getCol() const
{
    return _col;
}

JInt ColMajorCell::getRow() const
{
    return _row;
}

bool ColMajorCell::operator== (ColMajorCell const& cell) const
{
    return _col == cell.getCol()&&_row == cell.getRow();
}

bool ColMajorCell::operator< (ColMajorCell const& cell) const
{
    return (getCol()*1024 + getRow()) < (cell.getCol()*1024 + cell.getRow());
}

FD_NS_END