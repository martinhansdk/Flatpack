#ifndef _DXFWRITER_H_
#define _DXFWRITER_H_

#include "Nester.h"

class DXFWriter : public FileWriter {
 public:
  virtual void line(point_t p1, point_t p2, int color = 0);

}

#endif
