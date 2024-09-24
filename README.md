slashdiablo-maphack
===================

I am NOT the original author.
This version of the codebase I found on the internet, it has a bunch of things added on top of **https://github.com/planqi/slashdiablo-maphack**, which appears to be the main repo of this project.

I am merely extending it for the needs of our server's playerbase (www.auratum.net).

Feel free to use it, fork it or whatever, just make sure you give credits to the original authors.

# Changelog

- Removed some ugly overlay texts.
- Added FCR/FHR/FBR breakpoints to the player stats menu.
- Added session/lifetime kill trackers. Lifetime kills are stored in "statistics.json" file in the folder of the game.
- Added "Time to level up" tracker.
- Removed XP/s tracker.
- Modified "Autofill game" to append a number in case the game name contains only A-Z, so you don't have to start your runs with a number.
- Added stats and base stats display to the player stats menu.
- Increased chest size and made locked chests have orange color on the minimap.
- (1.13d) Added workaround for - Reveal Type Game/Act doesn't reveal certain levels, mainly ones whose entrance is a Red Portal. Pindle area, Nihlatak Temple, Arcane, Duriel.
- (1.13d) Commented out ChannelInput intercept.
- Added Merc's skills and levels in the StatDisplay menu (8).
- Added autostasher for jewels, runes and gems.
