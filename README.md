
A little 
# Client - server
project based off of an [online tutorial](http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html).

If you build it w/ make, you can start src/server \<port\> and src/client \<hostname\> \<port\> and play.

You can:

  + send message to server: type "m \<message\>" into the console
  + request ls or pwd from server: type "c l" or "c c"
  + request and print file from server: type "c f \<filename\>"
  + execute arbitrary (single worded) command on server and get output: type "c e \<command\>"
  + kill server remotely: "c k"
  + kill client: "q"
  
For error checking, debugging and testing as well as the basic Makefile outline and project structure,
the project relies heavily on material from [Learn C the hard way](https://learncodethehardway.org/c/) by Zed A Shaw. 
If you want to get into C, I definitely recommend it.
