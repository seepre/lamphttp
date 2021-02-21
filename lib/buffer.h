//
// Created by HEADS on 2021/2/20.
//

#ifndef BUFFER_H
#define BUFFER_H

#define INIT_BUFFER_SIZE    65536

// data buffer
struct buffer {
    char *data;      // buffer data
    int read_index;  // buffer read index
    int write_index; // buffer write index
    int total_size;  // buffer total size
};


struct buffer *buffer_new();

void buffer_free(struct buffer *buf);

int buffer_writeable_size(struct buffer *buf);

int buffer_readable_size(struct buffer *buf);

int buffer_front_spare_size(struct buffer *buf);

// write data into buffer
int buffer_append(struct buffer *buf, void *data, int size);

int buffer_append_char(struct buffer *buf, char *data);

int buffer_append_string(struct buffer *buf, char *data);

// write data int buffer from socket
int buffer_socket_read(struct buffer *buf, int fd);

char buffer_read_char(struct buffer *buf);

// find data from buffer
char* buffer_find_CRLF(struct buffer *buf);

#endif //BUFFER_H
