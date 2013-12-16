allegro_hand_roboticslab
========================

Allegro Hand RoboticsLab Project and Source


Contents
--------

* 1 Download Project
* 2 Other files
 * 2.1 Control XDL
 * 2.2 Device DMLs
* 3 MSVS Solution
* 4 Left or Right
* 5 Using Your CAN Interface



To avoid changing numerous paths in the project and source code, it is best to place the project repository into the following directory:

[RoboticsLab Install Directory]\examples\casestudy\RealtimeRobotics\allegro_hand_roboticslab\
If the path does not exist, please create the necessary folders.


Other files
-----------

Within the allegro_hand_roboticslab directory, you will find a folder titled "other_files". Within this folder is a controll DLL file and two XDL files used to link to the controller DLL and set properties (like left or right) for the hand controller. Move both of these files to the following directory:
```
[RoboticsLab Install Directory]\bin\controls\
```
Both of these files are identical except for the property, "whichHand", which specifies if the hand is a left or right hand.

Also in the folder, "other_files", is a "models" directory. Replicate the path found in this directory in you RoboticsLab install directory:
```
[RoboticsLab Install Directory]\bin\models\...
```

Device DMLs
-----------

The encoder and motor DMLs hold information like joint directions and offsets. These DMLs are therefore made specifically for each Allegro Hand. The DML files for your Allegro Hand can be downloaded from your customer page on the Allegro Hand wiki. Look for the link just below the the table of directions and offsets for the respective hand on your page.

**Wiki:** [simlab.co.kr/wiki/allegrohand](http://simlab.co.kr/wiki/allegrohand)

The DML folder contains 16 encoder DMLs and 16 motor DMLs along with DMLS for CAN and other other communication devices. Copy these files to the following folder, replacing all duplicates. You may need admin privileges to do this.
```
[RoboticsLab Install Directory]\bin\models\Etc\ERHand\devices\
```
**Note:** If device DMLs are not available on your customer-specific page, please email 
**Alex Alspach** alexalspach@simlab.co.kr


MSVS Solution
-------------

The MSVS2008 solution is located in the folder "msvc9" in the "allegro_hand_roboticslab" directory.

**Important:** For the ability to launch rPlayer, rPlot and rLicenseManager, you must run MSVS with administrator privileges.

There are two applications included in this solution:

* **simAllegroHand_dist** runs a dynamic simulation of the Allegro Hand using the control_AHAS control plugin.
* **appAllegroHand_dist** runs the actual Allegro Hand hardware via a CAN interface using the same controller, control_AHAS.

**Note:** For both of these projects, you will need to set the working directory to either:

```
$(RLAB_BIN_PATH)
```
or if you put the files in the suggested directory, you can set it to
```
../../../../../../bin
```

To run either of the applications, set it as the start-up project (right-click the project > Set as StartUp Project), compile, and hit "run"!


Left or Right
-------------

Find this line in both applications, simAllegroHand_dist and appAllegroHand_dist:
```
string_type control_path = _T("controls/control_AHAS_R.xdl");     // For a right Allegro Hand
```
If you are using a right Allegro Hand, you can leave this line as is. If you are using a left hand, then simply change the "R" to an "L" like so:
```
string_type control_path = _T("controls/control_AHAS_L.xdl");     // For a left Allegro Hand
```

Using Your CAN Interface
------------------------

Out of the box, Allegro Hand and Allegro Application Studio/RoboticsLab support five different CAN interfaces:

* Softing PCI
* ESD PCI
* Kvaser PCI
* Peak PCI
* Peak USB
* NI USB

To use any of these CAN interfaces, you must load the proper Allegro Hand AML file. In the application, appAllegroHand_dist, find the line:
```
string_type aml_path = _T("models/Etc/ERHand/ERHand_RT_PeakCAN.aml");
```
Surrounding this line, you will see commented lines referring to each of your twelve (12) options for Allegro Hands. There are six (6) CAN options available and a separate model for Left and Right hands. Comment out the default right-handed Peak CAN USB model and un-comment the one you will be using.

The source code for all of these interfaces is included in the Allegro Hand MSVS solution above.


If you need to implement support for another CAN interface, please see the following page for support:

* [Allegro Hand CAN Protocol (wiki)](http://simlab.dyndns.org:9000/AllegroHandWiki/index.php/CAN_Protocol)
* [Allegro Hand CAN Protocol (Korean, pdf)](http://simlab.dyndns.org:9000/AllegroHandWiki/images/a/a2/AllegroHandCanProtocol_KR.pdf)


**Note:** Please see the RoboticsLab documentation for information on creating a CAN System Device and including it in the Allegro Hand AML model file. 