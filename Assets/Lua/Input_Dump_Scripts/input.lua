local inputlabels = {"A", "B", "Select", "Start", "Right", "Left", "Up", "Down"}
 
function getinputbyte()
  if not movie.isloaded() then
    return 0
  end
 
  local t = movie.getinput(emu.framecount())
  local b = 0
 
  for i = 0, 7 do
    if t[inputlabels[i + 1]] then
      b = b + bit.lshift(1, i)
    end
  end
 
  return b
end
 
function tryinput(file)
  local input = getinputbyte()
 
  if input ~= lastinput then
    local cycles = emu.totalexecutedcycles();
	--note: use 486376 for Wacky Racers
    cycles = (cycles) / 512;
   
    file:write(string.format("%08X %04X", cycles, input))
    file:write("\n")
    lastinput = input
  end
end
 
local timestamps = io.open("timestamps.txt", "w")
local lastinput = -1
 
tryinput(timestamps)
 
while emu.framecount() <= movie.length() do
 
  tryinput(timestamps)
 
  emu.frameadvance()
end
 
tryinput(timestamps)
timestamps:close()
 
client.pause()