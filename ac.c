/*
 * =====================================================================================
 *
 *       Filename:  ac.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2016年02月18日 13时59分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *<F3>
 * =====================================================================================
 */
#include "ac.h"


int ac_array_init(ac_array_t *array, size_t size, int n)
{
	array->nelts = 0;
	array->size = size;
	array->nalloc = n;
	array->elts = malloc(n *size);
	return 0;
}


void *ac_array_push(ac_array_t *a)
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


void ac_array_clear(ac_array_t *a)
{
	a->nelts = 0;
	return;
}


void ac_array_destory(ac_array_t *a)
{
	if (a->elts != NULL) {
		free(a->elts);
	}
	return;
}


#define ac_string(str)			{sizeof(str)-1, (u_char *)str}

static ac_str_t ac_strings[] = {
	ac_string("thie"),
	ac_string("her"),
	ac_string("his"),
	ac_string("thfslijg"),
	{0, NULL},
};
static char		match_src[] = "afddfagewggdgthfslijghiserhersr";
int main(int argc, char *argv[])
{
	ac_str_t	*value;
	int			count = 0;

	for (value = ac_strings; value->len; value++) {
		acs_log_error(1, 0, "count:%d, \tdata:%s, \tlen:%lu.", count, value->data, value->len);
		count++;
	}
	  
	acs_constructor_t		*acs;
	acs = acs_construct(&ac_strings[0], count);
	if (acs == NULL) {
		fprintf(stdout, "hehe");
		exit(1);
	}

	ac_result_t	res;
	memset(&res, 0, sizeof(res));

	if (acs_match(acs, (u_char *)match_src, sizeof(match_src) - 1, &res) == 1) {
		fprintf(stderr, "matched.\n");
		fprintf(stderr, "matched start:%d.\n", res.match_begin);
		fprintf(stderr, "matched end:%d.\n", res.match_end);
		fprintf(stderr, "matched index:%d.\n", res.pattern_idx);
	} else {
		fprintf(stderr, "Not match.\n");
	}

	void *buf = ac_convert(acs);
	if (buf == NULL) {
		fprintf(stderr, "ac convert succeed.\n");
	}

	res = ac_match(buf, match_src, sizeof(match_src) - 1);
	if (res.match_begin != -1) {
		fprintf(stderr, "matched.\n");
		fprintf(stderr, "matched start:%d.\n", res.match_begin);
		fprintf(stderr, "matched end:%d.\n", res.match_end);
		fprintf(stderr, "matched index:%d.\n", res.pattern_idx);
	} else {
		fprintf(stderr, "Not match.\n");
	}
	acs_constructor_free(acs);
	return 0;
}
