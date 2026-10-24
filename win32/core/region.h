/*
 * region.h:  Regions (HRGN) as sets of non-overlapping rectangles: update
 *   regions, clipping, and the region API.
 */

#ifndef W32_CORE_REGION_H
#define W32_CORE_REGION_H

#include "core/win32.h"

#include <vector>

namespace w32 {

class Region
{
public:
   Region() = default;
   explicit Region(const RECT &r) { Add(r); }

   bool Empty() const { return rects_.empty(); }
   const std::vector<RECT> &Rects() const { return rects_; }
   RECT Bounds() const;
   bool Contains(int x, int y) const;
   bool Intersects(const RECT &r) const;

   void Clear() { rects_.clear(); }
   void Add(const RECT &r);                 // union
   void Add(const Region &r);
   void Subtract(const RECT &r);
   void Subtract(const Region &r);
   void Intersect(const RECT &r);
   void Intersect(const Region &r);
   void Offset(int dx, int dy);

private:
   std::vector<RECT> rects_;                // disjoint, non-empty
};

// The region object behind hrgn (a GDI object), or nullptr.
Region *GetRegion(HRGN hrgn);

}  // namespace w32

#endif
