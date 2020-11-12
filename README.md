# Multiple client chat app with C
This is a simple implementation of a server client chat system where once one client connects to the server it can communicate with others.
The main part of code was taken from https://gist.github.com/Abhey/47e09377a527acfc2480dbc5515df872

Some modification were made and some commands added.

The server program takes one argument as the port for connection:
server [port number]


The client program takes 3 arguments:
client [hostname] [port number] [username]

The commands that are available:

join [groupID]:
Any user can connect to a group with ID of [groupID] if the group doesn't exist it will be added and the user will join the group.

send [groupID] [message]:
The user can send messages in groups that is a member of them. everyone in that group will see the message in the format:
username: message

leave [groupID]:
The user will leave the group with ID of groupID and can no longer send messages in that group or see messages from it.

quit:
For client program will cancel the thread and close the socket connection and exit the program normally.