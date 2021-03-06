# brushless-motor-esc

Finished schematic capture and board layout of the first prototype for my brushless motor electronic speed controller. Currently waiting to receive my PCB from JLCPCB. Currently, the PCB consists of the MOSFETs, MOSFET drivers, and back-EMF detection circuit. I will be pairing this PCB with a STM32G071 nucleo board for prototyping purposes. Future revisions of this ESC will include a microcontroller and be able to function by itself, similar to a commercially available ESC.
![image1](https://i.imgur.com/letI5YO.png)
![image1](https://i.imgur.com/zCAdD6G.png)

Updates and Progress:

### Update 1
I've received the boards from JLC and manually soldered the components using a soldering iron. Due to the small size and the large number of components, the soldering took a couple of hours on each board.

![image1](https://i.imgur.com/4pH9yyL.jpg)

### Update 2
As I turned powered on my first esc, I heard a spark and then immediately turned off the power. The smell of a burnt chip was easy to detect. I visually inspected the board and saw that one of the traces had burned off. I probed the componenets with a multimeter. The Phase A MOSFETs and driver were broken. It took me two days to find out what I did wrong and gain enough confidence to power on a second board. (I only had enough components for two boards). 

It turns out that my connection between the microcontroller and my ESC board were not consistent with my firmware. This caused both the high and low side mosfets in the same phase of the motor to be turned on at the exact same time, which leads to a short. Afterwards, I fixed the connection and was extremely careful about making sure that the connections were correct this time. I validated the switching signals using a logic analyzer. 

My initial firmware was simple. My goal was to make the motor spin at a slow and constant RPM. At very low speeds, back EMF detection isn't needed. 

![image1](https://i.imgur.com/ZAZos75.png)
This is the switching sequence I am using to make my motor rotate. The solid looking parts on the AH, BH, and CH signals are actually PWM signals.

Demo 1: https://youtu.be/RlPoKeSwlMs
Demo 2: https://youtu.be/G03LiWif8WU






