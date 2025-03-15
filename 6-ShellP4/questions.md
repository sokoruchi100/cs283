1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The remote client determines when a command's output is fully received when it finds the specified delimiter sent by the server which tells the client this is the end of the message. In this example, the delimiter sent by the server is the EOF character. When handling partial reads, the client would need to make multiple recv calls in a loop to continuously obtain data and print it out to stdout for the user. It will continue to loop until it finds the EOF character, signalling the end of the message, or an error of some kind which results in breaking out of the loop.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

A networked shell protocol could use a method where the first character detected is the beginning and the delimiter is the end of the command, which could be a null character or EOF. Another way is by sending a header which contains various metadata, specifically one part gives information on the length of the data. Then, the other side would be able to know how long the message is and find the end. If this is not handled correctly, multiple commands might get concatenated together. The receiver might also misallocate memory and the errors in command parsing would be difficult to handle reliable command execution.

3. Describe the general differences between stateful and stateless protocols.

In a stateful protocol, the server and client maintain a continuous session where the server has its own stored state. Previous interactions influence current communication. TCP is an example of a stateful protocol. This offers richer functionality, but at the cost of more memory and resource management. In a stateless protocol, each request sent by the client is treated independently with no inherent knowledge of previous requests. The server does not maintain a state, and example of this would be UDP. It is much simpler and more scalable, but it requires the client to provide all the necessary info with each request.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

While it is unreliable because it won't guaranteee delivery, ordering, or error correction like with TCP, it is still useful. UDP has low overhead and latency, making it ideal for real time applications where speed is more important than reliability. UDP can be more efficient due to the less resources it requires. Simpler applications that can handle packet loss but require speed and efficiency will find UDP to be much more useful.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

Sockets API is enables applications to use network communication. It creates a unified interface for creating, configuring, sending, and receiving data over the network. Functions like socket(), bind(), listen(), accept(), connect(), send(), and recv() can establish both TCP and UDP communications.