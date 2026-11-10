# C4 model fragment - Offbeat-IoT/example-spotify-control
# Consumed by Offbeat-IoT/information/scripts/c4/build.py.

example_spotify_control = softwareSystem "Example Spotify Control" "Standalone ESP8266 example sketch that turns an ultrasonic distance sensor into a Spotify remote control. The sketch connects to the Offbeat websocket as a device, averages five distance measurements and maps distance bands to spotify.next, play, pause, previous, shuffle and volume commands, and is published as a documentation example." {
  tags "firmware"
}
