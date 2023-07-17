/*
When you create a maze, it is easiest to just draw the maze with dots
You can ignore 0b and commas and just create the maze.
Something like this:
..........
.........
 ........
 .......
  ......
  .....
   ....
   ...
    ..
    .
Then you add the preceding 0b- and 1-characters
0b..........
0b.........
0b1........
0b1.......
0b11......
0b11.....
0b111....
0b111...
0b1111..
0b1111.
Now we need to start counting becase the number of dots and 1 must always be
divisable by 8 so 8, 16, 24, and so on.
In our maze, the longest line is 10 dots so we need to add 6 1-characters
The rest of the lines need to be as long.
0b..........111111
0b.........1111111
0b1........1111111
0b1.......11111111
0b11......11111111
0b11.....111111111
0b111....111111111
0b111...1111111111
0b1111..1111111111
0b1111.11111111111
Next we group the characters into 8ths by placing a comma and 0b-character.
0b........,0b..111111
0b........,0b.1111111
0b1.......,0b.1111111
0b1.......,0b11111111
0b11......,0b11111111
0b11.....1,0b11111111
0b111....1,0b11111111
0b111...11,0b11111111
0b1111..11,0b11111111
0b1111.111,0b11111111
Then we add a comma behind every line.
0b........,0b..111111,
0b........,0b.1111111,
0b1.......,0b.1111111,
0b1.......,0b11111111,
0b11......,0b11111111,
0b11.....1,0b11111111,
0b111....1,0b11111111,
0b111...11,0b11111111,
0b1111..11,0b11111111,
0b1111.111,0b11111111,
And replace the dots with zeros.
0b00000000,0b00111111,
0b00000000,0b01111111,
0b10000000,0b01111111,
0b10000000,0b11111111,
0b11000000,0b11111111,
0b11000001,0b11111111,
0b11100001,0b11111111,
0b11100011,0b11111111,
0b11110011,0b11111111,
0b11110111,0b11111111,
Finally we can prepend the maze data with information about the size of the
maze as well as where in the maze, the player starts.
The first field is the actual size of the maze in bytes. We calculate it by
counting the number of lines and multiplying with the number of sections.
In above example, we have 10 lines each with 2 sections = 20 and then we
add the header information which is always 5 so the final size of the maze
comes to 25 bytes.
The next two fields are the width and height of the maze. Height is easy as
it is just the number of lines, in this case 10.
Width is the maximum width of the maze so you need to find the 0 or 0's
that are the furthest away from the beginning of a line and count how many
characters it is away. In our case, the first line is also the widest and
it is a total of 10 0's, so our width is 10.
The last two fields in the header are the 0-based X and Y coordinates that
the player should start at. You decide these coordinates your self, just
ensure that you start the player inside the maze.
In this case, we would like the player to start all the way to the left on
the second line so the X coordinate is 0 = most left
The Y coordinate is 1, as the first line would be 0.
So the final maze will look like this:
25,10,10,
00,01,
0b00000000,0b00111111,
0b00000000,0b01111111,
0b10000000,0b01111111,
0b10000000,0b11111111,
0b11000000,0b11111111,
0b11000001,0b11111111,
0b11100001,0b11111111,
0b11100011,0b11111111,
0b11110011,0b11111111,
0b11110111,0b11111111,
If you have any trouble creating mazes, have a look at some of the mazes
below to see how they are done or you can contact the author of this program
on the Uzebox forum or mail: jimmy at dansbo.dk
*/

#define MAX_LEVELS 2
const u8 __flash levels[] = {
// Level 1 - Hexadigital
59,18,18,		//size,width,height
11,16,			//start coordinates (zero based)
0b11110000,0b00100000,0b00111111,
0b00000101,0b10101110,0b10111111,
0b00111001,0b10101110,0b01111111,
0b11110011,0b10101111,0b00111111,
0b11000110,0b00000011,0b10111111,
0b10010110,0b10101011,0b10111111,
0b00100110,0b00000011,0b10111111,
0b01111110,0b11111011,0b10111111,
0b00000110,0b11111011,0b10111111,
0b11110110,0b11111011,0b10111111,
0b11110110,0b11111011,0b10111111,
0b11110010,0b00000000,0b00111111,
0b11111001,0b11111111,0b10111111,
0b11111100,0b00000000,0b01111111,
0b11111101,0b11111110,0b11111111,
0b11111101,0b11111110,0b11111111,
0b11011101,0b11101110,0b11111111,
0b11000001,0b11100000,0b11111111,

// Level 2 - StingX2
65,20,20,		//size,width,height
5,15,			//start coordinates (zero based)
0b11111111,0b11111111,0b11111111,
0b00000000,0b00111111,0b11111111,
0b01111111,0b11111111,0b11111111,
0b01111111,0b11111111,0b11111111,
0b01111111,0b11111111,0b11111111,
0b00000000,0b01111111,0b11111111,
0b11111111,0b01111111,0b11111111,
0b11111111,0b01111111,0b11111111,
0b11111111,0b01111111,0b11111111,
0b00000000,0b01111000,0b01111111,
0b00111111,0b10000011,0b00001111,
0b10011111,0b00111111,0b11101111,
0b11001110,0b01111111,0b11101111,
0b11100110,0b11111111,0b11001111,
0b11110000,0b11111111,0b10011111,
0b11110001,0b11111110,0b00111111,
0b11100101,0b11111100,0b11111111,
0b11001100,0b01110001,0b11111111,
0b10011111,0b00110111,0b11111111,
0b00111111,0b10000000,0b00001111,

// No more level
0				// When size 0, there are no more mazes.
};
