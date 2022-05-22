#include "stdafx.h"
#include "common.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>

int pointsImg1 = 200, pointsImg2 = 200;
std::vector<Point2f> img1Points;
std::vector<Point2f> img2Points;

typedef struct _TrianglePoint {
	Point p1;
	Point p2;
	Point p3;
	int indexP1;
	int indexP2;
	int indexP3;
} TrianglePoint;

void setCorrespondingPoints1(int event, int x, int y, int flags, void* param)
{
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
	Mat* src = (Mat*)param;
	if (event == EVENT_LBUTTONDOWN && pointsImg2 > 0)
	{
		pointsImg2--;
		img2Points.push_back(Point2f(x, y));
		(*src).at<Vec3b>(y, x) = Vec3b(0, 0, 255);
		imshow("Image 2", *src);
	}
}

bool draw = true;
void changeVisibilityDelaunay(int event, int x, int y, int flags, void* param)
{
	Mat* src = (Mat*)param;
	if (event == EVENT_LBUTTONDOWN)
	{
		draw = !draw;
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

		//Setting base points (corners and half of the sides)
		img1Points.push_back(Point2f(1, 1));
		img1Points.push_back(Point2f(1, img1.rows / 2));
		img1Points.push_back(Point2f(1, img1.rows - 1));
		img1Points.push_back(Point2f(img1.cols - 1, 1));
		img1Points.push_back(Point2f(img1.cols / 2, 1));
		img1Points.push_back(Point2f(img1.cols - 1, img1.rows - 1));
		img1Points.push_back(Point2f(img1.cols - 1, img1.rows / 2));
		img1Points.push_back(Point2f(img1.cols / 2, img1.rows - 1));

		img2Points.push_back(Point2f(1, 1));
		img2Points.push_back(Point2f(1, img2.rows / 2));
		img2Points.push_back(Point2f(1, img2.rows - 1));
		img2Points.push_back(Point2f(img2.cols - 1, 1));
		img2Points.push_back(Point2f(img2.cols / 2, 1));
		img2Points.push_back(Point2f(img2.cols - 1, img2.rows - 1));
		img2Points.push_back(Point2f(img2.cols - 1, img2.rows / 2));
		img2Points.push_back(Point2f(img2.cols / 2, img2.rows - 1));

		namedWindow("Image 1", 1);
		setMouseCallback("Image 1", setCorrespondingPoints1, &img1);
		imshow("Image 1", img1);

		namedWindow("Image 2", 1);
		setMouseCallback("Image 2", setCorrespondingPoints2, &img2);
		imshow("Image 2", img2);
		waitKey(0);

		destroyWindow("Image 1");
		destroyWindow("Image 2");
	}
}

std::pair<int, int> minimums(std::vector<Point2f> t) {
	int minX = t.at(0).x;
	int minY = t.at(0).y;
	for (int i = 1; i < t.size(); i++)
	{
		Point2f current = t.at(i);
		if (current.x < minX)
			minX = current.x;
		if (current.y < minY)
			minY = current.y;
	}

	return std::make_pair(minX, minY);
}

std::pair<int, int> maximums(std::vector<Point2f> t) {
	int maxX = t.at(0).x;
	int maxY = t.at(0).y;
	for (int i = 1; i < t.size(); i++)
	{
		Point2f current = t.at(i);
		if (current.x > maxX)
			maxX = current.x;
		if (current.y > maxY)
			maxY = current.y;
	}

	return std::make_pair(maxX, maxY);
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

void generateDelauney(Mat& img, std::vector<Point2f> points, std::vector<TrianglePoint>& delaunayResult) {

	Rect rect1(0, 0, img.cols, img.rows);
	Subdiv2D subdiv(rect1);

	for (std::vector<Point2f>::iterator it = points.begin(); it != points.end(); it++) {
		subdiv.insert(*it);
	}

	std::vector<Vec6f> triangleList;
	subdiv.getTriangleList(triangleList);
	std::vector<Point> pt(3);
	for (int i = 0; i < triangleList.size(); i++) {
		Vec6f t = triangleList[i];

		pt[0] = Point2f(cvRound(t[0]), cvRound(t[1]));
		pt[1] = Point2f(cvRound(t[2]), cvRound(t[3]));
		pt[2] = Point2f(cvRound(t[4]), cvRound(t[5]));

		int indexP1 = indexOf(pt[0], points);
		int indexP2 = indexOf(pt[1], points);
		int indexP3 = indexOf(pt[2], points);

		TrianglePoint currentTriangle = TrianglePoint{ pt[0], pt[1], pt[2], indexP1,indexP2,indexP3 };
		delaunayResult.push_back(currentTriangle);
	}
}

void drawDelaunay(Mat& img, std::vector<Point2f> points) {

	Rect rect1(0, 0, img.cols, img.rows);
	Subdiv2D subdiv(rect1);
	Scalar delaunay_color(0, 0, 0), points_color(0, 0, 255);

	for (std::vector<Point2f>::iterator it = points.begin(); it != points.end(); it++) {
		subdiv.insert(*it);
	}

	std::vector<Vec6f> triangleList;
	subdiv.getTriangleList(triangleList);
	Rect rect(0, 0, img.size().width, img.size().height);
	std::vector<Point> pt(3);
	for (int i = 0; i < triangleList.size(); i++) {
		Vec6f t = triangleList[i];

		pt[0] = Point2f(cvRound(t[0]), cvRound(t[1]));
		pt[1] = Point2f(cvRound(t[2]), cvRound(t[3]));
		pt[2] = Point2f(cvRound(t[4]), cvRound(t[5]));

		if (rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2]))
		{
			line(img, pt[0], pt[1], delaunay_color, 1, LINE_AA, 0);
			line(img, pt[1], pt[2], delaunay_color, 1, LINE_AA, 0);
			line(img, pt[2], pt[0], delaunay_color, 1, LINE_AA, 0);
		}
	}
}

