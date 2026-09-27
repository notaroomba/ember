<!--
  Exported from Hack Club Blueprint on 2026-09-27.
  Source: https://blueprint.hackclub.com/projects/1701
-->

## 11/14/2025 - Research  

_Time spent: 3.0h_  

This is another project that surged out of nowhere (like trace) but mainly because I want to be able to solder/reflow my own PCB's in my house to be able to save cost and make bigger and better projects in the future (\*cough\* \*cough\* robotic arm \*cough\* \*cough\*). 

While I could buy a hotplate in the shop, I feel like it's too small and I might need to reflow bigger PCB's so just in case, I want to build a large custom hotplate that has these characteristics:

- Powered by USB-C
- Portable/Easy to store
- OLED screen + knob and button to config
- Cool looking case (opportunity to learn CAD before making a robotic arm/rocket)

Looking through the internet to see if anyone has done anything like this, I stumbled on this [repo](https://github.com/ikajdan/reflow-hot-plate/tree/main). While cool looking, it uses an external power supply and has a relatively small heatbed (80mm x 80mm). I'm aiming for 4 times that (in terms of area) with a 160mm x 160mm heatbed (or even bigger).

Doing more research and looking at materials, making a heatbed out of aluminum and one layer of copper costs a lot for big boards (slabs of aluminum)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTEyMDUsInB1ciI6ImJsb2JfaWQifX0=--e276f1c5ff8ea473b0ff164d86f0c6826425259b/image.png)

Or even a bit smaller:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTEyMDYsInB1ciI6ImJsb2JfaWQifX0=--b43bac808648ff46d4fcbbd7c822dd037ca0642b/image.png)


BUT, JLCPCB HAS THIS NEW COOL THING CALLED FLEX HEATERS THAT ARE WAYY CHEAPER AND CAN GO FOR BIGGER BC THEY'RE ALL AT 9 USD! SO being the capitalist that I am, I am going to capitalize on these savings and use one.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTEyMDQsInB1ciI6ImJsb2JfaWQifX0=--a66926660cb47bd11bf0dc2b50131fb881906298/image.png)

Now all that's left is to design the board. As I want USB-C PD, I am thinking about using a STM32G0B1KE (32 pin version) as it's powerful enough to do everything that I want it to do. I also would need a 3V3 converter (LM3281YFQR). In addition to a mosfet to control the how much current is going through (PWM). I also plan on adding a bunch of redundancy like USB-C ESD protection, fuses, current sensors, and temperature sensors (Thermocouple and Pt1000). 

Since this is going to be a flexible heater, I will also need a way to "prop" it up in the air so that it doesn't burn my desk. I plan on using an aluminum sheet with a hole in each corner to insert a "leg" (screw or smth) to be able to prop it up. Then at the bottom I plan on having a nice case (titanium /j) for the PCB and screw terminals to connect it to the hotplate/heatpad/flex heater.  

## 11/15/2025 - Start of Schematic  

_Time spent: 7.0h_  

I first created a new KiCad project and then opened up STM32CubeMX and followed this [guide](https://wiki.st.com/stm32mcu/wiki/STM32StepByStep:Getting_started_with_USB-Power_Delivery_Sink) on the ST website to configure USB PD. I also changed the processor to the STM32G0B1CBT6 as the other one didn't have a second CC pin due to the pin count.

Yknow what, scratch that, I'm a Texan and should be loyal to TI. So I'm gonna go with a TPS25730D and then use an STM32WB55 to get bluetooth support (everythings better with bluetooth lol). Jokes aside, I am going with TI because I want to complete this project in 3 days (crazy right?) and I feel more comfortable with their chips (Also serves as practice for when I have to work with Athena's PD chip).

For the mosfet I will be using IRFB3207 and TLP183 to drive it (taken from the original repo) and for temperature sensors, MAX6675 (thermocouple to digital), and a PT1000 with opamps to filter/amplify the signal. 

I will also be using LM22676MR-ADJ as a good step down converter from high voltages (42V just in case I want more power lol).

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE0OTcsInB1ciI6ImJsb2JfaWQifX0=--16c54a0912f7e58c5ebae905dcf7809230b5b829/image.png)

I added the components and started wiring up the ESD and USB-C port first:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE0OTksInB1ciI6ImJsb2JfaWQifX0=--71e2a184aa15472a89afb997ca865ac6801c61a0/image.png)

