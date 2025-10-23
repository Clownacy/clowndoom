#ifndef LIBRETRO_CORE_OPTIONS_H__
#define LIBRETRO_CORE_OPTIONS_H__

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "libretro.h"

#define HAVE_NO_LANGEXTRA

#ifndef HAVE_NO_LANGEXTRA
#include "libretro_core_options_intl.h"
#endif

/*
 ********************************
 * VERSION: 2.0
 ********************************
 *
 * - 2.0: Add support for core options v2 interface
 * - 1.3: Move translations to libretro_core_options_intl.h
 *        - libretro_core_options_intl.h includes BOM and utf-8
 *          fix for MSVC 2010-2013
 *        - Added HAVE_NO_LANGEXTRA flag to disable translations
 *          on platforms/compilers without BOM support
 * - 1.2: Use core options v1 interface when
 *        RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION is >= 1
 *        (previously required RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION == 1)
 * - 1.1: Support generation of core options v0 retro_core_option_value
 *        arrays containing options with a single value
 * - 1.0: First commit
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
 ********************************
 * Core Option Definitions
 ********************************
*/

/* RETRO_LANGUAGE_ENGLISH */

/* Default language:
 * - All other languages must include the same keys and values
 * - Will be used as a fallback in the event that frontend language
 *   is not available
 * - Will be used as a fallback for any missing entries in
 *   frontend language definition */

struct retro_core_option_v2_category option_cats_us[] = {
	{
		/* Key. */
		"general",
		/* Label. */
		"General",
		/* Description. */
		"Options related to gameplay."
	},
	{
		/* Key. */
		"video",
		/* Label. */
		"Video",
		/* Description. */
		"Options related to graphical output."
	},
	{
		/* Key. */
		"audio",
		/* Label. */
		"Audio",
		/* Description. */
		"Options related to sound output."
	},
	{NULL, NULL, NULL}
};

