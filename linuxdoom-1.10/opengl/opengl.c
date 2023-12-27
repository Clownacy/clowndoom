#include "opengl.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "../i_system.h"
#include "../m_swap.h"
#include "../r_main.h"
#include "../r_state.h"
#include "../v_video.h"
#include "../w_wad.h"
#include "../z_zone.h"

#include "mathematics.h"

#define DO_QUAD(v1_x, v1_y, v1_z, v2_x, v2_y, v2_z, t_left, t_top, t_right, t_bottom) \
{ \
	Vertex vertices[4]; \
 \
	vertices[0].texture_coordinate[0] = t_bottom; \
	vertices[0].texture_coordinate[1] = t_left; \
	vertices[0].position[0] = v1_x; \
	vertices[0].position[1] = v1_y; \
	vertices[0].position[2] = v1_z; \
 \
	vertices[1].texture_coordinate[0] = t_bottom; \
	vertices[1].texture_coordinate[1] = t_right; \
	vertices[1].position[0] = v2_x; \
	vertices[1].position[1] = v1_y; \
	vertices[1].position[2] = v2_z; \
 \
	vertices[2].texture_coordinate[0] = t_top; \
	vertices[2].texture_coordinate[1] = t_left; \
	vertices[2].position[0] = v1_x; \
	vertices[2].position[1] = v2_y; \
	vertices[2].position[2] = v1_z; \
 \
	vertices[3].texture_coordinate[0] = t_top; \
	vertices[3].texture_coordinate[1] = t_right; \
	vertices[3].position[0] = v2_x; \
	vertices[3].position[1] = v2_y; \
	vertices[3].position[2] = v2_z; \
 \
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW); \
 \
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); \
}

typedef struct Vertex
{
	GLfloat position[3];
	GLfloat fake_position[3];
	GLfloat texture_coordinate[2];
} Vertex;

enum
{
	ATTRIBUTE_POSITION,
	ATTRIBUTE_FAKE_POSITION,
	ATTRIBUTE_TEXTURE_COORDINATE
};

static GLuint vertex_array_object, vertex_buffer_object, shader_program, palette_texture, colour_map_texture, zlight_texture;
static GLint uniform_model_view, uniform_projection, uniform_wall_plane_position, uniform_wall_plane_direction;
short y_rotate;

static const char vertex_shader[] = "\
#version 330 core\n\
\n\
uniform mat4 model_view;\n\
uniform mat4 projection;\n\
\n\
in vec3 position;\n\
in vec3 fake_position;\n\
in vec2 texture_coordinate;\n\
\n\
out vec3 fragment_position;\n\
out vec2 fragment_texture_coordinate;\n\
\n\
void main()\n\
{\n\
	vec4 view_space_position = vec4(position, 1.0) * model_view;\n\
	gl_Position =  view_space_position * projection;\n\
	fragment_position = view_space_position.xyz;\n\
	fragment_texture_coordinate = texture_coordinate;\n\
}\n\
";

static const char fragment_shader[] = "\
#version 330 core\n\
\n\
uniform mat4 projection;\n\
uniform isampler2D texture_sampler;\n\
uniform usampler2D colour_map_sampler, zlight_sampler;\n\
uniform sampler2D palette_sampler;\n\
uniform vec3 wall_plane_position;\n\
uniform vec3 wall_plane_direction;\n\
\n\
in vec3 fragment_position;\n\
in vec2 fragment_texture_coordinate;\n\
out vec4 output_colour;\n\
\n\
void main()\n\
{\n\
	int index = texture(texture_sampler, fragment_texture_coordinate).r;\n\
\n\
	if (index == 0)\n\
		discard;\n\
\n\
	uint dark = texture(zlight_sampler, vec2(max(0.0, min(1.0, (gl_FragCoord.z / 1.0) - 0.8)), 12.0 / 16.0)).r;\n\
\n\
	output_colour = texelFetch(palette_sampler, ivec2(texelFetch(colour_map_sampler, ivec2(index - 1, dark), 0).r, 0), 0);\n\
\n\
	if (length(wall_plane_direction) != 0.0)\n\
	{\n\
		vec3 view_direction = normalize(fragment_position);\n\
		vec3 from_plane_to_view = wall_plane_position;\n\
		float shortest_distance_from_view_to_plane = dot(from_plane_to_view, wall_plane_direction);\n\
		float depth_scalar = shortest_distance_from_view_to_plane / dot(view_direction, wall_plane_direction);\n\
		vec3 intersection_point = view_direction * depth_scalar;\n\
		vec4 intersection_point_projected = vec4(intersection_point, 1.0) * projection;\n\
		gl_FragDepth = intersection_point_projected.z / intersection_point_projected.w;\n\
		gl_FragDepth = (gl_FragDepth * gl_DepthRange.diff + gl_DepthRange.far + gl_DepthRange.near) / 2.0;\n\
		gl_FragDepth = intBitsToFloat(floatBitsToInt(gl_FragDepth) - 16);\n\
		gl_FragDepth = min(gl_FragDepth, intBitsToFloat(floatBitsToInt(gl_DepthRange.far) - 1));\n\
	}\n\
	else\n\
	{\n\
		gl_FragDepth = gl_FragCoord.z;\n\
	}\n\
}\n\
";

