#pragma once

typedef unsigned int u32;
typedef signed int s32;
typedef unsigned char byte;
typedef float f32;
typedef double f64;

typedef struct OSXPlatformWindow OSXPlatformWindow;

void initialize();
void createWindow(u32 w, u32 h, const char* title, u32 flags);
void run();