After that I finished wiring up the USB-C PD chip. (Yes I made sure that there were pull up resistors on the I2C lines)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE1MDcsInB1ciI6ImJsb2JfaWQifX0=--631ca4859633e19547c8760470243dad01304605/image.png)

  

## 11/16/2025 - Worked on Schematic  

_Time spent: 8.0h_  

I started wiring up the STM32 and configuring it in STM32CubeMX. THIS TIME LOOKING AT THE REFERENCE SCHEMATIC AND NOT COPY PASTING TO AVOID [ERRORS](https://blueprint.hackclub.com/projects/491).

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE2ODcsInB1ciI6ImJsb2JfaWQifX0=--ed81a1a63a0bc55aaf0d9423bbfe8c8544d54257/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE2ODYsInB1ciI6ImJsb2JfaWQifX0=--11bc1f1eb7ea836ba9a54a6cad92d868e6a04b09/image.png)

THIS TIME I ACTUALLY ADDED PULL UP RESISTORS ON THE I2C LINES AND DOUBLE CHECKED THE DECOUPLING CAPACITOR VALUES. 

I researched a bit about the different screens like the ones on the flipper zero but the standard I2C OLED is fine (SSD1306). I found this cool tool to create UI/UX so I'll put it here to save for later (https://arduinogfxtool.netlify.app/)

I also added in a bit more stuff like current sensor, buzzer, and board thermometer just in case. I might also add in NFC functionality to be able to wirelessly transfer temp data to the board/have presets set in NFC tags and I just put it on top and let it go (overkill but why half ass it).

I also added a 32MB flash to add images/graphcs onto the OLED (might be possible)

I hope to find this encoder somewhere: https://tech.alpsalpine.com/e/products/detail/EC11E18244A5/
 I also spent some time thinking about the case:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE2OTUsInB1ciI6ImJsb2JfaWQifX0=--a48c305d0bed9b7e47499c58a6c5673683f823ea/image.png)
(very crude)

I'd want it made out of a high quality plastic (PA11-HP Nylon) or acrylic to be able to see the board interior with rounded corners and space to fit the board and OLED/Encoder. I might also include one of those push-push-switches to turn it on or off. (sends signal to STM32 to disable USB-C PD and turns off the display/lights. OR I COULD DO THE BACK AS (PA11-HP Nylon) AND THE TOP COVER AS ACRYLIC, but I'm too tired to think so tmrw I'll finish the schematic and start the routing.

  

## 11/17/2025 1 AM - STM32 Pinout and Most Peripherals  

_Time spent: 5.0h_  

I started by first defining the pinout for the STM32, as I wanted to use most of the pins, I added some status LED's and a buzzer. Continuing on from yesterday's work, I skimmed through the datasheets of each component to see which extra pins I would need to add them immediately and not have to worry about adding them later. I had some trouble with the timers and flash as I could only use 2 timers (out of 4) so I needed a way to get input from the rotary encoder, set the heatbed, and also to control the buzzer. After researching a bit more, I ended up using LPTIM1 for the rotary encoder as I say that It had input for that.

After configuring all of the pins, my STM32 looked like this:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE4NzUsInB1ciI6ImJsb2JfaWQifX0=--b4ca11ae50e08ae26691687c6717ccd3317ed7d4/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE4NzYsInB1ciI6ImJsb2JfaWQifX0=--7fe2583c94c45274992013530ec5cb6d4cebfedf/image.png)

After that I went to work wiring up the rest of the peripherals like the flash, buzzer, board temperature sensor, current sensor, and NFC antenna.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE4ODIsInB1ciI6ImJsb2JfaWQifX0=--3621656f2177b52f4f3da0c7973d1c493fea46b1/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE4ODMsInB1ciI6ImJsb2JfaWQifX0=--973e4d05909c7d5dc02ff38f137d80299d129f24/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE4ODQsInB1ciI6ImJsb2JfaWQifX0=--8c1a1ff973fd9d84bf0d7f735a04c108e5804983/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE4ODUsInB1ciI6ImJsb2JfaWQifX0=--a987057c382a3be2f0476756e54102a721ab15ad/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE4ODYsInB1ciI6ImJsb2JfaWQifX0=--d221af3f0841ca2132edbc24376ee66387ce3144/image.png)

