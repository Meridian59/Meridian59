// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#define STRICT
#include <math.h>
//#include "d3drender.h"
//#include "matrix.h"
#include "client.h"

void MatrixZero(D3DMATRIX *matrix)
{
	matrix->_11 = 0;	matrix->_12 = 0;	matrix->_13 = 0;	matrix->_14 = 0;
	matrix->_21 = 0;	matrix->_22 = 0;	matrix->_23 = 0;	matrix->_24 = 0;
	matrix->_31 = 0;	matrix->_32 = 0;	matrix->_33 = 0;	matrix->_34 = 0;
	matrix->_41 = 0;	matrix->_42 = 0;	matrix->_43 = 0;	matrix->_44 = 0;
}

void MatrixIdentity(D3DMATRIX *matrix)
{
	MatrixZero(matrix);

	matrix->_11 = 1;
	matrix->_22 = 1;
	matrix->_33 = 1;
	matrix->_44 = 1;
}

void MatrixCopy(D3DMATRIX *result, D3DMATRIX *matrix)
{
	result->_11 = matrix->_11;
	result->_12 = matrix->_12;
	result->_13 = matrix->_13;
	result->_14 = matrix->_14;

	result->_21 = matrix->_21;
	result->_22 = matrix->_22;
	result->_23 = matrix->_23;
	result->_24 = matrix->_24;

	result->_31 = matrix->_31;
	result->_32 = matrix->_32;
	result->_33 = matrix->_33;
	result->_34 = matrix->_34;

	result->_41 = matrix->_41;
	result->_42 = matrix->_42;
	result->_43 = matrix->_43;
	result->_44 = matrix->_44;
}

void MatrixRotateX(D3DMATRIX *matrix, float angle)
{
	MatrixIdentity(matrix);

	matrix->_22 = (float)cos((double)angle);
	matrix->_23 = (float)sin((double)angle);
	matrix->_32 = (float)-sin((double)angle);
	matrix->_33 = (float)cos((double)angle);
}

void MatrixRotateY(D3DMATRIX *matrix, float angle)
{
	MatrixIdentity(matrix);

	matrix->_11 = (float)cos((double)angle);
	matrix->_13 = (float)-sin((double)angle);
	matrix->_31 = (float)sin((double)angle);
	matrix->_33 = (float)cos((double)angle);
}

void MatrixRotateZ(D3DMATRIX *matrix, float angle)
{
	MatrixIdentity(matrix);

	matrix->_11 = (float)cos((double)angle);
	matrix->_12 = (float)sin((double)angle);
	matrix->_21 = (float)-sin((double)angle);
	matrix->_22 = (float)cos((double)angle);
}

void MatrixTranslate(D3DMATRIX *matrix, float dx, float dy, float dz)
{
	MatrixIdentity(matrix);

	matrix->_41 = dx;
	matrix->_42 = dy;
	matrix->_43 = dz;
}

void MatrixScale(D3DMATRIX *matrix, float dw)
{
	MatrixIdentity(matrix);

	matrix->_11 = dw;
	matrix->_22 = dw;
	matrix->_33 = dw;
}

void MatrixTranspose(D3DMATRIX *result, D3DMATRIX *matrix)
{
	D3DMATRIX	temp;

	MatrixCopy(&temp, matrix);

	temp._12 = matrix->_21;
	temp._13 = matrix->_31;
	temp._14 = matrix->_41;

	temp._21 = matrix->_12;
	temp._23 = matrix->_32;
	temp._24 = matrix->_42;

	temp._31 = matrix->_13;
	temp._32 = matrix->_23;
	temp._34 = matrix->_43;

	temp._41 = matrix->_14;
	temp._42 = matrix->_24;
	temp._43 = matrix->_34;

	MatrixCopy(result, &temp);
}

