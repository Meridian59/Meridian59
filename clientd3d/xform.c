// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#include "client.h"

void XformMatrixPerspective(D3DMATRIX *matrix, float fovHor, float fovVert, float nearZ, float farZ)
{
	float	w, h, q;

	MatrixZero(matrix);

	w = 1.0f / (float)tan(fovHor * 0.5f);
	h = 1.0f / (float)tan(fovVert * 0.5f);
	q = farZ / (farZ - nearZ);

	matrix->_11 = w;
	matrix->_22 = h;
	matrix->_33 = q;
	matrix->_43 = -q * nearZ;
	matrix->_34 = 1;
}
