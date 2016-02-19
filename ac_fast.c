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


static void populate_root_goto_func(ac_buffer_t *, int *, acs_constructor_t *);
static ac_buffer_t *ac_alloc_buffer(acs_constructor_t *, ac_bufalloc_t *);
static uint32 ac_calc_state_size(acs_state_t *);

static uint32 ac_calc_state_size(acs_state_t *s)
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


static ac_buffer_t *ac_alloc_buffer(acs_constructor_t *acs, ac_bufalloc_t *alloc)
{
	acs_state_t			*root_state = acs->root;
	uint32 root_fanout = root_state->goto_num;

	//Step 1: Calculate the buffer size
	ac_offset_t			root_goto_ofst, state_ofst_ofst, first_state_ofst;

	//part 1 : buffer header
	uint32 sz = root_goto_ofst = sizeof(ac_buffer_t);

	//part 2 : Root-node's goto function
	if (likely(root_fanout != 255))
		sz += 256;
	else
		root_goto_ofst = 0;

	//part 3 : mapping of state's relative position.
	uint32		align = __alignof__(ac_offset_t);
	sz = (sz + align - 1) & ~(align - 1);
	state_ofst_ofst = sz;

	sz += sizeof(ac_offset_t) * acs->state_num;

	//part 4 : state's contents
	align = __alignof__(ac_state_t);
	sz = (sz + align - 1) & ~(align - 1);
	first_state_ofst = sz;

	uint32	state_sz = 0;
	ac_array_t		*all_states = &acs->all_states;
	acs_state_t		**base = all_states->elts;
	int i;
	int sum = all_states->nelts;
	for (i=0; i < sum; i++) {
		state_sz += ac_calc_state_size(base[i]);
	}

	state_sz -= ac_calc_state_size(root_state);
	sz += state_sz;

	//Step 2 : Allocate buffer, and populate header.
	ac_buffer_t		*buf = alloc->malloc(alloc->data, sz);;
	
	buf->hdr.magic_num = AC_MAGIC_NUM;
	buf->buf_len = sz;
	buf->root_goto_ofst = root_goto_ofst;
	buf->state_ofst_ofst = state_ofst_ofst;
	buf->first_state_ofst = first_state_ofst;
	buf->root_goto_num = root_fanout;
	buf->state_num = acs->state_num;
	return buf;
}

  
static void populate_root_goto_func(ac_buffer_t *buf, int * id_map, acs_constructor_t *acs)
{
	unsigned char	*buf_base = (unsigned char *)buf;
	InputTy			*root_gotos = (InputTy *)(buf_base + buf->root_goto_ofst);
	acs_state_t		*root_state = acs->root;

	uint32	new_id = 1;
	if (likely(root_state->goto_num != 255)) {
		bzero(root_gotos, 256 * sizeof(InputTy));
	}

	int i;
	 
	for (i = 0; i < 256; i++) {
		acs_state_t *state = root_state->goto_map[i];
		if (state == NULL) {
			continue;
		}
		if (likely(root_state->goto_num != 255)) {
			root_gotos[state->current] = new_id;
		}
		id_map[state->id] = new_id;
		new_id++;
	}
	return;
}



