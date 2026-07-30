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
**Total time spent: 2 hours**

# July 24: CAD
I recorded my CAD time using Hackatime In this Cad session i designed and started to build the main frame of the clock it has a similar concave barrel shape but is more curved at the ends to offer a slightly nicer aesthetic. I decided to build it in 2 parts, the main body and the screen holder, the screen holder was by far the most complex part about the design, first i had to design it to fit smoothly without too many bumps and then i had to figure out a way to attach it to the main body. I originally though about screws but realized that screws may protrude outside of the main body and look ugly so i decided to design a twist lock mechanism using 2 fingers that twist into the main body. this removes the need fro screws and simplifies the assembly.
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

**Total time spent: 2.5 hours**

# July 27: Redesign for CYD
I changed the design to accommodate a 2.8" touchscreen also more commonly known as the CYD which is a readily available, no solder low profile alternative tp using a TFT display and separate ESP 32 board. I removed the old twist lock mechanism in favor of the good old simple m3 screw which fits better into the design and is cleaner to look at, the front cover plate not has 2 options to attach. The first is press fit where you press fit it onto the display and the second is magnets, magnets are the preferred route but are more expensive so i decided to include a simpler alternative.
**Total time spent: 4.5 hours**

# July 30: CAD
I added the mount for the rotary encoder and developed the front magnetic cover for the screen. The front magnetic cover is going to be black PLA to blend into the screen when it is turned off. When adding the front cover plate i had to decide what method of attachment i want to use, i wanted it to be easily removable for future upgrades and repairs but also wanted it to be stable and strong. Eventually after some thinking and trying out twist lock ideas i decided on using some strong 6x3 neodymium magnets fairly priced at just 33 C per magnet and only needing 2 would add only 66 cents to the project. After i finished back i went back to remedy my previous failure of a design for the rotary encoder mount, it was clunky, ugly and didn't match at all, so i decided to change it. I first deleted all of the old things in the timeline and restarted with a blank slate, i made an offset plane and extruded a rectangle onto it to create the platform and base. off of this platform i made some forms that would hold it in place and added a screw hole. The whole board just slides in and connects using a JST connector on the back of the CYD which i left a space for on the disc that is inserted into the main body. I then positioned the board and designed the back plate so then it would match perfectly and slide into the grommett there previously.