/*
\n\

	uint dark = texture(zlight_sampler, vec2(min(4.0 / (128.0 * 4.0), 1.0), 0.0)).r;\n\

vec3 view_position = gl_FragCoord.xyz / vec3(2560 / 2, 1600 / 2, 1.0) - vec3(1, 1, 0); \n\
	output_colour = vec4(view_position + vec3(1, 1, 0) / vec3(2, 2, 1), 1.0); \n\

	output_colour.r = gl_FragDepth*gl_FragDepth;\n\
	output_colour.g = 0.3;\n\
	output_colour.b = gl_FragDepth*gl_FragDepth;\n\

		vec3 view_position = fragment_position.xyz;\n\
		view_position = gl_FragCoord.xyz / vec3(2560/2, 1600/2, 1) - vec3(1, 1, 0);\n\
		view_position.z = (view_position.z - 0.5) * 2.0;\n\
		vec3 view_direction = normalize(view_position);\n\
		vec3 from_plane_to_view = wall_plane_position;\n\
		float shortest_distance_from_view_to_plane = dot(from_plane_to_view, wall_plane_direction);\n\
		float depth_scalar = shortest_distance_from_view_to_plane / dot(view_direction, wall_plane_direction);\n\
		gl_FragDepth = view_direction.z * depth_scalar / 2.0 + 0.5;\n\
		gl_FragDepth *= 1.0001;\n\

*/

static void* MallocError(const size_t size)
{
	void* const memory = malloc(size);

	if (memory == NULL)
		I_Error("Failed to allocate memory.");

	return memory;
}

static GLuint LoadShader(const GLenum type, const char* const source, const GLint source_length)
{
	GLuint shader;

	shader = glCreateShader(type);

	if (shader == 0)
	{
		I_Error("glCreateShader failed.");
	}
	else
	{
		GLint log_length;
		char *log_buffer;
		GLint shader_compiled;

		glShaderSource(shader, 1, &source, &source_length);
		glCompileShader(shader);

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

		log_buffer = log_length == 0 ? NULL : (char*)MallocError(log_length);

		if (log_buffer != NULL)
			glGetShaderInfoLog(shader, log_length, NULL, log_buffer);

		glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled);

		if (!shader_compiled)
		{
			I_Error("Failed to compile shader: %s", log_buffer != NULL ? log_buffer : "No log available.");
			free(log_buffer);
		}
		else
		{
			I_Info("Shader info log: %s", log_buffer != NULL ? log_buffer : "No log available.");
			free(log_buffer);
			return shader;
		}

		glDeleteShader(shader);
	}

	return 0;
}

static GLuint LoadProgram(const char* const vertex_source, const GLint vertex_source_length, const char* const fragment_source, const GLint fragment_source_length)
{
	GLuint vertex_shader;

	vertex_shader = LoadShader(GL_VERTEX_SHADER, vertex_source, vertex_source_length);

	if (vertex_shader == 0)
	{
		I_Error("Failed to compile vertex shader.");
	}
	else
	{
		GLuint fragment_shader;

		fragment_shader = LoadShader(GL_FRAGMENT_SHADER, fragment_source, fragment_source_length);

		if (fragment_shader == 0)
		{
			I_Error("Failed to compile fragment shader.");
		}
		else
		{
			GLuint program;

			program = glCreateProgram();

			if (program == 0)
			{
				I_Error("glCreateProgram failed.");
			}
			else
			{
				GLint log_length;
				char *log_buffer;
				GLint program_linked;

				glBindAttribLocation(program, ATTRIBUTE_POSITION, "position");
				glBindAttribLocation(program, ATTRIBUTE_FAKE_POSITION, "fake_position");
				glBindAttribLocation(program, ATTRIBUTE_TEXTURE_COORDINATE, "texture_coordinate");

				glAttachShader(program, vertex_shader);
				glAttachShader(program, fragment_shader);
				glLinkProgram(program);

				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

				log_buffer = log_length == 0 ? NULL : (char*)MallocError(log_length);

				if (log_buffer != NULL)
					glGetProgramInfoLog(program, log_length, NULL, log_buffer);

				glGetProgramiv(program, GL_LINK_STATUS, &program_linked);

				if (!program_linked)
				{
					I_Error("Failed to link program: %s", log_buffer != NULL ? log_buffer : "No log available.");
					free(log_buffer);
				}
				else
				{
					I_Info("Program info log: %s", log_buffer != NULL ? log_buffer : "No log available.");
					free(log_buffer);

					/* Mark the shaders for deletion so that they're automatically
					deleted when the program they're attached to is deleted. */
					glDeleteShader(vertex_shader);
					glDeleteShader(fragment_shader);

					return program;
				}

				glDeleteProgram(program);
			}

			glDeleteShader(fragment_shader);
		}

		glDeleteShader(vertex_shader);
	}

	return 0;
}

