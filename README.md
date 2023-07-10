# Client-server-chat-room-using-socket-programming-in-the-C-programming-language-with-GUI

# INTRODUCTION
With the proliferation of internet-enabled devices, communication has become easier and more accessible than ever before. One of the most popular forms of communication on the internet is messaging, which enables people to send and receive messages in real-time. This project aims to build a client-server chat room that enables users to communicate with each other seamlessly.

# PLATFORM AND LANGUAGES
The chat room will be developed using the C programming language, which is widely used for system programming due to its low-level capabilities. The chat room will use the TCP protocol for communication, as it is a reliable, connection-oriented protocol that guarantees the delivery of packets to the receiver in the same order as they were sent.

To build the graphical user interface (GUI) for the chat room, we will be using the GTK (GIMP Toolkit) library. GTK is an open-source library commonly used for creating GUIs in Linux-based systems. It provides a set of tools, widgets, and graphical elements that can be used to build an interactive and visually appealing user interface.

# METHODOLOGY
The chat room will be implemented using socket programming in C. Socket programming is a low-level programming interface that enables processes to communicate with each other over a network. With socket programming, we can establish a connection between a client and a server, allowing for bidirectional communication.

The chat room will accommodate multiple clients and servers, allowing for simultaneous communication between users. Whenever a client connects to the chat room server, the server is notified of the connection, ensuring all active clients are accounted for.

To establish a connection with the TCP server, clients will access the specific port number assigned to the server. Each machine has its unique port number, and this port number will be used to ensure that communication between clients and the server is seamless.

The chat room will feature a user-friendly GUI interface that makes it easy for users to navigate and interact with the application. The GUI interface will be designed using the GTK library, which provides a set of tools, widgets, and graphical elements that make the chat room visually appealing and interactive.

# Features
The chat room application will be designed with the following features to ensure a smooth and seamless communication experience for users:

o	Multiple Clients and Servers: The chat room application will be designed to support multiple clients and servers, enabling simultaneous communication between users.

o	TCP Protocol: The chat room application will use the TCP protocol to establish and maintain network connections, ensuring reliable delivery of messages.

o	Notifications: Whenever a client connects or disconnects from the chat room server, the server will be notified along with the client's username. This feature ensures that all active clients are accounted for, and the server is aware of the current state of the chat room.

o	User-Friendly GUI: The chat room application will feature a user-friendly GUI interface designed using the GTK library. The GUI interface will be interactive and visually appealing, providing users with a seamless and easy-to-use interface.

