#include "stdafx.h"
#include "common.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>

int pointsImg1 = 80, pointsImg2 = 80;
std::vector<Point2f> img1Points;
std::vector<Point2f> img2Points;

typedef struct _Triangle {
	int indexP1;
	int indexP2;
	int indexP3;
}Triangle;

typedef struct _TrianglePoint {
	Point p1;
	Point p2;
	Point p3;
} TrianglePoint;

void setCorrespondingPoints1(int event, int x, int y, int flags, void* param)
{
	//More examples: http://opencvexamples.blogspot.com/2014/01/detect-mouse-clicks-and-moves-on-image.html
	Mat* src = (Mat*)param;
	if (event == EVENT_LBUTTONDOWN && pointsImg1 > 0)
	{
		pointsImg1--;
		img1Points.push_back(Point2f(x, y));
		(*src).at<Vec3b>(y, x) = Vec3b(0, 0, 255);
		imshow("Image 1", *src);
	}
}

void setCorrespondingPoints2(int event, int x, int y, int flags, void* param)
{
	//More examples: http://opencvexamples.blogspot.com/2014/01/detect-mouse-clicks-and-moves-on-image.html
	Mat* src = (Mat*)param;
	if (event == EVENT_LBUTTONDOWN && pointsImg2 > 0)
	{
		pointsImg2--;
		img2Points.push_back(Point2f(x, y));
		(*src).at<Vec3b>(y, x) = Vec3b(0, 0, 255);
		imshow("Image 2", *src);
	}
}

//Part1: Setting corresponding points
void setCorrespodingPoints(Mat& img1, Mat& img2) {
	char fname1[MAX_PATH];
	char fname2[MAX_PATH];

	while (openFileDlg(fname1) && openFileDlg(fname2))
	{
		img1 = imread(fname1, IMREAD_COLOR);
		img2 = imread(fname2, IMREAD_COLOR);

		namedWindow("Image 1", 1);
		setMouseCallback("Image 1", setCorrespondingPoints1, &img1);
		imshow("Image 1", img1);

		namedWindow("Image 2", 1);
		setMouseCallback("Image 2", setCorrespondingPoints2, &img2);
		imshow("Image 2", img2);
		waitKey(0);
	}
}

//Part2: The Delaunay Triangulation
int indexOf(Point currentPoint, std::vector<Point2f> points) {
	for (int i = 0; i < points.size(); i++) {
		Point2f point = points.at(i);
		if (point.x == currentPoint.x && point.y == currentPoint.y)
			return i;
	}
	return 0;
}

TrianglePoint generateDelaunayTriangle(Vec6f triangleList) {
	std::vector<Point> pt(3);
	pt[0] = Point(cvRound(triangleList[0]), cvRound(triangleList[1]));
	pt[1] = Point(cvRound(triangleList[2]), cvRound(triangleList[3]));
	pt[2] = Point(cvRound(triangleList[4]), cvRound(triangleList[5]));

	return TrianglePoint{ pt[0], pt[1], pt[2] };
}

std::vector<TrianglePoint> generateDelaunayTriangles() {

}

void drawDelaunay(Mat& img, Subdiv2D& subdiv, Scalar delaunay_color, std::vector<Triangle>& delaunayResult, int indexImg) {

	std::vector<Vec6f> triangleList;
	subdiv.getTriangleList(triangleList);
	std::vector<Point> pt(3);

	for (int i = 0; i < triangleList.size(); i++) {
		Vec6f t = triangleList[i];

		pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
		pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
		pt[2] = Point(cvRound(t[4]), cvRound(t[5]));

		int p1Index = indexOf(pt[0], indexImg == 1 ? img1Points : img2Points);
		int p2Index = indexOf(pt[1], indexImg == 1 ? img1Points : img2Points);
		int p3Index = indexOf(pt[2], indexImg == 1 ? img1Points : img2Points);
		Triangle currentTriangle = Triangle{ p1Index, p2Index, p3Index };
		delaunayResult.push_back(currentTriangle);

		line(img, pt[0], pt[1], delaunay_color, 1, LINE_AA, 0);
		line(img, pt[1], pt[2], delaunay_color, 1, LINE_AA, 0);
		line(img, pt[2], pt[0], delaunay_color, 1, LINE_AA, 0);

	}
}

std::vector<Triangle> delaunayTriangulation(Mat img, int indexImg) {

	std::vector<Triangle> delaunayResult;
	Rect rect1(0, 0, img.cols, img.rows);
	Subdiv2D subdiv1(rect1);
	Scalar delaunay_color(255, 255, 255), points_color(0, 0, 255);

	std::vector<Point2f>::iterator endIterator = indexImg == 1 ? img1Points.end() : img2Points.end();

	for (std::vector<Point2f>::iterator it = indexImg == 1 ? img1Points.begin() : img2Points.begin(); it != endIterator; it++) {
		subdiv1.insert(*it);
	}

	drawDelaunay(img, subdiv1, delaunay_color, delaunayResult, indexImg);
	std::cout << delaunayResult.size() << "\n";

	return delaunayResult;
}

