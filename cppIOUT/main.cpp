/*
* Example of using IOU Tracker C++ Implementation
* Author Lucas Wals
*/
#pragma once
#include <fstream>
#include <cstdio>
#include <string>
#include <iomanip>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "dirent.h"
#include "IOUT.h"
#include "UA-DETRAC.h"

/******************************************************************************
* DEFINE SECTION
******************************************************************************/

#define DEFAULT_OUTPUT "results"
#define DEFAULT_DETECTIONS_TYPE "DETRAC"
#define DEFAULT_DETECTIONS_FILE "detections.txt"
#define SHOW_BOXES 1 // Program will output boxes after finish tracking
#define SAVE_BOXES 1 // Save the boxes on image file
// Turn on/off features depending if this will be embedded in UA-DETRAC toolkit or not
#define USE_IN_DETRAC 0

/******************************************************************************
* EXTRA FUNCTIONS
******************************************************************************/
int pgm_select(const struct dirent *entry)
{
	return strstr(entry->d_name, ".pgm") != NULL;
}

int ppm_select(const struct dirent *entry)
{
	return strstr(entry->d_name, ".ppm") != NULL;
}

int jpg_select(const struct dirent *entry)
{
	return strstr(entry->d_name, ".jpg") != NULL;
}

/******************************************************************************
* MAIN
******************************************************************************/

int main(int argc, char *argv[])
{
	std::cout << "IOU C++ implementation" << std::endl;

	// General variables
	std::string input_folder;
	std::string output_folder = DEFAULT_OUTPUT;
	std::string sequence;
	char* detections_type = DEFAULT_DETECTIONS_TYPE;
	char* detections_file = DEFAULT_DETECTIONS_FILE;
	// Image files variables
	struct dirent **filelist;
	int fcount = -1;
	std::vector< Track > drawing_tracks;	// List of Tracks that are being drawed
	// IOUT necessary variables
	std::vector< std::vector<BoundingBox> > detections;	// list of detections
	std::vector< Track > tracks;	// list of resulting tracks
	// TODO: Make a setting struct or initilization for different Detectors!
	// Tracker Configuration for CompACT detector
	float sigma_l = 0;		// low detection threshold
	float sigma_h = 0.2;		// high detection threshold
	float sigma_iou = 0.5;	// IOU threshold
	float t_min = 2;		// minimum track length in frames

	/// Read arguments
#if USE_IN_DETRAC
	// executable_name sequence input_folder
	if (argc >= 2)
	{
		sequence = argv[1];
		input_folder = argv[2];
	}
	else
	{
		std::cout << "Incorrect arguments. See example below: " << std::endl;
		std::cout << "cppIOUT sequence input_folder " << std::endl;
		std::cout << "cppIOUT MVI_20011 C:/Detrac/MVI_20011" << std::endl;
	}
#else 
	// executable_name sequence input_folder det_type det_file output_folder
	if (argc >= 6)
	{
		sequence = argv[1];
		input_folder = argv[2];
		detections_type = argv[3];
		detections_file = argv[4];
		output_folder = argv[5];
	}
	else
	{
		std::cout << "Incorrect arguments. See example below: " << std::endl;
		std::cout << "cppIOUT sequence input_folder detections_from detections_file output_folder" << std::endl;
		std::cout << "cppIOUT MVI_20011 C:/Detrac/MVI_20011 DETRAC C:/Detrac/Detections/MVI_20011.txt C:/Detrac/TrackResults/MVI_20011" << std::endl;
	}
#endif
	

	std::cout << "input folder: " << input_folder << std::endl;

	/// Loading detections part
	// First verify that the detections file exists
	std::ifstream detStream(detections_file);
	if (!detStream || detStream.eof())
	{
		std::cout << "ERROR -> Detection file not found or empty" << std::endl;
//		cv::waitKey(0);
		return 0;
	}
	else
	{
		std::cout << "Detection file found. Loading detections..." << std::endl;
		// Load all detections before starting tracking.
		read_detections(detStream, detections);
	}

	std::cout << "Frames > " << detections.size() << std::endl;
	/*
	std::vector<BoundingBox> test = detections[0];
	for (int i = 0; i < test.size(); i++)
	{
		std::cout << test[i].x << std::endl;
	}
	*/

#if !USE_IN_DETRAC
#if SHOW_BOXES
	/// Looking for the images files
	fcount = scandir(input_folder.c_str(), &filelist, jpg_select, alphasort);
	if (fcount <= 0)
	{
		std::cout << "ERROR -> Input images directory not found or empty" << std::endl;
		cv::waitKey(0);
		return 0;
	}
	else
		std::cout << "Found " << fcount << " images" << std::endl;
#endif // SHOW_BOXES
#endif // NOT USE_IN_DETRAC
	/// Looking for the output directory part. Create it if does not exit
	DIR * dir = opendir(output_folder.c_str());
	if (dir == NULL)
	{
		std::cout << "\tWARNING -> Output folder does not exist -> try to create it" << std::endl;
		if (system(("mkdir " + output_folder).c_str()))
		{
			std::cout << "\tERROR -> Failed to create directory" << std::endl;
			return 0;
		}
	}
	closedir(dir);

	/// Start tracking
	tracks = track_iou(sigma_l, sigma_h, sigma_iou, t_min, detections);
	std::cout << "Last Track ID > " << tracks.back().id << std::endl;

#if !USE_IN_DETRAC
#if SHOW_BOXES
	std::cout << "Displaying results on window..." << std::endl;
	/// Show results on image
	char filename[255];
	cv::Mat image;

	cv::namedWindow("Display Tracking", cv::WINDOW_AUTOSIZE);
	for (int frame = 0; frame < detections.size(); frame++)
	{
		// Load the current image
		sprintf_s(filename, "%s/%s", input_folder.c_str(), filelist[frame]->d_name);
		image = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
		// Grab all the tracks that start in current frame
		for (auto track : tracks)
		{
			if (track.start_frame == frame)
				drawing_tracks.push_back(track);
		}
		// Write all the boxes into the image	
		//std::vector<BoundingBox> frameBoxes = bboxes[frame];
		//for (int j = 0; j < drawing_tracks.size(); j++)
		for (auto dt : drawing_tracks)
		{
			int box_index = frame - dt.start_frame;
			//BoundingBox b = frameBoxes[j];
			if (box_index < dt.boxes.size() )
			{
				BoundingBox b = dt.boxes[box_index];
				cv::rectangle(image, cv::Point(b.x, b.y), cv::Point(b.x + b.w, b.y + b.h), cv::Scalar(0, 0, 255), 2);
				cv::putText(image, std::to_string(dt.id), cv::Point(b.x + b.w - b.w / 2, b.y + b.h - 5), 1, 1, cv::Scalar(0, 255, 255), 2);
			}
//			else
//				drawing_tracks.
		}
#if SAVE_BOXES
		/// Save the images
		sprintf_s(filename, "%s/%s", output_folder.c_str(), filelist[frame]->d_name);
		cv::imwrite(filename, image);
#endif
		imshow("Display Tracking", image);
		cv::waitKey(50);
	}
	std::cout << "Displaying images finished!!" << std::endl;
#endif // SHOW BOXES
#else // Write results only when using in Detrac toolkit
	// detections.size() would be the amount of frames here
	write_results(sequence, output_folder, detections.size(), tracks);
#endif // NOT IN_DETRAC
}