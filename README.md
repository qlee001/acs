# acs
a C version of aho corasick algorithm as a lua module, similar as cloudflare's lua-aho-corasick module
cloudflare's lua-aho-corasick module: https://github.com/cloudflare/lua-aho-corasick.git

aho-corasick-lua
================

C and Lua Implementation of the Aho-Corasick (AC) string matching algorithm
(http://dl.acm.org/citation.cfm?id=360855).


An example usage is shown bellow:

```lua
local ac = require "ac"
local dict = {"string1", "string", "etc"}
local acinst = ac.create(dict)
local offset = 0
local match_begin, match_end, pattern_idx = ac.match(acinst, "mystring", offset)
```

For efficiency reasons, the implementation is slightly different from the
standard AC algorithm in that it doesn't return a set of strings in the dictionary
that match the given string, instead it only returns one of them in case the string
matches.
