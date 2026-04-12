//
//  ColMajorCell.h
//  FastDTW-x
//
//  Created by Melo Yao on 12/4/13.
//  Copyright (c) 2013 melo.yao. All rights reserved.
//

#ifndef __FastDTW_x__ColMajorCell__
#define __FastDTW_x__ColMajorCell__

#include "Foundation.h"

FD_NS_START

class ColMajorCell
{
    JInt _col;
    JInt _row;
    
public:
    ColMajorCell();
    
    ColMajorCell(JInt col, JInt row);
    
    ColMajorCell(const ColMajorCell& cell);
    
    JInt getCol() const;
    
    JInt getRow() const;
    
    bool operator== (ColMajorCell const& cell) const;
    
    bool operator< (ColMajorCell const& cell) const;
};

FD_NS_END
#endif /* defined(__FastDTW_x__ColMajorCell__) */
