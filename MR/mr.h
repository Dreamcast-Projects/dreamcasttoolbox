#ifndef MR_H
#define MR_H

//typedef struct Mr M;

Twiddler* twiddler_create(int width, int height);
void twiddler_destroy(Twiddler* twiddler);

int twiddler_index(Twiddler* twiddler, int i);
int twiddler_index_xy(Twiddler* twiddler, int x, int y);

#endif
