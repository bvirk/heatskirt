# heatskirt hardware
Combined pro miro (ATMEGA32U4) app, terminal program and prebuild C++ files deployment.

and

A this piece of hardware used to turn on a heat element as function of temperature, moisture and time.
![heatcontroller](https://github.com/bvirk/heatskirt/blob/main/img/heatcontroller.png)

The heatskirt is made of a heating wire embedded in an adhesive at the bottom of the walls, an given a airflow fender.

![fender](https://github.com/bvirk/heatskirt/blob/main/img/fender.png)

The beauty that gots the job af controlling this, is a 8 Mhtz ATMEGA32U4 in its  mount on a tiny pro micro board in this box.

![promicro](https://github.com/bvirk/heatskirt/blob/main/img/promicro.png)

Besides USB, the sensors is a DHT22 humidity and temperature sensor, a NTC resistor and a SSD1306 OLED display. The power supply delivers 180ma at 5v. The separation to high voltage can be seen where removed copper makes the veroboard shine lighter. The white brick that is sticking out under the BT138 triac is a moc3041 zero crossing detect optocoupler.

# heatskirt software suite

The heatskirt software consists of three programs
- consoleshell terminal
- consoleshellDeploy prebuild task
- the AVR program

## consoleshell terminal

consoleshell expose AVR commands the way console programs is exposed to a shell and is made to be invoked with arguments. It uses gnu readline to offer history, command completion and editing facilities. 

benefits:

- verbosity don't fills flash
- ease of use
- automated dialog (time, calculating drift)

## consoleshellDeploy prebuild task

consoleshellDeploy lifts off some boilerplate coding with repspect to C++ project management in AVR programming, and exposes from avr code, settings needed for consoleshell.
- which commands exist
- what are the commands syntax (argument)
- which source files exists i the AVR program (for trace alerts byond reset)

## The AVR program

#### circuit
![circuit](https://github.com/bvirk/heatskirt/blob/main/img/heatskirtCircuit.jpg)

#### pins used in source


