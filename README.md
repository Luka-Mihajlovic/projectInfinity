# projectInfinity
# Created in February 2021!! This is OLD, but is still an interesting project that I'm fond of and would like to remake in the future.

The main idea was seeing how far I could go with to create a functioning game in C without any intense libraries or engines, so just constraining myself to things like <string.h> for string manipulation etc.

Working on this was one of my first forays into game design, so I went with a simple text-based roguelike.
All of the graphics are from images converted into ascii through third-party means, then stored within files. All items and enemies can be easily 'modded' in by adding them to the respective folders using the template files provided.
The game is endless, only finishing once the player dies or quits the game. There is currently no data saving implemented, but all high scores are tracked in files.

It was fun creating some of the game logic and implementing it, especially around the data structures within folders and all of the challenges that came from there. 
File I/O was one of the priorities, but the main goal was trying to create a fun looking user interface with console limitations. I did not want to use the Curses library for this project due to personal challenges.

## Simply run the "main.exe" application and enjoy! No further setup required, but the game is currently only available in Serbian.
