
#ifndef FILE_READER_H
#define FILE_READER_H

#include "vehicle.h"
#include "geometry.h"

// generator file paths
extern const char* basedir;
extern const char* files[4];

void readVehiclesFromFiles();

#endif