After that, I routed the buck converter to convert from PPHV to 3.3V:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE4ODcsInB1ciI6ImJsb2JfaWQifX0=--9da6487cbc33dfe0ec250ac99c5bb26bb0bfec22/image.png)

I looked carefully in the datasheet and made sure that I was using the right inductor and capacitors (looked them upon LCSC and added the part numbers). 

I also bom optimized a bit by using as many 4.7K pull-up resistors as I could to not have extra costs for different resistor values.

All that's left is to route up the external temperature sensors and also add in the connections to the heatplate. ALSO THE ROTARY ENCODER AND OLED DISPLAY. I plan on using screw terminals and making holes in the acrylic top case to be able to screw/unscrew them. I also plan on 







  

## 11/17/2025 1 PM - Finished Schematic and Footprints  

_Time spent: 5.0h_  

I routed up the external temperature sensors, using chips from MAXIM and ended up with this:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE5NDEsInB1ciI6ImJsb2JfaWQifX0=--41b342d74121a0da0ee7c7cb9f41560d3a221dfe/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE5NDIsInB1ciI6ImJsb2JfaWQifX0=--1e3911356282fb6acd9695f231698e2f25b50b72/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE5NDMsInB1ciI6ImJsb2JfaWQifX0=--d49fe91c052f56743669e0b5151899b8e259abfe/image.png)

I then routed up the PWM hotplate channel (design taken from [here](https://github.com/ikajdan/reflow-hot-plate/tree/main)) and ended up with this:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE5NDUsInB1ciI6ImJsb2JfaWQifX0=--1f0673decebcc06fb5cfb92f31795e5ae700f905/image.png)

I then proceeded to add all of the LEDs and try to find in the OLED and rotary encoder footprints and add them in:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE5NzcsInB1ciI6ImJsb2JfaWQifX0=--d6ea1bcaf8f0c568844e8a31db2125efc6bb8a4f/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE5NzgsInB1ciI6ImJsb2JfaWQifX0=--8bcbf3cb9345c644dddc9a534798fa93c97bfdd7/image.png)

Then I organized the board and ran an ERC to make sure that I didn't have any unconnected items. After that I found a cool font that I am going to use for all of the logos and icons and design and added it in.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE5NzksInB1ciI6ImJsb2JfaWQifX0=--4118132c45ae47244684bfc6427e5604c22e3f6e/image.png)

Then I updated the footprints for all of the components (making sure not to use 0201) and then imported it into the schematic.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTE5ODMsInB1ciI6ImJsb2JfaWQifX0=--4a0b16d08f767747591357c3d3eec594d3258f72/image.png)





  

## 11/18/2025 - Start PCB Layout  

_Time spent: 6.0h_  

After finishing the schematic and selecting the footprints, I went over to the PCB and started grouping the components (ICs) together along with their passives.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTIyMTgsInB1ciI6ImJsb2JfaWQifX0=--03f2d388c4a6d861fb6271c51a41ff8e8db17629/image.png)
(STM32 for example)

and continued until I had the rest of the components laid out. After that, I made a board outline of 50mmx150mm and then started placing the components there. From what I had in mind, I wanted the NFC tag on the opposite side of the USB-C connector and I also wanted the USB-C PPHV line to go through the top away from the other components. I wanted the Bluetooth antenna in a corner away from any noisy components so I decided to place it in the corner opposite of the PPHV line. The screw terminals I wanted in the top and preferably in the center, as well as the OLED and rotary encoder. After playing around with the layout I got something like this:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTIyMTksInB1ciI6ImJsb2JfaWQifX0=--8bad7e53266b779dd236674863e5b08f3a4c2b61/image.png)

I started routing a bit but needed to work on Accelerate so I'm gonna take a break lol.  

## 11/19/2025 - PCB Routing  

_Time spent: 5.0h_  

