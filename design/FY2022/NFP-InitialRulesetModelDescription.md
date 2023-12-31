Initial Ruleset Model Description JSON Output
================

**Jason Glazer, GARD Analytics**

 - March 29, 2022
 - April 27, 2022 (revision relecting change to Python application)
 - June 2, 2022 (add details to design document)

## Justification for New Feature ##

Demostrate the initial implementation of a new JSON based output file that conforms to the draft ASHRAE Standard 229 
Ruleset Model Description Schema to show the feasibilty of the schema and uncover problems with implementation. 

The title, purpose, and scope of ASHRAE Standard 229 are:

Title: 

- Protocols for Evaluating Ruleset Implementation in Building Performance Modeling Software

Purpose: 

-  This standard establishes tests and acceptance criteria for implementation of rulesets (e.g., modeling 
rules) and related reporting in building performance modeling software.

Scope:

-   This standard applies to building performance modeling software that implements rulesets.
-   This standard applies to rulesets associated with new or existing buildings and their systems, system controls, 
their sites, and other aspects of buildings described by the ruleset implementation being evaluated.

ASHRAE Standard 229 has not been published or gone through public review yet and is being developed
by the ASHRAE SPC 229P committee. The intention of the standard is to provide code officials and rating
authorities with files that they can use with a Ruleset Checking Tool (currently under development at 
PNNL) to automatically check if a ruleset (such as 90.1 Appendix G, RESNET, California Title 24 performance 
paths, or Canada National Energy Code for Buildings performance path) has been implemented correctly. 
Since each EnergyPlus IDF file could generate a RMD file, the Ruleset Checking Tool will be able to see if the 
changes between the RMD files correspond to rules in the ruleset by looking at both the baseline
and proposed RMD file. 

Revisions to the title, purpose and scope are currently being considered but are not expected to 
substantially change the Ruleset Model Description Schema.

## E-mail and  Conference Call Conclusions ##

Based on feedback, several changes are being made and are reflected in this updated NFP:

- Separate Python application creates the RMD file
- In its own repo for now
- Based primarily on output for EnergyPlus (in JSON format) (and maybe the EIO) but also reads some input
- Enhance EnergyPlus to add outputs on case-by-case basis with those added to output ones that are likely
 to help other use-cases besides this
- Read epJSON input directly for now. If this becomes too many items the other approach is to read a new 
output file generated by Energyplus that is very �thin� and just echos the inputs needed. This would reduce 
the changes in two places if input change and would ensure consistency of all files
- Fix EP-Launch to save JSON output format files (eplusout.JSON and eplusout-hourly.JSON) using user-provided file name.
New issue posted as #9403
- Don�t worry about compliance parameters for now, just ignore them. In future maybe merge a JSON file that contains them.

## Overview ##

Initial implementation of the draft ASHRAE Standard 229 Ruleset Model Description (RMD) in a Python-based 
utility that may be part of the EnergyPlus installer in the future which would produce a new JSON output file as 
described in Issue #8970. 

https://github.com/NREL/EnergyPlus/issues/8970

The RMD schema is being developed at:

https://github.com/open229/ruleset-model-report-schema

and specifically, the JSON schema file is available here:

https://github.com/open229/ruleset-model-description-schema/blob/master/docs229/ASHRAE229.schema.json

Examples of the schema can be found here:

https://github.com/open229/ruleset-model-description-schema/tree/master/examples/0.0.5

Some more stable aspects of the schema will be implemented first such as data groups for:

- Space
- Surface
- Construction
- Material
- InteriorLighting
- MiscellanousEquipment 

The initial effort will focus on trying to implement as many stable data groups as possible. The new 
EnergyPlus Space input object will facilitate implementation separating space and zone related data 
elements. 

The RMD file would be produced by running a new Python utility that produced the RMD file.


## Approach ##

While most of the schema data groups and data elements are based on inputs to simulation programs, 
for EnergyPlus, many are also produced as part of the tabular reports and these will be the main 
source used by the Python utility to produce the RMD file. The tabular output from EnergyPlus will 
be extracted using the JSON output format (the user would have to use the Output:JSON input object) 
which represents the tabular results in a JSON format, which will be easy for the Python utility
to read and parse using the Python JSON library.