std::vector<Point2f> findLocationOfFeaturePoints(double alpha) {
	std::vector<Point2f> morphImgPoints;

	for (unsigned i = 0; i < img1Points.size(); ++i) {
		float morphX = (1 - alpha) * img1Points[i].x + alpha * img2Points[i].x;
		float morphY = (1 - alpha) * img1Points[i].y + alpha * img2Points[i].y;
		morphImgPoints.push_back(Point2f(morphX, morphY));
	}

	return morphImgPoints;
}

std::vector<Triangle> computeAffineTransform(Mat& img, std::vector<Point2f> morphedPoints, std::vector<Point2f> src, std::vector<Point2f> dst) {
	/*for (unsigned i = 0; i < src.size(); ++i) {
		std::vector<Point2f> srcTriangle;
		srcTriangle.push_back(img1Points[src[i].indexP1]);
		srcTriangle.push_back(img1Points[src[i].indexP2]);
		srcTriangle.push_back(img1Points[src[i].indexP3]);

		std::vector<Point2f> dstTriangle;
		dstTriangle.push_back(morphedPoints[dst[i].indexP1]);
		dstTriangle.push_back(morphedPoints[dst[i].indexP2]);
		dstTriangle.push_back(morphedPoints[dst[i].indexP3]);*/
	Mat warpMat = getAffineTransform(src, dst);

	warpAffine(src, img, warpMat, img.size(), INTER_LINEAR, BORDER_REFLECT_101);
	//}
}

// Warps and alpha blends triangular regions from img1 and img2 to img
void morphTriangle(Mat& img1, Mat& img2, Mat& img, TrianglePoint& t1, TrianglePoint& t2, std::vector<Point2f>& t, double alpha)
{

	// Find bounding rectangle for each triangle
	Rect r = boundingRect(t);
	Rect r1 = boundingRect(t1);
	Rect r2 = boundingRect(t2);

	// Offset points by left top corner of the respective rectangles
	std::vector<Point2f> t1Rect, t2Rect, tRect;
	std::vector<Point> tRectInt;
	for (int i = 0; i < 3; i++)
	{
		tRect.push_back(Point2f(t[i].x - r.x, t[i].y - r.y));
		tRectInt.push_back(Point(t[i].x - r.x, t[i].y - r.y)); // for fillConvexPoly

		t1Rect.push_back(Point2f(t1[i].x - r1.x, t1[i].y - r1.y));
		t2Rect.push_back(Point2f(t2[i].x - r2.x, t2[i].y - r2.y));
	}

	// Get mask by filling triangle
	Mat mask = Mat::zeros(r.height, r.width, CV_32FC3);
	fillConvexPoly(mask, tRectInt, Scalar(1.0, 1.0, 1.0), 16, 0);

	// Apply warpImage to small rectangular patches
	Mat img1Rect, img2Rect;
	img1(r1).copyTo(img1Rect);
	img2(r2).copyTo(img2Rect);

	Mat warpImage1 = Mat::zeros(r.height, r.width, img1Rect.type());
	Mat warpImage2 = Mat::zeros(r.height, r.width, img2Rect.type());

	computeAffineTransform(warpImage1, img1Rect, t1Rect, tRect);
	computeAffineTransform(warpImage2, img2Rect, t2Rect, tRect);

	//applyAffineTransform(warpImage1, img1Rect, t1Rect, tRect);
	//applyAffineTransform(warpImage2, img2Rect, t2Rect, tRect);

	// Alpha blend rectangular patches
	Mat imgRect = (1.0 - alpha) * warpImage1 + alpha * warpImage2;

	// Copy triangular region of the rectangular patch to the output image
	multiply(imgRect, mask, imgRect);
	multiply(img(r), Scalar(1.0, 1.0, 1.0) - mask, img(r));
	img(r) = img(r) + imgRect;


}


void morphImage()
{
	Mat img1, img2;
	double alpha = 0.5;

	//empty average image
	Mat imgMorph = Mat::zeros(img1.size(), CV_32FC3);

	// Setting the points on the image
	setCorrespodingPoints(img1, img2);
	for (int i = 0; i < img1Points.size(); i++) {
		std::cout << img1Points << std::endl << std::endl;
	}
	for (int i = 0; i < img2Points.size(); i++) {
		std::cout << img2Points << std::endl << std::endl;
	}

	// The Delauney Triangulation
	std::vector<Triangle> t1 = delaunayTriangulation(img1, 1);
	std::vector<Triangle> t2 = delaunayTriangulation(img2, 2);

	std::vector<Point2f> morphImgPoints = findLocationOfFeaturePoints(alpha);

	for (int i = 0; i < t2.size(); ++i) {
		morphTriangle(img1, img2, imgMorph, t1[i], t2[i], )
	}

	imshow("Delaunay Triangulation img1", img1);
	imshow("Delaunay Triangulation img2", img2);

	waitKey(0);

}