After coming back, I had a thought. There will probably be space under the OLED so I can add in components under it. Although there will be resistors and stuff under it, I can always put a sheet of paper or smth to separate it.

![0J4234.1200](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI0NzMsInB1ciI6ImJsb2JfaWQifX0=--e77ea6fce3b4a7dfbf1599961a7da1d02619485e/0J4234.1200.jpg)

![1657fac49c2cba1010eac9aaf2038c93abfc49b5_2_557x500](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI0NzQsInB1ciI6ImJsb2JfaWQifX0=--941042b1e137a9b2c259eb2d4bc49b46fcf5f214/1657fac49c2cba1010eac9aaf2038c93abfc49b5_2_557x500.jpeg)

I decided to reorganize the board to edit the space between the components:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI0NzgsInB1ciI6ImJsb2JfaWQifX0=--137d0251ad4b78bb5736cbd78cfa9a04091d8563/image.png)

After a while of procrastination and burnout, I finally started something:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI1NTEsInB1ciI6ImJsb2JfaWQifX0=--fd63424cdfbc075d7eaa19549773b0d3f739da39/image.png)

I still have a bit of routing to go and then I'll send it for review in the KiCad Discord (first time lol) and also the blueprint channel before rendering it :sob:  

## 11/20/2025 - Design Schematic  

_Time spent: 8.0h_  

After working for a bit, I finally finished routing up most of the PCB:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI3MzcsInB1ciI6ImJsb2JfaWQifX0=--4873b8c4e8a2fba376e5089d6e8a24622719d6f5/image.png)

I still have to impedance match the antenna and add in the 3D models and fix the silkscreen but for the most part it's done.

After that I fixed up the silkscreen aligning everything perfectly (ocd)
![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI3NjAsInB1ciI6ImJsb2JfaWQifX0=--e72d2285d2479dad520fdf1cd7060d20a66f3329/image.png)

I entered the lock in call and finished impedance matching, edited the silkscreen, and added a few more aesthetic touches.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI3NzYsInB1ciI6ImJsb2JfaWQifX0=--320281287806893dc19ff3d18be346a9fcdf5dcf/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI3OTQsInB1ciI6ImJsb2JfaWQifX0=--9b2f50b68763c3dd5d135922bb4dba0f0397cfc2/image.png)
  

## 11/21/2025 - Change Buck and Heatbed  

_Time spent: 7.0h_  

After posting it in the KiCad Discord I got the feedback to use another buck converter (TPS56339DDC) as electrolytic caps don't like heat so I rewired the buck converter and then routed it up on the PCB.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5MzIsInB1ciI6ImJsb2JfaWQifX0=--d009a5accf841b7ef43a082a0c398befab5cfe65/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5MzMsInB1ciI6ImJsb2JfaWQifX0=--68fe02d217a5c9044d35c6721a5f100b3dd6185d/image.png)

This gave me more space to put the MOSFET in the middle to dissipate more heat.

Moving on to the hotplate/heatbed, I found out that the design I am basing this off of has this schematic and PCB:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5NDAsInB1ciI6ImJsb2JfaWQifX0=--1bfd73c545aede4a2db67eb389273a6db35ffa21/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5NDEsInB1ciI6ImJsb2JfaWQifX0=--192b0deb51626b12a6eab599940b2b3aacafb816/image.png)

and I found out that these values come from KiCad's calculator:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5NDIsInB1ciI6ImJsb2JfaWQifX0=--4d3559129c8e95542459825c9f381dca8abe3942/image.png)

So to design my own, I wanted to do the same. Looking through JLCPCB's Flexible Heater capabilities, I found out that the thickness of the material can be from 30um to 50um (silicone)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5NDMsInB1ciI6ImJsb2JfaWQifX0=--93f9e0d937b8b4791ed0b36386c3ec0e89c077b3/image.png)

Looking at the voltage drop and power consumption, I only have 20V and 100W to play around with and the 3.3V buck converted needs a minimum of 5V so I have to change the length to make sure that I have at least 5V (with a bit of leeway) to be able to use as much area as I can.