struct retro_core_option_v2_definition option_defs_us[] = {
	{
		/* Key. */
		"clowndoom_show_messages",
		/* Label. */
		"General > Show Messages",
		/* Categorised label. */
		"Show Messages",
		/* Description. */
		"Display messages in the top-left corner of the screen.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"general",
		/* Values. */
		{
			{"enabled", NULL},
			{"disabled", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"enabled"
	},
	{
		/* Key. */
		"clowndoom_mouse_sensitivity",
		/* Label. */
		"General > Mouse Sensitivity",
		/* Categorised label. */
		"Mouse Sensitivity",
		/* Description. */
		"Adjusts the sensitivity of the first-person aiming.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"general",
		/* Values. */
		{
			{"0", NULL},
			{"1", NULL},
			{"2", NULL},
			{"3", NULL},
			{"4", NULL},
			{"5", NULL},
			{"6", NULL},
			{"7", NULL},
			{"8", NULL},
			{"9", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"5"
	},
	{
		/* Key. */
		"clowndoom_move_with_mouse",
		/* Label. */
		"General > Move with Mouse",
		/* Categorised label. */
		"Move with Mouse",
		/* Description. */
		"Allow the player to move forward and backward with the mouse.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"general",
		/* Values. */
		{
			{"enabled", NULL},
			{"disabled", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"disabled"
	},
	{
		/* Key. */
		"clowndoom_always_run",
		/* Label. */
		"General > Run by Default",
		/* Categorised label. */
		"Run by Default",
		/* Description. */
		"Makes the run button make the player character walk instead.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"general",
		/* Values. */
		{
			{"enabled", NULL},
			{"disabled", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"enabled"
	},
	{
		/* Key. */
		"clowndoom_always_strafe",
		/* Label. */
		"General > Strafe by Default",
		/* Categorised label. */
		"Strafe by Default",
		/* Description. */
		"Makes the left and right buttons make the player character strafe instead.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"general",
		/* Values. */
		{
			{"enabled", NULL},
			{"disabled", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"enabled"
	},
	{
		/* Key. */
		"clowndoom_default_compatibility_level",
		/* Label. */
		"General > Default Compatibility Level",
		/* Categorised label. */
		"Default Compatibility Level",
		/* Description. */
		"Set game behaviour to match a particular revision; important for demos. Restart to apply.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"general",
		/* Values. */
		{
			{"2", "Doom v1.9"},
			{"3", "Ultimate Doom"},
			{"4", "Final Doom"},
			{NULL, NULL},
		},
		/* Default value. */
		"3"
	},
	{
		/* Key. */
		"clowndoom_graphic_detail",
		/* Label. */
		"Video > Graphic Detail",
		/* Categorised label. */
		"Graphic Detail",
		/* Description. */
		"Lower to improve performance.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{ "Low", NULL},
			{ "High", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"High"
	},
	{
		/* Key. */
		"clowndoom_screen_size",
		/* Label. */
		"Video > Screen Size",
		/* Categorised label. */
		"Screen Size",
		/* Description. */
		"Shrink the screen to improve performance.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{ "3", NULL},
			{ "4", NULL},
			{ "5", NULL},
			{ "6", NULL},
			{ "7", NULL},
			{ "8", NULL},
			{ "9", NULL},
			{"10", NULL},
			{"11", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"10"
	},
	{
		/* Key. */
		"clowndoom_use_gamma",
		/* Label. */
		"Video > Gamma Correction",
		/* Categorised label. */
		"Gamma Correction",
		/* Description. */
		"Adjust display brightness.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{"0", "Off"},
			{"1", "Level 1"},
			{"2", "Level 2"},
			{"3", "Level 3"},
			{"4", "Level 4"},
			{NULL, NULL},
		},
		/* Default value. */
		"0"
	},
	{
		/* Key. */
		"clowndoom_aspect_ratio_correction",
		/* Label. */
		"Video > Aspect Ratio Correction",
		/* Categorised label. */
		"Aspect Ratio Correction",
		/* Description. */
		"Makes the pixels non-square to restore the DOS release's aspect ratio.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{"enabled", NULL},
			{"disabled", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"enabled"
	},
	{
		/* Key. */
		"clowndoom_full_colour",
		/* Label. */
		"Video > Full Colour",
		/* Categorised label. */
		"Full Colour",
		/* Description. */
		"Renders using more than 256 colours.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{"enabled", NULL},
			{"disabled", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"disabled"
	},
	{
		/* Key. */
		"clowndoom_prototype_light_amplification_visor_effect",
		/* Label. */
		"Video > Prototype Light Amplification Visor Effect",
		/* Categorised label. */
		"Prototype Light Amplification Visor Effect",
		/* Description. */
		"Uses the night-vision effect from the Press Release prototype.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{"enabled", NULL},
			{"disabled", NULL},
			{NULL, NULL},
		},
		/* Default value. */
		"disabled"
	},
	{
		/* Key. */
		"clowndoom_horizontal_resolution",
		/* Label. */
		"Video > Horizontal Resolution",
		/* Categorised label. */
		"Horizontal Resolution",
		/* Description. */
		"Vertical resolution is automatically adjusted. Restart to apply.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{ "320", NULL},
			{ "640", NULL},
			{"1280", NULL},
			{"1920", NULL},
			{"2560", NULL},
			{"3200", NULL},
		},
		/* Default value. */
		"320"
	},
	{
		/* Key. */
		"clowndoom_aspect_ratio",
		/* Label. */
		"Video > Aspect Ratio",
		/* Categorised label. */
		"Aspect Ratio",
		/* Description. */
		"Aspect ratio of video output. Restart to apply.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{"1.25",                "5:4"},
			{"1.333333333333333",   "4:3"},
			{"1.6",               "16:10"},
			{"1.777777777777778",  "16:9"},
			{"2.333333333333333",  "21:9"},
		},
		/* Default value. */
		"1.333333333333333"
	},
	{
		/* Key. */
		"clowndoom_hud_scale",
		/* Label. */
		"Video > HUD Scale",
		/* Categorised label. */
		"HUD Scale",
		/* Description. */
		"Scaling to counteract increased internal resolution. Restart to apply.",
		/* Categorised description. */
		NULL,
		/* Category. */
		"video",
		/* Values. */
		{
			{"1x", NULL},
			{"2x", NULL},
			{"4x", NULL},
			{"8x", NULL},
		},
		/* Default value. */
		"1x"
	},
	{
		/* Key. */
		"clowndoom_sfx_volume",
		/* Label. */
		"Audio > SFX Volume",
		/* Categorised label. */
		"SFX Volume",
		/* Description. */
		NULL,
		/* Categorised description. */
		NULL,
		/* Category. */
		"audio",
		/* Values. */
		{
			{ "0", NULL},
			{ "1", NULL},
			{ "2", NULL},
			{ "3", NULL},
			{ "4", NULL},
			{ "5", NULL},
			{ "6", NULL},
			{ "7", NULL},
			{ "8", NULL},
			{ "9", NULL},
			{"10", NULL},
			{"11", NULL},
			{"12", NULL},
			{"13", NULL},
			{"14", NULL},
			{"15", NULL},
		},
		/* Default value. */
		"8"
	},
	{
		/* Key. */
		"clowndoom_music_volume",
		/* Label. */
		"Audio > Music Volume",
		/* Categorised label. */
		"Music Volume",
		/* Description. */
		NULL,
		/* Categorised description. */
		NULL,
		/* Category. */
		"audio",
		/* Values. */
		{
			{ "0", NULL},
			{ "1", NULL},
			{ "2", NULL},
			{ "3", NULL},
			{ "4", NULL},
			{ "5", NULL},
			{ "6", NULL},
			{ "7", NULL},
			{ "8", NULL},
			{ "9", NULL},
			{"10", NULL},
			{"11", NULL},
			{"12", NULL},
			{"13", NULL},
			{"14", NULL},
			{"15", NULL},
		},
		/* Default value. */
		"8"
	},
	{NULL, NULL, NULL, NULL, NULL, NULL, {{NULL, NULL}}, NULL}
};

struct retro_core_options_v2 options_us = {
   option_cats_us,
   option_defs_us
};

/*
 ********************************
 * Language Mapping
 ********************************
*/

#ifndef HAVE_NO_LANGEXTRA
struct retro_core_options_v2 *options_intl[RETRO_LANGUAGE_LAST] = {
   &options_us, /* RETRO_LANGUAGE_ENGLISH */
   NULL,        /* RETRO_LANGUAGE_JAPANESE */
   &options_fr, /* RETRO_LANGUAGE_FRENCH */
   NULL,        /* RETRO_LANGUAGE_SPANISH */
   NULL,        /* RETRO_LANGUAGE_GERMAN */
   NULL,        /* RETRO_LANGUAGE_ITALIAN */
   NULL,        /* RETRO_LANGUAGE_DUTCH */
   NULL,        /* RETRO_LANGUAGE_PORTUGUESE_BRAZIL */
   NULL,        /* RETRO_LANGUAGE_PORTUGUESE_PORTUGAL */
   NULL,        /* RETRO_LANGUAGE_RUSSIAN */
   NULL,        /* RETRO_LANGUAGE_KOREAN */
   NULL,        /* RETRO_LANGUAGE_CHINESE_TRADITIONAL */
   NULL,        /* RETRO_LANGUAGE_CHINESE_SIMPLIFIED */
   NULL,        /* RETRO_LANGUAGE_ESPERANTO */
   NULL,        /* RETRO_LANGUAGE_POLISH */
   NULL,        /* RETRO_LANGUAGE_VIETNAMESE */
   NULL,        /* RETRO_LANGUAGE_ARABIC */
   NULL,        /* RETRO_LANGUAGE_GREEK */
   NULL,        /* RETRO_LANGUAGE_TURKISH */
   NULL,        /* RETRO_LANGUAGE_SLOVAK */
   NULL,        /* RETRO_LANGUAGE_PERSIAN */
   NULL,        /* RETRO_LANGUAGE_HEBREW */
   NULL,        /* RETRO_LANGUAGE_ASTURIAN */
   NULL,        /* RETRO_LANGUAGE_FINNISH */
   NULL,        /* RETRO_LANGUAGE_INDONESIAN */
   NULL,        /* RETRO_LANGUAGE_SWEDISH */
   NULL,        /* RETRO_LANGUAGE_UKRAINIAN */
   NULL,        /* RETRO_LANGUAGE_CZECH */
};
#endif

/*
 ********************************
 * Functions
 ********************************
*/

/* Handles configuration/setting of core options.
 * Should be called as early as possible - ideally inside
 * retro_set_environment(), and no later than retro_load_game()
 * > We place the function body in the header to avoid the
 *   necessity of adding more .c files (i.e. want this to
 *   be as painless as possible for core devs)
 */

static void libretro_set_core_options(retro_environment_t environ_cb)
{
   unsigned version  = 0;
#ifndef HAVE_NO_LANGEXTRA
   unsigned language = 0;
#endif

   if (!environ_cb(RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION, &version))
      version = 0;

   if (version >= 2)
   {
#ifndef HAVE_NO_LANGEXTRA
      struct retro_core_options_v2_intl core_options_intl;

      core_options_intl.us    = &options_us;
      core_options_intl.local = NULL;

      if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
          (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH))
         core_options_intl.local = options_intl[language];

      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL, &core_options_intl);
#else
      environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2, &options_us);
#endif
   }
   else
   {
      size_t i, j;
      size_t option_index              = 0;
      size_t num_options               = 0;
      struct retro_core_option_definition
            *option_v1_defs_us         = NULL;
#ifndef HAVE_NO_LANGEXTRA
      size_t num_options_intl          = 0;
      struct retro_core_option_v2_definition
            *option_defs_intl          = NULL;
      struct retro_core_option_definition
            *option_v1_defs_intl       = NULL;
      struct retro_core_options_intl
            core_options_v1_intl;
#endif
      struct retro_variable *variables = NULL;
      char **values_buf                = NULL;

      /* Determine total number of options */
      while (true)
      {
         if (option_defs_us[num_options].key)
            num_options++;
         else
            break;
      }

      if (version >= 1)
      {
         /* Allocate US array */
         option_v1_defs_us = (struct retro_core_option_definition *)
               calloc(num_options + 1, sizeof(struct retro_core_option_definition));

         /* Copy parameters from option_defs_us array */
         for (i = 0; i < num_options; i++)
         {
            struct retro_core_option_v2_definition *option_def_us = &option_defs_us[i];
            struct retro_core_option_value *option_values         = option_def_us->values;
            struct retro_core_option_definition *option_v1_def_us = &option_v1_defs_us[i];
            struct retro_core_option_value *option_v1_values      = option_v1_def_us->values;

            option_v1_def_us->key           = option_def_us->key;
            option_v1_def_us->desc          = option_def_us->desc;
            option_v1_def_us->info          = option_def_us->info;
            option_v1_def_us->default_value = option_def_us->default_value;

            /* Values must be copied individually... */
            while (option_values->value)
            {
               option_v1_values->value = option_values->value;
               option_v1_values->label = option_values->label;

               option_values++;
               option_v1_values++;
            }
         }

#ifndef HAVE_NO_LANGEXTRA
         if (environ_cb(RETRO_ENVIRONMENT_GET_LANGUAGE, &language) &&
             (language < RETRO_LANGUAGE_LAST) && (language != RETRO_LANGUAGE_ENGLISH) &&
             options_intl[language])
            option_defs_intl = options_intl[language]->definitions;

         if (option_defs_intl)
         {
            /* Determine number of intl options */
            while (true)
            {
               if (option_defs_intl[num_options_intl].key)
                  num_options_intl++;
               else
                  break;
            }

            /* Allocate intl array */
            option_v1_defs_intl = (struct retro_core_option_definition *)
                  calloc(num_options_intl + 1, sizeof(struct retro_core_option_definition));

            /* Copy parameters from option_defs_intl array */
            for (i = 0; i < num_options_intl; i++)
            {
               struct retro_core_option_v2_definition *option_def_intl = &option_defs_intl[i];
               struct retro_core_option_value *option_values           = option_def_intl->values;
               struct retro_core_option_definition *option_v1_def_intl = &option_v1_defs_intl[i];
               struct retro_core_option_value *option_v1_values        = option_v1_def_intl->values;

               option_v1_def_intl->key           = option_def_intl->key;
               option_v1_def_intl->desc          = option_def_intl->desc;
               option_v1_def_intl->info          = option_def_intl->info;
               option_v1_def_intl->default_value = option_def_intl->default_value;

               /* Values must be copied individually... */
               while (option_values->value)
               {
                  option_v1_values->value = option_values->value;
                  option_v1_values->label = option_values->label;

                  option_values++;
                  option_v1_values++;
               }
            }
         }

         core_options_v1_intl.us    = option_v1_defs_us;
         core_options_v1_intl.local = option_v1_defs_intl;

         environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL, &core_options_v1_intl);
#else
         environ_cb(RETRO_ENVIRONMENT_SET_CORE_OPTIONS, option_v1_defs_us);
#endif
      }
      else
      {
         /* Allocate arrays */
         variables  = (struct retro_variable *)calloc(num_options + 1,
               sizeof(struct retro_variable));
         values_buf = (char **)calloc(num_options, sizeof(char *));

         if (!variables || !values_buf)
            goto error;

         /* Copy parameters from option_defs_us array */
         for (i = 0; i < num_options; i++)
         {
            const char *key                        = option_defs_us[i].key;
            const char *desc                       = option_defs_us[i].desc;
            const char *default_value              = option_defs_us[i].default_value;
            struct retro_core_option_value *values = option_defs_us[i].values;
            size_t buf_len                         = 3;
            size_t default_index                   = 0;

            values_buf[i] = NULL;

            if (desc)
            {
               size_t num_values = 0;

               /* Determine number of values */
               while (true)
               {
                  if (values[num_values].value)
                  {
                     /* Check if this is the default value */
                     if (default_value)
                        if (strcmp(values[num_values].value, default_value) == 0)
                           default_index = num_values;

                     buf_len += strlen(values[num_values].value);
                     num_values++;
                  }
                  else
                     break;
               }

               /* Build values string */
               if (num_values > 0)
               {
                  buf_len += num_values - 1;
                  buf_len += strlen(desc);

                  values_buf[i] = (char *)calloc(buf_len, sizeof(char));
                  if (!values_buf[i])
                     goto error;

                  strcpy(values_buf[i], desc);
                  strcat(values_buf[i], "; ");

                  /* Default value goes first */
                  strcat(values_buf[i], values[default_index].value);

                  /* Add remaining values */
                  for (j = 0; j < num_values; j++)
                  {
                     if (j != default_index)
                     {
                        strcat(values_buf[i], "|");
                        strcat(values_buf[i], values[j].value);
                     }
                  }
               }
            }

            variables[option_index].key   = key;
            variables[option_index].value = values_buf[i];
            option_index++;
         }

         /* Set variables */
         environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
      }

error:
      /* Clean up */

      if (option_v1_defs_us)
      {
         free(option_v1_defs_us);
         option_v1_defs_us = NULL;
      }

#ifndef HAVE_NO_LANGEXTRA
      if (option_v1_defs_intl)
      {
         free(option_v1_defs_intl);
         option_v1_defs_intl = NULL;
      }
#endif

      if (values_buf)
      {
         for (i = 0; i < num_options; i++)
         {
            if (values_buf[i])
            {
               free(values_buf[i]);
               values_buf[i] = NULL;
            }
         }

         free(values_buf);
         values_buf = NULL;
      }

      if (variables)
      {
         free(variables);
         variables = NULL;
      }
   }
}

#ifdef __cplusplus
}
#endif

#endif
