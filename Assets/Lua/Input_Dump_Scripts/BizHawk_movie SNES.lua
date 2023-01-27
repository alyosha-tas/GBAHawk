-- Run this with the movie; when the movie is over, ADVANCE TWO MORE FRAMES, then STOP!

local movie_loaded = false;     -- This tells if the program has initialized for the movie
local movie_filename = "";      -- Will hold the movie file name (.fm2)
local output_filename = "";     -- Will hold the output file name (.r08)
local fhnd;                     -- For ouputing to the file

local lagged = false;
local prev_input = {};
local temp = {};

local skip_lag = true;
local frame = 0;


function update_bitfile(fhnd, prev_input)
    -- Order of buttons: RLDUTSBA
    local player_L = {0, 0};
	local player_H = {0, 0};
	--print(tostring(1 | 2));
	
	if (prev_input["P1 Right"] == true)  		then player_L[1] = 1; end;
	if (prev_input["P1 Left"] == true)   		then player_L[1] = player_L[1]+2; end;
	if (prev_input["P1 Down"] == true)   		then player_L[1] = player_L[1]+4; end;
	if (prev_input["P1 Up"] == true)    		then player_L[1] = player_L[1]+8; end;
	if (prev_input["P1 Start"] == true)  		then player_L[1] = player_L[1]+16; end;
	if (prev_input["P1 Select"] == true) 		then player_L[1] = player_L[1]+32; end;
	if (prev_input["P1 Y"] == true)     		then player_L[1] = player_L[1]+64; end;
	if (prev_input["P1 B"] == true)    			then player_L[1] = player_L[1]+128; end;
	
	if (prev_input["P2 Right"] == true)  		then player_L[2] = 1; end;
	if (prev_input["P2 Left"] == true)   		then player_L[2] = player_L[2]+2; end;
	if (prev_input["P2 Down"] == true)   		then player_L[2] = player_L[2]+4; end;
	if (prev_input["P2 Up"] == true)    		then player_L[2] = player_L[2]+8; end;
	if (prev_input["P2 Start"] == true)  		then player_L[2] = player_L[2]+16; end;
	if (prev_input["P2 Select"] == true) 		then player_L[2] = player_L[2]+32; end;
	if (prev_input["P2 Y"] == true)     		then player_L[2] = player_L[2]+64; end;
	if (prev_input["P2 B"] == true)    			then player_L[2] = player_L[2]+128; end;

	if (prev_input["P1 R"] == true)  			then player_H[1] = 16; end;
	if (prev_input["P1 L"] == true)   			then player_H[1] = player_H[1]+32; end;
	if (prev_input["P1 X"] == true)   			then player_H[1] = player_H[1]+64; end;
	if (prev_input["P1 A"] == true)    			then player_H[1] = player_H[1]+128; end;
	
	if (prev_input["P2 R"] == true)  			then player_H[2] = 16; end;
	if (prev_input["P2 L"] == true)   			then player_H[2] = player_H[2]+32; end;
	if (prev_input["P2 X"] == true)   			then player_H[2] = player_H[2]+64; end;
	if (prev_input["P2 A"] == true)    			then player_H[2] = player_H[2]+128; end;

    fhnd:write(string.char(player_L[1]));
	fhnd:write(string.char(player_H[1]));
	fhnd:write(string.char(player_L[1]));
	fhnd:write(string.char(player_H[1]));
	fhnd:write(string.char(player_L[1]));
	fhnd:write(string.char(player_H[1]));
	fhnd:write(string.char(player_L[1]));
	fhnd:write(string.char(player_H[1]));
	
    fhnd:write(string.char(player_L[2]));
	fhnd:write(string.char(player_H[2]));
	fhnd:write(string.char(player_L[2]));
	fhnd:write(string.char(player_H[2]));
	fhnd:write(string.char(player_L[2]));
	fhnd:write(string.char(player_H[2]));
	fhnd:write(string.char(player_L[2]));
	fhnd:write(string.char(player_H[2]));
	
end

--emu.speedmode("turbo")

while (true) do

    
    -- Mandatory
    emu.frameadvance();
	
	    -- If a movie is loaded into the emulator
    if (movie.isloaded() == true) then
        -- When a movie is loaded for the first time, we need to do some setup
        if (movie_loaded == false) then
            -- First, restart the movie at the beginning
            --movie.playbeginning();
            
            -- Lets make up the output filename
            -- Take the video name, remove the .fm2 and replace with .r08
            movie_filename = movie.filename();
            output_filename = string.sub(movie_filename, 0, string.len(movie_filename)-4) .. ".frame.r16m";
            
            -- Print it out for debugging
            print("Writing to " .. output_filename .. "...");
            
            -- Setup the file handle to write to it, open in binary mode
            fhnd = io.open(output_filename, "wb+");

            if (fhnd == nil) then
                print("Error opening movie file!");
            end
           
            -- Now we are ready to go.
            movie_loaded = true;
        end
		
		-- Check for lag frames and record it
        if (emu.islagged() == true) then
            lagged = true;
        else
            lagged = false;
        end
		
		-- Also read the input for the previous frame
        prev_input = movie.getinput(emu.framecount()-1);
          	
        -- There are some issues with starting this code, so we pretend to skip the first frame
        -- I believe the problem is because when the video is loaded it is at frame 0 which has no input
        -- We need to skip this frame, look for lag, then start watching for input
        if (emu.framecount() > 0) then
            -- We need to skip any lag frames and only output frames where the console is looking for input
            if (lagged == true) then
                fhnd:write("");
            else
                frame = frame + 1;
                
                -- Convert the buttons from the movie into byte format and store it to the file
                update_bitfile(fhnd, prev_input);
            end
        end
        
    else
        -- If the movie has ended, then our work here is done. Clean up
        if (movie_loaded == true) then
            update_bitfile(fhnd, prev_input);

            fhnd:close();
            
            print("DONE");
            movie_loaded = false;
            frame = 0;
        end
    end
end