I started researching about Hilbert Curves (very interesting) and also how there is a KiCad plugin to generate it:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5ODEsInB1ciI6ImJsb2JfaWQifX0=--64d08ee6da6733124b21ea8a0d6009f8a1f8ef96/image.png)

I was playing around with it as I am trying to generate curves to fill the space in the heating element.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5OTIsInB1ciI6ImJsb2JfaWQifX0=--7c335998f35a28ae6325a65fee43fa3439e6dfb5/image.png)

I then ended up woth this 400mm x 400mm board design AND THE BEST PART IS THAT IT'S ONLY 9 USD ON JLCPCB:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTI5OTMsInB1ciI6ImJsb2JfaWQifX0=--aba7af2b0fb3a2309471862c59f0d6b33e125766/image.png)

So after making the heatbed and the board I now am going to design a case in OnShape.







  

## 11/24/2025 - CAD (Canadian Dollars)  

_Time spent: 16.0h_  

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTMyNjYsInB1ciI6ImJsb2JfaWQifX0=--052212f9c34c7966a4fb60951608868267fa1ca4/image.png)

After procrastinating a bit, I decided to make the case. I had in mind to have the bottom with holes for USBcC and also have a space to add in the cables for the temp sensors and heatplate. I also thought about adding in a slope for the NFC section so that it will work for phones that have a limited range.

Then I posted the board in r/PrintedCircuitBoards and also in the KiCad discord and got some feedback for the board. I then had to change out the optocoupler for a gate driver and also added another screw hole and made them more symmetrical.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTM3ODQsInB1ciI6ImJsb2JfaWQifX0=--77e0ca32211b114c5d956ed3d7df8186ab159667/image.png)


After that I worked on the case and tried to make it look good:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTM4MjgsInB1ciI6ImJsb2JfaWQifX0=--e3b8a36d68bd58749d81c4e1786caf376b4b028b/image.png)

I also found this cool image because I want the USB-C cable flush with the socket:

![CGFE8](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTM4MzIsInB1ciI6ImJsb2JfaWQifX0=--c394bee77b35ac34f5c9b50eff9a1cc0d4470f5b/CGFE8.png)

Then after that I finished the bottom case

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTM4ODIsInB1ciI6ImJsb2JfaWQifX0=--6e73c44a40c194074e6bcc100ca23d0262d4fc03/image.png)

but I didn't know how to connect the top case to it so I will research that later.

After sleeping I thought about using heatset inserts and then also the different types of screws that I can use:

![Types-of-Screw-Heads](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQxMjAsInB1ciI6ImJsb2JfaWQifX0=--59fe576330a967e1f5a8f8cf915962560ebf5671/Types-of-Screw-Heads.jpg)

I think a sloted head would look decent as I wanted something that has a cylindrical head.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQxMjEsInB1ciI6ImJsb2JfaWQifX0=--e4545984542440f353d281cbaec805a250131ef0/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQxMjIsInB1ciI6ImJsb2JfaWQifX0=--c3e048c68d26f0aedc5214f8b68974c16b814b55/image.png)

After a bit I finally finished the CAD!
![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQxNTgsInB1ciI6ImJsb2JfaWQifX0=--54ce2c886310f810f656ce3f42b7d82aed95afca/image.png)



  

## 11/25/2025 1 AM - Blender and BOM  

_Time spent: 5.0h_  

After that I decided to put it in blender and get some nice renders of the PCB and case:

![banner](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQyNzAsInB1ciI6ImJsb2JfaWQifX0=--8d25b00e6979559ca5fe1c790e45b0fa16f1ac2a/banner.png)


![render](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQyNjksInB1ciI6ImJsb2JfaWQifX0=--a087d5fb543fd90add4d4918fa324fbe60563aac/render.png)


![render1](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQyNTQsInB1ciI6ImJsb2JfaWQifX0=--fd4720f809c05f35a9491c633a0d5a9ffb8617cf/render1.png)

![render2](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQyNTMsInB1ciI6ImJsb2JfaWQifX0=--66b9a5ad9473db0d4ddcd82de8845cb719ceef58/render2.png)