For EnergyPlus inputs that are not summarized into EnergyPlus tabular output reports, two options
will be considered. Either the epJSON input will be read directly by the Python utility to extract
the input value or that input will be added to an existing or new tabular report table. Each input
will be considered on a case-by-case basis and it more likely to be added as a new tabular output
if it is seen as having general value to users outside of the RMD generation use case. One advantage
of creating new outputs for each input is that it does not require the Python utility to read from 
multiple files so no syncronization issues are possible. Another advantage is that input fields can 
be renamed in the Energy+.idd file and that would not impact the Python utility if all values are 
from tabular output. If many values are read from input, in the future a new JSON file could be 
created by EnergyPlus just to convey these values to the Python utility. This new JSON file would 
be a simple echo of parts of the epJSON input file but just for specific values.

Some data elements that are more closely aligned with compliance parameters do not correspond with 
EnergyPlus inputs or outputs, possible approaches for handling these are shown in the Compliance 
Parameters section below.

In addition, some data elements in data groups may be difficult to implement and in 
those cases, they may be skipped in this initial implementation which is trying to 
cover as many data elements as quickly as possible.

It is unlikely that the entire schema will be implemented during this initial effort
so the focus will be on data groups shown in the Overview section. If addition
data groups can be implented under this effort they would be other data groups that
have been approved by the SPC 229P committee including:

- Zone
- Infiltration
- SurfaceOpticalProperties
- Subsurface
- Transformer
- Schedule
- Calendar
- Weather
- Elevator
- ExteriorLighting

Elements in the Zone data group related to HVAC are unlikely to be implemetned since 
many other HVAC portions of the schema have not been approved by the ASHRAE Standard 229P 
committee yet.

For each RMD data group added an analysis of the values in the field need to be performed. 
The following is an example for the Surface data group.

