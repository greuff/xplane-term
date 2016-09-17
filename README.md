# xplane-term

An X-Plane plugin that provides a shell-like command line interface.

![Screenshot 1](/screenshot1.png?raw=true)

## Usage

Press `Tab` to open and close the command line window. Enter a command and press enter, which will also close the window.

### Commands available

* `nav1 115.5`: Tune NAV1 to 115.50 MHz. Also available for `nav2`.
* `com1 122.65`: Tune COM1 to 122.65 MHz. Also available for `com2`.
* `adf1 313`: Tune ADF1 to 313 kHz. Also available for `adf2`.
* `squawk 7000`: Set squawk to 7000.
* `ap`: Autopilot on/off toggle.
* `fd`: Flight Director on/off toggle.
* `ap hdg`: Autopilot HDG mode toggle.
* `ap nav`: Autopilot NAV mode toggle.
* `ap alt`: Autopilot ALT ARM mode toggle.
* `ap vs`: Autopilot vertical speed mode toggle.
* `ap app`: Autopilot approach mode toggle.
* `ap flc` (alias `ap ias`): Autopilot flightlevel-change-mode toggle.
* `hdg sync` (alias `hs`): Heading sync (sets heading bug to current magnetic heading).
* `hdg 334`: Set heading bug to 334 degrees.
* `ias 140`: Set hold-indicated-airspeed to 140 kts.
* `ias sync`: Indicated airspeed sync (sets hold-indicated-airspeed to current indicated airspeed).
* `crs 120`: Set navigation course to 120 degrees.
* `vs 500`: Set vertical speed to +500 feet per minute. Also supports negative values.
* `vs sync`: Vertical speed sync (sets autopilot vertical speed to current vertical speed).
* `alt 9500`: Set altitude to 9500 feet.
* `alt sync`: Altitude sync (sets autopilot altitude to current altitude).
* `qnh 1012.3`: Set QNH to 1012.3 hPa.
* `dh 200`: Set decision height to 200 feet.

## Build

### Mac OS X

Open the `Projects\Mac\xplane-term.xcodeproj` file with Xcode and build. Works with Xcode 7.3.1 on Mac OS X 10.11.6.

### Linux

Enter `Projects\Linux` and execute `make -f xplane-term.mk`. 

### Windows

Open the `Projects\Win\xplaneTerm.sln` file with Visual Studio and build. 32- and 64-bit-configurations are available.

## Installation

Copy the product `xplane-term.xpl` file to `X-Plane 10\Resources\Plugins`.
