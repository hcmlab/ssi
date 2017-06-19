import dollar


def consume(info, sin, board, opts, vars): 

	points = []
	s = sin[0]
	num = s.num
	for i in range(0,num):
		points.append((s[i*2+1],s[i*2+0]))

	recognizer = dollar.Recognizer ()
	result = recognizer.Recognize(points)
	print(result.Name)