static void UploadMatrix(const GLint uniform, const Matrix4* const matrix)
{
	size_t y;
	GLfloat gl_matrix[4][4];

	for (y = 0; y < 4; ++y)
	{
		size_t x;

		for (x = 0; x < 4; ++x)
			gl_matrix[y][x] = matrix->m[y][x];
	}

	glUniformMatrix4fv(uniform, 1, GL_FALSE, &gl_matrix[0][0]);
}

void OpenGL_Initialise(void)
{
	Matrix4 projection_matrix;
	unsigned char *palette, *colour_maps;

	gladLoadGL();
	OpenGL_Clear();

	glEnable(GL_DEPTH_TEST);
	/*glEnable(GL_CULL_FACE);*/
	/*glEnable(GL_TEXTURE_2D);*/ /* Not necessary in Core OpenGL. */
	/*
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	*/
	/*glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NOTEQUAL, (GLfloat)0);*/ /* Does not exist in Core OpenGL. */

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenVertexArrays(1, &vertex_array_object);
	glBindVertexArray(vertex_array_object);
	glGenBuffers(1, &vertex_buffer_object);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);

	glVertexAttribPointer(ATTRIBUTE_POSITION, D_COUNT_OF(((Vertex*)0)->position), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glVertexAttribPointer(ATTRIBUTE_FAKE_POSITION, D_COUNT_OF(((Vertex*)0)->fake_position), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, fake_position));
	glVertexAttribPointer(ATTRIBUTE_TEXTURE_COORDINATE, D_COUNT_OF(((Vertex*)0)->texture_coordinate), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coordinate));

	glEnableVertexAttribArray(ATTRIBUTE_POSITION);
	glEnableVertexAttribArray(ATTRIBUTE_FAKE_POSITION);
	glEnableVertexAttribArray(ATTRIBUTE_TEXTURE_COORDINATE);

	shader_program = LoadProgram(vertex_shader, sizeof(vertex_shader) - 1, fragment_shader, sizeof(fragment_shader));

	glUseProgram(shader_program);

	uniform_model_view = glGetUniformLocation(shader_program, "model_view");
	uniform_projection = glGetUniformLocation(shader_program, "projection");
	uniform_wall_plane_position = glGetUniformLocation(shader_program, "wall_plane_position");
	uniform_wall_plane_direction = glGetUniformLocation(shader_program, "wall_plane_direction");

	glUniform1i(glGetUniformLocation(shader_program, "texture_sampler"), 0);
	glUniform1i(glGetUniformLocation(shader_program, "palette_sampler"), 1);
	glUniform1i(glGetUniformLocation(shader_program, "colour_map_sampler"), 2);
	glUniform1i(glGetUniformLocation(shader_program, "zlight_sampler"), 3);

	M4_Perspective(&projection_matrix, 64.0, (240.0 * 16.0 / 10.0) / 200.0, 10.0, 10000.0);
	UploadMatrix(uniform_projection, &projection_matrix);

	/* Upload colour maps. */
	palette = (unsigned char*)V_GetPalette(NULL)[0];

	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &palette_texture);
	glBindTexture(GL_TEXTURE_2D, palette_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	colour_maps = (unsigned char*)W_CacheLumpName("COLORMAP", PU_STATIC);

	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &colour_map_texture);
	glBindTexture(GL_TEXTURE_2D, colour_map_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 256, 32, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, colour_maps);

	Z_ChangeTag(colour_maps, PU_CACHE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE0);

	Z_Free(palette);

	/* For debugging (finding backfaces and ensuring that lines are rendered instead of segs). */
	/*glPolygonMode(GL_FRONT, GL_LINE);*/ /* Does not work in Core OpenGL. */
}

