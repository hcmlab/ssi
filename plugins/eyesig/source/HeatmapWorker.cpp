#include "HeatmapWorker.h"
#include "math.h"
#include <iostream>


namespace ssi {



	HeatmapWorker::HeatmapWorker(	
						int width, 
						int height, 
						bool flip, 
						float influenceradius,  
						float minimumDisplayPercentage, 
						int horizontalRegions, 
						int verticalRegions, 
						bool decreaseOverTime, 
						float decreaseTick,
						float decreaseFactor,
						bool smoothen, 
						int filterRepetitions
						)
	{
		_width = width;
		_height = height;
		_imageFlipped = flip;
		_influenceRadius = influenceradius;
		_minimumDisplaypercentage = minimumDisplayPercentage;
		_horizontalRegions = horizontalRegions;
		_verticalRegions = verticalRegions;
		_smoothen = smoothen;
		_filterRepetitions = filterRepetitions;
		_decreaseOverTime = decreaseOverTime;
		_decreaseTick = decreaseTick;
		_decreaseFactor = decreaseFactor;

		_gazeMaxCount = 0;
		_gazeTotalCount = 0;
		_gazeAvgCount = 0;

		//allocating memory for the grid
		_densityGrid = new int*[_width];
		_densityGrid[0] = new int[ width * height]; 
		for (int i = 1; i < width; ++i)
			_densityGrid[i] = _densityGrid[0] + i * height;

		//initializing with zeros
		for(int i = 0; i < width; i++) {
			for(int j = 0; j < height; j++) {
				_densityGrid[i][j] = 0;
			}
		}

		_colorGrid = new BYTE*[width];
		_colorGrid[0] = new BYTE[ width * height * 3];
		//allocating memory for the color grid
		for (int i = 1; i < width; ++i) {
			_colorGrid[i] = _colorGrid[0] + i * height * 3;
		}

		//initializing with zeros
		for(int i = 0; i < width; i++) {
			for(int j = 0; j < height * 3; j+=3) {
				_colorGrid[i][j] = 155;
				_colorGrid[i][j+1] = 0;
				_colorGrid[i][j+2] = 0;
			}
		}

		float _boxFilterKernelTmp[][3] = { 
			{1.0f,1.0f,1.0f}, 
			{1.0f,2.0f,1.0f}, 
			{1.0f,1.0f,1.0f}
		}; 

		memcpy(_boxFilterKernel, _boxFilterKernelTmp, 3*3*sizeof(float));

		if(_decreaseOverTime) {
			_regionTimer = new DWORD[_horizontalRegions*_verticalRegions];
			for(int i = 0; i < _horizontalRegions*_verticalRegions; i++) {
				_regionTimer[i] = ::timeGetTime ();
			}
		}

	}


	HeatmapWorker::~HeatmapWorker(void)
	{ 
		delete[] _densityGrid[0];
		delete[] _densityGrid;
		delete[] _colorGrid[0];
		delete[] _colorGrid;
		delete[] _regionTimer;
	}


	void HeatmapWorker::run() {

		if(_decreaseOverTime) {
			for(int i = 0; i < _horizontalRegions*_verticalRegions; i++) {
				DWORD startTime =  ::timeGetTime () / 1000;
				DWORD endTime =  _regionTimer[i] / 1000;
				double elapsedTimeInSeconds = ((::timeGetTime () - _regionTimer[i])) / 1000;
				if(elapsedTimeInSeconds >= _decreaseTick) {
					decreaseArea(i);
					_regionTimer[i] = ::timeGetTime ();
				}
			}
		}

		for(int i = 0; i < _width; i++) {
			for(int j = 0; j < _height; j++) {
				setColorForPointFromDensity(i, j);
			}
		}
		/*for(int k = 0; k < 3; k++)
		boxFiltering();*/
	}

	void HeatmapWorker::enter() {
	}

	void HeatmapWorker::flush() {
	}


	void HeatmapWorker::increeaseGazePointCount(int x, int y) {
		//increasing every pixelhitcount in the influence area by one 
		y = _imageFlipped ? y : _height - y;
		if(x >= 0 && x <= _width && y >= 0 && y <= _height) {
			std::pair<int,int> coords;
			//density_counter[point_x-1][point_y-1]++;
			for(int yr =-_influenceRadius; yr<=_influenceRadius; yr++) {
				for(int xr=-_influenceRadius; xr<=_influenceRadius; xr++) {
					if(xr*xr+yr*yr <= _influenceRadius*_influenceRadius && (x-1+xr) >= 0 && (x-1+xr) < _width && (y-1+yr) >= 0 && (y-1+yr) < _height) {
						//calculating the decrease of influence according to the distance from the center
						float euklidDist = sqrt((float)(xr*xr+yr*yr));
						_densityGrid[x-1+xr][y-1+yr] += _influenceRadius - euklidDist; 
						_gazeMaxCount = max(_densityGrid[x-1+xr][y-1+yr], _gazeMaxCount);
						_gazeTotalCount++;
						if(_decreaseOverTime)
							_regionTimer[getRegionNumber(x-1+xr, y-1+yr)] = ::timeGetTime ();
					}
				}
			}
		}
	}


