time gcc -o benne  shaders.c distance_fields.c cb_ui.c cb_string.c benne.c -I/usr/include/freetype2 -I/usr/include/libpng16 -lfreetype -lm -lglfw -lGL -lGLEW -Wno-write-strings && ./benne; rm ./benne