Then I finished up the BOM, created a logo, and cleaned up the README:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQyNTYsInB1ciI6ImJsb2JfaWQifX0=--9863593193426eff2d9b613d7f2688c27f8f3480/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQyNzEsInB1ciI6ImJsb2JfaWQifX0=--f3c4de41c98c1f2cf0bc0ce5df396c1bc6b6e85c/image.png)
  

## 11/25/2025 10 AM - Blueprint Banner  

_Time spent: 0.5h_  

This is just to update the blueprint banner after waiting on it to render, I just need to write 150 characters to fill up the requirement so that I can post this.

![banner](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTQzMTAsInB1ciI6ImJsb2JfaWQifX0=--2c8d57e30050eff15486c8caf2df342c92818d17/banner.png)
  

## 12/16/2025 - USB-C PD Working + Jingle Bells  

_Time spent: 5.0h_  

![ember](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUwNzAsInB1ciI6ImJsb2JfaWQifX0=--b7cf4ad17e2ff59a3462b7b1c4d629ca5e83499e/ember.jpeg)

So I finally got the PCB and it looks cool and even better was that when I plugged it in, it didn't burn up like Cyberboard V1 (wooo) so then I started trying to turn on the LEDs (first thing you always have to do) and the STM32 kept resetting and I didn't know why until later debugging (commenting out peripherals until it worked) I found out that I had mislabled an input pin as output:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUwNjksInB1ciI6ImJsb2JfaWQifX0=--a61c42944be686bb8da968c74cece8bec47228ab/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUwNzUsInB1ciI6ImJsb2JfaWQifX0=--afdc43162c1efae0348acca3cb16f1fb52713af9/image.png)

After that I started configuring the USB-C PD controller to get the status of my laptop and aja after a bit of configurint I finally was able to get PDO's (power data objects) that specify what different power options the source has:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUwODYsInB1ciI6ImJsb2JfaWQifX0=--3175d6f99ed07a8564a660fa4ffe5615a8dcfe0d/image.png)

Sadly my laptop only provides 5V@1.5A so yea...

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUwODcsInB1ciI6ImJsb2JfaWQifX0=--8372e086ccbe4f7b9355095cf20c9e6304b837cb/image.png)

moving on, I wanna try and get the speaker running and lookign at the datasheet I found this cool graph:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUwODgsInB1ciI6ImJsb2JfaWQifX0=--e0c4d007deb749839f940b84f1acf60c28a81ca1/image.png)

but nothign telling me at which speeds it's running at so I will make my own driver that can control the PCB duty cycle

After messing around I got jingle bells playing on the board lol.




  

## 12/17/2025 - NFC + PWM + Board Temp Sensor + First error  

_Time spent: 10.0h_  

I started getting a driver for the NFC chip and added it in, and also struggled a bit with the I2C address as I kept overwriting it whenever I read / write to the tag.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUyOTAsInB1ciI6ImJsb2JfaWQifX0=--d2675cfd7bf1c41a20d0fc032c651ed02da1fa47/image.png)


After that I worked on getting a basic PWM driver for the hotplate and I confirmed that the MOSFET was working.

I tested it with a multimeter at 1 Hz (it was switching lol) and then moved on to the board temp sensor. That was also really simple as there were drivers on Github and at first I found one for the tps116, after reading through the datasheet and this pull request, it was the same driver for the tps119 just with a different device id:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUyOTEsInB1ciI6ImJsb2JfaWQifX0=--85729dbe5a746024e3b225e890a79ea0d9a4f8e4/image.png)

Moving on to the current sensor, I found out that I had wired it wrong:
![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUyOTMsInB1ciI6ImJsb2JfaWQifX0=--52125715a05b4b103f9bf7d792f544fb6f1488c6/image.png)

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUyOTQsInB1ciI6ImJsb2JfaWQifX0=--04dfb61ec2d91e706228d02aee98fe33a95aa4b9/image.png)
(VS needed to be connected to 3V3 but luckily the regulator is next to it so I can solder a small wire connecting them:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUyOTIsInB1ciI6ImJsb2JfaWQifX0=--dd94a0df83e4e40bf69b1dc8d453caf96c8dfff5/image.png)