	void HeatmapWorker::setColorValueForPoint(int red, int green, int blue, int x, int y) {

		Lock lock(_colorGridMutex);

		_colorGrid[x][y*3] = red;
		_colorGrid[x][y*3+1] = green;
		_colorGrid[x][y*3+2] = blue;
	}

	int HeatmapWorker::getColorValueForPoint(int color, int x, int y) {
		switch(color) {
		case col::RED: 
			return _colorGrid[x][y*3] ;
		case col::GREEN:
			return _colorGrid[x][y*3+1];
		case col::BLUE:
			return _colorGrid[x][y*3+2];
		}
	}

	void HeatmapWorker::setColorForPointFromDensity(int x, int y) {
		float red = 1, green = 1, blue = 1;
		float value = (float)_densityGrid[x][y]/_gazeMaxCount; // _gazeAvgCount;//
		if(value >= _minimumDisplaypercentage) 
			getHeatMapColor(value, &red, &green, &blue);

		setColorValueForPoint(red*255, green*255, blue*255, x, y);
	}

	void HeatmapWorker::getHeatMapColor(float value, float *red, float *green, float *blue)
	{
		const int NUM_COLORS = 7;
		static float color[NUM_COLORS][3] = {{1,1,1}, {0,0,1}, {0,1,1}, {0,1,0}, {1,1,0}, {1, 0.5, 0} ,{1,0,0} };
		// A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.

		int idx1;        // |-- Our desired color will be between these two indexes in "color".
		int idx2;        // |
		float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

		if(value <= 0)      {  idx1 = idx2 = 0;            }    // accounts for an input <=0
		else if(value >= 1)  {  idx1 = idx2 = NUM_COLORS-1; }    // accounts for an input >=0
		else
		{
			value = value * (NUM_COLORS-1);        // Will multiply value by 3.
			idx1  = floor(value);                  // Our desired color will be after this index.
			idx2  = idx1+1;                        // ... and before this index (inclusive).
			fractBetween = value - float(idx1);    // Distance between the two indexes (0-1).
		}

		*red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
		*green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
		*blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
	}


	void HeatmapWorker::boxFiltering() {

		std::vector<int> temp (_width * _height);
		float denominator = 0.0f ;
		float red, green, blue ;
		int ired, igreen, iblue, xNeighbour, yNeighbour, redTmp, greenTmp,blueTmp ;

		for (int i=0;i< 3;i++)
			for(int j = 0; j < 3; j++)
				denominator += _boxFilterKernel[i][j] ;
		if (denominator==0.0f) denominator = 1.0f ;
		//iterating through every imagepoint
		for (int i=0;i<_width;i++) {
			for (int j=0;j<_height;j++) {		

				red = green = blue = 0.0f ;

				//iterating over neighbours
				for(int xOffset = 0; xOffset <= 2; xOffset++) {
					for(int yOffset = 0; yOffset <= 2; yOffset++) {
						xNeighbour = i + (-1 + xOffset);
						yNeighbour = j + (-1 + yOffset);
						if(xNeighbour >= 0 && xNeighbour < _width && yNeighbour >= 0 && yNeighbour < _height) {
							redTmp =  getColorValueForPoint(col::RED, xNeighbour, yNeighbour) * _boxFilterKernel[xOffset][yOffset];
							greenTmp = getColorValueForPoint(col::GREEN, xNeighbour, yNeighbour) *  _boxFilterKernel[xOffset][yOffset];
							blueTmp = getColorValueForPoint(col::BLUE, xNeighbour, yNeighbour) * _boxFilterKernel[xOffset][yOffset];
							red += redTmp;
							green += greenTmp;
							blue += blueTmp;

						}
					}
				}

				ired = (int)(red / denominator) ;
				igreen = (int)(green / denominator) ; 
				iblue = (int)(blue / denominator) ;
				setColorValueForPoint(ired, igreen, iblue, i, j);
			}
		}
	}

	int HeatmapWorker::getRegionNumber(int x, int y) {
		int horizontal = (x*_horizontalRegions)/_width;
		int vertical = (y*_verticalRegions)/_height;
		return horizontal * _verticalRegions + vertical;
	}

	void HeatmapWorker::decreaseArea(int area) {
		int xStart, xEnd, yStart, yEnd;
		xStart = (area % _horizontalRegions) * (_width / _horizontalRegions);
		xEnd = xStart + _width / _horizontalRegions;
		yStart = (area / _horizontalRegions)   * (_height / _verticalRegions);
		yEnd = yStart + _height / _verticalRegions;
		bool updateNeeded = false;
		for(int i = xStart; i < xEnd; i++) {
			for(int j = yStart; j < yEnd; j++) {
				if(_densityGrid[i][j] == _gazeMaxCount)
					updateNeeded = true;
				_densityGrid[i][j]  = _densityGrid[i][j] * (1-_decreaseFactor);
			}
		}
		if(updateNeeded)
			updateMax();
	}

	void HeatmapWorker::updateMax() {
		for(int i = 0; i < _width; i++)
			for(int j = 0; j < _height; j++)
				if(_densityGrid[i][j] > _gazeMaxCount)
					_gazeMaxCount = _densityGrid[i][j];
	}	


}