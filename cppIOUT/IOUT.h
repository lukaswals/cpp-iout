/******************************************************************************
* C++ Implementation of IOUT tracking algorithm
* More info on <http://elvera.nue.tu-berlin.de/files/1517Bochinski2017.pdf>
* Author Lucas Wals
******************************************************************************/
#pragma once
#include <vector>

/******************************************************************************
* STRUCTS
******************************************************************************/
struct BoundingBox
{
	// x-component of top left coordinate
	float x;
	// y-component of top left coordinate
	float y;
	// width of the box
	float w;
	// height of the box
	float h;
	// score of the box;
	float score;
};

struct Track
{
	std::vector<BoundingBox> boxes;
	float max_score;
	int start_frame;
	int id;
};

// Return the IoU between two boxes
inline float intersectionOverUnion(BoundingBox box1, BoundingBox box2);
// Returns the index of the bounding box with the highest IoU
inline int highestIOU(BoundingBox box, std::vector<BoundingBox> boxes);
// Starts IOUT tracker
std::vector< Track > track_iou(float sigma_l, float sigma_h, float sigma_iou, float t_min,
	std::vector< std::vector<BoundingBox> > detections);
// Give an ID to the result tracks from "track_iou"
// Method useful the way IOU is implemented in Python
//void enumerate_tracks();