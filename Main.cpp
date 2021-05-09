#include "stdafx.h"
#include "common.h"
#include <random>
#include <iostream>
#include <fstream>
#include <stdlib.h>


int di[] = { 0, -1, -1, -1, 0, 1, 1, 1 };
int dj[] = { 1, 1, 0, -1, -1, -1, 0, 1 };


bool isInside(Mat aux, float i, float j) {

	if (i >= 0.0 && i < 1.0 * aux.rows)
		if (j >= 0.0 && j < 1.0 * aux.cols)
			return true;
	return false;
}



std::vector<int> coords(Mat src, Vec3b color) {

	int cmax = -999, cmin = 999, rmax = -999, rmin = 999;
	std::vector<int> aux;

	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++)
			if (src.at<Vec3b>(i, j)[0] == color.val[0] &&
				src.at<Vec3b>(i, j)[1] == color.val[1] &&
				src.at<Vec3b>(i, j)[2] == color.val[2])
			{
				if (i > rmax)
					rmax = i;
				if (i < rmin)
					rmin = i;
				if (j > cmax)
					cmax = j;
				if (j < cmin)
					cmin = j;
			}

	aux.push_back(rmax);
	aux.push_back(rmin);
	aux.push_back(cmax);
	aux.push_back(cmin);

	return aux;
}



Mat convert_to_binary(Mat src) {


	int trshld;
	int check;
	int t;

	t = 150;

	Mat dst = src.clone();

	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++)
			if (src.at<uchar>(i, j) < t)
				dst.at<uchar>(i, j) = 0;
			else dst.at<uchar>(i, j) = 255;

	return dst;

}


Mat dilation(Mat src, int n) {

	Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC1);
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++) {
			dst.at<uchar>(i, j) = 255;
		}
	for (int z = 0; z < n; z++) {
		for (int i = 0; i < src.rows; i++)
			for (int j = 0; j < src.cols; j++) {
				if (src.at<uchar>(i, j) == 0)
					for (int k = 0; k < 8; k++) {
						Point neighbour = Point(i + di[k], j + dj[k]);

						if (isInside(dst, neighbour.x, neighbour.y))
							dst.at<uchar>(neighbour.x, neighbour.y) = 0;
					}
			}
	}
	return dst;
}



Mat erosion(Mat src, int n) {

	Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC1);
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++) {
			dst.at<uchar>(i, j) = 255;
		}
	for (int z = 0; z < n; z++) {
		for (int i = 0; i < src.rows; i++)
			for (int j = 0; j < src.cols; j++) {
				if (src.at<uchar>(i, j) == 0) {
					int ok = 0;
					for (int k = 0; k < 8 && ok != 1; k++) {

						Point neighbour = Point(i + di[k], j + dj[k]);
						if (isInside(dst, neighbour.x, neighbour.y)) {
							if (src.at<uchar>(neighbour.x, neighbour.y) != 0) {
								ok = 1;
							}
						}
					}

					if (ok == 0)
						dst.at<uchar>(i, j) = 0;
				}
			}
	}
	return dst;

}

Mat opening(Mat src, int n) {

	Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC1);
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++) {
			dst.at<uchar>(i, j) = 255;
		}
	for (int z = 0; z < n; z++) {
		dst = erosion(src, 1);
		dst = dilation(dst, 1);
	}
	return dst;
}

Mat closing(Mat src, int n) {

	Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC1);
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++) {
			dst.at<uchar>(i, j) = 255;
		}

	for (int z = 0; z < n; z++) {
		dst = dilation(src, 1);
		dst = erosion(dst, 1);
	}
	return dst;
}


Mat contour_extraction(Mat src) {


	Mat erz = erosion(src, 1);

	Mat dst = Mat::zeros(src.rows, src.cols, CV_8UC1);

	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++) {
			dst.at<uchar>(i, j) = 255;
		}

	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++) {
			if (src.at<uchar>(i, j) != erz.at<uchar>(i, j))
				dst.at<uchar>(i, j) = 0;
		}

	return dst;
}


