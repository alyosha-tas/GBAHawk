local timestamps = io.open("timestamps.r16m", "wb")
local lastinput = -1


function getinputbyte_p1_L()
  if not movie.isloaded() then
    return 0
  end
 
  --local t = movie.getinput(emu.framecount() - 1)
  local t = movie.getinput(emu.framecount())
  local b = 0

  if (t["P1 Right"] == true)    then b = b+1; end;
  if (t["P1 Left"] == true)     then b = b+2; end;
  if (t["P1 Down"] == true) 	then b = b+4; end;
  if (t["P1 Up"] == true)  		then b = b+8; end;
  if (t["P1 Start"] == true)    then b = b+16; end;
  if (t["P1 Select"] == true)   then b = b+32; end;
  if (t["P1 Y"] == true)   		then b = b+64; end;
  if (t["P1 B"] == true)  		then b = b+128; end;
 
  return b
end

function getinputbyte_p1_H()
  if not movie.isloaded() then
    return 0
  end
 
  --local t = movie.getinput(emu.framecount() - 1)
  local t = movie.getinput(emu.framecount())
  local b = 0
  
  if (t["P1 R"] == true)    then b = b+16; end;
  if (t["P1 L"] == true)   	then b = b+32; end;
  if (t["P1 X"] == true)   	then b = b+64; end;
  if (t["P1 A"] == true)  	then b = b+128; end;
 
  return b
end

function getinputbyte_p2_L()
  if not movie.isloaded() then
    return 0
  end
 
  --local t = movie.getinput(emu.framecount() - 1)
  local t = movie.getinput(emu.framecount())
  local b = 0
 
  if (t["P2 Right"] == true)    then b = b+1; end;
  if (t["P2 Left"] == true)     then b = b+2; end;
  if (t["P2 Down"] == true) 	then b = b+4; end;
  if (t["P2 Up"] == true)  		then b = b+8; end;
  if (t["P2 Start"] == true)    then b = b+16; end;
  if (t["P2 Select"] == true)   then b = b+32; end;
  if (t["P2 Y"] == true)   		then b = b+64; end;
  if (t["P2 B"] == true)  		then b = b+128; end;
 
  return b
end

function getinputbyte_p2_H()
  if not movie.isloaded() then
    return 0
  end
 
  --local t = movie.getinput(emu.framecount() - 1)
  local t = movie.getinput(emu.framecount())
  local b = 0
 
  if (t["P2 R"] == true)    then b = b+16; end;
  if (t["P2 L"] == true)   	then b = b+32; end;
  if (t["P2 X"] == true)   	then b = b+64; end;
  if (t["P2 A"] == true)  	then b = b+128; end;
 
  return b
end
 
function tryinput()
  local input_p1_L = getinputbyte_p1_L()
  local input_p2_L = getinputbyte_p2_L()
  local input_p1_H = getinputbyte_p1_H()
  local input_p2_H = getinputbyte_p2_H()
  
  timestamps:write(string.char(input_p1_L))
  timestamps:write(string.char(input_p1_H))
  timestamps:write(string.char(input_p1_L))
  timestamps:write(string.char(input_p1_H))
  timestamps:write(string.char(input_p1_L))
  timestamps:write(string.char(input_p1_H))
  timestamps:write(string.char(input_p1_L))
  timestamps:write(string.char(input_p1_H))
  
  timestamps:write(string.char(input_p2_L))
  timestamps:write(string.char(input_p2_H))
  timestamps:write(string.char(input_p2_L))
  timestamps:write(string.char(input_p2_H))
  timestamps:write(string.char(input_p2_L))
  timestamps:write(string.char(input_p2_H))
  timestamps:write(string.char(input_p2_L))
  timestamps:write(string.char(input_p2_H))
  
  
  -- expand to multitap later
  
end
 
event.oninputpoll(tryinput)
 
--tryinput()
 
while emu.framecount() <= movie.length() do
  emu.frameadvance()
end
 
tryinput()
timestamps:close()
 
client.pause()