void OpenGL_Deinitialise(void)
{
	glDeleteProgram(shader_program);
	glDeleteBuffers(1, &vertex_buffer_object);
	glDeleteVertexArrays(1, &vertex_array_object);
}

void OpenGL_Clear(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGL_Render(void)
{
}

static void DoWall(const double top, const double bottom, const double wall_width, const short texture, const seg_t* const line_segment, const d_bool peg_to_bottom, const double y_offset)
{
	const line_t* const line = line_segment->linedef;
	const side_t* const side = line_segment->sidedef;

	/* Correct vertex-winding for back sides. */
	const d_bool is_front_side = &sides[line->sidenum[0]] == side;
	const vertex_t* const v1 = is_front_side ? line->v1 : line->v2;
	const vertex_t* const v2 = is_front_side ? line->v2 : line->v1;

	const short translated_texture = texturetranslation[texture];
	const double texture_width = texturewidth[translated_texture] / (double)FRACUNIT;
	const double texture_height = textureheight[translated_texture] / (double)FRACUNIT;
	const GLfloat texture_coordinate_width = wall_width / texture_width;
	const GLfloat texture_coordinate_height = (top - bottom) / texture_height;
	const GLfloat texture_coordinate_x_offset = side->textureoffset / (double)FRACUNIT / texture_width;
	const GLfloat texture_coordinate_y_offset = ((side->rowoffset / (double)FRACUNIT) + y_offset) / texture_height;
	const GLfloat texture_coordinate_left = texture_coordinate_x_offset;
	const GLfloat texture_coordinate_top = peg_to_bottom ? texture_coordinate_y_offset - texture_coordinate_height : texture_coordinate_y_offset;
	const GLfloat texture_coordinate_right = texture_coordinate_x_offset + texture_coordinate_width;
	const GLfloat texture_coordinate_bottom = peg_to_bottom ? texture_coordinate_y_offset : texture_coordinate_y_offset + texture_coordinate_height;

	glBindTexture(GL_TEXTURE_2D, texturehwhandle[translated_texture].handle);

	DO_QUAD(-v1->x / (double)FRACUNIT, bottom, v1->y / (double)FRACUNIT, -v2->x / (double)FRACUNIT, top, v2->y / (double)FRACUNIT, texture_coordinate_left, texture_coordinate_top, texture_coordinate_right, texture_coordinate_bottom);
}

static void DoFlat(const double height, const line_t* const line, const int flat)
{
#if 0
	(void)height;
	(void)line_segment;
#else
	Vertex vertices[3];

	const double texture_width = 64;
	const double texture_height = 64;
	const double v1_x = -line->v1->x / (double)FRACUNIT;
	const double v1_y = line->v1->y / (double)FRACUNIT;
	const double v2_x = -line->v2->x / (double)FRACUNIT;
	const double v2_y = line->v2->y / (double)FRACUNIT;

	glBindTexture(GL_TEXTURE_2D, flathwhandle[flattranslation[flat]].handle);

	vertices[0].texture_coordinate[0] = -v1_x / texture_width;
	vertices[0].texture_coordinate[1] = -v1_y / texture_height;
	vertices[0].position[0] = v1_x;
	vertices[0].position[1] = height;
	vertices[0].position[2] = v1_y;
	vertices[0].fake_position[0] = v1_x;
	vertices[0].fake_position[1] = height;
	vertices[0].fake_position[2] = v1_y;

	vertices[1].texture_coordinate[0] = viewx / (double)FRACUNIT / texture_width;
	vertices[1].texture_coordinate[1] = -viewy / (double)FRACUNIT / texture_height;
	vertices[1].position[0] = -viewx / (double)FRACUNIT;
	vertices[1].position[1] = height;
	vertices[1].position[2] = viewy / (double)FRACUNIT;
//	vertices[1].position[0] = v2_x;
//	vertices[1].position[1] = height - 100000;
//	vertices[1].position[2] = v2_y;
	vertices[1].fake_position[0] = vertices[0].fake_position[0] + (vertices[2].fake_position[0] - vertices[0].fake_position[0]) / 2;
	vertices[1].fake_position[1] = vertices[0].fake_position[1] + (vertices[2].fake_position[1] - vertices[0].fake_position[1]) / 2;
	vertices[1].fake_position[2] = vertices[0].fake_position[2] + (vertices[2].fake_position[2] - vertices[0].fake_position[2]) / 2;

	vertices[2].texture_coordinate[0] = -v2_x / texture_width;
	vertices[2].texture_coordinate[1] = -v2_y / texture_height;
	vertices[2].position[0] = v2_x;
	vertices[2].position[1] = height;
	vertices[2].position[2] = v2_y;
	vertices[2].fake_position[0] = v2_x;
	vertices[2].fake_position[1] = height;
	vertices[2].fake_position[2] = v2_y;

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
	
	glDrawArrays(GL_TRIANGLES, 0, 3);
#endif
}

void OpenGL_DrawLine(seg_t* const line_segment)
{
	Matrix4 model_view_matrix;
	Vector3 translation_vector;

	const line_t* const line = line_segment->linedef;
	side_t* const side = line_segment->sidedef;

	const double front_ceiling_height = line_segment->frontsector->ceilingheight / (double)FRACUNIT;
	const double front_floor_height = line_segment->frontsector->floorheight / (double)FRACUNIT;
	const d_bool unpeg_bottom = (line->flags & ML_DONTPEGBOTTOM) != 0;
	const double wall_width = sqrt(((double)line->dx * line->dx) + ((double)line->dy * line->dy)) / FRACUNIT;

	/* As an optimisation, we draw whole lines rather than individual segs.
	   Since multiple segs use the same line, we need to prevent the same
	   line from being drawn multiple times. */
	if (side->validcount == validcount)
		return;

	side->validcount = validcount;

	M4_RotationX(&model_view_matrix, CC_DEGREE_TO_RADIAN(y_rotate / 32.0));
	M4_RotateYTo(&model_view_matrix, CC_DEGREE_TO_RADIAN(-(viewangle * 360.0 / 4294967296.0 + 90.0)));
	translation_vector.v[0] = viewx / (double)FRACUNIT;
	translation_vector.v[1] = -viewz / (double)FRACUNIT;
	translation_vector.v[2] = -viewy / (double)FRACUNIT;
	M4_TranslateTo(&model_view_matrix, &translation_vector);

	UploadMatrix(uniform_model_view, &model_view_matrix);

	if (line_segment->backsector == NULL)
	{
		DoWall(front_ceiling_height, front_floor_height, wall_width, side->midtexture, line_segment, unpeg_bottom, 0);
	}
	else
	{
		double mid_top, mid_bottom;

		const double back_ceiling_height = line_segment->backsector->ceilingheight / (double)FRACUNIT;
		const double back_floor_height = line_segment->backsector->floorheight / (double)FRACUNIT;
		const d_bool unpeg_top = (line->flags & ML_DONTPEGTOP) != 0;

		if (front_ceiling_height > back_ceiling_height)
		{
			DoWall(front_ceiling_height, back_ceiling_height, wall_width, side->toptexture, line_segment, !unpeg_top, 0);
			mid_top = back_ceiling_height;
		}
		else
		{
			mid_top = front_ceiling_height;
		}

		if (front_floor_height < back_floor_height)
		{
			DoWall(back_floor_height, front_floor_height, wall_width, side->bottomtexture, line_segment, d_false, unpeg_bottom ? front_ceiling_height - back_floor_height : 0);
			mid_bottom = back_floor_height;
		}
		else
		{
			mid_bottom = front_floor_height;
		}

		if (side->midtexture != 0)
			DoWall(mid_top, mid_bottom, wall_width, side->midtexture, line_segment, unpeg_bottom, 0);
	}

	glUniform3f(uniform_wall_plane_direction, (GLfloat)0, (GLfloat)0, (GLfloat)0);
}

void OpenGL_DrawLineFlats(const struct sector_s* const sector, struct line_s* const line)
{
	Matrix4 model_view_matrix;
	Vector3 translation_vector;

	M4_RotationX(&model_view_matrix, CC_DEGREE_TO_RADIAN(y_rotate / 32.0));
	M4_RotateYTo(&model_view_matrix, CC_DEGREE_TO_RADIAN(-(viewangle * 360.0 / 4294967296.0 + 90.0)));
	translation_vector.v[0] = viewx / (double)FRACUNIT;
	translation_vector.v[1] = -viewz / (double)FRACUNIT;
	translation_vector.v[2] = -viewy / (double)FRACUNIT;
	M4_TranslateTo(&model_view_matrix, &translation_vector);
	
	UploadMatrix(uniform_model_view, &model_view_matrix);

	angle_t             angle1;
	angle_t             angle2;
	angle_t             span;

	/* OPTIMIZE: quickly reject orthogonal back sides. */
	angle1 = R_PointToAngle(line->v1->x, line->v1->y);
	angle2 = R_PointToAngle(line->v2->x, line->v2->y);

	/* Clip to view edges. */
	/* OPTIMIZE: make constant out of 2*clipangle (FIELDOFVIEW). */
	span = angle1 - angle2;

	if (span < ANG180 || line->backsector != NULL)
	{
		double selected_ceiling_height = span < ANG180 ? line->frontsector->ceilingheight / (double)FRACUNIT : line->backsector->ceilingheight / (double)FRACUNIT;
		double selected_floor_height = span < ANG180 ? line->frontsector->floorheight / (double)FRACUNIT : line->backsector->floorheight / (double)FRACUNIT;
		int ceiling_flat = span < ANG180 ? line->frontsector->ceilingpic : line->backsector->ceilingpic;
		int floor_flat = span < ANG180 ? line->frontsector->floorpic : line->backsector->floorpic;

		#if 1
			{
				Vector4 vertex, projected_vectices[3];
				Vector3 deltas[2], cross;

				vertex.v[0] = -(line->v1->x / (double)FRACUNIT);
				vertex.v[1] = 0;
				vertex.v[2] = line->v1->y / (double)FRACUNIT;
				vertex.v[3] = 1.0;
				V4_MultiplyM4(&projected_vectices[0], &vertex, &model_view_matrix);
				//projected_vectices[0] = vertex;

				vertex.v[0] = -(line->v2->x / (double)FRACUNIT);
				vertex.v[1] = 0;
				vertex.v[2] = line->v2->y / (double)FRACUNIT;
				vertex.v[3] = 1.0;
				V4_MultiplyM4(&projected_vectices[1], &vertex, &model_view_matrix);
				//projected_vectices[1] = vertex;

				vertex.v[0] = -(line->v1->x / (double)FRACUNIT);
				vertex.v[1] = 1;
				vertex.v[2] = line->v1->y / (double)FRACUNIT;
				vertex.v[3] = 1.0;
				V4_MultiplyM4(&projected_vectices[2], &vertex, &model_view_matrix);
				//projected_vectices[2] = vertex;

				deltas[0].v[0] = projected_vectices[1].v[0] - projected_vectices[0].v[0];
				deltas[0].v[1] = projected_vectices[1].v[1] - projected_vectices[0].v[1];
				deltas[0].v[2] = projected_vectices[1].v[2] - projected_vectices[0].v[2];
				deltas[1].v[0] = projected_vectices[2].v[0] - projected_vectices[0].v[0];
				deltas[1].v[1] = projected_vectices[2].v[1] - projected_vectices[0].v[1];
				deltas[1].v[2] = projected_vectices[2].v[2] - projected_vectices[0].v[2];

				V3_Cross(&cross, &deltas[0], &deltas[1]);
				V3_NormaliseTo(&cross);

				glUniform3f(uniform_wall_plane_position, projected_vectices[0].v[0], projected_vectices[0].v[1], projected_vectices[0].v[2]);
				glUniform3f(uniform_wall_plane_direction, cross.v[0], cross.v[1], cross.v[2]);
			#if 0
				Vertex vertices[3];

				Matrix4 fuck = M4_IDENTITY;
				UploadMatrix(&fuck);

				glBindTexture(GL_TEXTURE_2D, 1);

				vertices[0].texture_coordinate[0] = 0;
				vertices[0].texture_coordinate[1] = 0;
				vertices[0].position[0] = projected_vectices[0].v[0];
				vertices[0].position[1] = projected_vectices[0].v[1];
				vertices[0].position[2] = projected_vectices[0].v[2];

				vertices[1].texture_coordinate[0] = 0;
				vertices[1].texture_coordinate[1] = 0;
				vertices[1].position[0] = projected_vectices[1].v[0];
				vertices[1].position[1] = projected_vectices[1].v[1];
				vertices[1].position[2] = projected_vectices[1].v[2];

				vertices[2].texture_coordinate[0] = 0;
				vertices[2].texture_coordinate[1] = 0;
				vertices[2].position[0] = projected_vectices[2].v[0];
				vertices[2].position[1] = projected_vectices[2].v[1];
				vertices[2].position[2] = projected_vectices[2].v[2];

				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

				glDrawArrays(GL_TRIANGLES, 0, 3);
			#endif
			}
		#endif

			if (selected_ceiling_height > viewz / (double)FRACUNIT)
				DoFlat(selected_ceiling_height, line, ceiling_flat);

			if (selected_floor_height < viewz / (double)FRACUNIT)
				DoFlat(selected_floor_height, line, floor_flat);
	}

	glUniform3f(uniform_wall_plane_direction, (GLfloat)0, (GLfloat)0, (GLfloat)0);
}

void OpenGL_DrawRect(const int x1, const int x2, const int y1, const int y2, const OpenGL_Texture* const hw_texture)
{
#if 1
	(void)x1;
	(void)x2;
	(void)y1;
	(void)y2;
	(void)hw_texture;
#else
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 320, 240, 0, 0, 1);

	glBindTexture(GL_TEXTURE_2D, hw_texture->handle);
	printf("y1 %d\ny2 %d\n\n", y1, y2);
	glBegin(GL_QUADS);

	glColor3f(1.0f, 1.0f, 1.0f);
	glTexCoord2i(1, 0);
	glVertex2i(x1, y1);
	glTexCoord2i(1, 1);
	glVertex2i(x2, y1);
	glTexCoord2i(0, 1);
	glVertex2i(x2, y2);
	glTexCoord2i(0, 0);
	glVertex2i(x1, y2);

	glEnd();
#endif
}