void MatrixMultiply(D3DMATRIX *result, D3DMATRIX *matrixA, D3DMATRIX *matrixB)
{
	D3DMATRIX	temp;

	// row 0
	temp._11 = matrixA->_11 * matrixB->_11 +
					matrixA->_12 * matrixB->_21 +
					matrixA->_13 * matrixB->_31 +
					matrixA->_14 * matrixB->_41;

	temp._12 = matrixA->_11 * matrixB->_12 +
					matrixA->_12 * matrixB->_22 +
					matrixA->_13 * matrixB->_32 +
					matrixA->_14 * matrixB->_42;

	temp._13 = matrixA->_11 * matrixB->_13 +
					matrixA->_12 * matrixB->_23 +
					matrixA->_13 * matrixB->_33 +
					matrixA->_14 * matrixB->_43;

	temp._14 = matrixA->_11 * matrixB->_14 +
					matrixA->_12 * matrixB->_24 +
					matrixA->_13 * matrixB->_34 +
					matrixA->_14 * matrixB->_44;

	// row 1
	temp._21 = matrixA->_21 * matrixB->_11 +
					matrixA->_22 * matrixB->_21 +
					matrixA->_23 * matrixB->_31 +
					matrixA->_24 * matrixB->_41;

	temp._22 = matrixA->_21 * matrixB->_12 +
					matrixA->_22 * matrixB->_22 +
					matrixA->_23 * matrixB->_32 +
					matrixA->_24 * matrixB->_42;

	temp._23 = matrixA->_21 * matrixB->_13 +
					matrixA->_22 * matrixB->_23 +
					matrixA->_23 * matrixB->_33 +
					matrixA->_24 * matrixB->_43;

	temp._24 = matrixA->_21 * matrixB->_14 +
					matrixA->_22 * matrixB->_24 +
					matrixA->_23 * matrixB->_34 +
					matrixA->_24 * matrixB->_44;

	// row 2
	temp._31 = matrixA->_31 * matrixB->_11 +
					matrixA->_32 * matrixB->_21 +
					matrixA->_33 * matrixB->_31 +
					matrixA->_34 * matrixB->_41;

	temp._32 = matrixA->_31 * matrixB->_12 +
					matrixA->_32 * matrixB->_22 +
					matrixA->_33 * matrixB->_32 +
					matrixA->_34 * matrixB->_42;

	temp._33 = matrixA->_31 * matrixB->_13 +
					matrixA->_32 * matrixB->_23 +
					matrixA->_33 * matrixB->_33 +
					matrixA->_34 * matrixB->_43;

	temp._34 = matrixA->_31 * matrixB->_14 +
					matrixA->_32 * matrixB->_24 +
					matrixA->_33 * matrixB->_34 +
					matrixA->_34 * matrixB->_44;

	// row 3
	temp._41 = matrixA->_41 * matrixB->_11 +
					matrixA->_42 * matrixB->_21 +
					matrixA->_43 * matrixB->_31 +
					matrixA->_44 * matrixB->_41;

	temp._42 = matrixA->_41 * matrixB->_12 +
					matrixA->_42 * matrixB->_22 +
					matrixA->_43 * matrixB->_32 +
					matrixA->_44 * matrixB->_42;

	temp._43 = matrixA->_41 * matrixB->_13 +
					matrixA->_42 * matrixB->_23 +
					matrixA->_43 * matrixB->_33 +
					matrixA->_44 * matrixB->_43;

	temp._44 = matrixA->_41 * matrixB->_14 +
					matrixA->_42 * matrixB->_24 +
					matrixA->_43 * matrixB->_34 +
					matrixA->_44 * matrixB->_44;

	MatrixCopy(result, &temp);
}

void MatrixMultiplyVector(custom_xyzw *pResult, D3DMATRIX *pMatrix,
						  custom_xyzw *pVector)
{
	custom_xyzw	temp;

	temp.x = pVector->x * pMatrix->_11 +
		pVector->y * pMatrix->_21 +
		pVector->z * pMatrix->_31 +
		pVector->w * pMatrix->_41;

	temp.y = pVector->x * pMatrix->_12 +
		pVector->y * pMatrix->_22 +
		pVector->z * pMatrix->_32 +
		pVector->w * pMatrix->_42;

	temp.z = pVector->x * pMatrix->_13 +
		pVector->y * pMatrix->_23 +
		pVector->z * pMatrix->_33 +
		pVector->w * pMatrix->_43;

	temp.w = pVector->x * pMatrix->_14 +
		pVector->y * pMatrix->_24 +
		pVector->z * pMatrix->_34 +
		pVector->w * pMatrix->_44;

	pResult->x = temp.x;
	pResult->y = temp.y;
	pResult->z = temp.z;
	pResult->w = temp.w;
}
