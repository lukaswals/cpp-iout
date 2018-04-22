/*
* File containing all the related functions to use the
* UA-DETRAC dataset
* More info on <http://detrac-db.rit.albany.edu/>
* Added by Lucas
*/
#pragma once
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iomanip>

#include "IOUT.h"
#include "CSV.h"

//using namespace std;

// Read all the provided detections on a vector of vectors
void read_detections(std::ifstream& file, std::vector< std::vector<BoundingBox> >& detections)
{
	std::vector<BoundingBox> frame_detections;
	int actualFrame = 1;

	// If the column number X doesn't exists, it will do nothing
	for (CSVIterator loop(file); loop != CSVIterator(); ++loop)
	{
		int det_frame = std::stoi((*loop)[0]);
		//std::cout << "Detected in Frame N " << det_frame << "\n";
		while (actualFrame < det_frame) // We read all detections for previous frame
		{
			actualFrame++;
			detections.push_back(frame_detections);
			frame_detections.clear();
		}
		BoundingBox b = { 
			std::stof((*loop)[2]), std::stof((*loop)[3]),
			std::stof((*loop)[4]), std::stof((*loop)[5]), 
			std::stof((*loop)[6]) };
		frame_detections.push_back(b);
	}
	// Last detections are not added in the loop, so we add them here
	detections.push_back(frame_detections);
	// Then we check how 
}

// To output only up to 2 decimals
std::string to_string_with_precision(float value)
{
	int decimals = 0;
	if (value != 0)
		decimals = 2;

	std::ostringstream out;
	out << std::fixed << std::setprecision(decimals) << value;
	return out.str();
}

// Store results in a file in a format that will be loaded later and reshaped
// into a matrix by a matlab script
void write_results(std::string& sequence, std::string& output_folder, int total_frames,
	std::vector< Track >& tracks)
{
	char filename[255];
	sprintf_s(filename, "%s/%s.txt", output_folder, sequence);
	std::ofstream out(filename);
	std::string line;

	std::cout << "Creating result file" << std::endl;
	for (auto t : tracks)
	{
//		std::cout << "Track " << t.id << " ~ started in frame -> " << t.start_frame  << std::endl;
		for (auto b : t.boxes)
		{
			line.append(to_string_with_precision(b.x) + ",");
			line.append(to_string_with_precision(b.y) + ",");
			line.append(to_string_with_precision(b.w) + ",");
			line.append(to_string_with_precision(b.h) + ",");
			line.append( std::to_string(t.start_frame+1) + "," );
			line.append( std::to_string(t.id) );
			// Write to files
			out << line << std::endl;
			// Empty the strings
			line.clear();
		}
	}
	std::cout << "Finish writing result file" << std::endl;
	// Close the file
	out.close();

}
