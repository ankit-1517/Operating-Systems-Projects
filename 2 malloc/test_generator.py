import numpy as np
np.random.seed(0)

n = 20
allocate = []
alloted = [0]*20
free = []
print_command = 'my_heapinfo();\nprint_free_list();'
for i in range(n):
	s = 'void* a' + str(i) + ' = my_alloc(' + str(80*np.random.randint(1, 10)) + ');'
	allocate.append(s)
	s = 'my_free(a' + str(i) + ');'
	free.append(s)
	if np.random.randint(0, 3)%3 == 0:
		free.append(s)

while len(allocate) + len(free) > 0:
	if len(allocate) > 0 and np.random.randint(0, 2)%2 > 0:
		# allocate
		index = np.random.randint(0, len(allocate))
		s = allocate[index]
		print(s)
		print(print_command)
		allocate.pop(index)
		index = int(s.split("=")[0].split("a")[1].strip(" "))
		alloted[index] = 1
	else:
		# free
		index = np.random.randint(0, len(free))
		s = free[index]
		ind = int(s.split(")")[0].split("a")[1].strip(" "))
		if alloted[ind] == 1:
			print(s)
			print(print_command)
			free.pop(index)