void findLocationOfFeaturePoints(double alpha, std::vector<Point2f>& morphImgPoints) {

	for (unsigned i = 0; i < img1Points.size(); ++i) {
		float morphX = (1 - alpha) * img1Points[i].x + alpha * img2Points[i].x;
		float morphY = (1 - alpha) * img1Points[i].y + alpha * img2Points[i].y;
		morphImgPoints.push_back(Point2f(morphX, morphY));
	}
}

Mat findAffineTransform(std::vector<Point2f> srcTri, std::vector<Point2f> dstTri) {
	Mat source(3, 3, CV_32F);
	Mat dest(3, 3, CV_32F);

	for (int i = 0; i < srcTri.size(); ++i) {
		source.at<float>(0, i) = srcTri[i].x;
		source.at<float>(1, i) = srcTri[i].y;
		source.at<float>(2, i) = 1;
	}

	for (int i = 0; i < dstTri.size(); ++i) {
		dest.at<float>(0, i) = dstTri[i].x;
		dest.at<float>(1, i) = dstTri[i].y;
		dest.at<float>(2, i) = 1;
	}

	Mat T = dest * source.inv();

	Mat result(2, 3, CV_32F);

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 3; j++) {
			result.at<float>(i, j) = T.at<float>(i, j);
		}
	}

	return result;
}

void computeAffineTransform(Mat& img, Mat& src, std::vector<Point2f> srcTri, std::vector<Point2f> dstTri) {
	Mat affineTransform = findAffineTransform(srcTri, dstTri);

	/*std::cout << img.rows << " : " << img.cols << "\n";
	std::cout << src.rows << " : " << src.cols << "\n";
	std::cout << "AFFINE\n";
	for (int i = 0; i < affineTransform.rows; i++)
	{
		for (int j = 0; j < affineTransform.cols; j++)
		{
			std::cout << affineTransform.at<float>(i, j) << "\t";
		}
		std::cout << "\n";
	}

	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			int newX = affineTransform.at<float>(0, 0) * i + affineTransform.at<float>(0, 1) * j + affineTransform.at<float>(0, 2);
			int newY = affineTransform.at<float>(1, 0) * i + affineTransform.at<float>(1, 1) * j + affineTransform.at<float>(1, 2);
			if (newX < 0)
				newX = 0;
			if (newX > src.rows - 1)
				newX = src.rows - 1;
			if (newY < 0)
				newY = 0;
			if (newY > src.cols - 1)
				newY = src.cols - 1;

			img.at<float>(i, j) = src.at<float>(i, j);
		}
	}*/

	warpAffine(src, img, affineTransform, img.size(), INTER_LINEAR, BORDER_REFLECT_101);
}

