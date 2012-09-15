// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#ifndef	MATRIX_H
#define	MATRIX_H

void	MatrixZero(D3DMATRIX *matrix);
void	MatrixIdentity(D3DMATRIX *matrix);
void	MatrixCopy(D3DMATRIX *result, D3DMATRIX *matrix);
void	MatrixRotateX(D3DMATRIX *matrix, float angle);
void	MatrixRotateY(D3DMATRIX *matrix, float angle);
void	MatrixRotateZ(D3DMATRIX *matrix, float angle);
void	MatrixTranslate(D3DMATRIX *matrix, float dx, float dy, float dz);
void	MatrixScale(D3DMATRIX *matrix, float dw);
void	MatrixTranspose(D3DMATRIX *result, D3DMATRIX *matrix);
void	MatrixMultiply(D3DMATRIX *result, D3DMATRIX *matrixA, D3DMATRIX *matrixB);
void	MatrixMultiplyVector(struct custom_xyzw *pResult, D3DMATRIX *pMatrix,
							 struct custom_xyzw *pVector);

#endif