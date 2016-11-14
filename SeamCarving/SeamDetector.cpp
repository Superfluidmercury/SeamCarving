#include "stdafx.h"

SeamDetector::SeamDetector( cv::Mat &originalImage ) {
	originalImageMatrix = *new cv::Mat( originalImage );
	width = originalImageMatrix.cols;
	height = originalImageMatrix.rows;

	energyMatrix = *new cv::Mat( height, width, CV_32SC1 );
	seamMatrix = *new cv::Mat( height, width, CV_32SC1 );
}

/*
 * Dummy function - works by stupidity, not any actual energy.
 * TODO: IMPLEMENT ACTUAL ENERGY FUNCTION
 */
void SeamDetector::prepareEnergyMatrix() {
	for ( int row = 0; row < height; ++row ) {
		for ( int col = 0; col < width; ++col ) {
			auto color = originalImageMatrix.at<cv::Vec3b>( CvPoint( col, row ) );
			int sumColor = color[ 0 ] + color[ 1 ] + color[ 2 ];
			energyMatrix.at<int>( CvPoint( col, row ) ) = sumColor;
		}
	}
}


void SeamDetector::findVerticalSeam() {
	seamMatrix = energyMatrix.clone();
	verticalSeam.clear();

	for ( auto i = 1; i < height; ++i ) {
		iterateVerticalSeamMatrix( i );
	}

	traceVerticalSeam();
}

void SeamDetector::iterateVerticalSeamMatrix( int row ) {
	for ( auto col = 0; col < width; ++col ) {
		int lowestNeighbourAbove = 0;
		int left = std::numeric_limits<int>::max();
		int right = std::numeric_limits<int>::max();
		int top = seamMatrix.at<int>( CvPoint( col, row - 1 ) );
		if ( col > 0 ) {
			left = seamMatrix.at<int>( CvPoint( col - 1, row - 1 ) );
		}
		if ( col < width - 1 ) {
			right = seamMatrix.at<int>( CvPoint( col + 1, row - 1 ) );
		}

		lowestNeighbourAbove = ( left < top ) ? left : top;
		lowestNeighbourAbove = ( right < lowestNeighbourAbove ) ? right : lowestNeighbourAbove;

		seamMatrix.at<int>( CvPoint( col, row ) ) += lowestNeighbourAbove;
	}
}

void SeamDetector::traceVerticalSeam() {
	findVerticalSeamStartingPoint();


	for ( int row = height - 2; row > 0; --row ) {
		iterateVerticalSeam( row );
	}
}

void SeamDetector::findVerticalSeamStartingPoint() {
	int startingPoint = 0;
	int minimumEnergy = std::numeric_limits<int>::max();
	for ( int col = 0; col < seamMatrix.cols; ++col ) {
		int energyAtIndex = seamMatrix.at<int>( CvPoint( col, height - 1 ) );
		if ( energyAtIndex < minimumEnergy ) {
			startingPoint = col;
			minimumEnergy = energyAtIndex;
		}
	}

	verticalSeam.push_back( startingPoint );
}

void SeamDetector::iterateVerticalSeam( int row ) {
	// Initialize default values.
	int previousIndex = verticalSeam.back();    // The index of the last point of the seam. Only check top left, top top and top right from here.
	char indexShift = 0;                        // Will indicate where to move next. Should only ever be -1, 0 or 1.

	int leftEnergy;
	int rightEnergy;
	int topEnergy = seamMatrix.at<int>( CvPoint( previousIndex, row - 1 ) );
	int lowestEnergyAbove = topEnergy;

	if ( previousIndex > 0 ) {
		leftEnergy = seamMatrix.at<int>( CvPoint( previousIndex - 1, row - 1 ) );
		if ( leftEnergy < lowestEnergyAbove ) {
			lowestEnergyAbove = leftEnergy;
			indexShift = -1;
		}
	}

	if ( previousIndex < width - 1 ) {
		rightEnergy = seamMatrix.at<int>( CvPoint( previousIndex + 1, row - 1 ) );
		if ( rightEnergy < lowestEnergyAbove ) {
			indexShift = 1;
		}
	}

	verticalSeam.push_back( previousIndex + indexShift );
}

void SeamDetector::drawVerticalSeam() {
	auto p = verticalSeam.begin();
	int rowIndex = height - 1;
	for ( ; p != verticalSeam.end(); ++p, --rowIndex ) {
		originalImageMatrix.at<cv::Vec3b>( CvPoint( *p, rowIndex ) ) = cv::Vec3b( 255, 0, 0 );
	}
}

void SeamDetector::removeVerticalSeam() {
	int rowIndex = height - 1;
	for ( auto p = verticalSeam.begin(); p != verticalSeam.end(); ++p ) {

		for ( int colIndex = *p; colIndex < width - 1; ++colIndex ) {
			// TODO: Change those lines into time-efficent memmove calls!
			originalImageMatrix.at<cv::Vec3b>( CvPoint( colIndex, rowIndex ) ) = originalImageMatrix.at<cv::Vec3b>( CvPoint( colIndex + 1, rowIndex ) );
			energyMatrix.at<int>( CvPoint( colIndex, rowIndex ) ) = energyMatrix.at<int>( CvPoint( colIndex + 1, rowIndex ) );
		}

		originalImageMatrix.at<cv::Vec3b>( CvPoint( width - 1, rowIndex ) ) = cv::Vec3b( 0, 0, 0 );
		energyMatrix.at<int>( CvPoint( width - 1, rowIndex ) ) = std::numeric_limits<int>::max();
		--rowIndex;
	}

	--width;
}


