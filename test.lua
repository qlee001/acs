local ac = require 'ac'

--local pattern = {'heiser', 'fsaeg', 'fgsaewtg', 'fsagsggrwr', 'herisfaehg', 'thierer'}
local pattern = {"he", "she", "his", "her", "str\0ing"}
local inst = ac.create(pattern)
if not inst then
    print('ac create failed')
end

print('inst type:', type(inst))

local begin, match_end, idx = ac.match(inst, 'rfeauitghherheiserrfaseg', 0)
if not begin then
    print('Not match')
else
    print('Match at:', begin, ' end:', match_end, ' idx:', idx)
end
    
