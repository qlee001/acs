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
	struct acs_state_s		*goto_map[CHAR_SET_SIZE];
	struct acs_state_s		*fail_link;
};

//内存分配函数
typedef void *(acs_malloc_func)(size_t);
typedef void *(acs_free_func)(void *);

typedef struct {
	acs_state_t				root;
	u_char					root_char[CHAR_SET_SIZE];
	ac_array_t				all_states;

} acs_constructor_t;


typedef enum {
	IMPL_SLOW_VARIANT = 1;
	IMPL_FAST_VARIANT = 2;
} impl_var_t;

typedef struct {
	unsigned char		magic_num;
	unsigned char		impl_variant;
} buf_header_t;



typedef struct {
	int		match_begin;
	int		match_end;
	int		pattern_idx;
} ac_result_t;

static ac_t;




//内存分配函数
typedef void *(acs_malloc_func)(size_t);
typedef void *(acs_free_func)(void *);












#endif
