-- Adapt class names according to your classes set for the "FileSampleWriter"!
classnames = {"start", "stop", "next", "previous", "restart"}
counter = 0

-- size of array "classnames"
maxcount = 5*1

function interpret (data, sname, ename, estate, isotime)
	counter = counter + 1
	output = classnames[counter]
	
	print("recorded gesture: " .. classnames[counter])
	
	if counter == maxcount then
		print("You can now stop the pipeline or record more samples!")
		print("")
		counter = 0
	end
	
	print("")
	print("next gesture: " .. classnames[counter + 1])
	return output
end