| RMD Data Element             | Data Type                   | EnergyPlus Input Object      | EnergyPlus Field                   | Output Table     | Output Row/Column| Notes                                                                  |
| ---------------------------- | ----------------------------| ---------------------------- | ---------------------------------- | ---------------- | -----------------| ---------------------------------------------------------------------- |
| id                           | ID                          | BuildingSurface:Detailed     | Name                               |                  |                  | Many other input objects could contain this (Wall:Exterior, Roof, etc� |
| reporting\_name              | String                      | BuildingSurface:Detailed     | Name                               |                  |                  |                                                                        |
| notes                        | String                      |                              |                                    |                  |                  | Compliance Parameter                                                   |
| subsurfaces                  | \[{Subsurface}\]            | FenestrationSurface:Detailed | Building Surface Name              |                  |                  | Many other input objects could contain this (Window, Door, GlazedDoor, etc.) |
| classification               | enumeration                 | BuildingSurface:Detailed     | Surface Type                       |                  |                  |                                                                        |
| area                         | Numeric                     |                              |                                    | Envelope Summary | Gross Area       |                                                                        |
| tilt                         | Numeric                     |                              |                                    | Envelope Summary | Tilt             |                                                                        |
| azimuth                      | Numeric                     |                              |                                    | Envelope Summary | Azimuth          |                                                                        |
| adjacent\_to                 | enumeration                 |                              |                                    |                  |                  |This would be complicated to implement                                  |
| adjacent\_zone               | $ID                         | BuildingSurface:Detailed     | Outside Boundary Condition Object  |                  |                  |                                                                        |
| does\_cast\_shade            | Boolean                     | ShadowCalculation            | Shading Zone Group 1 ZoneList Name |                  |                  |                                                                        |
| construction                 | {Construction}              | BuildingSurface:Detailed     | Construction Name                  |                  |                  |                                                                        |
| surface\_optical\_properties | {SurfaceOpticalProperties}  | Construction                 | Outside Layer                      |                  |                  |                                                                        |
| status\_type                 | enumeration                 |                              |                                    |                  |                  | Compliance Parameter                                                   |

To understand how often data elements would be populated by either EnergyPlus input or 
EnergyPlus output as the source of the RMD data elements, a special tagged version of the 
schema file was created here:

https://github.com/open229/ruleset-model-description-schema/pull/105

The table below summarizes the findings across the initial target data groups. Please note
that other data groups may have distributions that are signficantly different than this.

| Data group            | #elements | input or output | input only | output only | neither |
| --------------------- | --------- | --------------- | ---------- | ----------- | ------- |
| Space                 | 15        | 6               | 2          | 1           | 6       |
| Surface               | 14        | 7               | 1          | 3           | 3       |
| Construction          | 14        | 2               | 3          | 1           | 8       |
| Material              | 8         | 7               | 0          | 0           | 1       |
| InteriorLighting      | 10        | 5               | 0          | 0           | 5       |
| MiscellanousEquipment | 11        | 7               | 1          | 0           | 3       |
| TOTAL                 | 72        | 34              | 7          | 5           | 26      |

The ones that are not available from the tabular output files are shown below:

| Data element                                                 | Object/Field                                                 |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| Space.service_water_heating_uses                             | WaterUse:Equipment/Name  or HotWaterEquipment/Name           |
| Space.occupant_sensible_heat_gain                            | People/Sensible  Heat Fraction                               |
| Surface.surface_optical_properties                           | Construction/Outside  Layer                                  |
| Construction.primary_layers                                  | Construction/\*                                               |
| Construction.has_radiant_heating                             | ZoneHVAC:\*Radiant:\*\*Name                                    |
| Construction.has_radiant_cooling                             | ZoneHVAC:CoolingPanel:RadiantConvective:Water/Surface \* Name |
| MiscellaneousEquipment.  remaining_fraction_chilled_water_loop | \*Equipment/ Fraction  Lost and LoadProfile:Plant/ Load Schedule Name |

Note that the two of these data elements (Surface.surface_optical_properties and Construction.primary_layers) could be populated 
using results shown in the EIO file. These are likely to be migrated to the tabular output file.

### Compliance Parameters ###

The Ruleset Model Description schema includes some data elements that do not correspond to 
EnergyPlus (or probably any simulation program) inputs or outputs. These data elements most 
often have to do with compliance. In the example above for Surface, the status\_type data 
element has possible enumeration values of:

- NEW
- ALTERED
- EXISTING
- EXISTING_PLUS_NEW

It is unlikely that most simulation programs would know such a thing about a surface but it is
very likely that the modelers would know. There are quite a few data elements that would fall into 
this category of Compliance Parameters including Notes that occur in almost every data group in
the RMD. There are multiple ways this type of data element populated in the RMD file:

- The user could add those data elements and values by hand after EnergyPlus generates the RMD file
- The IDF file could have new compliance:surface (and other) input objects that populate the compliance 
data elements in the RMD file
- A JSON file could be created by user that gets merged each time that EnergyPlus runs to populate those 
data elements in the RMD file

Each option has advantages and disadvantages. At this point, since this is an initial implementation, we are 
recommending that no functionality be added related to the compliance parameters and that this can be considered
for future work.

## Testing/Validation/Data Sources ##

Since the output JSON file needs to comply with a JSON schema, it should be easy to confirm that it is valid. 
The Python jsonschema library can be used to confirm that the RMD file is consistent with the schema.

## Input Output Reference Documentation ##

No specific changes for input are expected.

Additional tabular outputs will be considered on a case-by-case basis as described above.

## Outputs Description ##

The new output file is a JSON file following the schema described here:

https://github.com/open229/ruleset-model-description-schema

## Engineering Reference ##

No changes

## Example File and Transition Changes ##

None required.

## Design Document ##

The following describes the design:

 - A new Python utility will be created separate from EnergyPlus that can
 eventually be packaged with the EnergyPlus installer. It will be developed in
 its own repository but eventually this may be merged or linked from the
 EnergyPlus repository.

 - The new Python utility will read the JSON files that EnergyPlus produces when
 the output:JSON input object is used as the primary source of information. As a
 secondary source of information, the epJSON input file will be read. It is
 possible that the EIO file will be read as part of this effort. 

 - The Ruleset Model Description (RMD) format will be produced by the utility and
 is also a JSON format.

 - Verification that the RMD output produced by the new utility is consistent
 with the RMD schema will be performed by using the jsonschema Python library
 "validate" method.

 - The PathLib library is expected to be used for accessing files.

 - The unittest library is expected to be used for providing unit testing. The
 goal is to have tests for almost all of the methods.

 - At this point no changes to EnergyPlus are expected as part of this but
 issues may be added for features that are not working or are needed. For
 example, #9419 was added due to the initialization summary not being produced
 in the output:JSON file. 

 - Only a subset of data groups from the RMD schema will be generated and only
 data elements that are most direct will be implemented. This is expected to be
 the first step in an ongoing effort to fully implement the RMD schema as an
 output format.

## References ##

https://github.com/open229/ruleset-model-description-schema



