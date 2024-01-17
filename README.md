# MultiMedia-Networking

Welcome to the RTSP/RTP/RTCP Video Streaming Project implemented in C++ for multimedia networking. This project focuses on building a video streaming server and client using Real-Time Streaming Protocol (RTSP), Real-time Transport Protocol (RTP), and Real-time Transport Control Protocol (RTCP). The server streams MJPEG video in the form of concatenated JPEG images, responding to various client requests.

Key Features:

RTSP Implementation:

Follows the specification outlined in RFC 7826 (Real Time Streaming Protocol).
Allows client functionalities like play, pause, reposition, and stop, with corresponding server responses.

RTP Implementation:

Complies with RFC 3550 (Real-time Transport Protocol).
Server packetizes video data into RTP packets, setting RTP header fields and copying payload accordingly.
Client handles received RTP packets for seamless video playback.

RTCP Implementation:

Adheres to RFC 3550 (Real-time Transport Control Protocol).
Supports the transmission and reception of SR (Sender Report), RR (Receiver Report), and BYE RTCP packets.

CImg Library Integration:

Utilizes the "CImg Library" (http://cimg.eu/) for video playback.
Example for playing a sequence of images can be found at the e-learning course website (https://elearning.just.edu.jo).
The library is used exclusively for playing local files in a specific directory.

Language and Platform:

Developed in C++ and designed to run on Linux.
Independent implementation of RTSP/RTP/RTCP, without external code dependencies.