void OpenGL_DrawThing(const fixed_t x, const fixed_t y, const fixed_t z, const float angle, const OpenGL_Texture* const hw_texture, const d_bool flip)
{
	Matrix4 model_view_matrix;
	Vector3 translation_vector;

	glBindTexture(GL_TEXTURE_2D, hw_texture->handle);

	M4_RotationX(&model_view_matrix, CC_DEGREE_TO_RADIAN(y_rotate / 32.0));
	M4_RotateYTo(&model_view_matrix, CC_DEGREE_TO_RADIAN(-(angle - 90.0)));
	translation_vector.v[0] = (x - viewx) / (double)FRACUNIT;
	translation_vector.v[1] = (z - viewz) / (double)FRACUNIT;
	translation_vector.v[2] = -(y - viewy) / (double)FRACUNIT;
	M4_TranslateTo(&model_view_matrix, &translation_vector);
	M4_RotateYTo(&model_view_matrix, CC_DEGREE_TO_RADIAN(angle - 90.0));

	UploadMatrix(uniform_model_view, &model_view_matrix);

	DO_QUAD(-hw_texture->height / 2, -hw_texture->width / 2, 0, hw_texture->height / 2, hw_texture->width / 2, 0, flip ? 1 : 0, 0, flip ? 0 : 1, 1);
}

