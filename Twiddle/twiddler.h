#ifndef TWIDDLER_H
#define TWIDDLER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Twiddler Twiddler;

Twiddler* twiddler_create(int width, int height);
void twiddler_destroy(Twiddler* twiddler);

int twiddler_index(Twiddler* twiddler, int i);
int twiddler_index_xy(Twiddler* twiddler, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