int di4[4] = { -1, 0, 1, 0 };
int dj4[4] = { 0,-1, 0, 1 };
int di8[8] = { -1,-1,-1, 0, 0, 1, 1, 1 };
int dj8[8] = { -1, 0, 1,-1, 1,-1, 0, 1 };

Mat BFS(Mat src, int& label, int n) {

	Mat labels = Mat::zeros(src.rows, src.cols, CV_32SC1);
	label = 0;

	std::cout << src.rows << " , " << src.cols << "\n";

	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++)
			if (src.at<uchar>(i, j) == 0 && labels.at<int>(i, j) == 0)
			{
				label++;
				std::queue<Point> Q;
				labels.at<int>(i, j) = label;
				Q.push(Point(i, j));

				while (!Q.empty()) {

					Point q = Q.front();
					Q.pop();

					if (n == 4) {
						for (int aux = 0; aux < 4; aux++) {
							Point neighbor(q.x + di4[aux], q.y + dj4[aux]);
							if (isInside(labels, neighbor.x, neighbor.y) && src.at<uchar>(neighbor.x, neighbor.y) == 0 && labels.at<int>(neighbor.x, neighbor.y) == 0)
							{
								labels.at<int>(neighbor.x, neighbor.y) = label;
								Q.push(neighbor);
							}
						}
					}
					else
						for (int aux = 0; aux < 8; aux++) {
							Point neighbor(q.x + di8[aux], q.y + dj8[aux]);
							if (isInside(labels, neighbor.x, neighbor.y) && src.at<uchar>(neighbor.x, neighbor.y) == 0 && labels.at<int>(neighbor.x, neighbor.y) == 0)
							{
								labels.at<int>(neighbor.x, neighbor.y) = label;
								Q.push(neighbor);
							}
						}
				}
			}

	return labels;
}


Mat gen_colors(Mat src) {

	int n = 8;

	int label;
	Mat labels = BFS(src, label, n);
	Mat dst = Mat(src.rows, src.cols, CV_8UC3);

	std::default_random_engine gen;
	std::uniform_int_distribution<int> d(0, 255);
	Vec3b* colors = (Vec3b*)calloc(label, sizeof(Vec3b));

	for (int i = 0; i < label; i++) {
		colors[i][0] = d(gen);
		colors[i][1] = d(gen);
		colors[i][2] = d(gen);
	}

	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++)
			if (labels.at<int>(i, j) > 0) {
				dst.at<Vec3b>(i, j)[0] = colors[labels.at<int>(i, j)][0];
				dst.at<Vec3b>(i, j)[1] = colors[labels.at<int>(i, j)][1];
				dst.at<Vec3b>(i, j)[2] = colors[labels.at<int>(i, j)][2];
			}
			else {
				dst.at<Vec3b>(i, j)[0] = 255;
				dst.at<Vec3b>(i, j)[1] = 255;
				dst.at<Vec3b>(i, j)[2] = 255;
			}


	return dst;
}



