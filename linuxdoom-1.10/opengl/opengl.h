#ifndef OPENGL_H
#define OPENGL_H

#include <stddef.h>

#include "glad/include/glad/glad.h"

#include "../doomtype.h"
#include "../m_fixed.h"

typedef struct OpenGL_Texture
{
	GLuint handle;
	int width, height;
} OpenGL_Texture;

struct line_s;
struct seg_s;
struct sector_s;
struct subsector_s;

void OpenGL_Initialise(void);
void OpenGL_Deinitialise(void);
void OpenGL_Clear(void);
void OpenGL_Render(void);
void OpenGL_DrawLine(struct seg_s *s_line);
void OpenGL_DrawLineFlats(const struct sector_s *sector, struct line_s *line);
void OpenGL_DrawRect(int x1, int x2, int y1, int y2, const OpenGL_Texture *hw_texture);
void OpenGL_DrawThing(fixed_t x, fixed_t y, fixed_t z, float angle, const OpenGL_Texture *hw_texture, d_bool flip);
void OpenGL_DrawSectorFlats(const struct sector_s* const sector);
void OpenGL_DrawSubSectorFlats(const struct subsector_s *subsector);
void OpenGL_LoadTexture(OpenGL_Texture *hw_texture, const short *pixels, size_t width, size_t height);
void OpenGL_PatchToTexture(OpenGL_Texture *hw_texture, const int patch_number);
void OpenGL_TextureToHWTexture(OpenGL_Texture *hw_texture, const int texture_number);
void OpenGL_UploadZLight(const unsigned char *zlight);

#endif /* OPENGL_H */
