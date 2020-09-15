import random
import string



def main():
    #creating 3 random strings of length 10
    mystring1 = ''.join([random.choice(string.ascii_lowercase) for n in range(10)])
    mystring2 = ''.join([random.choice(string.ascii_lowercase) for n in range(10)])
    mystring3 = ''.join([random.choice(string.ascii_lowercase) for n in range(10)])

    #creates file if not already created
    file1 = open("file1.txt", "w+")
    file1.write("%s\n" % mystring1)  #writes the string to file with a newline char at the end
    file1.close()
    #file2
    file2 = open("file2.txt", "w+")
    file2.write("%s\n" % mystring2)
    file2.close()
    #file3
    file3 = open("file3.txt", "w+")
    file3.write("%s\n" % mystring3)
    file3.close()
    #printing the 3 random strings to stdout
    print(mystring1)
    print(mystring2)
    print(mystring3)

    #creating 3 random ints and printing them out as well as their product
    num1 = random.randint(1, 43)
    num2 = random.randint(1, 43)
    print(num1)
    print(num2)
    print(num1*num2)




main()
