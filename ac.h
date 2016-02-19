/*
 * =====================================================================================
 *
 *       Filename:  ac.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2016年02月17日 13时58分23秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __AC_H__
#define __AC_H__


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>


typedef unsigned short	uint16;
typedef unsigned int	uint32;
typedef unsigned long	uint64;
typedef unsigned char	InputTy;

#ifdef DEBUG
	#define ASSERT(c) if (!(c)) \
		{ fprintf(stderr, "%s:%d Assert: %s\n", __FILE__, __LINE__, #c); abort(); }

#else
	#define ASSERT(c) ((void)0)
#endif

#define likely(x)		__builtin_expect((x), 1)
#define unlikely(x)		__builtin_expect((x), 0)

#ifndef offsetof
#define offsetof(st, m)	((size_t)(&((st *)0)->m))	
#endif


#define CHAR_SET_SIZE  256

#define AC_MAGIC_NUM	0x5a

#define bool			int
#define true			1
#define false			0


typedef struct {
	size_t				len;
	u_char				*data;
} ac_str_t;

typedef struct {
	void				*elts;
	uint32				nelts;
	uint32				nalloc;
	size_t				size;
} ac_array_t;

typedef struct acs_state_s		acs_state_t;


struct acs_state_s {
	int						id;
	int						pattern_idx;
	int						depth;
	int						terminal;
	int						goto_num;
	int						current;
	struct acs_state_s		*goto_map[CHAR_SET_SIZE];
	struct acs_state_s		*fail_link;
};


typedef void *(*malloc_func_pt)(uint64, size_t);
typedef void (*free_func_pt)(uint64, void *);

typedef struct {
    malloc_func_pt          malloc;
    free_func_pt            free;
    uint64                  data;
} ac_bufalloc_t;


typedef struct {
	acs_state_t				*root;
	u_char					root_char[CHAR_SET_SIZE];
	int						state_num;
	ac_array_t				all_states;
	int						state_id;
} acs_constructor_t;



typedef struct {
	unsigned char		magic_num;
	unsigned char		reserve;
} buf_header_t;


typedef struct {
	int		match_begin;
	int		match_end;
	int		pattern_idx;
} ac_result_t;


typedef uint32		ac_offset_t;
typedef uint32		ac_state_id;

typedef struct {
	buf_header_t	hdr;

	uint32			buf_len;
	ac_offset_t		root_goto_ofst;
	ac_offset_t		state_ofst_ofst;
	ac_offset_t		first_state_ofst;
	uint16			root_goto_num;
	uint16			state_num;
} ac_buffer_t;

typedef struct {
	ac_state_id		first_kid;
	ac_offset_t		fail_link;
	short			depth;
	unsigned short	terminal;
	unsigned char	goto_num;
	InputTy			input_vect[1];
} ac_state_t;



int ac_array_init(ac_array_t *array, size_t size, int n);
void *ac_array_push(ac_array_t *a);
void ac_array_destory(ac_array_t *a);
void ac_array_clear(ac_array_t *a);

int acs_log_error(int level, int erno, char *fmt, ...);

acs_constructor_t *acs_constructor_create(void);
void acs_constructor_free(acs_constructor_t *acs);
acs_constructor_t *acs_construct(ac_str_t *value, int size);
int acs_match(acs_constructor_t *acs, u_char *src, size_t len, ac_result_t *res);

void *ac_convert(acs_constructor_t *acs, ac_bufalloc_t *);
ac_result_t ac_match(ac_buffer_t *buf, const char *str, uint32 len);
#endif
