# 3D Models for ClockClock 24 Replica

This directory contains all 3D models used in the project. They are split into parts used in the final build and parts used during testing or as assembly aids. All models and assemblies are also available on OnShape:

- [Final clock assembly](https://cad.onshape.com/documents/a12d0058dc8f8a8e500f6aec/w/9550b1995dbe2a83d5e85df4/e/b2a42626349ffc4887ecd531?renderMode=0&uiState=69db827f4f1d1b92c5202bb7)
- [Testing models](https://cad.onshape.com/documents/4a68624e57a3036aa9611f7d/w/db09c1e86378c64516826126/e/82d90400d78a886373dde4d7?renderMode=0&uiState=69db82afff7d1e7a5f6a9189)

The STL files in this directory are ready to print. You can also edit the models on OnShape and export them yourself. Note that some parts of the build are not 3D-printed. See the CAD assemblies for the full picture.

All parts were printed on a BambuLab P1S using [Polymaker PolyTerra™ PLA Cotton White](https://polymaker.com/product/polyterra-pla/) to get the purest white possible.

## Final Models

| File | Description |
|------|-------------|
| `lower_hand.stl` | Hour clock hand |
| `mounting_aid.stl` | Alignment tool for positioning the mounting bracket on the front panel |
| `mounting_bracket.stl` | Connects a 2×3 motor array to the wooden frame |
| `pcb_mount.stl` | PCB mount with a center hole and indentation for the clock hands. Three of these connect two PCBs into a 2×3 array |
| `pcb_mount_bottom.stl` | Same as `pcb_mount.stl` but with corrected hole positions to compensate for a placement mistake in the PCB design (the PCB itself was never updated) |
| `psu_bracket.stl` | Clamps the PSU barrel jack to the wooden frame |
| `screw_spacer.stl` | Raises the PCBs slightly off the mounting brackets |
| `upper_hand.stl` | Minute clock hand |
| `standing_bracket.stl` | Add stands for a testing prototype (see video below) |
| `distance_bracket.stl` | Connect PCBs to create a testing prototype (see video below) |

![Testing prototype](../docs/images/testing_prototype.gif)
<p align="center"><em>Testing prototype in action</em></p>

