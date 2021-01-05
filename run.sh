# This file contains the commands to launch the server and the client
# Use each commands in a screen or separate terminal

# Compile and start server
gcc src/server.c -o build/TUCS_server && ./build/TUCS_server

# Compile and start client with an argument (file path)
gcc src/client.c -o build/TUCS_client && ./build/TUCS_client /home/erwan/test