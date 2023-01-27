local coreName = movie.getheader()["Core"]
local powerOnCycles = 0
--local modeSwitchGBAToGBCDelay = 948.84375
local modeSwitchGBAToGBCDelay = 0
local inputlabels
local powerButtonLabel
local coreToGBAAudioSampleRatio

if coreName == "Gambatte" then
  inputlabels = {"A", "B", "Select", "Start", "Right", "Left", "Up", "Down"}
  powerButtonLabel = "Power"
  coreToGBAAudioSampleRatio = 512
elseif coreName == "GBHawk" or coreName == "SubGBHawk" then
  inputlabels = {"P1 A", "P1 B", "P1 Select", "P1 Start", "P1 Right", "P1 Left", "P1 Up", "P1 Down"}
  powerButtonLabel = "P1 Power"
  coreToGBAAudioSampleRatio = 1024
else
  error("Unknown Emulator Core")
end

function getinputbyte(t)
  if not movie.isloaded() then
    return 0
  end
  
  local b = 0
  
  for i = 0, 7 do
    if t[inputlabels[i + 1]] then
      b = b + bit.lshift(1, i)
    end
  end
  
  return b
end

function tryinput(file)
  local t = movie.getinput(emu.framecount())
  local input = getinputbyte(t)
  local cycles = emu.totalexecutedcycles()
  if t[powerButtonLabel] then
    powerOnCycles = cycles
  end
  
  if input ~= lastinput then
	  cycles = math.ceil(((cycles - powerOnCycles) / coreToGBAAudioSampleRatio) + modeSwitchGBAToGBCDelay)
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