void *ac_convert(acs_constructor_t *acs, ac_bufalloc_t *alloc)
{
	//Step 1: Some preparation stuff.
	//
	

	//Step 2: allocate buffer to accommodate the entire AC graph.
	ac_buffer_t		*buf = ac_alloc_buffer(acs, alloc);
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

	acs_state_t		*root_state = acs->root;
	acs_state_t		**goto_map = root_state->goto_map;

	populate_root_goto_func(buf, id_map, acs);
	buf->root_goto_num = root_state->goto_num;
	id_map[root_state->id] = 0;
	
	int i;
	ac_state_id		id = 1;
	
	for (i=0; i < 256; i++) {
		acs_state_t	*state = goto_map[i];
		if (state != NULL) {
			acs_state_t **tmp = ac_array_push(&wl);
			if (tmp == NULL) {
				//TODO.
			}
			*tmp = state;
			id_map[state->id] = id;
			id++;
		}
	}

	ac_offset_t		*state_ofst_vect = (ac_offset_t *)(buf_base + buf->state_ofst_ofst);
	ac_offset_t		ofst = buf->first_state_ofst;
	uint32 idx;
	for (idx = 0; idx < wl.nelts; idx++) {
		acs_state_t		**base = wl.elts;
		acs_state_t		*old_s = base[idx];
		ac_state_t		*new_s = (ac_state_t *)(buf_base + ofst);
		ac_state_id		state_id = idx + 1;
		//TODO id??
		//
		
		state_ofst_vect[state_id] = ofst;

		new_s->first_kid = wl.nelts + 1;
		new_s->depth = old_s->depth;
		new_s->terminal = old_s->terminal?old_s->pattern_idx + 1 : 0;

		uint32 gotonum = old_s->goto_num;
		new_s->goto_num = gotonum;

		//Populate the "input" field.
		uint32 input_idx = 0;
		uint32 id = wl.nelts + 1;
		InputTy	*input_vect = new_s->input_vect;

		acs_state_t		**goto_map = old_s->goto_map;
		for (i = 0; i < 256; i++) {
			acs_state_t *state = goto_map[i];
			if (state == NULL)
				continue;
			input_vect[input_idx] = state->current;
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

	acs_state_t		**base = wl.elts;
	for (i = 0; i < wl.nelts; i++) {
		acs_state_t		*slow_s = base[i];
		ac_state_id		fast_s_id = id_map[slow_s->id];
		ac_state_t		*fast_s = (ac_state_t *)(buf_base + state_ofst_vect[fast_s_id]);
		acs_state_t		*fl = slow_s->fail_link;
		if (fl != NULL) {
			ac_state_id	id = id_map[fl->id];
			fast_s->fail_link = id;
		} else
			fast_s->fail_link = 0;
	}
		
	free(id_map);
	ac_array_clear(&wl);
	return buf;
}

static inline ac_state_t *
ac_get_state_addr(unsigned char *buf_base, ac_offset_t *state_ofst_vect, uint32 state_id) {
	ASSERT((state_id != 0) && "root node is handled in a special way");
	ASSERT(state_id < ((ac_buffer_t *)buf_base)->state_num);
	return (ac_state_t *)(buf_base + state_ofst_vect[state_id]);
}

static bool inline
ac_binary_search_input(InputTy *input_vect, int vect_len, InputTy input, int *idx) {
	if (vect_len <= 8) {
		int i = 0;
		for (; i < vect_len; i++) {
			if (input_vect[i] == input) {
				*idx = i;
				return true;
			}
		}
		return false;
	}

	int low = 0, high = vect_len - 1;
	while (low <= high) {
		int mid = (low + high) >> 1;
		InputTy mid_c = input_vect[mid];
		if (input < mid_c) {
			high = mid - 1;
		} else if (input > mid_c)
			low = mid + 1;
		else {
			*idx = mid;
			return true;
		}
	}
	return false;
}


ac_result_t ac_match(ac_buffer_t *buf, const char *str, uint32 len) {
	unsigned char	*buf_base = (unsigned char *)(buf);
	unsigned char	*root_goto = buf_base + buf->root_goto_ofst;
	ac_offset_t		*state_ofst_vect = (ac_offset_t *)(buf_base + buf->state_ofst_ofst);

	uint32			idx = 0;
	ac_state_t		*state = NULL;
	if (likely(buf->root_goto_num != 255)) {
		while (idx < len) {
			unsigned char c = str[idx++];
			unsigned char kid_id = root_goto[c];
			if (kid_id) {
				state = ac_get_state_addr(buf_base, state_ofst_vect, kid_id);
				break;
			}
		}
	} else {
		idx = 1;
		state = ac_get_state_addr(buf_base, state_ofst_vect, *str);
	}

	ac_result_t r = {-1, -1, -1};
	if (likely(state != NULL)) {
		if (unlikely(state->terminal)) {
			r.match_begin = idx - state->depth;
			r.match_end = idx - 1;
			r.pattern_idx = state->terminal - 1;
			return r;
		}
	}
	
	while (idx < len) {
		unsigned char c = str[idx];
		int res, found;
		found = ac_binary_search_input(state->input_vect, state->goto_num, c, &res);
		if (found) {
			uint32 kid = state->first_kid + res;
			state = ac_get_state_addr(buf_base, state_ofst_vect, kid);
			idx++;
		} else {
			ac_state_id fl = state->fail_link;
			if (fl == 0) {
				while (idx < len) {
					unsigned char c = str[idx++];
					unsigned char kid_id = root_goto[c];
					if (kid_id) {
						state = ac_get_state_addr(buf_base, state_ofst_vect, kid_id);
						break;
					}
				}
			} else {
				state = ac_get_state_addr(buf_base, state_ofst_vect, fl);
			}
		}

		//Check to see if the state is terminal state?
		if (state->terminal) {
			ac_result_t r;
			r.match_begin = idx - state->depth;
			r.match_end = idx - 1;
			r.pattern_idx = state->terminal - 1;
			return r;
		}
	}
	return r;

}

