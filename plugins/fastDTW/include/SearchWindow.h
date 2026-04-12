//
//  SearchWindow.h
//  FastDTW-x
//
//  Created by Melo Yao on 12/4/13.
//  Copyright (c) 2013 melo.yao. All rights reserved.
//

#ifndef __FastDTW_x__SearchWindow__
#define __FastDTW_x__SearchWindow__

#include "Foundation.h"
#include <vector>
#include <algorithm>
#include "ColMajorCell.h"

FD_NS_START
class SearchWindow;

class SearchWindowIterator
{
    JInt _currentI;
    JInt _currentJ;
    const SearchWindow* _window;
    JBool _hasMoreElements;
    JInt _expectedModCount;
    
protected:
    SearchWindowIterator(const SearchWindow* w);
public:
    ~SearchWindowIterator();
    JBool hasNext() const;
    ColMajorCell next();
    
    friend class SearchWindow;
};

using namespace std;
class SearchWindow
{
    vector<JInt> _minValues;
    vector<JInt> _maxValues;
    JInt _maxJ;
    JInt _size;
    JInt _modCount;
    
    void expandSearchWindow(JInt radius);
public:
    SearchWindow(JInt tsIsize, JInt tsJsize);
    
    virtual ~SearchWindow();
    
    JBool isInWindow(JInt i,JInt j) const;
    
    JInt minI() const;
    
    JInt maxI() const;
    
    JInt minJForI(JInt i) const;

    JInt maxJForI(JInt i) const;
    
    JInt minJ() const;
    
    JInt maxJ() const;
    
    JInt size() const;
    
    JInt getModCount() const;
    
    SearchWindowIterator iterator() const;
    
protected:
    void expandWindow(JInt radius);
    
    void markVisited(JInt col, JInt row);
    
};

FD_NS_END
#endif /* defined(__FastDTW_x__SearchWindow__) */
