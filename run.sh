gcc src/client.c -o build/TUCS_client
gcc src/server.c -o build/TUCS_server

sudo screen -dmS TUCS_server bash -c './build/TUCS_server'
./build/TUCS_client
