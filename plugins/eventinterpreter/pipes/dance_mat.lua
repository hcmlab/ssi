-- interpretation for simple dance mat with 11 fields
-- (c)2015 Andreas Seiderer

function string:split(delimiter)
  local result = { }
  local from = 1
  local delim_from, delim_to = string.find( self, delimiter, from )
  while delim_from do
    table.insert( result, string.sub( self, from , delim_from-1 ) )
    from = delim_to + 1
    delim_from, delim_to = string.find( self, delimiter, from )
  end
  table.insert( result, string.sub( self, from ) )
  return result
end

oldoutput = ""

function interpret (data)

	local splitstr = string.split(data,",")
	
	local valuecount = splitstr[1]
	
	if valuecount ~= "10" then
		return "wrong device"
	end
	
	-- local outputbtn = ""
	
	local btndown = splitstr[3] == "1"
	-- if (btndown) then outputbtn = "btndown" end
	
	local btnup = splitstr[4]  == "1"
	-- if (btnup) then outputbtn = "btnup" end
	
	
	local btnleft = splitstr[2] == "1"
	-- if (btnleft) then outputbtn = "btnleft" end
	
	local btnright = splitstr[5] == "1"
	-- if (btnright) then outputbtn = "btnright" end
	
	
	local btncenter = splitstr[12] ~= "127"
	-- if (btncenter) then outputbtn = "btncenter" end
	
	
	local btnbelowright = splitstr[7] == "1"
	-- if (btnbelowright) then outputbtn = "btnbelowright" end
	
	local btnbelowleft = splitstr[6] == "1"
	-- if (btnbelowleft) then outputbtn = "btnbelowleft" end
	
	
	local btntopright = splitstr[9] == "1"
	-- if (btntopright) then outputbtn = "btntopright" end
	
	local btntopleft = splitstr[8] == "1"
	-- if (btntopleft) then outputbtn = "btntopleft" end
	
	
	local btnstart = splitstr[11] == "1"
	-- if (btnstart) then outputbtn = "btnstart" end
	
	local btnselect = splitstr[10] == "1"
	-- if (btnselect) then outputbtn = "btnselect" end
	
	
	local output = ""
	
	local left = btnselect or btntopleft or btnleft or btnbelowleft
	local middle =  btndown or btncenter or btnup
	local right = btnstart or btntopright or btnright or btnbelowright
	
	
	if ((left and middle) or (right and middle) or (right and left)) then 
		output = "balanced" 
	elseif(left and not right) then
		output = "left_only"
	elseif (right and not left) then
		output = "right_only"
	elseif (right or middle or left) then
		output = "balanced"
	else
		output = "no_presence"
	end

	if (oldoutput == output) then
		output = ""
	else
		oldoutput = output
	end
	

	
	return output
end