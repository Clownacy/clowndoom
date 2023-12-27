#include "mathematics.h"

#include <math.h>
#include <stddef.h>

void V3_Add(Vector3* const output, const Vector3* const input_a, const Vector3* const input_b)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = input_a->v[i] + input_b->v[i];
}

void V3_Subtract(Vector3* const output, const Vector3* const input_a, const Vector3* const input_b)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = input_a->v[i] - input_b->v[i];
}

void V3_Multiply(Vector3* const output, const Vector3* const input_a, const Vector3* const input_b)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = input_a->v[i] * input_b->v[i];
}

void V3_Scale(Vector3* const output, const Vector3* const vector, const double scale)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = vector->v[i] * scale;
}

void V3_Reciprocal(Vector3* const output, const Vector3* const input)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = 1.0 / input->v[i];
}

double V3_Length(const Vector3* const vector)
{
	return sqrt(V3_Dot(vector, vector));
}

void V3_Normalise(Vector3* const output, const Vector3* const input)
{
	const double scale = 1.0 / V3_Length(input);
	size_t i;

	for (i = 0; i < CC_COUNT_OF(input->v); ++i)
		output->v[i] = input->v[i] * scale;
}

void V3_MultiplyM4(Vector3* const output_vector, const Vector3* const input_vector, const Matrix4* const matrix)
{
	size_t y;

	for (y = 0; y < CC_COUNT_OF(output_vector->v); ++y)
	{
		size_t x;

		output_vector->v[y] = 0.0;

		for (x = 0; x < CC_COUNT_OF(input_vector->v); ++x)
			output_vector->v[y] += input_vector->v[x] * matrix->m[y][x];
	}
}

void V3_Cross(Vector3* const output_vector, const Vector3* const vector_a, const Vector3* const vector_b)
{
	output_vector->v[0] = vector_a->v[1] * vector_b->v[2] - vector_a->v[2] * vector_b->v[1];
	output_vector->v[1] = vector_a->v[2] * vector_b->v[0] - vector_a->v[0] * vector_b->v[2];
	output_vector->v[2] = vector_a->v[0] * vector_b->v[1] - vector_a->v[1] * vector_b->v[0];
}

double V3_Dot(const Vector3* const vector_a, const Vector3* const vector_b)
{
	double sum;
	size_t i;

	sum = 0.0;

	for (i = 0; i < CC_COUNT_OF(vector_a->v); ++i)
		sum += vector_a->v[i] * vector_b->v[i];

	return sum;
}

void V3_FromIV3(Vector3* const output, const IVector3* const input)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = (double)input->v[i];
}

void V4_MultiplyM4(Vector4* const output_vector, const Vector4* const input_vector, const Matrix4* const matrix)
{
	size_t y;

	for (y = 0; y < CC_COUNT_OF(output_vector->v); ++y)
	{
		size_t x;

		output_vector->v[y] = 0.0;

		for (x = 0; x < CC_COUNT_OF(input_vector->v); ++x)
			output_vector->v[y] += input_vector->v[x] * matrix->m[y][x];
	}
}

void IV3_Add(IVector3* const output, const IVector3* const input_a, const IVector3* const input_b)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = input_a->v[i] + input_b->v[i];
}

void IV3_Subtract(IVector3* const output, const IVector3* const input_a, const IVector3* const input_b)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = input_a->v[i] - input_b->v[i];
}

void IV3_Invert(IVector3* const output, const IVector3* const input)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = -input->v[i];
}

void IV3_FromV3Trunc(IVector3* const output, const Vector3* const input)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = (cc_s32f)input->v[i];
}

void IV3_FromV3ReverseTrunc(IVector3* const output, const Vector3* const input)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
	{
		const float value = input->v[i];
		output->v[i] = (cc_s32f)(value < 0.0 ? floor(value) : ceil(value));
	}
}

void IV3_FromV3Floor(IVector3* const output, const Vector3* const input)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = (cc_s32f)floor(input->v[i]);
}

void IV3_FromV3Ceil(IVector3* const output, const Vector3* const input)
{
	size_t i;

	for (i = 0; i < CC_COUNT_OF(output->v); ++i)
		output->v[i] = (cc_s32f)ceil(input->v[i]);
}

