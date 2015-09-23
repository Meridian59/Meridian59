#define EPSILON     0.0001f
#define ISZERO(a)   ((a) > -EPSILON && (a) < EPSILON)

typedef struct V3
{
   float X;
   float Y;
   float Z;
} V3;

typedef struct V2
{
   float X;
   float Y;
} V2;

#define V3ADD(a,b,c) \
   (a)->X = (b)->X + (c)->X; \
   (a)->Y = (b)->Y + (c)->Y; \
   (a)->Z = (b)->Z + (c)->Z;

#define V3SUB(a,b,c) \
   (a)->X = (b)->X - (c)->X; \
   (a)->Y = (b)->Y - (c)->Y; \
   (a)->Z = (b)->Z - (c)->Z;

#define V3SCALE(a,b) \
   (a)->X *= b; \
   (a)->Y *= b; \
   (a)->Z *= b;

#define V3CROSS(a,b,c) \
   (a)->X = (b)->Y * (c)->Z - (b)->Z * (c)->Y; \
   (a)->Y = (b)->Z * (c)->X - (b)->X * (c)->Z; \
   (a)->Z = (b)->X * (c)->Y - (b)->Y * (c)->X;

#define V3DOT(a,b) ((a)->X * (b)->X + (a)->Y * (b)->Y + (a)->Z * (b)->Z)
#define V3LEN2(a)  (a)->X*(a)->X + (a)->Y*(a)->Y + (a)->Z*(a)->Z
#define V3LEN(a)   sqrtf(V3LEN2)

#define V2ADD(a,b,c) \
   (a)->X = (b)->X + (c)->X; \
   (a)->Y = (b)->Y + (c)->Y;

#define V2SUB(a,b,c) \
   (a)->X = (b)->X - (c)->X; \
   (a)->Y = (b)->Y - (c)->Y;;

#define V2SCALE(a,b) \
   (a)->X *= b; \
   (a)->Y *= b;

#define V2DOT(a,b) ((a)->X * (b)->X + (a)->Y * (b)->Y)
#define V2LEN2(a)  (a)->X*(a)->X + (a)->Y*(a)->Y
#define V2LEN(a)   sqrtf(V2LEN2)

// true if point (c) lies inside boundingbox defined by min/max of (a) and (b)
#define ISINBOX(a, b, c) \
   (fmin((a)->X, (b)->X) - EPSILON <= (c)->X && (c)->X <= fmax((a)->X, (b)->X) + EPSILON && \
    fmin((a)->Y, (b)->Y) - EPSILON <= (c)->Y && (c)->Y <= fmax((a)->Y, (b)->Y) + EPSILON)

// Möller–Trumbore intersection algorithm
// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
// Returns true if there is an intersection
__inline bool IntersectLineTriangle(V3* P1, V3* P2, V3* P3, V3* S, V3* E)
{
   V3 e1, e2;
   V3 p, q, t;
   float det, inv_det, u, v;
   float f;

   // vector from start to end
   V3 d;
   V3SUB(&d, E, S);

   // find vectors for two edges sharing V1
   V3SUB(&e1, P2, P1);
   V3SUB(&e2, P3, P1);

   // begin calculating determinant - also used to calculate u parameter
   V3CROSS(&p, &d, &e2);

   // if determinant is near zero, ray lies in plane of triangle
   det = V3DOT(&e1, &p);

   if (ISZERO(det))
      return false;

   inv_det = 1.0f / det;

   // calculate distance from V1 to ray origin
   V3SUB(&t, S, P1);

   // calculate u parameter and test bound
   u = V3DOT(&t, &p) * inv_det;

   // the intersection lies outside of the triangle
   if (u < 0.0f || u > 1.0f)
      return false;

   // prepare to test v parameter
   V3CROSS(&q, &t, &e1);

   // calculate v parameter and test bound
   v = V3DOT(&d, &q) * inv_det;

   // the intersection lies outside of the triangle
   if (v < 0.0f || u + v  > 1.0f)
      return false;

   f = V3DOT(&e2, &q) * inv_det;

   // note: we additionally check for < 1.0f
   // = LINE intersection, not ray
   if (f >= 0.0f && f <= 1.0f)
      return true;

   return false;
}

// Returns the minimum squared distance between
// point P and finite line segment given by P1 and P2
__inline float MinSquaredDistanceToLineSegment(V2* P, V2* Q1, V2* Q2)
{
   // finite line vector from start (Q1) to end (Q2)
   V2 d;
   V2SUB(&d, Q2, Q1);

   // squared length of Q1Q2 (squared distance betweem them)
   float len2 = V2LEN2(&d);

   // Q1 is on Q2 (no line)
   if (ISZERO(len2))
   {
      V2SUB(&d, Q1, P);
      return V2LEN2(&d);
   }
	
   V2 v1, v2, v3;
   V2SUB(&v1, P, Q1);
   V2SUB(&v2, Q2, Q1);

   float t = V2DOT(&v1, &v2) / len2;

   // Q1 is closest
   if (t < 0.0f)
   {
      V2SUB(&d, Q1, P);
      return V2LEN2(&d);
   }

   // Q2 is closest
   else if (t > 1.0f)
   {
      V2SUB(&d, Q2, P);
      return V2LEN2(&d);
   }

   // point on line is closest
   else
   {
      V2SCALE(&d, t);
      V2ADD(&v3, Q1, &d);
      V2SUB(&d, &v3, P);

      return V2LEN2(&d);
   }
}