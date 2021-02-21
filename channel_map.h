//
// Created by HEADS on 2021/2/20.
//

#ifndef CHANNEL_MAP_H
#define CHANNEL_MAP_H

#include "channel.h"

// channel map, key is socket fd
struct channel_map {
    void **entries;

    // The numer of entries available in entries
    int nentries;
};

int map_make_space(struct channel_map *map, int slot, int msize);

void map_init(struct channel_map *map);

void map_clear(struct channel_map *map);

#endif //CHANNEL_MAP_H
