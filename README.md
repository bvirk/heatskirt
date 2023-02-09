# heatskirt hardware
Combined pro miro (ATMEGA32U4) app, terminal program and prebuild c++ files deployment.

and

A this piece of hardware used to turn on a heat element as function of temperature, moisture and time.
![heatcontroller](https://github.com/bvirk/heatskirt/blob/main/img/heatcontroller.png)

The heatskirt is made of a heating wire embedded in an adhesive at the bottom of the walls, an given a airflow fender.

![fender](https://github.com/bvirk/heatskirt/blob/main/img/fender.png)

The beauty that gots the job af controlling this, is a 8 Mhtz ATMEGA32U4 in its  mount on a tiny pro micro board in this box.

![promicro](https://github.com/bvirk/heatskirt/blob/main/img/promicro.png)

Besides USB, the sensors is a DHT22 humidity and temperature sensor and a NTC resistor and a SSD1306 OLED display. The power supply delivers 180ma at 5v. The separation to high voltage can be seen where removed copper makes the veroboard is shine lighter. The white brick that is sticking out under the BT138 triac is a moc3041 zero crossing detect optocoupler.

# heatskirt software

## terminal

The terminal acts as a shell - commands an their arguments as we know it from a bash shell - history, command completion and editing facilities. No - its is not the tiny 32K that delivers that - the terminal is not that arduino IDE java thing, nor minicom or others - it is of own brew.

The AVR and this speciel terminal is one united thing for development and use. Two main benfits:

- verbosity don't fills flash
- ease of use

That's the aims - the goals isn't reached yet in this projects.

## AVR source structure

The flow happens in class methods from libraries.  
Heatskirt.ino only contains som initialisation and the loop() there, is only reached on failure, being indicated by a quick flashing led and sound.  
Classes is global instantiated where they are defined and made aware of each other by header files inclusion and the extern statement.
All executing of written source happens in a single thread - besides what exists barebone - millis(), serial methods and tone(...).

## c++ redundance

When a c++ project grows, it redundance begins to be bothersome - every method in both header file and where declaration occures, and, for some parts, in sync for both terminal and AVR program. The increasing dialog with the compiler, which the role of being the human controller of cohesion raises, becomes boring and silly time consuming. A prebuild program, named consoleshelldeploy, is executed before compiling for both AVR and the terminal.


## Style

Are classes a good thing?
Yes - on a 2k ram device - if they avoid using heap. Local and global storage for data! It becomes C stylish and with use of PROGMEM for strings. An intersting challenge, compareded to 'normal computers' - because you cant't just return any thing from a function.

The Arduino community offers obvoius and easy understandable code that uses heap behind the curtain. It is for newbies and the future of  huge ram sizes.
C++ is a monster! You can build thing that looks nice and obvious - conseptual sensible, but with a cost behind the curtain. The monster aspect lies in  that you can still avoid it and write effective assembly - C++, and C as assembly. 

The ConsoleShell and ConsoleShellDeploy uses c++20 features - on a desktop that is possible.   


