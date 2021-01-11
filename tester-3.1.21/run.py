import os

if __name__ == "__main__":
	print('----- START TESTER(3.1.21) -----')
	print('notice this tester is not 100% completed. passing (or failing) all the tests does not mean your code is ready for submission. all this tester does is to run different boards with your code and compare the input with what we assume should be the expected output. especially, this tester does not test correctness of mutex using, multi threading, structure of code, tile/gen hist or that it pass makefile given by the Segel')
	print('----- now try to compile -----')
	# os.system('make -f ./put_all_your_code_here/makefile')
	os.system('g++ -std=c++11 -pthread -o GameOfLife $(find put_all_your_code_here -type f -iregex ".*\.cpp") $(find put_all_your_code_here -type f -iregex ".*\.hpp")')  # if you want to compile without the Segel's makefile flags
	print('----- compile finished. now runs your code (should take no more than few seconds! if not - something went wrong...) -----')
	os.system('./GameOfLife')
	print('----- FINISHED TESTER(3.1.21) -----')
	print('notice this tester is not 100% completed. passing (or failing) all the tests does not mean your code is ready for submission. all this tester does is to run different boards with your code and compare the input with what we assume should be the expected output. especially, this tester does not test correctness of mutex using, multi threading, structure of code, tile/gen hist or that it pass makefile given by the Segel')
