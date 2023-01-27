local inputlabels = {"P1 A", "P1 B", "P1 Select", "P1 Start", "P1 Right", "P1 Left", "P1 Up", "P1 Down"}

local timestamps = io.open("timestamps.txt", "w")
local lastinput = -1

function getinputbyte()
  if not movie.isloaded() then
    return 0
  end
 
  --local t = movie.getinput(emu.framecount() - 1)
  local t = movie.getinput(emu.framecount())
  local b = 0
 
  for i = 0, 7 do
    if t[inputlabels[i + 1]] then
      b = b + bit.lshift(1, i)
    end
  end
 
  return b
end
 
function tryinput()
  local input = getinputbyte()
 
  if input ~= lastinput then
    local cycles = (emu.totalexecutedcycles() - 3) / 2;
    --note: use 486376 for Wacky Racers
	cycles = (cycles + 485808) / 512;
   
    timestamps:write(string.format("%08X %04X", math.ceil(cycles), input))
    timestamps:write("\n")
    lastinput = input
  end
end
 
event.oninputpoll(tryinput)
 
--tryinput()
 
while emu.framecount() <= movie.length() do
  emu.frameadvance()
end
 
tryinput()
timestamps:close()
 
client.pause()