/* Matrix4 */

void M4_Identity(Matrix4* const matrix)
{
	double *matrix_pointer;

	matrix_pointer = &matrix->m[0][0];

	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
}

void M4_RotationX(Matrix4* const matrix, const double angle)
{
	const double angle_sin = sin(angle);
	const double angle_cos = cos(angle);

	double *matrix_pointer;

	matrix_pointer = &matrix->m[0][0];

	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = angle_cos;
	*matrix_pointer++ = -angle_sin;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = angle_sin;
	*matrix_pointer++ = angle_cos;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
}

void M4_RotationY(Matrix4* const matrix, const double angle)
{
	const double angle_sin = sin(angle);
	const double angle_cos = cos(angle);

	double *matrix_pointer;

	matrix_pointer = &matrix->m[0][0];

	*matrix_pointer++ = angle_cos;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = angle_sin;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = -angle_sin;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = angle_cos;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
}

void M4_RotationZ(Matrix4* const matrix, const double angle)
{
	const double angle_sin = sin(angle);
	const double angle_cos = cos(angle);

	double *matrix_pointer;

	matrix_pointer = &matrix->m[0][0];

	*matrix_pointer++ = angle_cos;
	*matrix_pointer++ = -angle_sin;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = angle_sin;
	*matrix_pointer++ = angle_cos;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
}

void M4_Multiply(Matrix4* const output_matrix, const Matrix4* const matrix_a, const Matrix4* const matrix_b)
{
	size_t y;

	for (y = 0; y < CC_COUNT_OF(output_matrix->m); ++y)
	{
		size_t x;

		for (x = 0; x < CC_COUNT_OF(output_matrix->m); ++x)
		{
			size_t i;

			output_matrix->m[y][x] = 0.0;

			for (i = 0; i < CC_COUNT_OF(output_matrix->m); ++i)
				output_matrix->m[y][x] += matrix_a->m[y][i] * matrix_b->m[i][x];
		}
	}
}

void M4_Translation(Matrix4* const matrix, const Vector3* const vector)
{
	double *matrix_pointer;
	const double *vector_pointer;

	matrix_pointer = &matrix->m[0][0];
	vector_pointer = &vector->v[0];

	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = *vector_pointer++;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = *vector_pointer++;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
	*matrix_pointer++ = *vector_pointer++;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
}

void M4_Scale(Matrix4* const matrix, const double x, const double y, const double z)
{
	double *matrix_pointer;

	matrix_pointer = &matrix->m[0][0];

	*matrix_pointer++ = x;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = y;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = z;
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
}

void M4_Frustrum(Matrix4* const matrix, const double left, const double right, const double top, const double bottom, const double near, const double far)
{
	double *matrix_pointer;

	matrix_pointer = &matrix->m[0][0];

	*matrix_pointer++ = (2.0 * near) / (right - left);
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = (right + left) / (right - left);
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = (2.0 * near) / (top - bottom);
	*matrix_pointer++ = (top + bottom) / (top - bottom);
	*matrix_pointer++ = 0.0;

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = -((far + near) / (far - near));
	*matrix_pointer++ = -((2.0 * far * near) / (far - near));

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = -1.0;
	*matrix_pointer++ = 0.0;
}

void M4_Perspective(Matrix4* const matrix, const double field_of_vision, const double aspect_ratio, const double near, const double far)
{
	const double top = tan(CC_DEGREE_TO_RADIAN(field_of_vision) / 2.0) * near;
	const double right = top * aspect_ratio;

	M4_Frustrum(matrix, -right, right, top, -top, near, far);
}

void M4_Ortho(Matrix4* const matrix, const double left, const double right, const double top, const double bottom, const double near, const double far)
{
	double *matrix_pointer;

	matrix_pointer = &matrix->m[0][0];

	*matrix_pointer++ = 2.0 / (right - left);
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = -((right + left) / (right - left));

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 2.0 / (top - bottom);
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = -((top + bottom) / (top - bottom));

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = -2.0 / (far - near);
	*matrix_pointer++ = -((far + near) / (far - near));

	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 0.0;
	*matrix_pointer++ = 1.0;
}
