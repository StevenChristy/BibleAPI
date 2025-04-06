# BibleAPI Plugin for Unreal Engine

The BibleAPI Plugin for Unreal Engine provides a convienient way to add biblical teaching to your game. Databases from https://github.com/scrollmapper/bible_databases can be modified using the included python script to add tables that allow for extraction of words from verses. Please check the licensing of any translation you use prior to incorporating it into your game as some may have terms. As an example, I have included the ASV translation which has already been processed with the python script.

## Use Cases

* Random Verse for Loading Screens.
* Collecting Words to form a Verse.
* Collecting Verses to form a Chapter.
* Providing a real copy of the Bible to read in game.

## Known Issues

* The python script may incorrectly mark words that are capitalized as proper nouns even when they are not. 
* There is currently no support for word similarity, so fish and fishes are treated as seperate words.
