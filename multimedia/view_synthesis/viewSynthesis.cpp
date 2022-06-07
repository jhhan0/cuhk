#include "bmp.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define Baseline 30.0
#define Focal_Length 100
#define Image_Width 35.0
#define Image_Height 35.0
#define Resolution_Row 512
#define Resolution_Col 512
#define View_Grid_Row 9
#define View_Grid_Col 9

struct Point3d
{
	double x;
	double y;
	double z;
	Point3d(double x_, double y_, double z_) :x(x_), y(y_), z(z_) {}
};

struct Point2d
{
	double x;
	double y;
	Point2d(double x_, double y_) :x(x_), y(y_) {}
};


int main(int argc, char** argv)
{
	if(argc < 5 || argc > 6)
	{
		cout << "Arguments prompt: viewSynthesis.exe <LF_dir> <X Y Z> OR: viewSynthesis.exe <LF_dir> <X Y Z> <focal_length>" << endl;
		return 0;
	}
	string LFDir = argv[1];
	double Vx = stod(argv[2]), Vy = stod(argv[3]), Vz = stod(argv[4]);
	double targetFocalLen = 100;
	if(argc == 6)
	{
		targetFocalLen = stod(argv[5]);
	}
	

	vector<Bitmap> viewImageList;
	//! loading light field views
	for (int i = 0; i < View_Grid_Col * View_Grid_Row; i++)
	{
		char name[128];
		sprintf(name, "/cam%03d.bmp", i + 1);
		string filePath = LFDir + name;
		Bitmap view_i(filePath.c_str());
		viewImageList.push_back(view_i);
	}

	Bitmap targetView(Resolution_Col, Resolution_Row);
	cout << "Synthesizing image from viewpoint: (" << Vx << "," << Vy << "," << Vz << ") with focal length: " << targetFocalLen << endl;
	//! resample pixels of the target view one by one
	for (int r = 0; r < Resolution_Row; r++)
	{
		for (int c = 0; c < Resolution_Col; c++)
		{
			Point3d rayRGB(0, 0, 0);
			//! resample the pixel value of this ray: TODO
			Point3d viewPoint3d(Vx, Vy, Vz);

			// Basic Requirement: target viewpoint locates on the camera array plane;
			// i.e. the viewpoint coordinate = (X, Y, 0) where X and Y are in range [-120, 120].
			if (viewPoint3d.z == 0) 
			{
				Point2d viewPlane = {
					(viewPoint3d.x / Baseline) + 4, ((240 - viewPoint3d.y) / Baseline) - 4
				};

				double alpha = viewPlane.x - floor(viewPlane.x);
				double beta = ceil(viewPlane.y) - viewPlane.y;

				// find four neighbor viewpoints
				int viewPoint_bl = floor(viewPlane.y) * 9 + floor(viewPlane.x); // viewPoint Bottom Left
				int viewPoint_br = floor(viewPlane.y) * 9 + ceil(viewPlane.x); // viewPoint Bottom Right
				int viewPoint_tl = ceil(viewPlane.y) * 9 + floor(viewPlane.x); // viewPoint Top Left
				int viewPoint_tr = ceil(viewPlane.y) * 9 + ceil(viewPlane.x); // viewPoint Top Right

				unsigned char rgb_bl[3], rgb_br[3], rgb_tl[3], rgb_tr[3]; // RGB Bottom Left, Bottom Right, Top Left, Top Right respectively

				// get RGB color from four neighbor viewpoints 
				viewImageList[viewPoint_bl].getColor(c, r, rgb_bl[0], rgb_bl[1], rgb_bl[2]);
				viewImageList[viewPoint_br].getColor(c, r, rgb_br[0], rgb_br[1], rgb_br[2]);
				viewImageList[viewPoint_tl].getColor(c, r, rgb_tl[0], rgb_tl[1], rgb_tl[2]);
				viewImageList[viewPoint_tr].getColor(c, r, rgb_tr[0], rgb_tr[1], rgb_tr[2]);

				// Bilinear interpolation
				unsigned char upper[3], lower[3], target[3];
				for (int i = 0; i < 3; i++)
				{
					upper[i] = (1 - alpha) * rgb_tl[i] + alpha * rgb_tr[i];
					lower[i] = (1 - alpha) * rgb_bl[i] + alpha * rgb_br[i];
					target[i] = (1 - beta) * upper[i] + beta * lower[i];
				};
				// save resampled target pixel value
				rayRGB = {
					(double)target[0], (double)target[1], (double)target[2]
				};
			}
			// Enhanced Features: target viewpoint locates in the 3D space region with variable focal length;
			// i.e. the viewpoint coordinate = (X, Y, Z) where X and Y are in range [-120, 120], Z >= 0 and focal length f > 0.
			else 
			{
				// resample the value of ray of the target view
				double viewToImage = (viewPoint3d.z / targetFocalLen);
				// find focal ratio
				// - If ratio is less than 1, the result image will be enlarged. 
				// - If ratio is greater that 1, the result image will be shrinked.
				double focalRatio = Focal_Length / targetFocalLen;
				if (focalRatio >= 1.0) 
				{
					double targetPixel = Resolution_Col / focalRatio;
					int initial_Pixel = (int)(targetPixel / 2);
					int end_Pixel = (int)(targetPixel / 2 + targetPixel); 
					if (c > initial_Pixel && c < end_Pixel && r > initial_Pixel && r < end_Pixel)
					{
						// convert a pixel index to its image coordinate (35 x 35)
						Point2d imagePlane = {
							((c * Image_Width) / (Resolution_Col - 1)) - 17, ((r * Image_Height) / (Resolution_Row - 1)) - 17
						};
						// compute intersection point on 2D view plane (8 by 8) based on the coordinate of image plane and the direction of ray of target view.
						Point2d viewPlane = {
							((viewPoint3d.x + viewToImage * imagePlane.x) / Baseline) + 4, ((240 - (viewPoint3d.y + viewToImage * imagePlane.y)) / Baseline) - 4
						};
					}
					else
					{
						targetView.setColor(c, r, (unsigned char)rayRGB.x, (unsigned char)rayRGB.y, (unsigned char)rayRGB.z);
					}
					
				}
				else
				{
					 
				}
			}


			//! record the resampled pixel value
			targetView.setColor(c, r, (unsigned char)rayRGB.x, (unsigned char)rayRGB.y, (unsigned char)rayRGB.z);
		}
	}

	string savePath = "newView.bmp";
	targetView.save(savePath.c_str());
	cout << "Result saved!" << endl;
	return 0;
}