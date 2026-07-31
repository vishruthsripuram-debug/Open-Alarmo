---
title: "Open-Alarmo"
author: "Vishruth-S"
description: "An open source easy to build Alarm clock"
created_at: "2026-07-23"
---
# July 24: Planning project
**I started planning my project**
I wanted to make an alarm clock that would wake me up at morn ing but also do it in a cool way and then an idea popped into my head one from 6 months ago when i saw the Nintendo Alarmo getting released. It was perfect, everything except that t was locked down, made by Nintendo and was too expensive. I set out to build one for a budget of just $20 USD.

It started with inspiration from the Nintendo alarm clock and then i decided to ditch the knob and instead have a web app to control it and set alarms for easier access, control and usability. I decided to keep the nice red design and the black front screen as it looked nice and didn't have any problems with fitting components.
<img width="284" height="284" alt="image" src="https://github.com/user-attachments/assets/4c89563c-e4b1-4f6c-a39a-fba2930139fc" />

**Total time spent: 2 hours**

# July 24: CAD
I recorded my CAD time using Hackatime In this Cad session i designed and started to build the main frame of the clock it has a similar concave barrel shape but is more curved at the ends to offer a slightly nicer aesthetic. I decided to build it in 2 parts, the main body and the screen holder, the screen holder was by far the most complex part about the design, first i had to design it to fit smoothly without too many bumps and then i had to figure out a way to attach it to the main body. I originally though about screws but realized that screws may protrude outside of the main body and look ugly so i decided to design a twist lock mechanism using 2 fingers that twist into the main body. this removes the need fro screws and simplifies the assembly.
<img width="899" height="697" alt="Inside view" src="https://github.com/user-attachments/assets/3852721e-6eb4-4013-8ebb-e84b5aeabb83" />

**Total time spent: 3.5 hours**

# July 25: Shopping for components
Went shopping for the components
I found a rotary encoder that would connect to the CYD via a jst connector

I found everything on Aliexpress as it was cheapest and found the versions that had models available on Grabcad so design would be easier.

The updated BOM
also updated on Github readme

| Name | Cost (AUD) | Merchant | Purpose |
| :--- | :--- | :--- | :--- |
| CYD display | 16.79 | Aliexpress | To display the content |
| 1W speaker | 4.70 | aliexpress | to make sound |
| JST 1.25 10cm wire | 1.00 | Aliexpress | To connect rotary encoder to CYD |
| KY - 040 rotary encoder | 3.36 | Aliexpress | To act as the dial for the display |
| **Total** | **$25.85** | | |

<img width="997" height="783" alt="Screenshot 2026-07-29 at 12 32 48 PM" src="https://github.com/user-attachments/assets/e1ce6b83-0595-4891-981b-cd5df471f438" />

**Total time spent: 2.5 hours**

# July 27: Redesign for CYD
I changed the design to accommodate a 2.8" touchscreen also more commonly known as the CYD which is a readily available, no solder low profile alternative tp using a TFT display and separate ESP 32 board. I removed the old twist lock mechanism in favor of the good old simple m3 screw which fits better into the design and is cleaner to look at, the front cover plate not has 2 options to attach. The first is press fit where you press fit it onto the display and the second is magnets, magnets are the preferred route but are more expensive so i decided to include a simpler alternative.
<img width="567" height="531" alt="Front view withoutcover" src="https://github.com/user-attachments/assets/53915edc-beaa-43ef-810d-d622ab3d5df0" />

**Total time spent: 4.5 hours**

# July 30: CAD
I added the mount for the rotary encoder and developed the front magnetic cover for the screen. The front magnetic cover is going to be black PLA to blend into the screen when it is turned off. When adding the front cover plate i had to decide what method of attachment i want to use, i wanted it to be easily removable for future upgrades and repairs but also wanted it to be stable and strong. Eventually after some thinking and trying out twist lock ideas i decided on using some strong 6x3 neodymium magnets fairly priced at just 33 C per magnet and only needing 2 would add only 66 cents to the project. After i finished back i went back to remedy my previous failure of a design for the rotary encoder mount, it was clunky, ugly and didn't match at all, so i decided to change it. I first deleted all of the old things in the timeline and restarted with a blank slate, i made an offset plane and extruded a rectangle onto it to create the platform and base. off of this platform i made some forms that would hold it in place and added a screw hole. The whole board just slides in and connects using a JST connector on the back of the CYD which i left a space for on the disc that is inserted into the main body. I then positioned the board and designed the back plate so then it would match perfectly and slide into the grommett there previously.
<img width="40" height="59" alt="magnet" src="https://github.com/user-attachments/assets/55fe52ee-1411-40bb-bd1c-c6101fa8d9e1" />
<img width="634" height="538" alt="Without screen cover" src="https://github.com/user-attachments/assets/ab837e6c-d533-4ca4-acef-e7db117b87fb" />
<img width="598" height="411" alt="Screenshot 2026-07-30 at 6 12 59 PM" src="https://github.com/user-attachments/assets/2c6deedf-dc11-47a2-b72b-e1092bd91515" />



**Total time spent: 2.5 hours**

# July 31: CAD
I designed the rotary dial cap, this is similar to the dial used to control the Nintendo Alarmo, although my variant will be touchscreen. The dial can be used to set alarms, timers and other time related activities, the dial can also be used to play games on the clock, it is a fun feature similar to the playdate. I had to figure out how to use a different type of filet, an asymmetrical fillet to get the desired shape. The clock design is now almost complete the last remaining feature is the speaker hole and the haptic motor for a satisfying feel everytime the wheel is turned.

<img width="638" height="626" alt="Front view" src="https://github.com/user-attachments/assets/29325b46-c45f-45f1-8197-aeec1d061dbf" />
<img width="381" height="225" alt="Rotary dial" src="https://github.com/user-attachments/assets/5398b696-2bd7-4c2d-b80b-a494e610e078" />
<img width="197" height="96" alt="rotary encoder" src="https://github.com/user-attachments/assets/9315cdd1-ca80-46b3-b702-8ad556170055" />


**Total time spent: 2.5 hours**

# July 31: CAD: adding speaker grille and expanding USB port
I added the final touch which was the space for the speaker on the back of the device. The USB port expansion was a simple sketch and cut of the main body. The speaker grille is removable and vertical which i thought looked nice.
<img width="496" height="489" alt="Speaker hole" src="https://github.com/user-attachments/assets/d1cb5535-2ab0-4255-a62f-439260c15190" />
**Total time spent: 1.5 hours**