Anyways that wasn't something critical so I moved onto getting the drivers for the external temperature sensors. These functioned over SPI and on different channels to avoid different data length errors, I then got the drivers:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUyOTUsInB1ciI6ImJsb2JfaWQifX0=--0c575a7b7005774da01d27666880cbc66dd5a5ca/image.png)

and got them implemented:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUzMTIsInB1ciI6ImJsb2JfaWQifX0=--62597783f55da7fc24b867a7c94f9c1204010ddb/image.png)

after that it was time for the max6675:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUzMTYsInB1ciI6ImJsb2JfaWQifX0=--e892af44ebc2bc3e1f5e392761f938fe2bad6cf8/image.png)

I finished that but I still had a problem with the NFC, it wouldn't let me read or write to it and was unreliable at times so I went to look at the code and found out that it was trying to read and get a checksum of the data while it was being read/wrote (on field detect) and this caused a blocking operation on the storage which resulted in that unreliable behavior. After changing it to wait like 200ms it finally started working and I could get changes in the NFC reliably:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjUzNTIsInB1ciI6ImJsb2JfaWQifX0=--b8ccd93a2e9cc53cac4c3e4a2a91909727c5f986/image.png)

All that's left is to configure the screen and set up a PID algorithm for the heatbed (will finish that once I get the heatbed), get bluetooth to workm and get the flash / lcd working.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjU0NzQsInB1ciI6ImJsb2JfaWQifX0=--4d5113df9b12fbdf1c32b4330b5bcd01ae446b2c/image.png)


