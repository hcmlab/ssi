//
//  WrapPath.h
//  FastDTW-x
//
//  Created by Melo Yao on 12/5/13.
//  Copyright (c) 2013 melo.yao. All rights reserved.
//

#ifndef __FastDTW_x__WrapPath__
#define __FastDTW_x__WrapPath__

#include "Foundation.h"
#include <vector>
#include "ColMajorCell.h"
#include <iostream>

FD_NS_START
using namespace std;
class WarpPath
{
    vector<JInt> _tsIindexes;
    vector<JInt> _tsJindexes;
    
public:
    WarpPath(JInt initialCapacity);
    
    JInt size() const;
    
    JInt minI() const;
    
    JInt minJ() const;
    
    JInt maxI() const;
    
    JInt maxJ() const;
    
    void addFirst(JInt i, JInt j);
    
    void addLast(JInt i, JInt j);
    
    void getMatchingIndexesForI(JInt i,vector<JInt>& outVec) const;
    
    void getMatchingIndexesForJ(JInt j,vector<JInt>& outVec) const;
    
    void invertedCopy(WarpPath& path) const;
    
    void invert();
    
    ColMajorCell get(JInt index) const;
    
    bool operator==(const WarpPath& path) const;
    
    bool operator<(const WarpPath& path) const;
    
    void print(ostream& stream) const;

};

FD_NS_END
#endif /* defined(__FastDTW_x__WrapPath__) */
