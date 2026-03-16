#pragma once

#include "deepnest.h"
#include <string>
#include <vector>

std::vector<std::string> validate(const std::vector<deepnest::Polygon> &origPolys, const std::vector<deepnest::Polygon> &placed);