SO I took a bit of a break bc I got accepted into MIT and added in custom support to [play music](https://hackclub.slack.com/archives/C09SWT5DCGY/p1765950160479339?thread_ts=1762829869.254119&cid=C09SWT5DCGY) so yea lol I'ma sleep now  

## 12/26/2025 - NFC Sounds + Bluetooth + Screen Working  

_Time spent: 23.0h_  

I started in the airport trying to make some misc improvements and thought of making the board beep when an NFC tag is present, It took a little while bit I made it play a little jingle.

(Check my post in #50-days for the vid)

After that I was in another airport heading to blueprint and I was looking through this [repo](https://github.com/STMicroelectronics/STM32CubeWB/blob/7c5aa7dcd2c0abe787f922bb06a6213c724d08e4/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x/Release_Notes.html) and apparently I needed to flash the bluetooth firmware that I was going to use on the other processor otherwise it wouldn't work. After trying to flash the FUS, I bricked one of the boards because I didn't press and hold the boot button when it was editing the option bytes. But after correctly holding down the boot button, I fixed it and was able to connect to it from my phone:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MjYyNzYsInB1ciI6ImJsb2JfaWQifX0=--6d9642c4b29bf6de8e8fa14b9457445fb25f30c3/image.png)

So prototype happened and then I started working on the OLED screen. I had to solder it and after realizing that I had the wrong type of solder I tried fixing it and had to hold it in a certain position for the screen to work.

![WhatsApp Image 2025-12-25 at 23.23.14](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6Mjk3ODIsInB1ciI6ImJsb2JfaWQifX0=--6311990ac21c8121693f78f6c17b52943a7431fa/WhatsApp%20Image%202025-12-25%20at%2023.23.14.jpeg)

after that I got a circle running from this cool library:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6Mjk3ODMsInB1ciI6ImJsb2JfaWQifX0=--16c15c66d4fc3303ec754e3f77c290c176192295/image.png)

![WhatsApp Image 2025-12-25 at 23.23.14 (2)](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6Mjk3ODQsInB1ciI6ImJsb2JfaWQifX0=--2e4942198c3fe876dd6347aa7db111da513d60e8/WhatsApp%20Image%202025-12-25%20at%2023.23.14%20(2).jpeg)

and after a bit I got text and a basic UI although the fonts were wonky and I needed to import more:

![WhatsApp Image 2025-12-25 at 23.23.14 (1)](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6Mjk3ODUsInB1ciI6ImJsb2JfaWQifX0=--e3c8247faf0c31aac18a584d748c6bc373e27482/WhatsApp%20Image%202025-12-25%20at%2023.23.14%20(1).jpeg)

I still have a lot to go but thankfully my heatplate and case is coming in tomorrow so yea, I'll continue working on it.



  

## 12/27/2025 - UI + Case + Solder  

_Time spent: 7.0h_  

![IMG_0560](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MzA3ODksInB1ciI6ImJsb2JfaWQifX0=--4528412f0bf88e44bd927ac959cd4cf4eef06741/IMG_0560.jpg)


So I finally got some more fonts in after making a script that converts any ttf/otf font into a bitmap font and I imported it to make this cool menu screen.

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6Mjk4NDcsInB1ciI6ImJsb2JfaWQifX0=--9445d7e7f00b277b81ce6f5dbaf6279d8c1f9483/image.png)

I then worked on making a status screen to show the USB-C PD voltage as I can't use serial when connected to a power outlet so yea and I successfully got 20v although it was at 4.5A:

![WhatsApp Image 2025-12-27 at 14.13.54 (2)](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MzA3NzYsInB1ciI6ImJsb2JfaWQifX0=--074b3250049c020af9cbc480f2506cb474e9f76a/WhatsApp%20Image%202025-12-27%20at%2014.13.54%20(2).jpeg)


After a while I got a notification that my case finally came in so I went to get it and found out that everything fit perfectly lol

![image](https://hc-cdn.hel1.your-objectstorage.com/s/v3/ba8961e96be65d36_img_0560.jpg)

 All that was missing were the heatset inserts and screws. This also as around the time that I couldn't take the bad screen solder so I went out and bought some leaded solder:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MzA3NzUsInB1ciI6ImJsb2JfaWQifX0=--25388c2cc4b42562147da2b438d77005804d977d/image.png)

And then after that I finally was able to solder correctly and it looked amazing:

![WhatsApp Image 2025-12-27 at 14.13.54](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MzA3NzcsInB1ciI6ImJsb2JfaWQifX0=--9fae15e44b1ee12bfd6dfbab30f081d446295b81/WhatsApp%20Image%202025-12-27%20at%2014.13.54.jpeg)

I then started to look for ui libraries on the internet and stumbled upon this one:

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MzA3NzksInB1ciI6ImJsb2JfaWQifX0=--289bbf394dd000e8bb61432fb7a3c1e74a07d833/image.png)

but the only problem is that I will have to port it to C :(

For the UI, here's what I'm thinking, a main menu screen that says press to start and then you will be sent to a scrolleable menu,  it will have these entries:

- Start / Stop
- Status
- Settings
- About

The start stop button will start the hotplate controller and it will send you to a menu here you have to select between holding a temperature or a reflow profile, if you hold a temp then there will be a slider between 1-200C and for the reflow profile you will have the option to select it, no mater which option you select, there will b a confirm screen.

After you confirm, you will b sent to the status screen, this is basically where you can see how the heat plate is running and the temp and current raw and everything that you could possibly need.

In the settings bar you can change the behavior of the NFC or confirm or change the brightness of the OLED, also you can have the option to turn on or off Bluetooth discovery to control the board.

In the about scan it will how the firmware version and the credits.

Now that I have that planned out I will try and connect the hotplate to see how hot it can get:

![flexheater](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MzA3ODgsInB1ciI6ImJsb2JfaWQifX0=--7ebca31a06d5f22352920925a9a77c2d5d600f39/flexheater.png)





  

## 2/28/2026 - Finished stuffs  

_Time spent: 29.5h_  

Okay so basically I kinda want this shipped before BP ends and after switching computers like 5 times I forgot everything I've done in that time but I made it reflow trace among other things

I also added in a cool UI that has a menu screen where you can change the temperature among other things. I don't have any pictures bc I can't find them and yea... look at my commits to see my progress! I ordered a replacement part on Digikey for the temp sensor as I was dyslexic and wanted to solder it again but the order didn't go through after ordering it twice so yea...

I demoed this at MIT Blueprint and Clay saw it working once and I almost burned a hole through HQ trying to boil water. The ui could be better and yea

![image](https://blueprint.hackclub.com/user-attachments/blobs/proxy/eyJfcmFpbHMiOnsiZGF0YSI6MTEzMDU3LCJwdXIiOiJibG9iX2lkIn19--b9217e4166dde4fe91915b6997e08098ab90f71f/image.png)
  

