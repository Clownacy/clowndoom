#ifndef MATHEMATICS_H
#define MATHEMATICS_H

#include "../clownlibs/clowncommon.h"

typedef struct Vector3
{
	double v[3];
} Vector3;

typedef struct Vector4
{
	double v[4];
} Vector4;

typedef struct IVector2
{
	cc_s32f v[2];
} IVector2;

typedef struct IVector3
{
	cc_s32f v[3];
} IVector3;

typedef struct Matrix4
{
	double m[4][4];
} Matrix4;

#define M4_IDENTITY\
	{\
		{\
			{1.0, 0.0, 0.0, 0.0},\
			{0.0, 1.0, 0.0, 0.0},\
			{0.0, 0.0, 1.0, 0.0},\
			{0.0, 0.0, 0.0, 1.0}\
		}\
	}

/* Destination CAN be one of the sources. */
void V3_Add(Vector3 *output, const Vector3 *input_a, const Vector3 *input_b);
void V3_Subtract(Vector3 *output, const Vector3 *input_a, const Vector3 *input_b);
void V3_Multiply(Vector3 *output, const Vector3 *input_a, const Vector3 *input_b);
void V3_Scale(Vector3 *output, const Vector3 *vector, double scale);
#define V3_Invert(destination, source)     V3_Scale(destination, source, -1.0)
void V3_Reciprocal(Vector3 *output, const Vector3 *input);
void V3_Normalise(Vector3 *output, const Vector3 *input);

#define V3_AddTo(destination, source)      V3_Add(destination, destination, source)
#define V3_SubtractTo(destination, source) V3_Subtract(destination, destination, source)
#define V3_MultiplyTo(destination, source) V3_Multiply(destination, destination, source)
#define V3_ScaleTo(destination, source)    V3_Scale(destination, destination, source)
#define V3_InvertTo(destination)           V3_Invert(destination, destination)
#define V3_NormaliseTo(destination)        V3_Normalise(destination, destination)

/* Destination CANNOT be one of the sources. */
void V3_MultiplyM4(Vector3 *output_vector, const Vector3 *input_vector, const Matrix4 *matrix);
void V3_Cross(Vector3 *output_vector, const Vector3 *vector_a, const Vector3 *vector_b);

double V3_Dot(const Vector3 *vector_a, const Vector3 *vector_b);
double V3_Length(const Vector3 *vector);
void V3_FromIV3(Vector3 *output, const IVector3 *input);

void IV3_Add(IVector3 *output, const IVector3 *input_a, const IVector3 *input_b);
void IV3_Subtract(IVector3 *output, const IVector3 *input_a, const IVector3 *input_b);
void IV3_Invert(IVector3 *output, const IVector3 *input);
void IV3_FromV3Trunc(IVector3 *output, const Vector3 *input);
void IV3_FromV3ReverseTrunc(IVector3 *output, const Vector3 *input);
void IV3_FromV3Floor(IVector3 *output, const Vector3 *input);
void IV3_FromV3Ceil(IVector3 *output, const Vector3 *input);

#define IV3_AddTo(destination, source)      IV3_Add(destination, destination, source)
#define IV3_SubtractTo(destination, source) IV3_Subtract(destination, destination, source)
#define IV3_ScaleTo(destination, source)    IV3_Scale(destination, destination, source)
#define IV3_InvertTo(destination)           IV3_Invert(destination, destination)

/* Destination CANNOT be one of the sources. */
void V4_MultiplyM4(Vector4 *output_vector, const Vector4 *input_vector, const Matrix4 *matrix);

void M4_Identity(Matrix4 *matrix);
void M4_RotationX(Matrix4 *matrix, double angle);
void M4_RotationY(Matrix4 *matrix, double angle);
void M4_RotationZ(Matrix4 *matrix, double angle);
void M4_Multiply(Matrix4 *output_matrix, const Matrix4 *matrix_a, const Matrix4 *matrix_b);
void M4_Translation(Matrix4 *matrix, const Vector3 *vector);
void M4_Scale(Matrix4 *matrix, double x, double y, double z);

#define M4_Rotate(axis, output_matrix, input_matrix, angle) \
{ \
	Matrix4 rotation_matrix; \
	M4_Rotation##axis(&rotation_matrix, angle); \
	M4_Multiply(output_matrix, input_matrix, &rotation_matrix); \
}

#define M4_RotateTo(axis, matrix, angle) \
{ \
	Matrix4 output_matrix; \
	M4_Rotate(axis, &output_matrix, matrix, angle); \
	*(matrix) = output_matrix; \
}

#define M4_RotateX(output_matrix, input_matrix, angle) M4_Rotate(X, output_matrix, input_matrix, angle)
#define M4_RotateY(output_matrix, input_matrix, angle) M4_Rotate(Y, output_matrix, input_matrix, angle)
#define M4_RotateZ(output_matrix, input_matrix, angle) M4_Rotate(Z, output_matrix, input_matrix, angle)
#define M4_RotateXTo(matrix, angle) M4_RotateTo(X, matrix, angle)
#define M4_RotateYTo(matrix, angle) M4_RotateTo(Y, matrix, angle)
#define M4_RotateZTo(matrix, angle) M4_RotateTo(Z, matrix, angle)

#define M4_Translate(output_matrix, input_matrix, vector) \
{ \
	Matrix4 translation_matrix; \
	M4_Translation(&translation_matrix, vector); \
	M4_Multiply(output_matrix, input_matrix, &translation_matrix); \
}

#define M4_TranslateTo(matrix, vector) \
{ \
	Matrix4 output_matrix; \
	M4_Translate(&output_matrix, matrix, vector); \
	*(matrix) = output_matrix; \
}

void M4_Frustrum(Matrix4 *matrix, double left, double right, double top, double bottom, double near, double far);
void M4_Perspective(Matrix4 *matrix, double field_of_vision, double aspect_ratio, double near, double far);
void M4_Ortho(Matrix4 *matrix, double left, double right, double top, double bottom, double near, double far);

#endif /* MATHEMATICS_H */
