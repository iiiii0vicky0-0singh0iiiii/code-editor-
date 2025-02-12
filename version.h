#ifndef VERSION_H
#define VERSION_H

#define EDITOR_NAME "Code-editor"
#define EDITOR_VERSION_MAJOR 1
#define EDITOR_VERSION_MINOR 0
#define EDITOR_VERSION_PATCH 0

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define EDITOR_VERSION TOSTRING(EDITOR_VERSION_MAJOR) "." TOSTRING(EDITOR_VERSION_MINOR) "." TOSTRING(EDITOR_VERSION_PATCH)

#endif // VERSION_H
