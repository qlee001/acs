#include "ac.h"





static void *acs_array_push(acs_array_t *a);
static int acs_array_init(acs_array_t *array, size_t size, int n);


static acs_constructor_t *acs_constructor_create(void)
{
	acs_constructor_t	*acs;
	acs = malloc(sizeof(*acs));
	memset(acs, 0, sizeof(*acs));

	acs_array_init(&acs->all_states, sizeof(acs_state_t *), 1024);
	
	return acs;
}


static void acs_constructor_free(acs_constructor_t *acs)
{
	free(acs);
	return;
}


static acs_state_t	*state_alloc()
{
	acs_state_t		*p;
	p = malloc(sizeof(*p));
	return p;
}

static void state_free(acs_state_t *s)
{
	free(s);
	return;
}


#define acs_string(str)			{sizeof(str)-1, (u_char *)str}

static acs_str_t acs_strings[] = {
	acs_string("thie"),
	acs_string("hersr"),
	acs_string("hiser"),
	acs_string("thfslijg"),
	{0, NULL},
};
static char		match_src[] = "afdgdghiserhersr";

typedef struct {
	int			start;
	int			end;
	int			index;

} acs_match_result_t;

static int acs_log_error(int level, int erno, char *fmt, ...)
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

static acs_constructor_t *acs_construct(acs_str_t *value, int size);
static void acs_state_set_goto(acs_state_t *state, u_char c, acs_state_t *next);
static acs_state_t *acs_state_get_goto(acs_state_t *state, u_char c);
static int acs_add_pattern(acs_constructor_t *acs, acs_str_t *value, int index);
static int propagate_faillink(acs_constructor_t *acs);

static int acs_match(acs_constructor_t *acs, u_char *src, size_t len, acs_match_result_t *res);

int main(int argc, char *argv[])
{
	acs_str_t	*value;
	int			count = 0;

	for (value = acs_strings; value->len; value++) {
		acs_log_error(1, 0, "count:%d, \tdata:%s, \tlen:%lu.", count, value->data, value->len);
		count++;
	}


	  
	acs_constructor_t		*acs;
	acs = acs_construct(&acs_strings[0], count);
	if (acs == NULL) {
		fprintf(stdout, "hehe");
		exit(1);
	}

	acs_match_result_t	res;
	memset(&res, 0, sizeof(res));

	if (acs_match(acs, (u_char *)match_src, sizeof(match_src) - 1, &res) == 1) {
		fprintf(stderr, "matched.\n");
		fprintf(stderr, "matched start:%d.\n", res.start);
		fprintf(stderr, "matched end:%d.\n", res.end);
		fprintf(stderr, "matched index:%d.\n", res.index);
	} else {
		fprintf(stderr, "Not match.\n");
	}
	
	return 0;
}


static acs_constructor_t *acs_construct(acs_str_t *value, int size)
{
	acs_constructor_t		*acs;
	acs = acs_constructor_create();
	if (acs == NULL) {
		return NULL;
	}


	int count = 0;
	for (value = acs_strings; value->len; value++, count++) {
		acs_add_pattern(acs, value, count);
	}

	propagate_faillink(acs);

	return acs;
}


static int acs_match(acs_constructor_t *acs, u_char *str, size_t len, acs_match_result_t *res)
{
	acs_state_t		*root = &acs->root;
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
		res->start = idx - 1;
		res->end = idx - 1;
		res->index = state->index;
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
			res->start = idx - state->depth;
			res->end = idx - 1;
			res->index = state->index;
			return 1;
		}
					
	}

	return 0;
	
}





static int acs_array_init(acs_array_t *array, size_t size, int n)
{
	array->nelts = 0;
	array->size = size;
	array->nalloc = n;
	array->elts = malloc(n *size);
	return 0;
}

static void *acs_array_push(acs_array_t *a)
{
	if (a->nelts == a->nalloc) {
		u_char		*new;
		size_t		sz = a->size * a->nalloc * 2;
		new = malloc(sz);
		if (new == NULL) {
			return new;
		}

		memcpy(new, a->elts, sz / 2);
		free(a->elts);
		a->elts = new;
		a->nalloc = a->nalloc * 2;
	}

	u_char *start = a->elts;
	u_char *pos = start + a->nelts * a->size;
	a->nelts++;
	return pos;
}

static void acs_array_clear(acs_array_t *a)
{
	if (a->elts != NULL) {
		free(a->elts);
	}
	return;
}



static int propagate_faillink(acs_constructor_t *acs)
{
	acs_state_t		*root = &acs->root;
	acs_array_t		a;
	acs_state_t		*state;
	acs_array_init(&a, sizeof(acs_state_t *), 1024);

	u_char c = 0;
	for (; c < CHAR_SET_SIZE; c++) {
		if (root->goto_map[c] != NULL) {
			acs_state_t	**tmp;
			tmp = acs_array_push(&a);
			*tmp = root->goto_map[c];
			(*tmp)->fail_link = root;
		}
	}

	acs_state_t		*goto_save[CHAR_SET_SIZE];
	memcpy(goto_save, root->goto_map, sizeof(acs_state_t *) * CHAR_SET_SIZE);


	//int i ;
	for (c=0; c < CHAR_SET_SIZE; c++) {
		acs_state_t *s = acs_state_get_goto(root, c);
		if (s == NULL) {
			acs_state_set_goto(root, c, root);
		}
	}
	
	int current = 0;

	while (1) {

		for (c = 0; c < CHAR_SET_SIZE; c++) {
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
 				acs_state_t		**aaa = acs_array_push(&a);
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


static int acs_add_pattern(acs_constructor_t *acs, acs_str_t *value, int index)
{
	acs_state_t		*state = &acs->root;
	u_char			*str = value->data;
	int i = 0;
	for (i = 0; i < value->len; i++) {
		const char c = str[i];
		acs_state_t		*new = acs_state_get_goto(state, c);
		if (new == NULL) {
			new = state_alloc();
			new->depth = state->depth + 1;
			acs_state_set_goto(state, c, new);
		}
		state = new;
	}
	state->terminal = 1;
	state->index = index;
	return 0;
}

static acs_state_t *acs_state_get_goto(acs_state_t *state, u_char c)
{
	return state->goto_map[c];

}

static void acs_state_set_goto(acs_state_t *state, u_char c, acs_state_t *next)
{
	state->goto_map[c] = next;
	return;
}
