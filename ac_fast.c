/*
 * =====================================================================================
 *
 *       Filename:  ac_fast.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2016年02月17日 15时30分41秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "ac.h"



typedef uint32		ac_offset_t;
typedef uint32		ac_state_id;

typedef struct {
	buf_header_t	hdr;

	uint32			buf_len;
	AC_Ofst			root_goto_ofst;
	AC_Ofst			states_ofst_ofst;
	AC_Ofst			first_state_ofst;
	uint16			root_goto_num;
	uint16			state_num;
} ac_buffer_t;

typedef struct {
	ac_state_id		first_kid;
	ac_offset_t		fail_link;
	short			depth;
	unsigned short	ternimal;
	unsigned char	goto_num;
	InputTy			input_vect[1];
} ac_state_t;


typedef struct {
	acs_constructor_t	acs;

} ac_convert_t;



uint32 ac_calc_state_size(acs_state_t *s)
{
	ac_state_t		dummy;
	uint32	sz = offsetof(ac_state_t, input_vect);
	sz += s->goto_num * sizeof(dummy.input_vect[0]);

	if (sz < sizeof(ac_state_t)) {
		sz = sizeof(ac_state_t);
	}
	
	uint32 align = __alignof__(dummy);
	sz = (sz + align - 1) & ~(align - 1);
	return sz;
}


uint32 ac_alloc_buffer(acs_constructor_t *acs)
{
	acs_state_t			*root_state = &acs->root;
	uint32 root_fanout = root_state->goto_num;

	//Step 1: Calculate the buffer size
	ac_offset_t			root_goto_ofst, states_ofst_ofst, first_state_ofst;

	//part 1 : buffer header
	uint32 sz = root_goto_ofst = sizeof(ac_buffer_t);

	//part 2 : Root-node's goto function
	if (likely(root_fanout != 255))
		sz += 256;
	else
		//What to do?
		root_goto_ofst = 0;

	//part 3 : mapping of state's relative position.
	uint32		align = __alignof__(ac_offset_t);
	sz = (sz + align - 1) & ~(align - 1);
	state_ofst_ofst = sz;

	sz += sizeof(ac_offset_t) * acs->state_num;

	//part 4 : state's contents
	align = __alignof__(ac_state);
	sz = (sz + align - 1) & ~(align - 1);
	first_state_ofst = sz;

	uint32	state_sz = 0;
	ac_array_t		*all_states = &acs->all_states;
	acs_state_t		*base = all_states->elts;
	int i;
	int sum = all_states->nelts;
	for (i=0; i < sum; i++) {
		state_sz += ac_calc_state_size(&base[i]);
	}

	//TODO
	//state_sz -= ac_calc_state_size;

	sz += state_sz;

	//Step 2 : Allocate buffer, and populate header.
	ac_buffer_t		*buf = malloc(sz);

	buf->hdr.magic_num = AC_MAGIC_NUM;
	buf->hdr.impl_variant = IMPL_FAST_VARIANT;
	buf->buf_len = sz;
	buf->root_goto_ofst = root_goto_ofst;
	buf->states_ofst_ofst = states_ofst_ofst;
	buf->first_state_ofst = first_state_ofst;
	buf->root_goto_num = root_fanout;
	buf->state_num = acs.state_num;
	return buf;
}


void populate_root_goto_func(acs_constructor_t *acs, ac_buffer_t *buf, ac_array_t *goto_vect)
{
	unsigned char	*buf_base = (unsigned char *)buf;
	InputTy			*root_gotos = (InputTy *)(buf_base + buf->root_goto_ofst);
	acs_state_t		*root_state = &acs->root;

	int i;
	for (i=0; i < 256; i++) {
		
	}
}






ac_buffer_t *ac_convert(acs_constructor_t *acs)
{
	//Step 1: Some preparation stuff.
	//
	

	//Step 2: allocate buffer to accommodate the entire AC graph.
	ac_buffer_t		*buf = ac_alloc_buffer();
	if (buf == NULL) {
		return NULL;
	}

	unsigned char *buf_base = (unsigned char *)buf;

	//Step 3: Root node need special care.
	

	ac_array_t		wl;
	ac_array_init(&wl, sizeof(acs_state_t *), acs->all_states.nelts);
	if (wl.elts == NULL)
		return NULL;

	int				*id_map = malloc(sizeof(int) * acs->all_states.nelts);
	memset(id_map, 0, sizeof(int) * acs->all_states.nelts);

	acs_state_t		*root_state = &acs->root;
	acs_state_t		**goto_map = root_state->goto_map;
	int i;

	id_map[root_state->id] = 0;
	ac_state_id		id = 1;
	
	for (i=0; i < 256; i++) {
		acs_state_t	*state = goto_map[i];
		if (state != NULL) {
			acs_state_t **tmp = ac_array_push(wl);
			if (tmp == NULL) {
				//TODO.
			}
			*tmp = state;
			rd_map[state->id] = id;
			id++;
		}
	}

	ac_offset_t		*state_ofst_vect = (ac_offset_t *)(buf_base + buf->state_ofst_ofst);
	ac_offset_t		ofst = buf->first_state_ofst;
	uint32 idx;
	for (idx = 0; idx < wl.nelts; idx++) {
		acs_state_t		*old_s = wl[idx];
		acs_state_t		*new_s = (ac_state *)(buf_base + ofst);


		ac_state_id		state_id = idx + 1;
		//TODO id??
		//
		
		state_ofst_vect[state_id] = ofst;

		new_s->first_kid = wl.nelts + 1;
		new_s->depth = old_s->depth;
		new_s->terminal = old_s->terminal?old_s->id + 1 : 0;

		uint32 gotonum = old_s->goto_num;
		new_s->gotonum = gotonum;

		//Populate the "input" field.
		uint32 input_idx = 0;
		uint32 id = wl.nelts + 1;
		InputTy	*input_vect = new_s->input_vect;

		acs_state_t		**goto_map = old_s->goto_map;
		for (i = 0; i < 256; i++) {
			acs_state_t *state = goto_map[i];
			if (state == NULL)
				continue;
			input_vect[input_idx] = current;
			input_idx++;

			id_map[state->id] = id;
			id++;

			acs_state_t **tmp = ac_array_push(&wl);
			if (tmp == NULL) {
				//TODO
			}
			*tmp = state;
			
		}
		ofst += ac_calc_state_size(old_s);
	}

	for (i=0; i < wl.nelts; i++) {
		acs_state_t		*slow_s = base[i];
		ac_state_id		fast_s_id = id_map[slow_s->id];
		ac_state_t		fast_s = (ac_state_t *)(buf_base + state_ofst_vect[fast_s_id]);
		acs_stata_t		*fl = slow_s->fail_link;
		if (fl != NULL) {
			ac_state_id	id = id_map[fl->id];
			fast_s->fail_link = id;
		} else
			fast_s->fail_link = 0;
	}

	return buf;
}



