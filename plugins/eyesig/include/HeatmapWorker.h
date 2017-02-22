#pragma once

#ifndef SSI_HEATMAP_WORKER
#define SSI_HEATMAP_WORKER

#include "Windows.h"
#include "thread/Thread.h"
#include "thread/Lock.h"
#include <list>

namespace col {

	enum Color {
		RED,
		GREEN,
		BLUE
	};

}

namespace ssi {



	class HeatmapWorker : public Thread {

	public:
		HeatmapWorker(	int width, 
			int height, 
			bool flip, 
			float influenceradius = 40, 
			float minimumDisplayPercentage = 0.2, 
			int horizontalRegions = 1, 
			int verticalRegions = 1, 
			bool decreaseOverTime = false, 
			float decreaseTick = 2,
			float decreaseFactor = 0.2,
			bool smoothen = false, 
			int filterRepetitions = 0
			);

		~HeatmapWorker(void);

		void increeaseGazePointCount(int x, int y);
		Mutex _colorGridMutex;
		BYTE **_colorGrid;



	protected:

		int _width, _height, _gazeMaxCount, _gazeTotalCount, _gazeAvgCount, _horizontalRegions, _verticalRegions, _filterRepetitions;
		float _influenceRadius, _minimumDisplaypercentage, _decreaseFactor, _decreaseTick;
		bool _imageFlipped, _smoothen, _decreaseOverTime;
		int **_densityGrid;	

		float _boxFilterKernel[3][3];

		DWORD *_regionTimer; 
		void boxFiltering();
		void boxFilteringPixel(int x, int y);
		void getHeatMapColor(float value, float *red, float *green, float *blue);
		void setColorForPointFromDensity(int x, int y);
		void setColorValueForPoint(int red, int green, int blue, int x, int y);
		void decreaseArea(int area);
		void HeatmapWorker::updateMax();
		int getColorValueForPoint(int color, int x, int y);
		int getRegionNumber(int x, int y);

		void enter ();
		void run ();
		void flush ();

	};

}

#endif