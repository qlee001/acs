#include "ac.h"



acs_constructor_t *acs_constructor_create(void);
void acs_constructor_free(acs_constructor_t *acs);
acs_constructor_t *acs_construct(ac_str_t *value, int size);
static acs_state_t	*state_alloc(acs_constructor_t *acs);
static void state_free(acs_state_t *s);

static void acs_state_set_goto(acs_state_t *state, u_char c, acs_state_t *next);
static acs_state_t *acs_state_get_goto(acs_state_t *state, u_char c);
static int acs_add_pattern(acs_constructor_t *acs, ac_str_t *value, int index);
static int propagate_faillink(acs_constructor_t *acs);

int acs_match(acs_constructor_t *acs, u_char *src, size_t len, ac_result_t *res);




acs_constructor_t *acs_constructor_create(void)
{
	acs_constructor_t	*acs;
	acs = malloc(sizeof(*acs));
	memset(acs, 0, sizeof(*acs));

	ac_array_init(&acs->all_states, sizeof(acs_state_t *), 1024);

	acs->root = state_alloc(acs);
	
	return acs;
}


void acs_constructor_free(acs_constructor_t *acs)
{
	if (acs->all_states.nelts) {
		ac_array_t		*a = &acs->all_states;
		acs_state_t		**base = a->elts;
		int				num = a->nelts;
		int i;
		for (i=0; i < num; i++) {
			state_free(base[i]);
		}

		ac_array_destory(&acs->all_states);
	}
	
	free(acs);
	return;
}


static acs_state_t	*state_alloc(acs_constructor_t *acs)
{
	acs_state_t		*p;
	p = malloc(sizeof(*p));
	bzero(p, sizeof(*p));

	acs_state_t		**tmp;
	tmp = ac_array_push(&acs->all_states);
	if (tmp == NULL) {
		free(p);
		return NULL;
	}
	*tmp = p;
	p->id = acs->state_id;
	acs->state_id++;
	acs->state_num++;
	return p;
}


static void state_free(acs_state_t *s)
{
	free(s);
	return;
}




int acs_log_error(int level, int erno, char *fmt, ...)
{
	int			n;
	char		buf[2048];
	char		*pos;
	va_list		args;

	pos = buf;
	va_start(args, fmt);
	n = vsprintf(pos, fmt, args);
	va_end(args);

	pos += n;
	*pos = '\n';
	pos++;
	*pos = '\0';
	if (fprintf(stderr, "%s", buf) < 0) {
		fprintf(stderr, "Write log error failed:%s", strerror(errno));
		return 0;
	}
	return 0;
}

/*  
static int acs_log_error(int level, int erno, char *fmt, ...)
{
	int			n;
	char		buf[2048];
	char		*pos;
	va_list		args;

	pos = buf;
	va_start(args, fmt);

	while (*fmt && buf < )
*/

/*  
static u_char * acs_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args)
{
	
	while (*fmt && buf < last) {

		if (*fmt == '%') {
			width = 0;
			sign = 1;
			hex = 0;
			max_width = 0;
			frac_width = 0;
			slen = (size_t)-1;


		} else {
			*buf++ = *fmt++;
		}
		return buf;
	}
	return buf;
}
*/



acs_constructor_t *acs_construct(ac_str_t *value, int size)
{
	acs_constructor_t		*acs;
	acs = acs_constructor_create();
	if (acs == NULL) {
		return NULL;
	}


	int count = 0;
	for (; value->len; value++, count++) {
		acs_add_pattern(acs, value, count);
	}

	propagate_faillink(acs);

	return acs;
}