void OpenGL_DrawSectorFlats(const struct sector_s* const sector)
{
	int i;

	for (i = 0; i < sector->linecount; ++i)
	{
		OpenGL_DrawLineFlats(sector, sector->lines[i]);
	}
}

void OpenGL_DrawSubSectorFlats(const subsector_t* const subsector)
{
#if 1
	(void)subsector;
#else
	int i;

	const seg_t* const segments = &segs[subsector->firstline];
	const vertex_t* const v1 = segments[0].v1;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(-(viewangle * 360.0f / 4294967296.0f + 90.0f), 0.0f, 1.0f, 0.0f);
	glTranslatef(viewx / (GLfloat)FRACUNIT, -viewz / (GLfloat)FRACUNIT, -viewy / (GLfloat)FRACUNIT);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);

	glBegin(GL_TRIANGLES);

	glColor3f(1.0f, 0.0f, 1.0f);

	for (i = 1; i < subsector->numlines; ++i)
	{
		const vertex_t* const v2 = segments[i].v1;
		const vertex_t* const v3 = segments[i].v2;

		glColor3f(1.0f * i / subsector->numlines, 0.0f, 0.0f);

		glVertex3i(-v3->x >> FRACBITS, subsector->sector->floorheight >> FRACBITS, v3->y >> FRACBITS);
		glVertex3i(-v2->x >> FRACBITS, subsector->sector->floorheight >> FRACBITS, v2->y >> FRACBITS);
		glVertex3i(-v1->x >> FRACBITS, subsector->sector->floorheight >> FRACBITS, v1->y >> FRACBITS);

		glVertex3i(-v1->x >> FRACBITS, subsector->sector->ceilingheight >> FRACBITS, v1->y >> FRACBITS);
		glVertex3i(-v2->x >> FRACBITS, subsector->sector->ceilingheight >> FRACBITS, v2->y >> FRACBITS);
		glVertex3i(-v3->x >> FRACBITS, subsector->sector->ceilingheight >> FRACBITS, v3->y >> FRACBITS);

		if (segments[i].v2->x != segments[(i + 1) % subsector->numlines].v1->x || segments[i].v2->y != segments[(i + 1) % subsector->numlines].v1->y)
		{
			const vertex_t* const v2 = segments[i].v2;
			const vertex_t* const v3 = segments[(i + 1) % subsector->numlines].v1;

			glVertex3i(-v3->x >> FRACBITS, subsector->sector->floorheight >> FRACBITS, v3->y >> FRACBITS);
			glVertex3i(-v2->x >> FRACBITS, subsector->sector->floorheight >> FRACBITS, v2->y >> FRACBITS);
			glVertex3i(-v1->x >> FRACBITS, subsector->sector->floorheight >> FRACBITS, v1->y >> FRACBITS);

			glVertex3i(-v1->x >> FRACBITS, subsector->sector->ceilingheight >> FRACBITS, v1->y >> FRACBITS);
			glVertex3i(-v2->x >> FRACBITS, subsector->sector->ceilingheight >> FRACBITS, v2->y >> FRACBITS);
			glVertex3i(-v3->x >> FRACBITS, subsector->sector->ceilingheight >> FRACBITS, v3->y >> FRACBITS);
		}
	}

	glEnd();

	glPushMatrix();
