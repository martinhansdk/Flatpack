#pragma once

#include "deepnest.h"

namespace writer {

typedef int Color;

class FileWriter {
  public:
    virtual void line(deepnest::Point p1, deepnest::Point p2, Color color = 0) = 0;
    // Called before/after each part's geometry to allow grouping in output formats.
    // Default implementations are no-ops (e.g. DXF does not support groups).
    virtual void beginGroup(const string &id) {}
    virtual void endGroup() {}
};

}