int acs_match(acs_constructor_t *acs, u_char *str, size_t len, ac_result_t *res)
{
	acs_state_t		*root = acs->root;
	acs_state_t		*state = root;
	int idx = 0;
	while (idx < len) {
		u_char c = str[idx];
		idx++;
		state = acs_state_get_goto(root, c);
		if (state) {
			break;
		}
	}

	if (unlikely(state->terminal == 1)) {
		res->match_begin = idx - 1;
		res->match_end = idx - 1;
		res->pattern_idx = state->pattern_idx;
		return 1;
	}

	while (idx < len) {
		u_char c = str[idx];
		acs_state_t		*gs = acs_state_get_goto(state, c);
		if (gs == NULL) {
			acs_state_t		*fl = state->fail_link;
			if (fl == root) {
				while (idx < len) {
					u_char c2 = str[idx];
					idx++;
					state = acs_state_get_goto(root, c2);
					if (state) {
						break;
					} 
				}
			} else {
				state = fl;
			}
		} else {
			idx++;
			state = gs;
		}

		if (state->terminal) {
			res->match_begin = idx - state->depth;
			res->match_end = idx - 1;
			res->pattern_idx = state->pattern_idx;
			return 1;
		}
					
	}

	return 0;
	
}








static int propagate_faillink(acs_constructor_t *acs)
{
	acs_state_t		*root = acs->root;
	ac_array_t		a;
	ac_array_init(&a, sizeof(acs_state_t *), 1024);

	int i = 0;
	for (; i < CHAR_SET_SIZE; i++) {
		u_char c = i;
		if (root->goto_map[c] != NULL) {
			acs_state_t	**tmp;
			tmp = ac_array_push(&a);
			*tmp = root->goto_map[c];
			(*tmp)->fail_link = root;
		}
	}

	acs_state_t		*goto_save[CHAR_SET_SIZE];
	memcpy(goto_save, root->goto_map, sizeof(acs_state_t *) * CHAR_SET_SIZE);


	for (i=0; i < CHAR_SET_SIZE; i++) {
		u_char c = i;
		acs_state_t *s = acs_state_get_goto(root, c);
		if (s == NULL) {
			acs_state_set_goto(root, c, root);
		}
	}
	
	int current = 0;

	while (1) {

		for (i = 0; i < CHAR_SET_SIZE; i++) {
			u_char	c = i;
			acs_state_t		**tmp = a.elts;
			acs_state_t		*s = tmp[current];
			acs_state_t		*fl = s->fail_link;
			
			acs_state_t		*tran = s->goto_map[c];
			acs_state_t		*tran_fl = NULL;

			if (tran == NULL)
				continue;
			
			acs_state_t		*fl_walk = fl;
			while (1) {
				acs_state_t		*t = acs_state_get_goto(fl_walk, c);
	 			if (t != NULL) {
	 				tran_fl = t;
					break;
				}

				fl_walk = fl_walk->fail_link;
			}
			tran->fail_link = tran_fl;
			{ 
				tran->fail_link = tran_fl;
 				acs_state_t		**aaa = ac_array_push(&a);
				*aaa = tran;
			}
		}
		current++;
		if (current >= a.nelts)
			break;

	}
	memcpy(root->goto_map, goto_save, sizeof(acs_state_t *) * CHAR_SET_SIZE);
	return 0;
}


static int acs_add_pattern(acs_constructor_t *acs, ac_str_t *value, int index)
{
	acs_state_t		*state = acs->root;
	u_char			*str = value->data;
	int i = 0;
	for (i = 0; i < value->len; i++) {
		const char c = str[i];
		acs_state_t		*new = acs_state_get_goto(state, c);
		if (new == NULL) {
			new = state_alloc(acs);
			new->depth = state->depth + 1;
			new->current = c;
			acs_state_set_goto(state, c, new);
		}
		state = new;
	}
	state->terminal = 1;
	state->pattern_idx = index;
	return 0;
}

static acs_state_t *acs_state_get_goto(acs_state_t *state, u_char c)
{
	return state->goto_map[c];

}

static void acs_state_set_goto(acs_state_t *state, u_char c, acs_state_t *next)
{
	state->goto_num++;
	state->goto_map[c] = next;
	return;
}
