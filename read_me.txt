
Aya AL Antari , Hammam AL Qura'an , Moawiah AL Doum

link of vid images : https://drive.google.com/drive/folders/1kS3yOh2EjsdvBGZ9yWdlpTuJTM2LpUEi?usp=sharing

open 2 terminals in the file directory
one for the server and for the client

in server terminal run this line:
step 1: g++ server.cpp -o server -pthread

client terminal:
step 2: g++ client.cpp -o client -O2 -L/usr/X11R6/lib -lm -lpthread -lX11

step 3: at server side run this:
./server 12345              //5 digit port number for server

step 4: at client side run this:
./client 127.0.0.1 12345 12346    //5 digit port number for server and client

done the video will start streaming.

be sure that imagemagick is insalled:
sudo apt update
sudo apt -y install imagemagick


