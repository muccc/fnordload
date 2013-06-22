
------------------------------------------------------------------------------------

                  Custom Enginnering S.p.A VKP80 CUPS driver

------------------------------------------------------------------------------------

Package description:

This is the CUPS printer driver package, containing:

1. Compiled printer drivers for VKP80 printer:
   - rastertoVKP80

2. Printer driver option setting help files, PPD file
   - VKP80.ppd.gz

3. setup script dor install the driver and the PPD file

Requirements:

This software requires that the following is present on your computer
the CUPS server & architecure (see www.cups.org)

Install Instructions

To begin using this software, please do the following:

1. Open a shell (bash, etc.)

2. Type ./setup script

3. Goto http://localhost:631 or use your favorite CUPS admin tool.

4. Add a new printer queue for your model.

5. Print test page and see the result

Driver Option Settings

1. Page Sizes:
   - 80mm * 80mm
   - 80mm * 120mm
   - 80mm * 160mm
   - 80mm * 200mm
   - 80mm Roll
   - ZOOM 80mm * 80mm
   - ZOOM 80mm * 120mm
   - ZOOM 80mm * 160mm
   - ZOOM 80mm * 200mm
   - ZOOM 80mm Roll
   the default page is 80mm * 160mm

2. Paper presentation:
   - 40 mm
   - 80 mm
   - 120 mm
   - 160 mm
   - 200 mm
   - 240 mm
   - 280 mm
   - max
   the default value is 160 mm

3. Print Density:
   - -50%
   - -37%
   - -25%
   - -12%
   - 0%
   - +12%
   - +25%
   - +37%
   - +50%
   the default is 0%

 4. Halftoning Algorithm:
  - Accurate
  - Standard
  - Radial
  - WTS