void detection() {

	char fname[MAX_PATH];

	if (openFileDlg(fname)) {

		Mat src = imread(fname, IMREAD_COLOR);
		Mat after_treshold = src.clone();


		Vec3b l = Vec3b(0, 0, 170);
		Vec3b u = Vec3b(100, 100, 255);

		std::vector<Vec3b> lowers, uppers;

		lowers.push_back(Vec3b(0, 0, 220));
		lowers.push_back(Vec3b(0, 0, 230));
		lowers.push_back(Vec3b(10, 0, 165));
		lowers.push_back(Vec3b(30, 30, 200));
		lowers.push_back(Vec3b(20, 20, 155));
		lowers.push_back(Vec3b(40, 30, 140));
		lowers.push_back(Vec3b(20, 0, 110));
		lowers.push_back(Vec3b(70, 50, 190));

		uppers.push_back(Vec3b(90, 40, 255));
		uppers.push_back(Vec3b(60, 70, 255));
		uppers.push_back(Vec3b(65, 10, 220));
		uppers.push_back(Vec3b(90, 90, 255));
		uppers.push_back(Vec3b(60, 50, 230));
		uppers.push_back(Vec3b(110, 70, 220));
		uppers.push_back(Vec3b(80, 40, 150));
		uppers.push_back(Vec3b(110, 90, 235));



		for (int i = 0; i < src.rows; i++)
			for (int j = 0; j < src.cols; j++)
				for (int k = 0; k < lowers.size(); k++)
					if (src.at<Vec3b>(i, j)[0] >= lowers[k][0] && src.at<Vec3b>(i, j)[0] <= uppers[k][0] &&
						src.at<Vec3b>(i, j)[1] >= lowers[k][1] && src.at<Vec3b>(i, j)[1] <= uppers[k][1] &&
						src.at<Vec3b>(i, j)[2] >= lowers[k][2] && src.at<Vec3b>(i, j)[2] <= uppers[k][2])
						after_treshold.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
					else if (after_treshold.at<Vec3b>(i, j) != Vec3b(0, 0, 0))
						after_treshold.at<Vec3b>(i, j) = Vec3b(255, 255, 255);

		Mat black_and_white = Mat::zeros(src.rows, src.cols, IMREAD_GRAYSCALE);
		for (int i = 0; i < src.rows; i++)
			for (int j = 0; j < src.cols; j++)
				black_and_white.at<uchar>(i, j) = (after_treshold.at<Vec3b>(i, j)[0] + after_treshold.at<Vec3b>(i, j)[1] + after_treshold.at<Vec3b>(i, j)[2]) / 3;


		black_and_white = convert_to_binary(black_and_white);
		black_and_white = opening(black_and_white, 1);
		black_and_white = dilation(black_and_white, 1);
		black_and_white = closing(black_and_white, 1);
		//imshow("after inchidere", labeled);

		Mat labeled = gen_colors(black_and_white);
		//imshow("culori", culori);

		black_and_white = contour_extraction(black_and_white);

		for (int i = 0; i < src.rows; i++)
			for (int j = 0; j < src.cols; j++)
				if (black_and_white.at<uchar>(i, j) != 255) {
					src.at<Vec3b>(i, j)[2] = 0;
					src.at<Vec3b>(i, j)[1] = 0;
					src.at<Vec3b>(i, j)[0] = 255;
				}

		std::vector<Vec3b> aux;

		for (int i = 0; i < src.rows; i++)
			for (int j = 0; j < src.cols; j++)
				if (labeled.at<Vec3b>(i, j)[0] != 255 &&
					labeled.at<Vec3b>(i, j)[1] != 255 &&
					labeled.at<Vec3b>(i, j)[2] != 255)
					aux.push_back(labeled.at<Vec3b>(i, j));

		auto end = aux.end();
		for (auto it = aux.begin(); it != end; ++it)
			end = std::remove(it + 1, end, *it);
		aux.erase(end, aux.end());


		int rmax, rmin, cmax, cmin;
		for (auto& i : aux)
		{
			std::vector<int> tmp = coords(labeled, i);

			rmax = tmp[0];
			rmin = tmp[1];
			cmax = tmp[2];
			cmin = tmp[3];
			//std::cout << tmp[0] << "," << tmp[1] << "," << tmp[2] << "," << tmp[3] << "\n";
			line(src, Point(cmin, rmin), Point(cmax, rmin), Vec3b(0, 255, 0), 3);
			line(src, Point(cmin, rmax), Point(cmax, rmax), Vec3b(0, 255, 0), 3);
			line(src, Point(cmin, rmin), Point(cmin, rmax), Vec3b(0, 255, 0), 3);
			line(src, Point(cmax, rmin), Point(cmax, rmax), Vec3b(0, 255, 0), 3);
			
		}


		imshow("src", src);
		waitKey(0);

	}

}

int main()
{
	int op;
	detection();
}