// Warps and alpha blends triangular regions from img1 and img2 to img
void morphTriangleTEST(Mat& img1, Mat& img2, Mat& img, std::vector<Point2f>& t1, std::vector<Point2f>& t2, std::vector<Point2f>& t, double alpha)
{
	std::vector<Point2f> t1Rect, t2Rect, tRect;
	std::vector<Point> tRectInt;
	std::pair<int, int> minT1 = minimums(t1);
	std::pair<int, int> minT2 = minimums(t2);
	std::pair<int, int> minT = minimums(t);
	std::pair<int, int> maxT1 = maximums(t1);
	std::pair<int, int> maxT2 = maximums(t2);
	std::pair<int, int> maxT = maximums(t);

	int THeight = maxT.second - minT.second + 1;
	int TWidth = maxT.first - minT.first + 1;
	int T1Height = maxT1.second - minT1.second + 1;
	int T1Width = maxT1.first - minT1.first + 1;
	int T2Height = maxT2.second - minT2.second + 1;
	int T2Width = maxT2.first - minT2.first + 1;

	Rect t1ROI(minT1.first, minT1.second, T1Width, T1Height);
	Rect t2ROI(minT2.first, minT2.second, T2Width, T2Height);
	Rect morphedROI(minT.first, minT.second, TWidth, THeight);
	Mat img1Rect, img2Rect;

	//Copying image rectangular area into two Matrices
	img1(t1ROI).copyTo(img1Rect);
	img2(t2ROI).copyTo(img2Rect);

	//Offsetting the points by their minimums and maximums
	for (int i = 0; i < 3; i++)
	{
		tRect.push_back(Point2f(t[i].x - minT.first, t[i].y - minT.second));
		tRectInt.push_back(Point(t[i].x - minT.first, t[i].y - minT.second));

		t1Rect.push_back(Point2f(t1[i].x - minT1.first, t1[i].y - minT1.second));
		t2Rect.push_back(Point2f(t2[i].x - minT2.first, t2[i].y - minT2.second));
	}

	Mat warpImage1 = Mat::zeros(THeight, TWidth, img1Rect.type());
	Mat warpImage2 = Mat::zeros(THeight, TWidth, img2Rect.type());

	// Afine transform & Image warping
	computeAffineTransform(warpImage1, img1Rect, t1Rect, tRect);
	computeAffineTransform(warpImage2, img2Rect, t2Rect, tRect);

	// Alpha blend
	Mat imgRect = (1.0 - alpha) * warpImage1 + alpha * warpImage2;

	Mat mask = Mat::zeros(THeight, TWidth, CV_32FC3);
	fillConvexPoly(mask, tRectInt, Scalar(1.0, 1.0, 1.0), 16, 0);

	multiply(imgRect, mask, imgRect);
	multiply(img(morphedROI), Scalar(1.0, 1.0, 1.0) - mask, img(morphedROI));
	img(morphedROI) = img(morphedROI) + imgRect;
}


const int alpha_slider_max = 100;
int alpha_slider;
double alpha = 0.0;
Mat dst, img1, img2;
std::vector<TrianglePoint> t1;
Scalar delaunay_color(255, 255, 255), points_color(0, 0, 255);
static void changeAlpha(int, void*)
{
	Mat imgMorph = Mat::zeros(img1.size(), CV_32FC3);
	alpha = (double)alpha_slider / alpha_slider_max;
	std::cout << "ALPHA: " << alpha << std::endl;

	setMouseCallback("Morphed", changeVisibilityDelaunay, &imgMorph);

	//do morphing
	std::vector<Point2f> morphImgPoints;
	findLocationOfFeaturePoints(alpha, morphImgPoints);

	for (int i = 0; i < t1.size(); ++i) {
		std::vector<Point2f> t1Vector;
		std::vector<Point2f> t2Vector;
		std::vector<Point2f> tVector;

		t1Vector.push_back(t1.at(i).p1);
		t1Vector.push_back(t1.at(i).p2);
		t1Vector.push_back(t1.at(i).p3);

		int indexP1 = t1.at(i).indexP1;
		int indexP2 = t1.at(i).indexP2;
		int indexP3 = t1.at(i).indexP3;

		t2Vector.push_back(img2Points.at(indexP1));
		t2Vector.push_back(img2Points.at(indexP2));
		t2Vector.push_back(img2Points.at(indexP3));

		Point2f p1 = (morphImgPoints.at(indexP1));
		Point2f p2 = (morphImgPoints.at(indexP2));
		Point2f p3 = (morphImgPoints.at(indexP3));
		tVector.push_back(p1);
		tVector.push_back(p2);
		tVector.push_back(p3);

		morphTriangleTEST(img1, img2, imgMorph, t1Vector, t2Vector, tVector, alpha);
	}

	if (draw)
		drawDelaunay(imgMorph, morphImgPoints);
	imshow("Morphed", imgMorph / 255.0);
}

void morphImage()
{
	setCorrespodingPoints(img1, img2);

	//empty average image
	Mat imgMorph = Mat::zeros(img1.size(), CV_32FC3);
	img1.convertTo(img1, CV_32F);
	img2.convertTo(img2, CV_32F);

	generateDelauney(img1, img1Points, t1);

	namedWindow("Morphed", WINDOW_AUTOSIZE); // Create Window

	char TrackbarName[50];
	sprintf(TrackbarName, "Alpha %d", alpha);
	createTrackbar(TrackbarName, "Morphed", &alpha_slider, alpha_slider_max, changeAlpha);
	changeAlpha(alpha_slider, 0);

	waitKey(0);

}