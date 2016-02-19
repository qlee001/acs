
/*
 * =====================================================================================
 *
 *       Filename:  ac_lua.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2016年02月19日 11时04分49秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <lua.h>
#include <lauxlib.h>
#include "ac.h"

static const char *lib_name = "ahocorasick";

static void *memory_alloc(uint64 ptr, size_t size)
{
    lua_State   *L;
    
    L = (lua_State *)ptr;
    return lua_newuserdata(L, size);
}

static void memory_free(uint64 ptr, void *buf)
{
    /*  Nothing to do */
    return;
}


static int lac_create(lua_State *L)
{
    int     input_tab = 1;
    luaL_checktype(L, input_tab, LUA_TTABLE);

    /* Init the "interator". */
    lua_pushnil(L);
    
    int     count = 0;
    size_t  sz = 0;
    while (lua_next(L, input_tab)) {
        size_t str_len;
        luaL_checklstring(L, -1, &str_len);
        sz += str_len;
        count++;
        /*  pop the value , but keep the key as the iterator*/
        lua_pop(L, 1);
    }

    char        *buf = malloc(sz);
    if (buf == NULL) {
        goto failed;
    }

    ac_array_t      wl;
    ac_array_init(&wl, sizeof(ac_str_t), count);
    if (!wl.elts) {
        goto free_str;
    }
    lua_pushnil(L);
    char        *pos = buf;
    input_tab = 1;
    while (lua_next(L, input_tab)) {
        size_t str_len;
        const char *str = luaL_checklstring(L, -1, &str_len);

        ac_str_t    *s = ac_array_push(&wl);
        s->data = (u_char *)pos;
        s->len = str_len;
        strncpy(pos, str, str_len);
        pos += str_len;

        lua_pop(L, 1);
    }
    // remove the table.
    lua_pop(L, 1);
    
    acs_constructor_t   *acs;
    acs = acs_construct(wl.elts, wl.nelts);
    if (acs == NULL) {
        goto free_array;
    }

    ac_bufalloc_t       alloc;
    alloc.malloc = memory_alloc;
    alloc.free = memory_free;
    alloc.data = (uint64)L;

    void *ac = ac_convert(acs, &alloc);
    if (ac == NULL) {
        goto free_acs;
    }

    free(buf);
    ac_array_destory(&wl);
    acs_constructor_free(acs);
    
    return 1;

free_acs:
    acs_constructor_free(acs);
free_array:
    ac_array_destory(&wl);
free_str:
    free(buf);
failed:
    return 0;
}


static int lac_match(lua_State *L)
{
    int n;
    n = lua_gettop(L);
    if (n != 2 && n != 3) {
        lua_pushnil(L);
        return 1;
    }

    buf_header_t *ac = (buf_header_t *)lua_touserdata(L, 1);
    if (!ac) {
        luaL_checkudata(L, 1, lib_name);
        return 0;
    }

    size_t  len;
    const char  *str;
    str = lua_tolstring(L, 2, &len);
    if (!str) {
        luaL_checkstring(L, 2);
        return 0;
    }

    int pos = 0;
    if (n == 3) {
        pos = luaL_checknumber(L, 3);
        if (pos <= 0) {
            pos = 0;
        }
    }
    const char *str2 = str + pos;

    ac_result_t r = ac_match((ac_buffer_t *)ac, str2, len - pos);
    if (r.match_begin != -1) {
        lua_pushinteger(L, r.match_begin);
        lua_pushinteger(L, r.match_end);
        lua_pushinteger(L, r.pattern_idx);
        return 3;
    }
    return 0;
}


static const struct luaL_Reg lib_funcs[] = {
    { "create", lac_create },
    { "match", lac_match },
    { 0, 0}
};

int luaopen_ac(lua_State *L) {
    luaL_newmetatable(L, lib_name);
    luaL_register(L, lib_name, lib_funcs);
    return 1;
}