void SeamDetector::findHorizontalSeam() {
	seamMatrix = energyMatrix.clone();
	horizontalSeam.clear();

	for ( int i = 1; i < width; ++i ) {
		iterateHorizontalSeamMatrix( i );
	}

	traceHorizontalSeam();
}

void SeamDetector::iterateHorizontalSeamMatrix( int col ) {
	for ( auto row = 0; row < height; ++row ) {
		int lowestNeighbourLeft = 0;
		int top = std::numeric_limits<int>::max();
		int bottom = std::numeric_limits<int>::max();
		int left = seamMatrix.at<int>( CvPoint( col - 1, row ) );
		if ( row > 0 ) {
			top = seamMatrix.at<int>( CvPoint( col - 1, row - 1 ) );
		}
		if ( row < height - 1 ) {
			bottom = seamMatrix.at<int>( CvPoint( col - 1, row + 1 ) );
		}

		lowestNeighbourLeft = ( top < left ) ? top : left;
		lowestNeighbourLeft = ( bottom < lowestNeighbourLeft ) ? bottom : lowestNeighbourLeft;

		seamMatrix.at<int>( CvPoint( col, row ) ) += lowestNeighbourLeft;
	}
}

void SeamDetector::traceHorizontalSeam() {
	findHorizontalSeamStartingPoint();


	for ( int col = width - 2; col > 0; --col ) {
		iterateHorizontalSeam( col );
	}
}

void SeamDetector::findHorizontalSeamStartingPoint() {
	int startingPoint = 0;
	int minimumEnergy = std::numeric_limits<int>::max();
	for ( int row = 0; row < seamMatrix.rows; ++row ) {
		int energyAtIndex = seamMatrix.at<int>( CvPoint( width - 1, row ) );
		if ( energyAtIndex < minimumEnergy ) {
			startingPoint = row;
			minimumEnergy = energyAtIndex;
		}
	}

	horizontalSeam.push_back( startingPoint );
}

void SeamDetector::iterateHorizontalSeam( int col ) {
	int previousIndex = horizontalSeam.back();    // The index of the last point of the seam. Only check top left, top top and top right from here.
	char indexShift = 0;                          // Will indicate where to move next. Should only ever be -1, 0 or 1.

	int topEnergy;
	int bottomEnergy;
	int leftEnergy = seamMatrix.at<int>( CvPoint( col - 1, previousIndex ) );
	int lowestEnergyLeft = leftEnergy;

	if ( previousIndex > 0 ) {
		topEnergy = seamMatrix.at<int>( CvPoint( col - 1, previousIndex - 1 ) );
		if ( topEnergy < lowestEnergyLeft ) {
			lowestEnergyLeft = topEnergy;
			indexShift = -1;
		}
	}

	if ( previousIndex < height - 1 ) {
		bottomEnergy = seamMatrix.at<int>( CvPoint( col - 1, previousIndex + 1 ) );
		if ( bottomEnergy < lowestEnergyLeft ) {
			indexShift = 1;
		}
	}

	horizontalSeam.push_back( previousIndex + indexShift );
}

void SeamDetector::drawHorizontalSeam() {
	auto p = horizontalSeam.begin();
	int colIndex = width - 1;
	for ( ; p != horizontalSeam.end(); ++p, --colIndex ) {
		originalImageMatrix.at<cv::Vec3b>( CvPoint( colIndex, *p ) ) = cv::Vec3b( 255, 0, 0 );
	}
}

void SeamDetector::removeHorizontalSeam() {
	int colIndex = width - 1;
	for ( auto p = horizontalSeam.begin(); p != horizontalSeam.end(); ++p ) {

		for ( int rowIndex = *p; rowIndex < height - 1; ++rowIndex ) {
			// TODO: Change those lines into time-efficent memmove calls!
			originalImageMatrix.at<cv::Vec3b>( CvPoint( colIndex, rowIndex ) ) = originalImageMatrix.at<cv::Vec3b>( CvPoint( colIndex, rowIndex + 1 ) );
			energyMatrix.at<int>( CvPoint( colIndex, rowIndex ) ) = energyMatrix.at<int>( CvPoint( colIndex, rowIndex + 1 ) );
		}

		originalImageMatrix.at<cv::Vec3b>( CvPoint( colIndex, height - 1 ) ) = cv::Vec3b( 0, 0, 0 );
		energyMatrix.at<int>( CvPoint( colIndex, height - 1 ) ) = std::numeric_limits<int>::max();
		--colIndex;
	}

	--height;
}

cv::Mat* SeamDetector::getImage() {
	return &originalImageMatrix;
}

cv::Mat* SeamDetector::getEnergyMatrix() {
	return &energyMatrix;
}

cv::Mat* SeamDetector::getSeamMatrix() {
	return &seamMatrix;
}

int SeamDetector::getHeight() const {
	return height;
}

int SeamDetector::getWidth() const {
	return width;
}

SeamDetector::~SeamDetector() {}
