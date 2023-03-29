# esp32 smart alarm clock

This project is just for fun, using an esp32 as an alarm clock that has a built-in web server for setting up alarms, etc.

Some plans, very tentative:
- Hardware:
  - esp32
  - SD card and adapter for storing sound files, web server files, alarm database, etc.
  - Alarm clock style segmented led display
  - photodiode for auto-brightness
  - button array for doing memory game to wake up instead of snoozing (or option for both)
  - vibration motors for extra annoyance
  - wake up lights (that get brighter closer to wake up time?)
  - 3D printed chassis (although I don't have a 3D printer lol)
- Software:
  - esp32 software:
    - Store alarms and their info in a database file (using [this](https://github.com/siara-cc/esp32_arduino_sqlite3_lib) maybe?)

       Alarm Name (text) | Time (text) | Days of the week (int, byte) | vibration (bool) | snooze (int, duration) | memory game (int, difficulty) | wake up light (int, minutes before) | sound file (text, either local file or url) | enabled (bool) | delete after (bool)
       --- | --- | --- | --- | --- | --- | --- | --- 
       Alarm 1 | HH:MM | 0b01001011 | true | 0 | 3 | 5 | https://www.youtube.com/watch?v=dQw4w9WgXcQ | true | false
    - Every minute check if an alarm should be played, or any of the other shenanigans
    - Days of the week: each bit of the byte is a day of the week, the 8th byte indicates that it should be run one time
    - Snooze: just make a new alarm with the same parameters at [snooze] minutes later, but the delete option as true
    - Memory game: like a simon says kind of memory game, just to get the brain flowing a little bit in the morning
    - Wake up light: check `cur_time > alarm_time - memory_light_minutes` to see if wake up light should go on, maybe make it smarter and fade on or something
  - Web gui:
    - Show current alarm clock time at the top of the web page
    - Could add in a "next alarm in x hours and x minutes"
    - Have general settings like volume, brightness (auto, set, light off), 
    - Load alarms from the database
    - Each alarm entry:
    
      1 [Alarm 1] [7:00] [AM/PM v] Repeat [Never/(Days) v] Vibration [x] Snooze [ ] Memory Game [Off/Easy/Medium/Hard v] Light [0/5/10/15/20/25/30 v] mins before Sound ['some/filepath/dropdown' v] Enabled [x] Delete (x)
    - Any change should update the database via post request
    - Button to add new alarms [+]
  - Could be fun to try something like a sleep playlist, where the alarm clock will stream a song or playlist for x minutes or something, then automatically turn off
