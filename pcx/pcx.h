#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "mat.h"
#include "types.h"

bool load_pcx(const std::string& filename, mat<vec3b>& img);