#endif
}

void OpenGL_LoadTexture(OpenGL_Texture* const hw_texture, const short* const pixels, const size_t width, const size_t height)
{
	hw_texture->width = width;
	hw_texture->height = height;
	glGenTextures(1, &hw_texture->handle);
	glBindTexture(GL_TEXTURE_2D, hw_texture->handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16I, width, height, 0, GL_RED_INTEGER, GL_SHORT, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
/*	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);*/
}

static void
DrawColumnInCache
( column_t*      patch,
  short*         cache,
  int            originy,
  int            cacheheight )
{
	int            count;
	int            position;
	unsigned char* source;

	while (patch->topdelta != 0xff)
	{
		int i;

		source = (unsigned char *)patch + 3;
		count = patch->length;
		position = originy + patch->topdelta;

		if (position < 0)
		{
			count += position;
			position = 0;
		}

		if (position + count > cacheheight)
			count = cacheheight - position;

		for (i = 0; i < count; ++i)
			cache[position + i] = source[i] + 1;

		patch = (column_t *)(  (unsigned char *)patch + patch->length + 4);
	}
}

void OpenGL_PatchToTexture(OpenGL_Texture* const hw_texture, const int patch_number)
{
	int x;

	const patch_t* const patch = (patch_t*)W_CacheLumpNum(patch_number, PU_CACHE);
	const int width = SHORT(patch->width);
	const int height = SHORT(patch->height);
	short* const buffer = (short*)Z_Malloc(width * height * sizeof(short), PU_STATIC, NULL);

	memset(buffer, 0, width * height * sizeof(short));

	for (x = 0; x < width; ++x)
	{
		column_t* const patchcol = (column_t *)((unsigned char *)patch + LONG(patch->columnofs[x]));

		DrawColumnInCache(patchcol, &buffer[x * height], 0, height);
	}

	OpenGL_LoadTexture(hw_texture, buffer, height, width);

	Z_Free(buffer);
}

void OpenGL_TextureToHWTexture(OpenGL_Texture* const hw_texture, const int texture_number)
{
	short*              block;
	texture_t*          texture;
	texpatch_t*         patch;
	patch_t*            realpatch;
	int                 x;
	int                 x1;
	int                 x2;
	int                 i;
	column_t*           patchcol;

	texture = textures[texture_number];

	block = (short*)Z_Malloc(texture->width * texture->height * sizeof(short),
		PU_STATIC,
		NULL);

	memset(block, 0, texture->width * texture->height * sizeof(short));

	/* Composite the columns together. */
	patch = texture->patches;

	for (i = 0, patch = texture->patches;
		i < texture->patchcount;
		i++, patch++)
	{
		realpatch = (patch_t*)W_CacheLumpNum(patch->patch, PU_CACHE);
		x1 = patch->originx;
		x2 = x1 + SHORT(realpatch->width);

		if (x1 < 0)
			x = 0;
		else
			x = x1;

		if (x2 > texture->width)
			x2 = texture->width;

		for (; x < x2; x++)
		{
			patchcol = (column_t *)((unsigned char *)realpatch
				+ LONG(realpatch->columnofs[x - x1]));
			DrawColumnInCache(patchcol,
				&block[x * texture->height],
				patch->originy,
				texture->height);
		}
	}

	OpenGL_LoadTexture(hw_texture, block, texture->height, texture->width);

	Z_Free(block);
}

void OpenGL_UploadZLight(const unsigned char* const zlight)
{
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &zlight_texture);
	glBindTexture(GL_TEXTURE_2D, zlight_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, MAXLIGHTZ, LIGHTLEVELS, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, zlight);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE0);
}
