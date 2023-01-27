local inputlabels = {"P1 A", "P1 B", "P1 Select", "P1 Start", "P1 Right", "P1 Left", "P1 Up", "P1 Down"}

local timestamps = io.open("timestamps.r08", "wb")
local lastinput = -1


function getinputbyte_p1()
  if not movie.isloaded() then
    return 0
  end
 
  --local t = movie.getinput(emu.framecount() - 1)
  local t = movie.getinput(emu.framecount())
  local b = 0
 
  if (t["P1 Right"] == true)  then b = 1; end;
  if (t["P1 Left"] == true)   then b = b+2; end;
  if (t["P1 Down"] == true)   then b = b+4; end;
  if (t["P1 Up"] == true)     then b = b+8; end;
  if (t["P1 Start"] == true)  then b = b+16; end;
  if (t["P1 Select"] == true) then b = b+32; end;
  if (t["P1 B"] == true)      then b = b+64; end;
  if (t["P1 A"] == true)      then b = b+128; end;
 
  return b
end

function getinputbyte_p2()
  if not movie.isloaded() then
    return 0
  end
 
  --local t = movie.getinput(emu.framecount() - 1)
  local t = movie.getinput(emu.framecount())
  local b = 0
 
  if (t["P2 Right"] == true)  then b = 1; end;
  if (t["P2 Left"] == true)   then b = b+2; end;
  if (t["P2 Down"] == true)   then b = b+4; end;
  if (t["P2 Up"] == true)     then b = b+8; end;
  if (t["P2 Start"] == true)  then b = b+16; end;
  if (t["P2 Select"] == true) then b = b+32; end;
  if (t["P2 B"] == true)      then b = b+64; end;
  if (t["P2 A"] == true)      then b = b+128; end;
 
  return b
end
 
function tryinput()
  local input_p1 = getinputbyte_p1()
  local input_p2 = getinputbyte_p2()
 
  timestamps:write(string.char(input_p1))
  timestamps:write(string.char(input_p2))
end
 
event.oninputpoll(tryinput)
 
--tryinput()
 
while emu.framecount() <= movie.length() do
  emu.frameadvance()
end
 
tryinput()
timestamps:close()
 
client.pause()