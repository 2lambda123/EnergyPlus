// EnergyPlus, Copyright (c) 1996-2020, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/HVACDXSystem.hh>
#include <EnergyPlus/HVACFan.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HVACUnitaryBypassVAV.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/TempSolveRoot.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZonePlenum.hh>

namespace EnergyPlus {

namespace HVACUnitaryBypassVAV {

    // Module containing the routines for modeling changeover-bypass VAV systems

    // MODULE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2006
    //       MODIFIED       B. Nigusse, FSEC - January 2012 - Added steam and hot water heating coils

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms needed to simulate changeover-bypass
    // variable-air-volume (CBVAV) systems, which are considered "Air Loop Equipment" in EnergyPlus

    // METHODOLOGY EMPLOYED:
    // Units are modeled as a collection of components: outside air mixer,
    // supply air fan, DX cooing coil, DX/gas/elec heating coil, and variable volume boxes.
    // Control is accomplished by calculating the load in all zones to determine a mode of operation.
    // The system will either cool, heat, or operate based on fan mode selection.

    // The CBVAV system is initialized with no load (coils off) to determine the outlet temperature.
    // A setpoint temperature is calculated on FirstHVACIteration = TRUE to force one VAV box fully open.
    // Once the setpoint is calculated, the inlet node mass flow rate on FirstHVACIteration = FALSE is used to
    // determine the bypass fraction. The simulation converges quickly on mass flow rate. If the zone
    // temperatures float in the deadband, additional iterations are required to converge on mass flow rate.

    // REFERENCES:
    // "Temp & VVT Commercial Comfort Systems," Engineering Training Manual, Technical Development Program, Carrier Corp., 1995.
    // "VariTrac Changeover Bypass VAV (Tracker System CB)," VAV-PRC003-EN, Trane Company, June 2004.
    // "Ventilation for Changeover-Bypass VAV Systems," D. Stanke, ASHRAE Journal Vol. 46, No. 11, November 2004.
    //  Lawrence Berkeley Laboratory. Nov. 1993. DOE-2 Supplement Version 2.1E, Winklemann et.al.

    // MODULE PARAMETER DEFINITIONS
    // Compressor operation
    int const On(1);  // Normal compressor operation
    int const Off(0); // Signal DXCoil that compressor should not run

    // Dehumidification control modes (DehumidControlMode) for Multimode units only
    int const DehumidControl_None(0);
    int const DehumidControl_Multimode(1);
    int const DehumidControl_CoolReheat(2);

    // Mode of operation
    int const CoolingMode(1); // System operating mode is cooling
    int const HeatingMode(2); // System operating mode is heating

    // Priority control mode (prioritized thermostat signal)
    int const CoolingPriority(1); // Controls CBVAV system based on cooling priority
    int const HeatingPriority(2); // Controls CBVAV system based on heating priority
    int const ZonePriority(3);    // Controls CBVAV system based on number of zones priority
    int const LoadPriority(4);    // Controls CBVAV system based on total load priority

    // Airflow control for contant fan mode
    int const UseCompressorOnFlow(1);  // Set compressor OFF air flow rate equal to compressor ON air flow rate
    int const UseCompressorOffFlow(2); // Set compressor OFF air flow rate equal to user defined value

    static std::string const fluidNameSteam("STEAM");
    static std::string const BlankString;

    // DERIVED TYPE DEFINITIONS

    // MODULE VARIABLE DECLARATIONS:

    int NumCBVAV(0);                    // Number of CBVAV systems in input file
    Real64 CompOnMassFlow(0.0);         // System air mass flow rate w/ compressor ON
    Real64 OACompOnMassFlow(0.0);       // OA mass flow rate w/ compressor ON
    Real64 CompOffMassFlow(0.0);        // System air mass flow rate w/ compressor OFF
    Real64 OACompOffMassFlow(0.0);      // OA mass flow rate w/ compressor OFF
    Real64 CompOnFlowRatio(0.0);        // fan flow ratio when coil on
    Real64 CompOffFlowRatio(0.0);       // fan flow ratio when coil off
    Real64 FanSpeedRatio(0.0);          // ratio of air flow ratio passed to fan object
    Real64 BypassDuctFlowFraction(0.0); // Fraction of unit mass flow that returns to inlet of CBVAV unit through bypass duct
    Real64 PartLoadFrac(0.0);           // Compressor part-load fraction
    Real64 SaveCompressorPLR(0.0);      // Holds DX compressor PLR from active DX coil
    Real64 TempSteamIn(100.0);          // steam coil steam inlet temperature
    Array1D_bool CheckEquipName;
    bool GetInputFlag(true); // Flag set to make sure you get input once

    // SUBROUTINE SPECIFICATIONS FOR MODULE

    // Object Data
    Array1D<CBVAVData> CBVAV;

    // Functions

    void clear_state()
    {
        CBVAV.deallocate();
        NumCBVAV = 0;
        CompOnMassFlow = 0.0;
        OACompOnMassFlow = 0.0;
        CompOffMassFlow = 0.0;
        OACompOffMassFlow = 0.0;
        CompOnFlowRatio = 0.0;
        CompOffFlowRatio = 0.0;
        FanSpeedRatio = 0.0;
        BypassDuctFlowFraction = 0.0;
        PartLoadFrac = 0.0;
        SaveCompressorPLR = 0.0;
        TempSteamIn = 100.0;
        CheckEquipName.deallocate();
        GetInputFlag = true;
    }

    void SimUnitaryBypassVAV(EnergyPlusData &state,
                             std::string const &CompName,   // Name of the CBVAV system
                             bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system time step
                             int const AirLoopNum,          // air loop index
                             int &CompIndex                 // Index to changeover-bypass VAV system
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Manages the simulation of a changeover-bypass VAV system. Called from SimAirServingZones.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CBVAVNum = 0;      // Index of CBVAV system being simulated
        Real64 QUnitOut = 0.0; // Sensible capacity delivered by this air loop system

        // First time SimUnitaryBypassVAV is called, get the input for all the CBVAVs
        if (GetInputFlag) {
            GetCBVAV(state);
            GetInputFlag = false;
        }

        // Find the correct changeover-bypass VAV unit
        if (CompIndex == 0) {
            CBVAVNum = UtilityRoutines::FindItemInList(CompName, CBVAV);
            if (CBVAVNum == 0) {
                ShowFatalError(state, "SimUnitaryBypassVAV: Unit not found=" + CompName);
            }
            CompIndex = CBVAVNum;
        } else {
            CBVAVNum = CompIndex;
            if (CBVAVNum > NumCBVAV || CBVAVNum < 1) {
                ShowFatalError(
                    state,
                    format(
                        "SimUnitaryBypassVAV:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}", CBVAVNum, NumCBVAV, CompName));
            }
            if (CheckEquipName(CBVAVNum)) {
                if (CompName != CBVAV(CBVAVNum).Name) {
                    ShowFatalError(state,
                                   format("SimUnitaryBypassVAV: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          CBVAVNum,
                                          CompName,
                                          CBVAV(CBVAVNum).Name));
                }
                CheckEquipName(CBVAVNum) = false;
            }
        }

        Real64 OnOffAirFlowRatio = 0.0; // Ratio of compressor ON airflow to average airflow over timestep
        bool HXUnitOn = true;           // flag to enable heat exchanger

        // Initialize the changeover-bypass VAV system
        InitCBVAV(state, CBVAVNum, FirstHVACIteration, AirLoopNum, OnOffAirFlowRatio, HXUnitOn);

        // Simulate the unit
        SimCBVAV(state, CBVAVNum, FirstHVACIteration, QUnitOut, OnOffAirFlowRatio, HXUnitOn);

        // Report the result of the simulation
        ReportCBVAV(state, CBVAVNum);
    }

    void SimCBVAV(EnergyPlusData &state,
                  int const CBVAVNum,            // Index of the current CBVAV system being simulated
                  bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                  Real64 &QSensUnitOut,          // Sensible delivered capacity [W]
                  Real64 &OnOffAirFlowRatio,     // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                  bool &HXUnitOn                 // flag to enable heat exchanger
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a changeover-bypass VAV system.

        // METHODOLOGY EMPLOYED:
        // Calls ControlCBVAVOutput to obtain the desired unit output

        QSensUnitOut = 0.0;  // probably don't need this initialization
        Real64 HeatingPower; // Power consumption of DX heating coil or electric heating coil [W]

        // zero the fan and DX coils electricity consumption
        DataHVACGlobals::DXElecCoolingPower = 0.0;
        DataHVACGlobals::DXElecHeatingPower = 0.0;
        DataHVACGlobals::ElecHeatingCoilPower = 0.0;
        SaveCompressorPLR = 0.0;

        // initialize local variables
        bool UnitOn = true;
        int OutletNode = CBVAV(CBVAVNum).AirOutNode;
        int InletNode = CBVAV(CBVAVNum).AirInNode;
        Real64 AirMassFlow = DataLoopNode::Node(InletNode).MassFlowRate;
        Real64 PartLoadFrac = 0.0;

        // set the on/off flags
        if (CBVAV(CBVAVNum).OpMode == DataHVACGlobals::CycFanCycCoil) {
            // cycling unit only runs if there is a cooling or heating load.
            if (CBVAV(CBVAVNum).HeatCoolMode == 0 || AirMassFlow < DataHVACGlobals::SmallMassFlow) {
                UnitOn = false;
            }
        } else if (CBVAV(CBVAVNum).OpMode == DataHVACGlobals::ContFanCycCoil) {
            // continuous unit: fan runs if scheduled on; coil runs only if there is a cooling or heating load
            if (AirMassFlow < DataHVACGlobals::SmallMassFlow) {
                UnitOn = false;
            }
        }

        DataHVACGlobals::OnOffFanPartLoadFraction = 1.0;

        if (UnitOn) {
            ControlCBVAVOutput(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, OnOffAirFlowRatio, HXUnitOn);
        } else {
            CalcCBVAV(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, QSensUnitOut, OnOffAirFlowRatio, HXUnitOn);
        }
        if (CBVAV(CBVAVNum).modeChanged) {
            // set outlet node SP for mixed air SP manager
            DataLoopNode::Node(CBVAV(CBVAVNum).AirOutNode).TempSetPoint = CalcSetPointTempTarget(CBVAVNum);
            if (CBVAV(CBVAVNum).OutNodeSPMIndex > 0) {                                          // update mixed air SPM if exists
                state.dataSetPointManager->MixedAirSetPtMgr(CBVAV(CBVAVNum).OutNodeSPMIndex).calculate(state); // update mixed air SP based on new mode
                SetPointManager::UpdateMixedAirSetPoints(state); // need to know control node to fire off just one of these, do this later
            }
        }

        // calculate delivered capacity
        AirMassFlow = DataLoopNode::Node(OutletNode).MassFlowRate;

        Real64 QTotUnitOut = AirMassFlow * (DataLoopNode::Node(OutletNode).Enthalpy - DataLoopNode::Node(InletNode).Enthalpy);

        Real64 MinOutletHumRat = min(DataLoopNode::Node(InletNode).HumRat, DataLoopNode::Node(OutletNode).HumRat);

        QSensUnitOut = AirMassFlow * (Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(OutletNode).Temp, MinOutletHumRat) -
                                      Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(InletNode).Temp, MinOutletHumRat));

        // report variables
        CBVAV(CBVAVNum).CompPartLoadRatio = SaveCompressorPLR;
        if (UnitOn) {
            CBVAV(CBVAVNum).FanPartLoadRatio = 1.0;
        } else {
            CBVAV(CBVAVNum).FanPartLoadRatio = 0.0;
        }

        CBVAV(CBVAVNum).TotCoolEnergyRate = std::abs(min(0.0, QTotUnitOut));
        CBVAV(CBVAVNum).TotHeatEnergyRate = std::abs(max(0.0, QTotUnitOut));
        CBVAV(CBVAVNum).SensCoolEnergyRate = std::abs(min(0.0, QSensUnitOut));
        CBVAV(CBVAVNum).SensHeatEnergyRate = std::abs(max(0.0, QSensUnitOut));
        CBVAV(CBVAVNum).LatCoolEnergyRate = std::abs(min(0.0, (QTotUnitOut - QSensUnitOut)));
        CBVAV(CBVAVNum).LatHeatEnergyRate = std::abs(max(0.0, (QTotUnitOut - QSensUnitOut)));

        if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::CoilDX_HeatingEmpirical) {
            HeatingPower = DataHVACGlobals::DXElecHeatingPower;
        } else if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingAirToAirVariableSpeed) {
            HeatingPower = DataHVACGlobals::DXElecHeatingPower;
        } else if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingElectric) {
            HeatingPower = DataHVACGlobals::ElecHeatingCoilPower;
        } else {
            HeatingPower = 0.0;
        }

        Real64 locFanElecPower = 0.0;
        if (CBVAV(CBVAVNum).FanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
            locFanElecPower = HVACFan::fanObjs[CBVAV(CBVAVNum).FanIndex]->fanPower();
        } else {
            locFanElecPower = Fans::GetFanPower(CBVAV(CBVAVNum).FanIndex);
        }

        CBVAV(CBVAVNum).ElecPower = locFanElecPower + DataHVACGlobals::DXElecCoolingPower + HeatingPower;
    }

    void GetCBVAV(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006
        //       MODIFIED       Bereket Nigusse, FSEC, April 2011: added OA Mixer object type

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for changeover-bypass VAV systems and stores it in CBVAV data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const getUnitaryHeatCoolVAVChangeoverBypass("GetUnitaryHeatCool:VAVChangeoverBypass");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;                      // Number of Alphas for each GetObjectItem call
        int NumNumbers;                     // Number of Numbers for each GetObjectItem call
        int IOStatus;                       // Used in GetObjectItem
        std::string CompSetFanInlet;        // Used in SetUpCompSets call
        std::string CompSetCoolInlet;       // Used in SetUpCompSets call
        std::string CompSetFanOutlet;       // Used in SetUpCompSets call
        std::string CompSetCoolOutlet;      // Used in SetUpCompSets call
        static bool ErrorsFound(false);     // Set to true if errors in input, fatal at end of routine
        static bool DXErrorsFound(false);   // Set to true if errors in get coil input
        static bool FanErrFlag(false);      // Error flag returned during CALL to GetFanType
        static bool errFlag(false);         // Error flag returned during CALL to mining functions
        Array1D_int OANodeNums(4);          // Node numbers of OA mixer (OA, EA, RA, MA)
        std::string HXDXCoolCoilName;       // Name of DX cooling coil used with Heat Exchanger Assisted Cooling Coil
        std::string MixerInletNodeName;     // Name of mixer inlet node
        std::string SplitterOutletNodeName; // Name of splitter outlet node
        bool OANodeErrFlag;                 // TRUE if DX Coil condenser node is not found
        bool DXCoilErrFlag;                 // used in warning messages

        Array1D_string Alphas(20, "");
        Array1D<Real64> Numbers(9, 0.0);
        Array1D_string cAlphaFields(20, "");
        Array1D_string cNumericFields(9, "");
        Array1D_bool lAlphaBlanks(20, true);
        Array1D_bool lNumericBlanks(9, true);

        // find the number of each type of CBVAV unit
        std::string CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass";
        NumCBVAV = inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        // allocate the data structures
        CBVAV.allocate(NumCBVAV);
        CheckEquipName.dimension(NumCBVAV, true);

        // loop over CBVAV units; get and load the input data
        for (int CBVAVNum = 1; CBVAVNum <= NumCBVAV; ++CBVAVNum) {
            int HeatCoilInletNodeNum = 0;
            int HeatCoilOutletNodeNum = 0;
            inputProcessor->getObjectItem(state,
                                          CurrentModuleObject,
                                          CBVAVNum,
                                          Alphas,
                                          NumAlphas,
                                          Numbers,
                                          NumNumbers,
                                          IOStatus,
                                          lNumericBlanks,
                                          lAlphaBlanks,
                                          cAlphaFields,
                                          cNumericFields);
            UtilityRoutines::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);

            CBVAV(CBVAVNum).Name = Alphas(1);
            CBVAV(CBVAVNum).UnitType = CurrentModuleObject;
            CBVAV(CBVAVNum).Sched = Alphas(2);
            if (lAlphaBlanks(2)) {
                CBVAV(CBVAVNum).SchedPtr = DataGlobalConstants::ScheduleAlwaysOn();
            } else {
                CBVAV(CBVAVNum).SchedPtr = ScheduleManager::GetScheduleIndex(state, Alphas(2)); // convert schedule name to pointer (index number)
                if (CBVAV(CBVAVNum).SchedPtr == 0) {
                    ShowSevereError(state, CurrentModuleObject + ' ' + cAlphaFields(2) + " not found = " + Alphas(2));
                    ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    ErrorsFound = true;
                }
            }

            CBVAV(CBVAVNum).MaxCoolAirVolFlow = Numbers(1);
            if (CBVAV(CBVAVNum).MaxCoolAirVolFlow <= 0.0 && CBVAV(CBVAVNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(1), Numbers(1)));
                ShowContinueError(state, cNumericFields(1) + " must be greater than zero.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).MaxHeatAirVolFlow = Numbers(2);
            if (CBVAV(CBVAVNum).MaxHeatAirVolFlow <= 0.0 && CBVAV(CBVAVNum).MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(2), Numbers(2)));
                ShowContinueError(state, cNumericFields(2) + " must be greater than zero.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow = Numbers(3);
            if (CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow < 0.0 && CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(3), Numbers(3)));
                ShowContinueError(state, cNumericFields(3) + " must be greater than or equal to zero.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).CoolOutAirVolFlow = Numbers(4);
            if (CBVAV(CBVAVNum).CoolOutAirVolFlow < 0.0 && CBVAV(CBVAVNum).CoolOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(4), Numbers(4)));
                ShowContinueError(state, cNumericFields(4) + " must be greater than or equal to zero.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).HeatOutAirVolFlow = Numbers(5);
            if (CBVAV(CBVAVNum).HeatOutAirVolFlow < 0.0 && CBVAV(CBVAVNum).HeatOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(5), Numbers(5)));
                ShowContinueError(state, cNumericFields(5) + " must be greater than or equal to zero.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow = Numbers(6);
            if (CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow < 0.0 && CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(6), Numbers(6)));
                ShowContinueError(state, cNumericFields(6) + " must be greater than or equal to zero.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).OutAirSchPtr = ScheduleManager::GetScheduleIndex(state, Alphas(3)); // convert schedule name to pointer (index number)
            if (CBVAV(CBVAVNum).OutAirSchPtr != 0) {
                if (!ScheduleManager::CheckScheduleValueMinMax(state, CBVAV(CBVAVNum).OutAirSchPtr, "<", 0.0, ">", 1.0)) {
                    ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, "The schedule values in " + cAlphaFields(3) + " must be 0 to 1.");
                    ErrorsFound = true;
                }
            }

            CBVAV(CBVAVNum).AirInNode = NodeInputManager::GetOnlySingleNode(state, Alphas(4),
                                                                            ErrorsFound,
                                                                            CurrentModuleObject,
                                                                            Alphas(1),
                                                                            DataLoopNode::NodeType_Air,
                                                                            DataLoopNode::NodeConnectionType_Inlet,
                                                                            1,
                                                                            DataLoopNode::ObjectIsParent);

            MixerInletNodeName = Alphas(5);
            SplitterOutletNodeName = Alphas(6);

            CBVAV(CBVAVNum).AirOutNode = NodeInputManager::GetOnlySingleNode(state, Alphas(7),
                                                                             ErrorsFound,
                                                                             CurrentModuleObject,
                                                                             Alphas(1),
                                                                             DataLoopNode::NodeType_Air,
                                                                             DataLoopNode::NodeConnectionType_Outlet,
                                                                             1,
                                                                             DataLoopNode::ObjectIsParent);

            CBVAV(CBVAVNum).SplitterOutletAirNode = NodeInputManager::GetOnlySingleNode(state, SplitterOutletNodeName,
                                                                                        ErrorsFound,
                                                                                        CurrentModuleObject,
                                                                                        Alphas(1),
                                                                                        DataLoopNode::NodeType_Air,
                                                                                        DataLoopNode::NodeConnectionType_Internal,
                                                                                        1,
                                                                                        DataLoopNode::ObjectIsParent);

            if (NumAlphas > 19 && !lAlphaBlanks(20)) {
                CBVAV(CBVAVNum).PlenumMixerInletAirNode = NodeInputManager::GetOnlySingleNode(state, Alphas(20),
                                                                                              ErrorsFound,
                                                                                              CurrentModuleObject,
                                                                                              Alphas(1),
                                                                                              DataLoopNode::NodeType_Air,
                                                                                              DataLoopNode::NodeConnectionType_Internal,
                                                                                              1,
                                                                                              DataLoopNode::ObjectIsParent);
                CBVAV(CBVAVNum).PlenumMixerInletAirNode = NodeInputManager::GetOnlySingleNode(state, Alphas(20),
                                                                                              ErrorsFound,
                                                                                              CurrentModuleObject,
                                                                                              Alphas(1) + "_PlenumMixerInlet",
                                                                                              DataLoopNode::NodeType_Air,
                                                                                              DataLoopNode::NodeConnectionType_Outlet,
                                                                                              1,
                                                                                              DataLoopNode::ObjectIsParent);
            }

            CBVAV(CBVAVNum).plenumIndex = ZonePlenum::getReturnPlenumIndexFromInletNode(state, CBVAV(CBVAVNum).PlenumMixerInletAirNode);
            CBVAV(CBVAVNum).mixerIndex = MixerComponent::getZoneMixerIndexFromInletNode(state, CBVAV(CBVAVNum).PlenumMixerInletAirNode);
            if (CBVAV(CBVAVNum).plenumIndex > 0 && CBVAV(CBVAVNum).mixerIndex > 0) {
                ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                ShowContinueError(state, "Illegal connection for " + cAlphaFields(20) + " = \"" + Alphas(20) + "\".");
                ShowContinueError(state, cAlphaFields(20) + " cannot be connected to both an AirloopHVAC:ReturnPlenum and an AirloopHVAC:ZoneMixer.");
                ErrorsFound = true;
            } else if (CBVAV(CBVAVNum).plenumIndex == 0 && CBVAV(CBVAVNum).mixerIndex == 0 && CBVAV(CBVAVNum).PlenumMixerInletAirNode > 0) {
                ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                ShowContinueError(state, "Illegal connection for " + cAlphaFields(20) + " = \"" + Alphas(20) + "\".");
                ShowContinueError(state, cAlphaFields(20) +
                                  " must be connected to an AirloopHVAC:ReturnPlenum or AirloopHVAC:ZoneMixer. No connection found.");
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).MixerInletAirNode = NodeInputManager::GetOnlySingleNode(state, MixerInletNodeName,
                                                                                    ErrorsFound,
                                                                                    CurrentModuleObject,
                                                                                    Alphas(1),
                                                                                    DataLoopNode::NodeType_Air,
                                                                                    DataLoopNode::NodeConnectionType_Internal,
                                                                                    1,
                                                                                    DataLoopNode::ObjectIsParent);

            CBVAV(CBVAVNum).MixerInletAirNode = NodeInputManager::GetOnlySingleNode(state, MixerInletNodeName,
                                                                                    ErrorsFound,
                                                                                    CurrentModuleObject,
                                                                                    Alphas(1) + "_Mixer",
                                                                                    DataLoopNode::NodeType_Air,
                                                                                    DataLoopNode::NodeConnectionType_Outlet,
                                                                                    1,
                                                                                    DataLoopNode::ObjectIsParent);

            CBVAV(CBVAVNum).SplitterOutletAirNode = NodeInputManager::GetOnlySingleNode(state, SplitterOutletNodeName,
                                                                                        ErrorsFound,
                                                                                        CurrentModuleObject,
                                                                                        Alphas(1) + "_Splitter",
                                                                                        DataLoopNode::NodeType_Air,
                                                                                        DataLoopNode::NodeConnectionType_Inlet,
                                                                                        1,
                                                                                        DataLoopNode::ObjectIsParent);

            CBVAV(CBVAVNum).OAMixType = Alphas(8);
            CBVAV(CBVAVNum).OAMixName = Alphas(9);

            errFlag = false;
            ValidateComponent(state, CBVAV(CBVAVNum).OAMixType, CBVAV(CBVAVNum).OAMixName, errFlag, CurrentModuleObject);
            if (errFlag) {
                ShowContinueError(state, "specified in " + CurrentModuleObject + " = \"" + CBVAV(CBVAVNum).Name + "\".");
                ErrorsFound = true;
            } else {
                // Get OA Mixer node numbers
                OANodeNums = MixedAir::GetOAMixerNodeNumbers(state, CBVAV(CBVAVNum).OAMixName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, "that was specified in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, "..OutdoorAir:Mixer is required. Enter an OutdoorAir:Mixer object with this name.");
                    ErrorsFound = true;
                } else {
                    CBVAV(CBVAVNum).MixerOutsideAirNode = OANodeNums(1);
                    CBVAV(CBVAVNum).MixerReliefAirNode = OANodeNums(2);
                    // CBVAV(CBVAVNum)%MixerInletAirNode  = OANodeNums(3)
                    CBVAV(CBVAVNum).MixerMixedAirNode = OANodeNums(4);
                }
            }

            if (CBVAV(CBVAVNum).MixerInletAirNode != OANodeNums(3)) {
                ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                ShowContinueError(state, "Illegal " + cAlphaFields(5) + " = " + MixerInletNodeName + '.');
                ShowContinueError(state, cAlphaFields(5) + " must be the same as the return air stream node specified in the OutdoorAir:Mixer object.");
                ErrorsFound = true;
            }

            if (CBVAV(CBVAVNum).MixerInletAirNode == CBVAV(CBVAVNum).AirInNode) {
                ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                ShowContinueError(state, "Illegal " + cAlphaFields(5) + " = " + MixerInletNodeName + '.');
                ShowContinueError(state, cAlphaFields(5) + " must be different than the " + cAlphaFields(4) + '.');
                ErrorsFound = true;
            }

            if (CBVAV(CBVAVNum).SplitterOutletAirNode == CBVAV(CBVAVNum).AirOutNode) {
                ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                ShowContinueError(state, "Illegal " + cAlphaFields(6) + " = " + SplitterOutletNodeName + '.');
                ShowContinueError(state, cAlphaFields(6) + " must be different than the " + cAlphaFields(7) + '.');
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).FanType = Alphas(10);
            CBVAV(CBVAVNum).FanName = Alphas(11);
            int fanOutletNode(0);

            // check that the fan exists
            bool errFlag = false;
            ValidateComponent(state, CBVAV(CBVAVNum).FanType, CBVAV(CBVAVNum).FanName, errFlag, CurrentModuleObject);
            if (errFlag) {
                ShowContinueError(state, "...occurs in " + CurrentModuleObject + ", unit=\"" + CBVAV(CBVAVNum).Name + "\".");
                ShowContinueError(state, "check " + cAlphaFields(10) + " and " + cAlphaFields(11));
                ErrorsFound = true;
                CBVAV(CBVAVNum).FanVolFlow = 9999.0;
            } else {
                if (UtilityRoutines::SameString(CBVAV(CBVAVNum).FanType, "Fan:SystemModel")) {
                    CBVAV(CBVAVNum).FanType_Num = DataHVACGlobals::FanType_SystemModelObject;
                    HVACFan::fanObjs.emplace_back(new HVACFan::FanSystem(state, CBVAV(CBVAVNum).FanName)); // call constructor
                    CBVAV(CBVAVNum).FanIndex = HVACFan::getFanObjectVectorIndex(state, CBVAV(CBVAVNum).FanName);
                    CBVAV(CBVAVNum).FanInletNodeNum = HVACFan::fanObjs[CBVAV(CBVAVNum).FanIndex]->inletNodeNum;
                    fanOutletNode = HVACFan::fanObjs[CBVAV(CBVAVNum).FanIndex]->outletNodeNum;
                    CBVAV(CBVAVNum).FanVolFlow = HVACFan::fanObjs[CBVAV(CBVAVNum).FanIndex]->designAirVolFlowRate;
                } else {
                    Fans::GetFanType(
                        state, CBVAV(CBVAVNum).FanName, CBVAV(CBVAVNum).FanType_Num, FanErrFlag, CurrentModuleObject, CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).FanInletNodeNum = Fans::GetFanInletNode(state, CBVAV(CBVAVNum).FanType, CBVAV(CBVAVNum).FanName, FanErrFlag);
                    fanOutletNode = Fans::GetFanOutletNode(state, CBVAV(CBVAVNum).FanType, CBVAV(CBVAVNum).FanName, ErrorsFound);
                    Fans::GetFanIndex(state, CBVAV(CBVAVNum).FanName, CBVAV(CBVAVNum).FanIndex, FanErrFlag, ObjexxFCL::Optional_string_const());
                    Fans::GetFanVolFlow(CBVAV(CBVAVNum).FanIndex, CBVAV(CBVAVNum).FanVolFlow);
                }
            }

            if (UtilityRoutines::SameString(Alphas(12), "BlowThrough")) {
                CBVAV(CBVAVNum).FanPlace = DataHVACGlobals::BlowThru;
            } else if (UtilityRoutines::SameString(Alphas(12), "DrawThrough")) {
                CBVAV(CBVAVNum).FanPlace = DataHVACGlobals::DrawThru;
            } else {
                ShowSevereError(state, CurrentModuleObject + " illegal " + cAlphaFields(12) + " = " + Alphas(12));
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::DrawThru) {
                if (CBVAV(CBVAVNum).SplitterOutletAirNode != fanOutletNode) {
                    ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, "Illegal " + cAlphaFields(6) + " = " + SplitterOutletNodeName + '.');
                    ShowContinueError(state, cAlphaFields(6) + " must be the same as the fan outlet node specified in " + cAlphaFields(10) + " = " +
                                      CBVAV(CBVAVNum).FanType + ": " + CBVAV(CBVAVNum).FanName + " when draw through " + cAlphaFields(11) +
                                      " is selected.");
                    ErrorsFound = true;
                }
            }

            if (CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxCoolAirVolFlow && CBVAV(CBVAVNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than the ",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            cAlphaFields(11),
                                            CBVAV(CBVAVNum).FanName) +
                                         cNumericFields(1));
                    ShowContinueError(state, ' ' + cNumericFields(1) + " is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, " Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).MaxCoolAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxHeatAirVolFlow && CBVAV(CBVAVNum).MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than the ",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            cAlphaFields(11),
                                            CBVAV(CBVAVNum).FanName) +
                                         cNumericFields(2));
                    ShowContinueError(state, ' ' + cNumericFields(2) + " is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, " Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).MaxHeatAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
            }

            //   only check that OA flow in cooling is >= SA flow in cooling when they are not autosized
            if (CBVAV(CBVAVNum).CoolOutAirVolFlow > CBVAV(CBVAVNum).MaxCoolAirVolFlow && CBVAV(CBVAVNum).CoolOutAirVolFlow != DataSizing::AutoSize &&
                CBVAV(CBVAVNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowWarningError(state, CurrentModuleObject + ": " + cNumericFields(4) + " cannot be greater than " + cNumericFields(1));
                ShowContinueError(state, ' ' + cNumericFields(4) + " is reset to the fan flow rate and the simulation continues.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).CoolOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
            }

            //   only check that SA flow in heating is >= OA flow in heating when they are not autosized
            if (CBVAV(CBVAVNum).HeatOutAirVolFlow > CBVAV(CBVAVNum).MaxHeatAirVolFlow && CBVAV(CBVAVNum).HeatOutAirVolFlow != DataSizing::AutoSize &&
                CBVAV(CBVAVNum).MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowWarningError(state, CurrentModuleObject + ": " + cNumericFields(5) + " cannot be greater than " + cNumericFields(2));
                ShowContinueError(state, ' ' + cNumericFields(5) + " is reset to the fan flow rate and the simulation continues.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).HeatOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
            }

            if (UtilityRoutines::SameString(Alphas(14), "Coil:Cooling:DX:SingleSpeed") ||
                UtilityRoutines::SameString(Alphas(14), "CoilSystem:Cooling:DX:HeatExchangerAssisted") ||
                UtilityRoutines::SameString(Alphas(14), "Coil:Cooling:DX:TwoStageWithHumidityControlMode") ||
                UtilityRoutines::SameString(Alphas(14), "Coil:Cooling:DX:VariableSpeed")) {

                CBVAV(CBVAVNum).DXCoolCoilType = Alphas(14);
                CBVAV(CBVAVNum).DXCoolCoilName = Alphas(15);

                if (UtilityRoutines::SameString(Alphas(14), "Coil:Cooling:DX:SingleSpeed")) {
                    CBVAV(CBVAVNum).DXCoolCoilType_Num = DataHVACGlobals::CoilDX_CoolingSingleSpeed;
                    CBVAV(CBVAVNum).DXCoilInletNode =
                        DXCoils::GetCoilInletNode(state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    CBVAV(CBVAVNum).DXCoilOutletNode =
                        DXCoils::GetCoilOutletNode(state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    if (DXErrorsFound) {
                        ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                        ShowContinueError(state, "Coil:Cooling:DX:SingleSpeed \"" + CBVAV(CBVAVNum).DXCoolCoilName + "\" not found.");
                        ErrorsFound = true;
                    } else {

                        DXCoilErrFlag = false;
                        DXCoils::GetDXCoilIndex(state,
                                                CBVAV(CBVAVNum).DXCoolCoilName,
                                                CBVAV(CBVAVNum).DXCoolCoilIndexNum,
                                                DXCoilErrFlag,
                                                CBVAV(CBVAVNum).DXCoolCoilType,
                                                ObjexxFCL::Optional_bool_const());
                        if (DXCoilErrFlag) ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");

                        //         Mine outdoor condenser node from DX coil object
                        OANodeErrFlag = false;
                        CBVAV(CBVAVNum).CondenserNodeNum =
                            DXCoils::GetCoilCondenserInletNode(state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, OANodeErrFlag);
                        if (OANodeErrFlag) ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    }
                } else if (UtilityRoutines::SameString(Alphas(14), "Coil:Cooling:DX:VariableSpeed")) {
                    CBVAV(CBVAVNum).DXCoolCoilType_Num = DataHVACGlobals::Coil_CoolingAirToAirVariableSpeed;
                    CBVAV(CBVAVNum).DXCoilInletNode = VariableSpeedCoils::GetCoilInletNodeVariableSpeed(state,
                        CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    CBVAV(CBVAVNum).DXCoilOutletNode = VariableSpeedCoils::GetCoilOutletNodeVariableSpeed(state,
                        CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    if (DXErrorsFound) {
                        ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                        ShowContinueError(state, "Coil:Cooling:DX:VariableSpeed \"" + CBVAV(CBVAVNum).DXCoolCoilName + "\" not found.");
                        ErrorsFound = true;
                    } else {
                        DXCoilErrFlag = false;
                        CBVAV(CBVAVNum).DXCoolCoilIndexNum = VariableSpeedCoils::GetCoilIndexVariableSpeed(state,
                            CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                        if (DXCoilErrFlag) ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                        OANodeErrFlag = false;
                        CBVAV(CBVAVNum).CondenserNodeNum =
                            VariableSpeedCoils::GetVSCoilCondenserInletNode(state, CBVAV(CBVAVNum).DXCoolCoilName, OANodeErrFlag);
                        if (OANodeErrFlag) ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    }

                } else if (UtilityRoutines::SameString(Alphas(14), "CoilSystem:Cooling:DX:HeatExchangerAssisted")) {
                    CBVAV(CBVAVNum).DXCoolCoilType_Num = DataHVACGlobals::CoilDX_CoolingHXAssisted;
                    HXDXCoolCoilName = HVACHXAssistedCoolingCoil::GetHXDXCoilName(
                        state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    CBVAV(CBVAVNum).DXCoilInletNode = HVACHXAssistedCoolingCoil::GetCoilInletNode(
                        state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    CBVAV(CBVAVNum).DXCoilOutletNode = HVACHXAssistedCoolingCoil::GetCoilOutletNode(
                        state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    if (DXErrorsFound) {
                        ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                        ShowContinueError(state, "CoilSystem:Cooling:DX:HeatExchangerAssisted \"" + CBVAV(CBVAVNum).DXCoolCoilName + "\" not found.");
                        ErrorsFound = true;
                    } else {
                        DXCoilErrFlag = false;
                        int ActualCoolCoilType = HVACHXAssistedCoolingCoil::GetCoilObjectTypeNum(
                            state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                        if (ActualCoolCoilType == DataHVACGlobals::CoilDX_CoolingSingleSpeed) {
                            DXCoils::GetDXCoilIndex(state,
                                                    HVACHXAssistedCoolingCoil::GetHXDXCoilName(
                                                        state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXCoilErrFlag),
                                                    CBVAV(CBVAVNum).DXCoolCoilIndexNum,
                                                    DXCoilErrFlag,
                                                    "Coil:Cooling:DX:SingleSpeed",
                                                    ObjexxFCL::Optional_bool_const());
                            if (DXCoilErrFlag) ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");

                            //         Mine outdoor condenser node from DX coil through HXAssistedDXCoil object
                            OANodeErrFlag = false;
                            CBVAV(CBVAVNum).CondenserNodeNum =
                                DXCoils::GetCoilCondenserInletNode(state, "Coil:Cooling:DX:SingleSpeed", HXDXCoolCoilName, OANodeErrFlag);
                            if (OANodeErrFlag) ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                        } else if (ActualCoolCoilType == DataHVACGlobals::Coil_CoolingAirToAirVariableSpeed) {
                            CBVAV(CBVAVNum).DXCoolCoilIndexNum = VariableSpeedCoils::GetCoilIndexVariableSpeed(state,
                                "Coil:Cooling:DX:VariableSpeed",
                                HVACHXAssistedCoolingCoil::GetHXDXCoilName(
                                    state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXCoilErrFlag),
                                DXCoilErrFlag);
                            if (DXCoilErrFlag) ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                            OANodeErrFlag = false;
                            CBVAV(CBVAVNum).CondenserNodeNum = VariableSpeedCoils::GetVSCoilCondenserInletNode(state,
                                HVACHXAssistedCoolingCoil::GetHXDXCoilName(state,
                                    CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXCoilErrFlag),
                                OANodeErrFlag);
                            if (OANodeErrFlag) ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                        }
                    }
                } else if (UtilityRoutines::SameString(Alphas(14), "Coil:Cooling:DX:TwoStageWithHumidityControlMode")) {
                    CBVAV(CBVAVNum).DXCoolCoilType_Num = DataHVACGlobals::CoilDX_CoolingTwoStageWHumControl;
                    CBVAV(CBVAVNum).DXCoilInletNode =
                        DXCoils::GetCoilInletNode(state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    CBVAV(CBVAVNum).DXCoilOutletNode =
                        DXCoils::GetCoilOutletNode(state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, DXErrorsFound);
                    if (DXErrorsFound) {
                        ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                        ShowContinueError(state, "Coil:Cooling:DX:TwoStageWithHumidityControlMode \"" + CBVAV(CBVAVNum).DXCoolCoilName + "\" not found.");
                        ErrorsFound = true;
                    } else {

                        DXCoilErrFlag = false;
                        DXCoils::GetDXCoilIndex(state,
                                                CBVAV(CBVAVNum).DXCoolCoilName,
                                                CBVAV(CBVAVNum).DXCoolCoilIndexNum,
                                                DXCoilErrFlag,
                                                CBVAV(CBVAVNum).DXCoolCoilType,
                                                ObjexxFCL::Optional_bool_const());
                        if (DXCoilErrFlag) ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");

                        //         Mine outdoor condenser node from multimode DX coil object
                        OANodeErrFlag = false;
                        CBVAV(CBVAVNum).CondenserNodeNum =
                            DXCoils::GetCoilCondenserInletNode(state, CBVAV(CBVAVNum).DXCoolCoilType, CBVAV(CBVAVNum).DXCoolCoilName, OANodeErrFlag);
                        if (OANodeErrFlag) ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    }
                }

            } else {
                ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                ShowContinueError(state, "Illegal " + cAlphaFields(14) + " = " + Alphas(14));
                ErrorsFound = true;
            }

            CBVAV(CBVAVNum).FanOpModeSchedPtr = ScheduleManager::GetScheduleIndex(state, Alphas(13)); // convert schedule name to pointer (index number)
            if (CBVAV(CBVAVNum).FanOpModeSchedPtr != 0) {
                if (!ScheduleManager::CheckScheduleValueMinMax(state, CBVAV(CBVAVNum).FanOpModeSchedPtr, "<", 0.0, ">", 1.0)) {
                    ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, "The schedule values in " + cAlphaFields(13) + " must be 0 to 1.");
                    ShowContinueError(state, "A value of 0 represents cycling fan mode, any other value up to 1 represents constant fan mode.");
                    ErrorsFound = true;
                }

                //     Check supply air fan operating mode for cycling fan, if NOT cycling fan set AirFlowControl
                if (!ScheduleManager::CheckScheduleValueMinMax(state,
                        CBVAV(CBVAVNum).FanOpModeSchedPtr, ">=", 0.0, "<=", 0.0)) { // Autodesk:Note Range is 0 to 0?
                    //       set air flow control mode,
                    //       UseCompressorOnFlow  = operate at last cooling or heating air flow requested when compressor is off
                    //       UseCompressorOffFlow = operate at value specified by user (no input for this object type, UseCompONFlow)
                    //       AirFlowControl only valid if fan opmode = DataHVACGlobals::ContFanCycCoil
                    if (CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow == 0.0) {
                        CBVAV(CBVAVNum).AirFlowControl = UseCompressorOnFlow;
                    } else {
                        CBVAV(CBVAVNum).AirFlowControl = UseCompressorOffFlow;
                    }
                }

            } else {
                if (!lAlphaBlanks(13)) {
                    ShowWarningError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, cAlphaFields(13) + " = " + Alphas(13) +
                                      " not found. Supply air fan operating mode set to constant operation and simulation continues.");
                }
                CBVAV(CBVAVNum).OpMode = DataHVACGlobals::ContFanCycCoil;
                if (CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow == 0.0) {
                    CBVAV(CBVAVNum).AirFlowControl = UseCompressorOnFlow;
                } else {
                    CBVAV(CBVAVNum).AirFlowControl = UseCompressorOffFlow;
                }
            }

            //   Check FanVolFlow, must be >= CBVAV flow
            if (CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow &&
                    CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize && CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow != 0.0) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than ",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            cAlphaFields(11),
                                            CBVAV(CBVAVNum).FanName) +
                                         cNumericFields(3));
                    ShowContinueError(state, ' ' + cNumericFields(3) + " is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, " Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
            }
            //   only check that OA flow when compressor is OFF is >= SA flow when compressor is OFF when both are not autosized and
            //   that MaxNoCoolHeatAirVolFlow is /= 0 (trigger to use compressor ON flow, see AirFlowControl variable initialization above)
            if (CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow > CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow &&
                CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow != DataSizing::AutoSize && CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize &&
                CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow != 0.0) {
                ShowWarningError(state, CurrentModuleObject + ": " + cNumericFields(6) + " cannot be greater than " + cNumericFields(3));
                ShowContinueError(state, ' ' + cNumericFields(6) + " is reset to the fan flow rate and the simulation continues.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
            }

            if (UtilityRoutines::SameString(Alphas(16), "Coil:Heating:DX:SingleSpeed") ||
                UtilityRoutines::SameString(Alphas(16), "Coil:Heating:DX:VariableSpeed") ||
                UtilityRoutines::SameString(Alphas(16), "Coil:Heating:Fuel") || UtilityRoutines::SameString(Alphas(16), "Coil:Heating:Electric") ||
                UtilityRoutines::SameString(Alphas(16), "Coil:Heating:Water") || UtilityRoutines::SameString(Alphas(16), "Coil:Heating:Steam")) {
                CBVAV(CBVAVNum).HeatCoilType = Alphas(16);
                CBVAV(CBVAVNum).HeatCoilName = Alphas(17);

                if (UtilityRoutines::SameString(Alphas(16), "Coil:Heating:DX:SingleSpeed")) {
                    CBVAV(CBVAVNum).HeatCoilType_Num = DataHVACGlobals::CoilDX_HeatingEmpirical;
                    DXCoilErrFlag = false;
                    CBVAV(CBVAVNum).MinOATCompressor =
                        DXCoils::GetMinOATCompressor(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, DXCoilErrFlag);
                    CBVAV(CBVAVNum).HeatingCoilInletNode =
                        DXCoils::GetCoilInletNode(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, DXCoilErrFlag);
                    CBVAV(CBVAVNum).HeatingCoilOutletNode =
                        DXCoils::GetCoilOutletNode(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, DXCoilErrFlag);
                    DXCoils::GetDXCoilIndex(state,
                                            CBVAV(CBVAVNum).HeatCoilName,
                                            CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                            DXCoilErrFlag,
                                            CBVAV(CBVAVNum).HeatCoilType,
                                            ObjexxFCL::Optional_bool_const());
                    if (DXCoilErrFlag) ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");

                } else if (UtilityRoutines::SameString(Alphas(16), "Coil:Heating:DX:VariableSpeed")) {
                    CBVAV(CBVAVNum).HeatCoilType_Num = DataHVACGlobals::Coil_HeatingAirToAirVariableSpeed;
                    DXCoilErrFlag = false;
                    CBVAV(CBVAVNum).DXHeatCoilIndexNum =
                        VariableSpeedCoils::GetCoilIndexVariableSpeed(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, DXCoilErrFlag);
                    CBVAV(CBVAVNum).MinOATCompressor = VariableSpeedCoils::GetVSCoilMinOATCompressor(state, CBVAV(CBVAVNum).HeatCoilName, DXCoilErrFlag);
                    CBVAV(CBVAVNum).HeatingCoilInletNode =
                        VariableSpeedCoils::GetCoilInletNodeVariableSpeed(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, DXCoilErrFlag);
                    CBVAV(CBVAVNum).HeatingCoilOutletNode =
                        VariableSpeedCoils::GetCoilOutletNodeVariableSpeed(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, DXCoilErrFlag);
                    if (DXCoilErrFlag) ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                } else if (UtilityRoutines::SameString(Alphas(16), "Coil:Heating:Fuel")) {
                    CBVAV(CBVAVNum).HeatCoilType_Num = DataHVACGlobals::Coil_HeatingGasOrOtherFuel;
                    CBVAV(CBVAVNum).MinOATCompressor = -999.9;
                    CBVAV(CBVAVNum).HeatingCoilInletNode =
                        HeatingCoils::GetCoilInletNode(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, ErrorsFound);
                    CBVAV(CBVAVNum).HeatingCoilOutletNode =
                        HeatingCoils::GetCoilOutletNode(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, ErrorsFound);
                } else if (UtilityRoutines::SameString(Alphas(16), "Coil:Heating:Electric")) {
                    CBVAV(CBVAVNum).HeatCoilType_Num = DataHVACGlobals::Coil_HeatingElectric;
                    CBVAV(CBVAVNum).MinOATCompressor = -999.9;
                    CBVAV(CBVAVNum).HeatingCoilInletNode =
                        HeatingCoils::GetCoilInletNode(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, ErrorsFound);
                    CBVAV(CBVAVNum).HeatingCoilOutletNode =
                        HeatingCoils::GetCoilOutletNode(state, CBVAV(CBVAVNum).HeatCoilType, CBVAV(CBVAVNum).HeatCoilName, ErrorsFound);
                } else if (UtilityRoutines::SameString(Alphas(16), "Coil:Heating:Water")) {
                    CBVAV(CBVAVNum).HeatCoilType_Num = DataHVACGlobals::Coil_HeatingWater;
                    errFlag = false;
                    CBVAV(CBVAVNum).CoilControlNode = WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", CBVAV(CBVAVNum).HeatCoilName, errFlag);
                    CBVAV(CBVAVNum).MaxHeatCoilFluidFlow =
                        WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", CBVAV(CBVAVNum).HeatCoilName, errFlag);
                    HeatCoilInletNodeNum = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", CBVAV(CBVAVNum).HeatCoilName, errFlag);
                    CBVAV(CBVAVNum).HeatingCoilInletNode = HeatCoilInletNodeNum;
                    HeatCoilOutletNodeNum = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", CBVAV(CBVAVNum).HeatCoilName, errFlag);
                    CBVAV(CBVAVNum).HeatingCoilOutletNode = HeatCoilOutletNodeNum;
                    if (errFlag) {
                        ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                        ErrorsFound = true;
                    }
                } else if (UtilityRoutines::SameString(Alphas(16), "COIL:HEATING:STEAM")) {
                    CBVAV(CBVAVNum).HeatCoilType_Num = DataHVACGlobals::Coil_HeatingSteam;
                    errFlag = false;
                    CBVAV(CBVAVNum).HeatCoilIndex = SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", CBVAV(CBVAVNum).HeatCoilName, errFlag);

                    HeatCoilInletNodeNum = SteamCoils::GetCoilAirInletNode(state, CBVAV(CBVAVNum).HeatCoilIndex, CBVAV(CBVAVNum).HeatCoilName, errFlag);
                    CBVAV(CBVAVNum).HeatingCoilInletNode = HeatCoilInletNodeNum;
                    CBVAV(CBVAVNum).CoilControlNode =
                        SteamCoils::GetCoilSteamInletNode(state, CBVAV(CBVAVNum).HeatCoilIndex, CBVAV(CBVAVNum).HeatCoilName, errFlag);
                    CBVAV(CBVAVNum).MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, CBVAV(CBVAVNum).HeatCoilIndex, errFlag);
                    int SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                    Real64 SteamDensity =
                        FluidProperties::GetSatDensityRefrig(state, fluidNameSteam, TempSteamIn, 1.0, SteamIndex, getUnitaryHeatCoolVAVChangeoverBypass);
                    if (CBVAV(CBVAVNum).MaxHeatCoilFluidFlow > 0.0) {
                        CBVAV(CBVAVNum).MaxHeatCoilFluidFlow =
                            SteamCoils::GetCoilMaxSteamFlowRate(state, CBVAV(CBVAVNum).HeatCoilIndex, errFlag) * SteamDensity;
                    }
                    HeatCoilOutletNodeNum = SteamCoils::GetCoilAirOutletNode(state, CBVAV(CBVAVNum).HeatCoilIndex, CBVAV(CBVAVNum).HeatCoilName, errFlag);
                    CBVAV(CBVAVNum).HeatingCoilOutletNode = HeatCoilOutletNodeNum;
                    if (errFlag) {
                        ShowContinueError(state, "...occurs in " + CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                        ErrorsFound = true;
                    }
                }
            } else {
                ShowSevereError(state, CurrentModuleObject + " illegal " + cAlphaFields(16) + " = " + Alphas(16));
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            if (CBVAV(CBVAVNum).DXCoilOutletNode != CBVAV(CBVAVNum).HeatingCoilInletNode) {
                ShowSevereError(state, CurrentModuleObject + " illegal coil placement. Cooling coil must be upstream of heating coil.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ErrorsFound = true;
            }

            if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::BlowThru) {
                if (CBVAV(CBVAVNum).SplitterOutletAirNode != CBVAV(CBVAVNum).HeatingCoilOutletNode) {
                    ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, "Illegal " + cAlphaFields(6) + " = " + SplitterOutletNodeName + '.');
                    ShowContinueError(state, cAlphaFields(6) +
                                      " must be the same as the outlet node specified in the heating coil object = " + CBVAV(CBVAVNum).HeatCoilType +
                                      ": " + CBVAV(CBVAVNum).HeatCoilName + " when blow through " + cAlphaFields(12) + " is selected.");
                    ErrorsFound = true;
                }
                if (CBVAV(CBVAVNum).MixerMixedAirNode != CBVAV(CBVAVNum).FanInletNodeNum) {
                    ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, "Illegal " + cAlphaFields(11) +
                                      ". The fan inlet node name must be the same as the mixed air node specified in the " + cAlphaFields(9) + " = " +
                                      CBVAV(CBVAVNum).OAMixName + " when blow through " + cAlphaFields(12) + " is selected.");
                    ErrorsFound = true;
                }
            }

            if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::DrawThru) {
                if (CBVAV(CBVAVNum).MixerMixedAirNode != CBVAV(CBVAVNum).DXCoilInletNode) {
                    ShowSevereError(state, CurrentModuleObject + ": " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state,
                        "Illegal cooling coil placement. The cooling coil inlet node name must be the same as the mixed air node specified in the " +
                        cAlphaFields(9) + " = " + CBVAV(CBVAVNum).OAMixName + " when draw through " + cAlphaFields(12) + " is selected.");
                    ErrorsFound = true;
                }
            }

            if (UtilityRoutines::SameString(Alphas(18), "CoolingPriority")) {
                CBVAV(CBVAVNum).PriorityControl = CoolingPriority;
            } else if (UtilityRoutines::SameString(Alphas(18), "HeatingPriority")) {
                CBVAV(CBVAVNum).PriorityControl = HeatingPriority;
            } else if (UtilityRoutines::SameString(Alphas(18), "ZonePriority")) {
                CBVAV(CBVAVNum).PriorityControl = ZonePriority;
            } else if (UtilityRoutines::SameString(Alphas(18), "LoadPriority")) {
                CBVAV(CBVAVNum).PriorityControl = LoadPriority;
            } else {
                ShowSevereError(state, CurrentModuleObject + " illegal " + cAlphaFields(18) + " = " + Alphas(18));
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ShowContinueError(state, "Valid choices are CoolingPriority, HeatingPriority, ZonePriority or LoadPriority.");
                ErrorsFound = true;
            }

            if (Numbers(7) > 0.0) {
                CBVAV(CBVAVNum).MinLATCooling = Numbers(7);
            } else {
                CBVAV(CBVAVNum).MinLATCooling = 10.0;
            }

            if (Numbers(8) > 0.0) {
                CBVAV(CBVAVNum).MaxLATHeating = Numbers(8);
            } else {
                CBVAV(CBVAVNum).MaxLATHeating = 50.0;
            }

            if (CBVAV(CBVAVNum).MinLATCooling > CBVAV(CBVAVNum).MaxLATHeating) {
                ShowWarningError(state, CurrentModuleObject + ": illegal leaving air temperature specified.");
                ShowContinueError(state, "Resetting " + cNumericFields(7) + " equal to " + cNumericFields(8) + " and the simulation continues.");
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).MinLATCooling = CBVAV(CBVAVNum).MaxLATHeating;
            }

            // Dehumidification control mode
            if (UtilityRoutines::SameString(Alphas(19), "None")) {
                CBVAV(CBVAVNum).DehumidControlType = DehumidControl_None;
            } else if (UtilityRoutines::SameString(Alphas(19), "")) {
                CBVAV(CBVAVNum).DehumidControlType = DehumidControl_None;
            } else if (UtilityRoutines::SameString(Alphas(19), "Multimode")) {
                if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::CoilDX_CoolingTwoStageWHumControl) {
                    CBVAV(CBVAVNum).DehumidControlType = DehumidControl_Multimode;
                } else {
                    ShowWarningError(state, "Invalid " + cAlphaFields(19) + " = " + Alphas(19));
                    ShowContinueError(state, "In " + CurrentModuleObject + " \"" + CBVAV(CBVAVNum).Name + "\".");
                    ShowContinueError(state, "Valid only with " + cAlphaFields(14) + " = Coil:Cooling:DX:TwoStageWithHumidityControlMode.");
                    ShowContinueError(state, "Setting " + cAlphaFields(19) + " to \"None\" and the simulation continues.");
                    CBVAV(CBVAVNum).DehumidControlType = DehumidControl_None;
                }
            } else if (UtilityRoutines::SameString(Alphas(19), "CoolReheat")) {
                if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::CoilDX_CoolingTwoStageWHumControl) {
                    CBVAV(CBVAVNum).DehumidControlType = DehumidControl_CoolReheat;
                } else {
                    ShowWarningError(state, "Invalid " + cAlphaFields(19) + " = " + Alphas(19));
                    ShowContinueError(state, "In " + CurrentModuleObject + " \"" + CBVAV(CBVAVNum).Name + "\".");
                    ShowContinueError(state, "Valid only with " + cAlphaFields(14) + " = Coil:Cooling:DX:TwoStageWithHumidityControlMode.");
                    ShowContinueError(state, "Setting " + cAlphaFields(19) + " to \"None\" and the simulation continues.");
                    CBVAV(CBVAVNum).DehumidControlType = DehumidControl_None;
                }
            } else {
                ShowSevereError(state, "Invalid " + cAlphaFields(19) + " =" + Alphas(19));
                ShowContinueError(state, "In " + CurrentModuleObject + " \"" + CBVAV(CBVAVNum).Name + "\".");
            }

            if (NumNumbers > 8) {
                CBVAV(CBVAVNum).minModeChangeTime = Numbers(9);
            }

            //   Initialize last mode of compressor operation
            CBVAV(CBVAVNum).LastMode = HeatingMode;

            if (CBVAV(CBVAVNum).FanType_Num != DataHVACGlobals::FanType_SimpleOnOff &&
                CBVAV(CBVAVNum).FanType_Num != DataHVACGlobals::FanType_SimpleConstVolume &&
                CBVAV(CBVAVNum).FanType_Num != DataHVACGlobals::FanType_SystemModelObject) {
                ShowSevereError(state, CurrentModuleObject + " illegal " + cAlphaFields(10) + " in fan object = " + CBVAV(CBVAVNum).FanName);
                ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                ShowContinueError(state, " The fan object type must be Fan:SystemModel, Fan:OnOff or Fan:ConstantVolume.");
                ErrorsFound = true;
            } else if (CBVAV(CBVAVNum).FanType_Num == DataHVACGlobals::FanType_SimpleOnOff ||
                       CBVAV(CBVAVNum).FanType_Num == DataHVACGlobals::FanType_SimpleConstVolume) {
                if (CBVAV(CBVAVNum).FanType_Num == DataHVACGlobals::FanType_SimpleOnOff &&
                    !UtilityRoutines::SameString(CBVAV(CBVAVNum).FanType, "Fan:OnOff")) {
                    ShowWarningError(state, CurrentModuleObject + " has " + cAlphaFields(10) + " = " + CBVAV(CBVAVNum).FanType +
                                     " which is inconsistent with the fan object.");
                    ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, " The fan object (" + CBVAV(CBVAVNum).FanName + ") is actually a valid fan type and the simulation continues.");
                    ShowContinueError(state, " Node connections errors may result due to the inconsistent fan type.");
                }
                if (CBVAV(CBVAVNum).FanType_Num == DataHVACGlobals::FanType_SimpleConstVolume &&
                    !UtilityRoutines::SameString(CBVAV(CBVAVNum).FanType, "Fan:ConstantVolume")) {
                    ShowWarningError(state, CurrentModuleObject + " has " + cAlphaFields(10) + " = " + CBVAV(CBVAVNum).FanType +
                                     " which is inconsistent with fan object.");
                    ShowContinueError(state, "Occurs in " + CurrentModuleObject + " = " + CBVAV(CBVAVNum).Name);
                    ShowContinueError(state, " The fan object (" + CBVAV(CBVAVNum).FanName + ") is actually a valid fan type and the simulation continues.");
                    ShowContinueError(state, " Node connections errors may result due to the inconsistent fan type.");
                }
            }

            // Add fan to component sets array
            if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::BlowThru) {
                CompSetFanInlet = DataLoopNode::NodeID(CBVAV(CBVAVNum).MixerMixedAirNode);
                CompSetFanOutlet = DataLoopNode::NodeID(CBVAV(CBVAVNum).DXCoilInletNode);
            } else {
                CompSetFanInlet = DataLoopNode::NodeID(CBVAV(CBVAVNum).HeatingCoilOutletNode);
                CompSetFanOutlet = SplitterOutletNodeName;
            }
            CompSetCoolInlet = DataLoopNode::NodeID(CBVAV(CBVAVNum).DXCoilInletNode);
            CompSetCoolOutlet = DataLoopNode::NodeID(CBVAV(CBVAVNum).DXCoilOutletNode);

            // Add fan to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name, CBVAV(CBVAVNum).FanType, CBVAV(CBVAVNum).FanName, CompSetFanInlet, CompSetFanOutlet);

            // Add cooling coil to component sets array
            BranchNodeConnections::SetUpCompSets(state, CBVAV(CBVAVNum).UnitType,
                                                 CBVAV(CBVAVNum).Name,
                                                 CBVAV(CBVAVNum).DXCoolCoilType,
                                                 CBVAV(CBVAVNum).DXCoolCoilName,
                                                 CompSetCoolInlet,
                                                 CompSetCoolOutlet);

            // Add heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(state, CBVAV(CBVAVNum).UnitType,
                                                 CBVAV(CBVAVNum).Name,
                                                 CBVAV(CBVAVNum).HeatCoilType,
                                                 CBVAV(CBVAVNum).HeatCoilName,
                                                 DataLoopNode::NodeID(CBVAV(CBVAVNum).HeatingCoilInletNode),
                                                 DataLoopNode::NodeID(CBVAV(CBVAVNum).HeatingCoilOutletNode));

            // Set up component set for OA mixer - use OA node and Mixed air node
            BranchNodeConnections::SetUpCompSets(state, CBVAV(CBVAVNum).UnitType,
                                                 CBVAV(CBVAVNum).Name,
                                                 CBVAV(CBVAVNum).OAMixType,
                                                 CBVAV(CBVAVNum).OAMixName,
                                                 DataLoopNode::NodeID(CBVAV(CBVAVNum).MixerOutsideAirNode),
                                                 DataLoopNode::NodeID(CBVAV(CBVAVNum).MixerMixedAirNode));

            BranchNodeConnections::TestCompSet(state, CBVAV(CBVAVNum).UnitType,
                                               CBVAV(CBVAVNum).Name,
                                               DataLoopNode::NodeID(CBVAV(CBVAVNum).AirInNode),
                                               DataLoopNode::NodeID(CBVAV(CBVAVNum).AirOutNode),
                                               "Air Nodes");

            //   Find air loop associated with CBVAV system
            for (int AirLoopNum = 1; AirLoopNum <= DataHVACGlobals::NumPrimaryAirSys; ++AirLoopNum) {
                for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents; ++CompNum) {
                        if (!UtilityRoutines::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name,
                                                         CBVAV(CBVAVNum).Name) ||
                            !UtilityRoutines::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf,
                                                         CBVAV(CBVAVNum).UnitType))
                            continue;
                        CBVAV(CBVAVNum).AirLoopNumber = AirLoopNum;
                        //         Should EXIT here or do other checking?
                        break;
                    }
                }
            }

            if (CBVAV(CBVAVNum).AirLoopNumber > 0) {
                CBVAV(CBVAVNum).NumControlledZones = state.dataAirLoop->AirToZoneNodeInfo(CBVAV(CBVAVNum).AirLoopNumber).NumZonesCooled;
                CBVAV(CBVAVNum).ControlledZoneNum.allocate(CBVAV(CBVAVNum).NumControlledZones);
                CBVAV(CBVAVNum).ActualZoneNum.allocate(CBVAV(CBVAVNum).NumControlledZones);
                CBVAV(CBVAVNum).ActualZoneNodeNum.allocate(CBVAV(CBVAVNum).NumControlledZones);
                CBVAV(CBVAVNum).CBVAVBoxOutletNode.allocate(CBVAV(CBVAVNum).NumControlledZones);
                CBVAV(CBVAVNum).ZoneSequenceCoolingNum.allocate(CBVAV(CBVAVNum).NumControlledZones);
                CBVAV(CBVAVNum).ZoneSequenceHeatingNum.allocate(CBVAV(CBVAVNum).NumControlledZones);

                CBVAV(CBVAVNum).ControlledZoneNum = 0;
                CBVAV(CBVAVNum).ActualZoneNum = 0;
                for (int AirLoopZoneNum = 1; AirLoopZoneNum <= state.dataAirLoop->AirToZoneNodeInfo(CBVAV(CBVAVNum).AirLoopNumber).NumZonesCooled;
                     ++AirLoopZoneNum) {
                    CBVAV(CBVAVNum).ControlledZoneNum(AirLoopZoneNum) =
                        state.dataAirLoop->AirToZoneNodeInfo(CBVAV(CBVAVNum).AirLoopNumber).CoolCtrlZoneNums(AirLoopZoneNum);
                    if (CBVAV(CBVAVNum).ControlledZoneNum(AirLoopZoneNum) > 0) {
                        CBVAV(CBVAVNum).ActualZoneNodeNum(AirLoopZoneNum) =
                            DataZoneEquipment::ZoneEquipConfig(CBVAV(CBVAVNum).ControlledZoneNum(AirLoopZoneNum)).ZoneNode;
                        CBVAV(CBVAVNum).ActualZoneNum(AirLoopZoneNum) =
                            DataZoneEquipment::ZoneEquipConfig(CBVAV(CBVAVNum).ControlledZoneNum(AirLoopZoneNum)).ActualZoneNum;
                        CBVAV(CBVAVNum).CBVAVBoxOutletNode(AirLoopZoneNum) =
                            state.dataAirLoop->AirToZoneNodeInfo(CBVAV(CBVAVNum).AirLoopNumber).CoolZoneInletNodes(AirLoopZoneNum);
                        // check for thermostat in controlled zone
                        bool FoundTstatZone = false;
                        for (int TstatZoneNum = 1; TstatZoneNum <= DataZoneControls::NumTempControlledZones; ++TstatZoneNum) {
                            if (DataZoneControls::TempControlledZone(TstatZoneNum).ActualZoneNum != CBVAV(CBVAVNum).ControlledZoneNum(AirLoopZoneNum))
                                continue;
                            FoundTstatZone = true;
                        }
                        if (!FoundTstatZone) {
                            ShowWarningError(state, CurrentModuleObject + " \"" + CBVAV(CBVAVNum).Name + "\"");
                            ShowContinueError(state, "Thermostat not found in zone = " +
                                              DataZoneEquipment::ZoneEquipConfig(CBVAV(CBVAVNum).ControlledZoneNum(AirLoopZoneNum)).ZoneName +
                                              " and the simulation continues.");
                            ShowContinueError(state, "This zone will not be controlled to a temperature setpoint.");
                        }
                        int zoneNum = CBVAV(CBVAVNum).ControlledZoneNum(AirLoopZoneNum);
                        int zoneInlet = CBVAV(CBVAVNum).CBVAVBoxOutletNode(AirLoopZoneNum);
                        int coolingPriority = 0;
                        int heatingPriority = 0;
                        // setup zone equipment sequence information based on finding matching air terminal
                        if (DataZoneEquipment::ZoneEquipConfig(zoneNum).EquipListIndex > 0) {
                            DataZoneEquipment::ZoneEquipList(DataZoneEquipment::ZoneEquipConfig(zoneNum).EquipListIndex)
                                .getPrioritiesForInletNode(state, zoneInlet, coolingPriority, heatingPriority);
                            CBVAV(CBVAVNum).ZoneSequenceCoolingNum(AirLoopZoneNum) = coolingPriority;
                            CBVAV(CBVAVNum).ZoneSequenceHeatingNum(AirLoopZoneNum) = heatingPriority;
                        }
                        if (CBVAV(CBVAVNum).ZoneSequenceCoolingNum(AirLoopZoneNum) == 0 ||
                            CBVAV(CBVAVNum).ZoneSequenceHeatingNum(AirLoopZoneNum) == 0) {
                            ShowSevereError(state, "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass, \"" + CBVAV(CBVAVNum).Name +
                                            "\": Airloop air terminal in the zone equipment list for zone = " +
                                            DataZoneEquipment::ZoneEquipConfig(zoneNum).ZoneName +
                                            " not found or is not allowed Zone Equipment Cooling or Heating Sequence = 0.");
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, "Controlled Zone node not found.");
                        ErrorsFound = true;
                    }
                }
            } else {
            }

        } // CBVAVNum = 1,NumCBVAV

        if (ErrorsFound) {
            ShowFatalError(state, "GetCBVAV: Errors found in getting " + CurrentModuleObject + " input.");
            ShowContinueError(state, "... Preceding condition causes termination.");
        }

        for (int CBVAVNum = 1; CBVAVNum <= NumCBVAV; ++CBVAVNum) {
            // Setup Report variables for the Fan Coils
            SetupOutputVariable(state, "Unitary System Total Heating Rate",
                                OutputProcessor::Unit::W,
                                CBVAV(CBVAVNum).TotHeatEnergyRate,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Total Heating Energy",
                                OutputProcessor::Unit::J,
                                CBVAV(CBVAVNum).TotHeatEnergy,
                                "System",
                                "Sum",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Total Cooling Rate",
                                OutputProcessor::Unit::W,
                                CBVAV(CBVAVNum).TotCoolEnergyRate,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Total Cooling Energy",
                                OutputProcessor::Unit::J,
                                CBVAV(CBVAVNum).TotCoolEnergy,
                                "System",
                                "Sum",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Sensible Heating Rate",
                                OutputProcessor::Unit::W,
                                CBVAV(CBVAVNum).SensHeatEnergyRate,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Sensible Heating Energy",
                                OutputProcessor::Unit::J,
                                CBVAV(CBVAVNum).SensHeatEnergy,
                                "System",
                                "Sum",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Sensible Cooling Rate",
                                OutputProcessor::Unit::W,
                                CBVAV(CBVAVNum).SensCoolEnergyRate,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Sensible Cooling Energy",
                                OutputProcessor::Unit::J,
                                CBVAV(CBVAVNum).SensCoolEnergy,
                                "System",
                                "Sum",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Latent Heating Rate",
                                OutputProcessor::Unit::W,
                                CBVAV(CBVAVNum).LatHeatEnergyRate,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Latent Heating Energy",
                                OutputProcessor::Unit::J,
                                CBVAV(CBVAVNum).LatHeatEnergy,
                                "System",
                                "Sum",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Latent Cooling Rate",
                                OutputProcessor::Unit::W,
                                CBVAV(CBVAVNum).LatCoolEnergyRate,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Latent Cooling Energy",
                                OutputProcessor::Unit::J,
                                CBVAV(CBVAVNum).LatCoolEnergy,
                                "System",
                                "Sum",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state,
                "Unitary System Electricity Rate", OutputProcessor::Unit::W, CBVAV(CBVAVNum).ElecPower, "System", "Average", CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Electricity Energy",
                                OutputProcessor::Unit::J,
                                CBVAV(CBVAVNum).ElecConsumption,
                                "System",
                                "Sum",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Fan Part Load Ratio",
                                OutputProcessor::Unit::None,
                                CBVAV(CBVAVNum).FanPartLoadRatio,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Compressor Part Load Ratio",
                                OutputProcessor::Unit::None,
                                CBVAV(CBVAVNum).CompPartLoadRatio,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Bypass Air Mass Flow Rate",
                                OutputProcessor::Unit::kg_s,
                                CBVAV(CBVAVNum).BypassMassFlowRate,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Air Outlet Setpoint Temperature",
                                OutputProcessor::Unit::C,
                                CBVAV(CBVAVNum).OutletTempSetPoint,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
            SetupOutputVariable(state, "Unitary System Operating Mode Index",
                                OutputProcessor::Unit::None,
                                CBVAV(CBVAVNum).HeatCoolMode,
                                "System",
                                "Average",
                                CBVAV(CBVAVNum).Name);
        }
    }

    void InitCBVAV(EnergyPlusData &state,
                   int const CBVAVNum,            // Index of the current CBVAV unit being simulated
                   bool const FirstHVACIteration, // TRUE if first HVAC iteration
                   int const AirLoopNum,          // air loop index
                   Real64 &OnOffAirFlowRatio,     // Ratio of compressor ON airflow to average airflow over timestep
                   bool &HXUnitOn                 // flag to enable heat exchanger
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006
        //       MODIFIED       B. Griffith, May 2009, EMS setpoint check
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the changeover-bypass VAV system components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations. The CBVAV system is simulated with no load (coils off) to
        // determine the outlet temperature. A setpoint temperature is calculated on FirstHVACIteration = TRUE.
        // Once the setpoint is calculated, the inlet mass flow rate on FirstHVACIteration = FALSE is used to
        // determine the bypass fraction. The simulation converges quickly on mass flow rate. If the zone
        // temperatures float in the deadband, additional iterations are required to converge on mass flow rate.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static std::string const RoutineName("InitCBVAV");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static bool MyOneTimeFlag(true);     // Initialization flag
        static Array1D_bool MyEnvrnFlag;     // Used for initializations each begin environment flag
        static Array1D_bool MySizeFlag;      // Used for sizing CBVAV inputs one time
        static Array1D_bool MyPlantScanFlag; // Used for initializations plant component for heating coils
        Real64 QSensUnitOut;                 // Output of CBVAV system with coils off
        Real64 OutsideAirMultiplier;         // Outside air multiplier schedule (= 1.0 if no schedule)
        static bool EMSSetPointCheck(false); // local temporary
        static bool ErrorsFound(false);      // Set to true if errors in input, fatal at end of routine
        Real64 QCoilActual;                  // actual CBVAV steam heating coil load met (W)
        bool ErrorFlag;                      // local error flag returned from data mining
        Real64 mdot;                         // heating coil fluid mass flow rate, kg/s

        int InNode = CBVAV(CBVAVNum).AirInNode;
        int OutNode = CBVAV(CBVAVNum).AirOutNode;

        // Do the one time initializations
        if (MyOneTimeFlag) {

            MyEnvrnFlag.allocate(NumCBVAV);
            MySizeFlag.allocate(NumCBVAV);
            MyPlantScanFlag.allocate(NumCBVAV);
            MyEnvrnFlag = true;
            MySizeFlag = true;
            MyPlantScanFlag = true;

            MyOneTimeFlag = false;
            // speed up test based on code from 16 years ago to correct cycling fan economizer defect
            // see https://github.com/NREL/EnergyPlusArchive/commit/a2202f8a168fd0330bf3a45392833405e8bd08f2
            // This test sets simple flag so air loop doesn't iterate twice each pass (reverts above change)
            // state.dataAirLoop->AirLoopControlInfo(AirLoopNum).Simple = true;
        }

        if (MyPlantScanFlag(CBVAVNum) && allocated(DataPlant::PlantLoop)) {
            if ((CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) ||
                (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam)) {
                if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {

                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            CBVAV(CBVAVNum).HeatCoilName,
                                                            DataPlant::TypeOf_CoilWaterSimpleHeating,
                                                            CBVAV(CBVAVNum).LoopNum,
                                                            CBVAV(CBVAVNum).LoopSide,
                                                            CBVAV(CBVAVNum).BranchNum,
                                                            CBVAV(CBVAVNum).CompNum,
                                                            ErrorFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            _,
                                                            _);
                    if (ErrorFlag) {
                        ShowFatalError(state, "InitCBVAV: Program terminated for previous conditions.");
                    }

                    CBVAV(CBVAVNum).MaxHeatCoilFluidFlow =
                        WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", CBVAV(CBVAVNum).HeatCoilName, ErrorsFound);

                    if (CBVAV(CBVAVNum).MaxHeatCoilFluidFlow > 0.0) {
                        Real64 FluidDensity = FluidProperties::GetDensityGlycol(state,
                                                                                DataPlant::PlantLoop(CBVAV(CBVAVNum).LoopNum).FluidName,
                                                                                DataGlobalConstants::HWInitConvTemp(),
                                                                                DataPlant::PlantLoop(CBVAV(CBVAVNum).LoopNum).FluidIndex,
                                                                                RoutineName);
                        CBVAV(CBVAVNum).MaxHeatCoilFluidFlow =
                            WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", CBVAV(CBVAVNum).HeatCoilName, ErrorsFound) * FluidDensity;
                    }

                } else if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam) {

                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            CBVAV(CBVAVNum).HeatCoilName,
                                                            DataPlant::TypeOf_CoilSteamAirHeating,
                                                            CBVAV(CBVAVNum).LoopNum,
                                                            CBVAV(CBVAVNum).LoopSide,
                                                            CBVAV(CBVAVNum).BranchNum,
                                                            CBVAV(CBVAVNum).CompNum,
                                                            ErrorFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            _,
                                                            _);

                    if (ErrorFlag) {
                        ShowFatalError(state, "InitCBVAV: Program terminated for previous conditions.");
                    }

                    CBVAV(CBVAVNum).MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, CBVAV(CBVAVNum).HeatCoilIndex, ErrorsFound);

                    if (CBVAV(CBVAVNum).MaxHeatCoilFluidFlow > 0.0) {
                        int SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                        Real64 FluidDensity = FluidProperties::GetSatDensityRefrig(state, fluidNameSteam, TempSteamIn, 1.0, SteamIndex, RoutineName);
                        CBVAV(CBVAVNum).MaxHeatCoilFluidFlow =
                            SteamCoils::GetCoilMaxSteamFlowRate(state, CBVAV(CBVAVNum).HeatCoilIndex, ErrorsFound) * FluidDensity;
                    }
                }

                // fill outlet node for heating coil
                CBVAV(CBVAVNum).CoilOutletNode = DataPlant::PlantLoop(CBVAV(CBVAVNum).LoopNum)
                                                     .LoopSide(CBVAV(CBVAVNum).LoopSide)
                                                     .Branch(CBVAV(CBVAVNum).BranchNum)
                                                     .Comp(CBVAV(CBVAVNum).CompNum)
                                                     .NodeNumOut;
                MyPlantScanFlag(CBVAVNum) = false;

            } else { // CBVAV is not connected to plant
                MyPlantScanFlag(CBVAVNum) = false;
            }
        } else if (MyPlantScanFlag(CBVAVNum) && !state.dataGlobal->AnyPlantInModel) {
            MyPlantScanFlag(CBVAVNum) = false;
        }

        if (!state.dataGlobal->SysSizingCalc && MySizeFlag(CBVAVNum)) {
            SizeCBVAV(state, CBVAVNum);
            // Pass the fan cycling schedule index up to the air loop. Set the air loop unitary system flag.
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CycFanSchedPtr = CBVAV(CBVAVNum).FanOpModeSchedPtr;
            //   Set UnitarySys flag to FALSE and let the heating coil autosize independently of the cooling coil
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySys = false;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).FanOpMode = CBVAV(CBVAVNum).OpMode;
            // check for set point manager on outlet node of CBVAV
            CBVAV(CBVAVNum).OutNodeSPMIndex = SetPointManager::getSPMBasedOnNode(
                state, OutNode, SetPointManager::iCtrlVarType::Temp, SetPointManager::SetPointManagerType::MixedAir, SetPointManager::CtrlNodeType::reference);
            MySizeFlag(CBVAVNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && MyEnvrnFlag(CBVAVNum)) {
            int MixerOutsideAirNode = CBVAV(CBVAVNum).MixerOutsideAirNode;
            Real64 RhoAir = DataEnvironment::StdRhoAir;
            // set the mass flow rates from the input volume flow rates
            CBVAV(CBVAVNum).MaxCoolAirMassFlow = RhoAir * CBVAV(CBVAVNum).MaxCoolAirVolFlow;
            CBVAV(CBVAVNum).CoolOutAirMassFlow = RhoAir * CBVAV(CBVAVNum).CoolOutAirVolFlow;
            CBVAV(CBVAVNum).MaxHeatAirMassFlow = RhoAir * CBVAV(CBVAVNum).MaxHeatAirVolFlow;
            CBVAV(CBVAVNum).HeatOutAirMassFlow = RhoAir * CBVAV(CBVAVNum).HeatOutAirVolFlow;
            CBVAV(CBVAVNum).MaxNoCoolHeatAirMassFlow = RhoAir * CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow;
            CBVAV(CBVAVNum).NoCoolHeatOutAirMassFlow = RhoAir * CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow;
            // set the node max and min mass flow rates
            DataLoopNode::Node(MixerOutsideAirNode).MassFlowRateMax = max(CBVAV(CBVAVNum).CoolOutAirMassFlow, CBVAV(CBVAVNum).HeatOutAirMassFlow);
            DataLoopNode::Node(MixerOutsideAirNode).MassFlowRateMaxAvail =
                max(CBVAV(CBVAVNum).CoolOutAirMassFlow, CBVAV(CBVAVNum).HeatOutAirMassFlow);
            DataLoopNode::Node(MixerOutsideAirNode).MassFlowRateMin = 0.0;
            DataLoopNode::Node(MixerOutsideAirNode).MassFlowRateMinAvail = 0.0;
            DataLoopNode::Node(InNode).MassFlowRateMax = max(CBVAV(CBVAVNum).MaxCoolAirMassFlow, CBVAV(CBVAVNum).MaxHeatAirMassFlow);
            DataLoopNode::Node(InNode).MassFlowRateMaxAvail = max(CBVAV(CBVAVNum).MaxCoolAirMassFlow, CBVAV(CBVAVNum).MaxHeatAirMassFlow);
            DataLoopNode::Node(InNode).MassFlowRateMin = 0.0;
            DataLoopNode::Node(InNode).MassFlowRateMinAvail = 0.0;
            DataLoopNode::Node(OutNode).Temp = DataLoopNode::Node(InNode).Temp;
            DataLoopNode::Node(OutNode).HumRat = DataLoopNode::Node(InNode).HumRat;
            DataLoopNode::Node(OutNode).Enthalpy = DataLoopNode::Node(InNode).Enthalpy;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerReliefAirNode) = DataLoopNode::Node(MixerOutsideAirNode);
            MyEnvrnFlag(CBVAVNum) = false;
            CBVAV(CBVAVNum).LastMode = HeatingMode;
            CBVAV(CBVAVNum).changeOverTimer = -1.0;
            //   set fluid-side hardware limits
            if (CBVAV(CBVAVNum).CoilControlNode > 0) {
                //    If water coil max water flow rate is autosized, simulate once in order to mine max water flow rate
                if (CBVAV(CBVAVNum).MaxHeatCoilFluidFlow == DataSizing::AutoSize) {
                    if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(
                            state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACIteration, CBVAV(CBVAVNum).HeatCoilIndex);
                        ErrorFlag = false;
                        Real64 CoilMaxVolFlowRate =
                            WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", CBVAV(CBVAVNum).HeatCoilName, ErrorFlag);
                        if (ErrorFlag) {
                            ErrorsFound = true;
                        }
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            Real64 FluidDensity = FluidProperties::GetDensityGlycol(state,
                                                                                    DataPlant::PlantLoop(CBVAV(CBVAVNum).LoopNum).FluidName,
                                                                                    DataGlobalConstants::HWInitConvTemp(),
                                                                                    DataPlant::PlantLoop(CBVAV(CBVAVNum).LoopNum).FluidIndex,
                                                                                    RoutineName);
                            CBVAV(CBVAVNum).MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * FluidDensity;
                        }
                    }
                    if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam) {
                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                CBVAV(CBVAVNum).HeatCoilName,
                                                                FirstHVACIteration,
                                                                CBVAV(CBVAVNum).HeatCoilIndex,
                                                                1.0,
                                                                QCoilActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                        ErrorFlag = false;
                        Real64 CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, CBVAV(CBVAVNum).HeatCoilIndex, ErrorFlag);
                        if (ErrorFlag) {
                            ErrorsFound = true;
                        }
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            int SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                            Real64 FluidDensity = FluidProperties::GetSatDensityRefrig(state, fluidNameSteam, TempSteamIn, 1.0, SteamIndex, RoutineName);
                            CBVAV(CBVAVNum).MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * FluidDensity;
                        }
                    }
                } // end of IF(CBVAV(CBVAVNum)%MaxHeatCoilFluidFlow .EQ. DataSizing::AutoSize)THEN

                PlantUtilities::InitComponentNodes(0.0,
                                                   CBVAV(CBVAVNum).MaxHeatCoilFluidFlow,
                                                   CBVAV(CBVAVNum).CoilControlNode,
                                                   CBVAV(CBVAVNum).CoilOutletNode,
                                                   CBVAV(CBVAVNum).LoopNum,
                                                   CBVAV(CBVAVNum).LoopSide,
                                                   CBVAV(CBVAVNum).BranchNum,
                                                   CBVAV(CBVAVNum).CompNum);

            } // end of IF(CBVAV(CBVAVNum)%CoilControlNode .GT. 0)THEN
        }     // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            MyEnvrnFlag(CBVAVNum) = true;
        }

        // IF CBVAV system was not autosized and the fan is autosized, check that fan volumetric flow rate is greater than CBVAV flow rates
        if (CBVAV(CBVAVNum).CheckFanFlow) {
            std::string CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass";

            if (!state.dataGlobal->DoingSizing && CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                //     Check fan versus system supply air flow rates
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxCoolAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "cooling is required ({:.7T}).",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            CBVAV(CBVAVNum).FanName,
                                            CBVAV(CBVAVNum).MaxCoolAirVolFlow));
                    ShowContinueError(state,
                        " The CBVAV system flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, " Occurs in Changeover-bypass VAV system = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).MaxCoolAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxHeatAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "heating is required ({:.7T}).",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            CBVAV(CBVAVNum).FanName,
                                            CBVAV(CBVAVNum).MaxHeatAirVolFlow));
                    ShowContinueError(state,
                        " The CBVAV system flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, " Occurs in Changeover-bypass VAV system = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).MaxHeatAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow && CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow != 0.0) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "no heating or cooling is needed ({:.7T}).",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            CBVAV(CBVAVNum).FanName,
                                            CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow));
                    ShowContinueError(state, " The CBVAV system flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state, " Occurs in Changeover-bypass VAV system = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
                //     Check fan versus outdoor air flow rates
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).CoolOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "cooling is required ({:.7T}).",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            CBVAV(CBVAVNum).FanName,
                                            CBVAV(CBVAVNum).CoolOutAirVolFlow));
                    ShowContinueError(state,
                        " The CBVAV outdoor flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, " Occurs in Changeover-bypass VAV system = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).CoolOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).HeatOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "heating is required ({:.7T}).",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            CBVAV(CBVAVNum).FanName,
                                            CBVAV(CBVAVNum).HeatOutAirVolFlow));
                    ShowContinueError(state,
                        " The CBVAV outdoor flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, " Occurs in Changeover-bypass VAV system = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).HeatOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "no heating or cooling is needed ({:.7T}).",
                                            CurrentModuleObject,
                                            CBVAV(CBVAVNum).FanVolFlow,
                                            CBVAV(CBVAVNum).FanName,
                                            CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow));
                    ShowContinueError(state, " The CBVAV outdoor flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state, " Occurs in Changeover-bypass VAV system = " + CBVAV(CBVAVNum).Name);
                    CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                }
                int MixerOutsideAirNode = CBVAV(CBVAVNum).MixerOutsideAirNode;
                Real64 RhoAir = DataEnvironment::StdRhoAir;
                // set the mass flow rates from the reset volume flow rates
                CBVAV(CBVAVNum).MaxCoolAirMassFlow = RhoAir * CBVAV(CBVAVNum).MaxCoolAirVolFlow;
                CBVAV(CBVAVNum).CoolOutAirMassFlow = RhoAir * CBVAV(CBVAVNum).CoolOutAirVolFlow;
                CBVAV(CBVAVNum).MaxHeatAirMassFlow = RhoAir * CBVAV(CBVAVNum).MaxHeatAirVolFlow;
                CBVAV(CBVAVNum).HeatOutAirMassFlow = RhoAir * CBVAV(CBVAVNum).HeatOutAirVolFlow;
                CBVAV(CBVAVNum).MaxNoCoolHeatAirMassFlow = RhoAir * CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow;
                CBVAV(CBVAVNum).NoCoolHeatOutAirMassFlow = RhoAir * CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow;
                // set the node max and min mass flow rates based on reset volume flow rates
                DataLoopNode::Node(MixerOutsideAirNode).MassFlowRateMax = max(CBVAV(CBVAVNum).CoolOutAirMassFlow, CBVAV(CBVAVNum).HeatOutAirMassFlow);
                DataLoopNode::Node(MixerOutsideAirNode).MassFlowRateMaxAvail =
                    max(CBVAV(CBVAVNum).CoolOutAirMassFlow, CBVAV(CBVAVNum).HeatOutAirMassFlow);
                DataLoopNode::Node(MixerOutsideAirNode).MassFlowRateMin = 0.0;
                DataLoopNode::Node(MixerOutsideAirNode).MassFlowRateMinAvail = 0.0;
                DataLoopNode::Node(InNode).MassFlowRateMax = max(CBVAV(CBVAVNum).MaxCoolAirMassFlow, CBVAV(CBVAVNum).MaxHeatAirMassFlow);
                DataLoopNode::Node(InNode).MassFlowRateMaxAvail = max(CBVAV(CBVAVNum).MaxCoolAirMassFlow, CBVAV(CBVAVNum).MaxHeatAirMassFlow);
                DataLoopNode::Node(InNode).MassFlowRateMin = 0.0;
                DataLoopNode::Node(InNode).MassFlowRateMinAvail = 0.0;
                DataLoopNode::Node(OutNode).Temp = DataLoopNode::Node(InNode).Temp;
                DataLoopNode::Node(OutNode).HumRat = DataLoopNode::Node(InNode).HumRat;
                DataLoopNode::Node(OutNode).Enthalpy = DataLoopNode::Node(InNode).Enthalpy;
                DataLoopNode::Node(CBVAV(CBVAVNum).MixerReliefAirNode) = DataLoopNode::Node(MixerOutsideAirNode);
                CBVAV(CBVAVNum).CheckFanFlow = false;
                if (CBVAV(CBVAVNum).FanVolFlow > 0.0) {
                    CBVAV(CBVAVNum).HeatingSpeedRatio = CBVAV(CBVAVNum).MaxHeatAirVolFlow / CBVAV(CBVAVNum).FanVolFlow;
                    CBVAV(CBVAVNum).CoolingSpeedRatio = CBVAV(CBVAVNum).MaxCoolAirVolFlow / CBVAV(CBVAVNum).FanVolFlow;
                    CBVAV(CBVAVNum).NoHeatCoolSpeedRatio = CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow / CBVAV(CBVAVNum).FanVolFlow;
                }
            }
        }

        if (CBVAV(CBVAVNum).FanOpModeSchedPtr > 0) {
            if (ScheduleManager::GetCurrentScheduleValue(state, CBVAV(CBVAVNum).FanOpModeSchedPtr) == 0.0) {
                CBVAV(CBVAVNum).OpMode = DataHVACGlobals::CycFanCycCoil;
            } else {
                CBVAV(CBVAVNum).OpMode = DataHVACGlobals::ContFanCycCoil;
            }
        }

        // Returns load only for zones requesting cooling (heating). If in deadband, Qzoneload = 0.
        if (FirstHVACIteration) CBVAV(CBVAVNum).modeChanged = false;
        GetZoneLoads(state, CBVAVNum);

        if (CBVAV(CBVAVNum).OutAirSchPtr > 0) {
            OutsideAirMultiplier = ScheduleManager::GetCurrentScheduleValue(state, CBVAV(CBVAVNum).OutAirSchPtr);
        } else {
            OutsideAirMultiplier = 1.0;
        }

        // Set the inlet node mass flow rate
        if (CBVAV(CBVAVNum).OpMode == DataHVACGlobals::ContFanCycCoil) {
            // constant fan mode
            if (CBVAV(CBVAVNum).HeatCoolMode == HeatingMode) {
                CompOnMassFlow = CBVAV(CBVAVNum).MaxHeatAirMassFlow;
                CompOnFlowRatio = CBVAV(CBVAVNum).HeatingSpeedRatio;
                OACompOnMassFlow = CBVAV(CBVAVNum).HeatOutAirMassFlow * OutsideAirMultiplier;
            } else if (CBVAV(CBVAVNum).HeatCoolMode == CoolingMode) {
                CompOnMassFlow = CBVAV(CBVAVNum).MaxCoolAirMassFlow;
                CompOnFlowRatio = CBVAV(CBVAVNum).CoolingSpeedRatio;
                OACompOnMassFlow = CBVAV(CBVAVNum).CoolOutAirMassFlow * OutsideAirMultiplier;
            } else {
                CompOnMassFlow = CBVAV(CBVAVNum).MaxNoCoolHeatAirMassFlow;
                CompOnFlowRatio = CBVAV(CBVAVNum).NoHeatCoolSpeedRatio;
                OACompOnMassFlow = CBVAV(CBVAVNum).NoCoolHeatOutAirMassFlow * OutsideAirMultiplier;
            }

            if (CBVAV(CBVAVNum).AirFlowControl == UseCompressorOnFlow) {
                if (CBVAV(CBVAVNum).LastMode == HeatingMode) {
                    CompOffMassFlow = CBVAV(CBVAVNum).MaxHeatAirMassFlow;
                    CompOffFlowRatio = CBVAV(CBVAVNum).HeatingSpeedRatio;
                    OACompOffMassFlow = CBVAV(CBVAVNum).HeatOutAirMassFlow * OutsideAirMultiplier;
                } else {
                    CompOffMassFlow = CBVAV(CBVAVNum).MaxCoolAirMassFlow;
                    CompOffFlowRatio = CBVAV(CBVAVNum).CoolingSpeedRatio;
                    OACompOffMassFlow = CBVAV(CBVAVNum).CoolOutAirMassFlow * OutsideAirMultiplier;
                }
            } else {
                CompOffMassFlow = CBVAV(CBVAVNum).MaxNoCoolHeatAirMassFlow;
                CompOffFlowRatio = CBVAV(CBVAVNum).NoHeatCoolSpeedRatio;
                OACompOffMassFlow = CBVAV(CBVAVNum).NoCoolHeatOutAirMassFlow * OutsideAirMultiplier;
            }
        } else {
            // cycling fan mode
            if (CBVAV(CBVAVNum).HeatCoolMode == HeatingMode) {
                CompOnMassFlow = CBVAV(CBVAVNum).MaxHeatAirMassFlow;
                CompOnFlowRatio = CBVAV(CBVAVNum).HeatingSpeedRatio;
                OACompOnMassFlow = CBVAV(CBVAVNum).HeatOutAirMassFlow * OutsideAirMultiplier;
            } else if (CBVAV(CBVAVNum).HeatCoolMode == CoolingMode) {
                CompOnMassFlow = CBVAV(CBVAVNum).MaxCoolAirMassFlow;
                CompOnFlowRatio = CBVAV(CBVAVNum).CoolingSpeedRatio;
                OACompOnMassFlow = CBVAV(CBVAVNum).CoolOutAirMassFlow * OutsideAirMultiplier;
            } else {
                CompOnMassFlow = CBVAV(CBVAVNum).MaxCoolAirMassFlow;
                CompOnFlowRatio = CBVAV(CBVAVNum).CoolingSpeedRatio;
                OACompOnMassFlow = CBVAV(CBVAVNum).CoolOutAirMassFlow * OutsideAirMultiplier;
            }
            CompOffMassFlow = 0.0;
            CompOffFlowRatio = 0.0;
            OACompOffMassFlow = 0.0;
        }

        // Check for correct control node at outlet of unit
        if (CBVAV(CBVAVNum).HumRatMaxCheck) {
            if (CBVAV(CBVAVNum).DehumidControlType > 0) {
                if (DataLoopNode::Node(OutNode).HumRatMax == DataLoopNode::SensedNodeFlagValue) {
                    if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        ShowWarningError(state, "Unitary System:VAV:ChangeOverBypass = " + CBVAV(CBVAVNum).Name);
                        ShowContinueError(state, "Use SetpointManager:SingleZone:Humidity:Maximum to place a humidity setpoint at the air outlet node of "
                                          "the unitary system.");
                        ShowContinueError(state, "Setting Dehumidification Control Type to None and simulation continues.");
                        CBVAV(CBVAVNum).DehumidControlType = 0;
                    } else {
                        // need call to EMS to check node
                        EMSSetPointCheck = false;
                        EMSManager::CheckIfNodeSetPointManagedByEMS(state, OutNode, EMSManager::iHumidityRatioMaxSetPoint, EMSSetPointCheck);
                        bool foundControl = DataLoopNode::NodeSetpointCheck(OutNode).needsSetpointChecking = false;
                        if (EMSSetPointCheck) {
                            // There is no plugin anyways, so we now we have a bad condition.
                            ShowWarningError(state, "Unitary System:VAV:ChangeOverBypass = " + CBVAV(CBVAVNum).Name);
                            ShowContinueError(state, "Use SetpointManager:SingleZone:Humidity:Maximum to place a humidity setpoint at the air outlet node "
                                              "of the unitary system.");
                            ShowContinueError(state,
                                "Or use an EMS Actuator to place a maximum humidity setpoint at the air outlet node of the unitary system.");
                            ShowContinueError(state, "Setting Dehumidification Control Type to None and simulation continues.");
                            CBVAV(CBVAVNum).DehumidControlType = 0;
                        } else if (!foundControl) {
                            // Couldn't find a control, but no error? There are some plugins, so adjust warning message accordingly
                            ShowWarningError(state, "Unitary System:VAV:ChangeOverBypass = " + CBVAV(CBVAVNum).Name);
                            ShowContinueError(state, "Use SetpointManager:SingleZone:Humidity:Maximum to place a humidity setpoint at the air outlet node "
                                              "of the unitary system.");
                            ShowContinueError(state,
                                "Or make sure you do actuate the value via an EMS Actuator or Python Actuator to place a maximum humidity setpoint");
                            ShowContinueError(state,
                                "at the air outlet node of the unitary system, or it will behave as if Dehumidification Control Type = None.");
                            // We do not change the DehumidControlType. There might be a PythonPlugin that will later actuate the HumRatMax
                            // All checks later are in the following form, and HumRatMax will be > 0 only if actually actuated (otherwise it's
                            // SensedNodeFlagValue = -999), so it's fine:
                            // `CBVAV(CBVAVNum).DehumidControlType == DehumidControl_Multimode) && DataLoopNode::Node(OutletNode).HumRatMax > 0.0)`
                        }
                    }
                }
                CBVAV(CBVAVNum).HumRatMaxCheck = false;
            } else {
                CBVAV(CBVAVNum).HumRatMaxCheck = false;
            }
        }

        // Set the inlet node mass flow rate
        if (ScheduleManager::GetCurrentScheduleValue(state, CBVAV(CBVAVNum).SchedPtr) > 0.0 && CompOnMassFlow != 0.0) {
            OnOffAirFlowRatio = 1.0;
            if (FirstHVACIteration) {
                DataLoopNode::Node(CBVAV(CBVAVNum).AirInNode).MassFlowRate = CompOnMassFlow;
                DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).MassFlowRate = CompOnMassFlow;
                DataLoopNode::Node(CBVAV(CBVAVNum).MixerOutsideAirNode).MassFlowRate = OACompOnMassFlow;
                DataLoopNode::Node(CBVAV(CBVAVNum).MixerReliefAirNode).MassFlowRate = OACompOnMassFlow;
                BypassDuctFlowFraction = 0.0;
                PartLoadFrac = 0.0;
            } else {
                if (CBVAV(CBVAVNum).HeatCoolMode != 0) {
                    PartLoadFrac = 1.0;
                } else {
                    PartLoadFrac = 0.0;
                }
                if (CBVAV(CBVAVNum).OpMode == DataHVACGlobals::CycFanCycCoil) {
                    BypassDuctFlowFraction = 0.0;
                } else {
                    if (CBVAV(CBVAVNum).PlenumMixerInletAirNode == 0) {
                        BypassDuctFlowFraction = max(0.0, 1.0 - (DataLoopNode::Node(CBVAV(CBVAVNum).AirInNode).MassFlowRate / CompOnMassFlow));
                    }
                }
            }
        } else {
            PartLoadFrac = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).AirInNode).MassFlowRate = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).AirOutNode).MassFlowRate = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).AirOutNode).MassFlowRateMaxAvail = 0.0;

            DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).MassFlowRate = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerOutsideAirNode).MassFlowRate = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerReliefAirNode).MassFlowRate = 0.0;

            OnOffAirFlowRatio = 1.0;
            BypassDuctFlowFraction = 0.0;
        }

        CalcCBVAV(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, QSensUnitOut, OnOffAirFlowRatio, HXUnitOn);

        // If unit is scheduled OFF, setpoint is equal to inlet node temperature.
        if (ScheduleManager::GetCurrentScheduleValue(state, CBVAV(CBVAVNum).SchedPtr) == 0.0) {
            CBVAV(CBVAVNum).OutletTempSetPoint = DataLoopNode::Node(InNode).Temp;
            return;
        }

        SetAverageAirFlow(state, CBVAVNum, OnOffAirFlowRatio);

        if (FirstHVACIteration) CBVAV(CBVAVNum).OutletTempSetPoint = CalcSetPointTempTarget(CBVAVNum);

        // The setpoint is used to control the DX coils at their respective outlet nodes (not the unit outlet), correct
        // for fan heat for draw thru units only (fan heat is included at the outlet of each coil when blowthru is used)
        CBVAV(CBVAVNum).CoilTempSetPoint = CBVAV(CBVAVNum).OutletTempSetPoint;
        if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::DrawThru) {
            CBVAV(CBVAVNum).CoilTempSetPoint -=
                (DataLoopNode::Node(CBVAV(CBVAVNum).AirOutNode).Temp - DataLoopNode::Node(CBVAV(CBVAVNum).FanInletNodeNum).Temp);
        }

        if (FirstHVACIteration) {
            if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingWater) {
                WaterCoils::SimulateWaterCoilComponents(state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACIteration, CBVAV(CBVAVNum).HeatCoilIndex);

                //     set air-side and steam-side mass flow rates
                DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).MassFlowRate = CompOnMassFlow;
                mdot = CBVAV(CBVAVNum).MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot,
                                                     CBVAV(CBVAVNum).CoilControlNode,
                                                     CBVAV(CBVAVNum).CoilOutletNode,
                                                     CBVAV(CBVAVNum).LoopNum,
                                                     CBVAV(CBVAVNum).LoopSide,
                                                     CBVAV(CBVAVNum).BranchNum,
                                                     CBVAV(CBVAVNum).CompNum);

                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(
                    state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACIteration, CBVAV(CBVAVNum).HeatCoilIndex, QCoilActual);
                CBVAV(CBVAVNum).DesignSuppHeatingCapacity = QCoilActual;

            } // from IF(MSHeatPump(MSHeatPumpNum)%SuppHeatCoilType == DataHVACGlobals::Coil_HeatingWater) THEN

            if (CBVAV(CBVAVNum).HeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam) {

                //     set air-side and steam-side mass flow rates
                DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).MassFlowRate = CompOnMassFlow;
                mdot = CBVAV(CBVAVNum).MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot,
                                                     CBVAV(CBVAVNum).CoilControlNode,
                                                     CBVAV(CBVAVNum).CoilOutletNode,
                                                     CBVAV(CBVAVNum).LoopNum,
                                                     CBVAV(CBVAVNum).LoopSide,
                                                     CBVAV(CBVAVNum).BranchNum,
                                                     CBVAV(CBVAVNum).CompNum);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        CBVAV(CBVAVNum).HeatCoilName,
                                                        FirstHVACIteration,
                                                        CBVAV(CBVAVNum).HeatCoilIndex,
                                                        1.0,
                                                        QCoilActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                CBVAV(CBVAVNum).DesignSuppHeatingCapacity = QCoilActual;

            } // from IF(CBVAV(CBVAVNum)%HeatCoilType_Num == DataHVACGlobals::Coil_HeatingSteam) THEN
        }     // from IF( FirstHVACIteration ) THEN

        if ((CBVAV(CBVAVNum).HeatCoolMode == 0 && CBVAV(CBVAVNum).OpMode == DataHVACGlobals::CycFanCycCoil) || CompOnMassFlow == 0.0) {
            PartLoadFrac = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).AirInNode).MassFlowRate = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).AirOutNode).MassFlowRateMaxAvail = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).MassFlowRate = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerOutsideAirNode).MassFlowRate = 0.0;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerReliefAirNode).MassFlowRate = 0.0;
        }
    }

    void SizeCBVAV(EnergyPlusData &state, int const CBVAVNum) // Index to CBVAV system
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing changeover-bypass VAV components.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays.

        int curSysNum = DataSizing::CurSysNum;
        int curOASysNum = DataSizing::CurOASysNum;

        if (curSysNum > 0 && curOASysNum == 0) {
            if (CBVAV(CBVAVNum).FanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanVecIndex = CBVAV(CBVAVNum).FanIndex;
                state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanModelTypeEnum = DataAirSystems::objectVectorOOFanSystemModel;
            } else {
                state.dataAirSystemsData->PrimaryAirSystems(curSysNum).SupFanNum = CBVAV(CBVAVNum).FanIndex;
                state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanModelTypeEnum = DataAirSystems::structArrayLegacyFanModels;
            }
            if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::BlowThru) {
                state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanLocation = DataAirSystems::fanPlacement::BlowThru;
            } else if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::DrawThru) {
                state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanLocation = DataAirSystems::fanPlacement::DrawThru;
            }
        }

        if (CBVAV(CBVAVNum).MaxCoolAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).MaxCoolAirVolFlow = DataSizing::FinalSysSizing(curSysNum).DesMainVolFlow;
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxCoolAirVolFlow && CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                    CBVAV(CBVAVNum).MaxCoolAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                    ShowWarningError(state, CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                    ShowContinueError(state, "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "in cooling mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                        "The maximum air flow rate in cooling mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (CBVAV(CBVAVNum).MaxCoolAirVolFlow < DataHVACGlobals::SmallAirVolFlow) {
                    CBVAV(CBVAVNum).MaxCoolAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state,
                    CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name, "maximum cooling air flow rate [m3/s]", CBVAV(CBVAVNum).MaxCoolAirVolFlow);
            }
        }

        if (CBVAV(CBVAVNum).MaxHeatAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).MaxHeatAirVolFlow = DataSizing::FinalSysSizing(curSysNum).DesMainVolFlow;
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxHeatAirVolFlow && CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                    CBVAV(CBVAVNum).MaxHeatAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                    ShowWarningError(state, CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                    ShowContinueError(state, "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "in heating mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                        "The maximum air flow rate in heating mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (CBVAV(CBVAVNum).MaxHeatAirVolFlow < DataHVACGlobals::SmallAirVolFlow) {
                    CBVAV(CBVAVNum).MaxHeatAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state,
                    CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name, "maximum heating air flow rate [m3/s]", CBVAV(CBVAVNum).MaxHeatAirVolFlow);
            }
        }

        if (CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow = DataSizing::FinalSysSizing(curSysNum).DesMainVolFlow;
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow && CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                    CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                    ShowWarningError(state, CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                    ShowContinueError(state, "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "when no heating or cooling is needed. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state, "The maximum air flow rate when no heating or cooling is needed is reset to the supply air fan flow rate and "
                                      "the simulation continues.");
                }
                if (CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow < DataHVACGlobals::SmallAirVolFlow) {
                    CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow = 0.0;
                }

                BaseSizer::reportSizerOutput(state, CBVAV(CBVAVNum).UnitType,
                                             CBVAV(CBVAVNum).Name,
                                             "maximum air flow rate when compressor/coil is off [m3/s]",
                                             CBVAV(CBVAVNum).MaxNoCoolHeatAirVolFlow);
            }
        }

        if (CBVAV(CBVAVNum).CoolOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).CoolOutAirVolFlow = DataSizing::FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).CoolOutAirVolFlow && CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                    CBVAV(CBVAVNum).CoolOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                    ShowWarningError(state, CBVAV(CBVAVNum).UnitType + " \"" + CBVAV(CBVAVNum).Name + "\"");
                    ShowContinueError(state, "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "in cooling mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                        "The outdoor air flow rate in cooling mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (CBVAV(CBVAVNum).CoolOutAirVolFlow < DataHVACGlobals::SmallAirVolFlow) {
                    CBVAV(CBVAVNum).CoolOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state, CBVAV(CBVAVNum).UnitType,
                                             CBVAV(CBVAVNum).Name,
                                             "maximum outside air flow rate in cooling [m3/s]",
                                             CBVAV(CBVAVNum).CoolOutAirVolFlow);
            }
        }

        if (CBVAV(CBVAVNum).HeatOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).HeatOutAirVolFlow = DataSizing::FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).HeatOutAirVolFlow && CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                    CBVAV(CBVAVNum).HeatOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                    ShowContinueError(state, "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "in heating mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                        "The outdoor air flow rate in heating mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (CBVAV(CBVAVNum).HeatOutAirVolFlow < DataHVACGlobals::SmallAirVolFlow) {
                    CBVAV(CBVAVNum).HeatOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state, CBVAV(CBVAVNum).UnitType,
                                             CBVAV(CBVAVNum).Name,
                                             "maximum outdoor air flow rate in heating [m3/s]",
                                             CBVAV(CBVAVNum).CoolOutAirVolFlow);
            }
        }

        if (CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, CBVAV(CBVAVNum).UnitType, CBVAV(CBVAVNum).Name);
                CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow = DataSizing::FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (CBVAV(CBVAVNum).FanVolFlow < CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow && CBVAV(CBVAVNum).FanVolFlow != DataSizing::AutoSize) {
                    CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow = CBVAV(CBVAVNum).FanVolFlow;
                    ShowContinueError(state, "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "when no heating or cooling is needed. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state, "The outdoor air flow rate when no heating or cooling is needed is reset to the supply air fan flow rate and "
                                      "the simulation continues.");
                }
                if (CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow < DataHVACGlobals::SmallAirVolFlow) {
                    CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state, CBVAV(CBVAVNum).UnitType,
                                             CBVAV(CBVAVNum).Name,
                                             "maximum outdoor air flow rate when compressor is off [m3/s]",
                                             CBVAV(CBVAVNum).NoCoolHeatOutAirVolFlow);
            }
        }
    }

    void ControlCBVAVOutput(EnergyPlusData &state,
                            int const CBVAVNum,            // Index to CBVAV system
                            bool const FirstHVACIteration, // Flag for 1st HVAC iteration
                            Real64 &PartLoadFrac,          // Unit part load fraction
                            Real64 &OnOffAirFlowRatio,     // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                            bool &HXUnitOn                 // flag to enable heat exchanger
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the part load fraction of the CBVAV system for this time step.

        // METHODOLOGY EMPLOYED:
        // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FullOutput = 0; // Unit full output when compressor is operating [W]
        PartLoadFrac = 0.0;

        if (ScheduleManager::GetCurrentScheduleValue(state, CBVAV(CBVAVNum).SchedPtr) == 0.0) return;

        // Get operating result
        PartLoadFrac = 1.0;
        CalcCBVAV(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, FullOutput, OnOffAirFlowRatio, HXUnitOn);

        if ((DataLoopNode::Node(CBVAV(CBVAVNum).AirOutNode).Temp - CBVAV(CBVAVNum).OutletTempSetPoint) > DataHVACGlobals::SmallTempDiff &&
            CBVAV(CBVAVNum).HeatCoolMode > 0 && PartLoadFrac < 1.0) {
            CalcCBVAV(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, FullOutput, OnOffAirFlowRatio, HXUnitOn);
        }
    }

    void CalcCBVAV(EnergyPlusData &state,
                   int const CBVAVNum,            // Unit index in fan coil array
                   bool const FirstHVACIteration, // Flag for 1st HVAC iteration
                   Real64 &PartLoadFrac,          // Compressor part load fraction
                   Real64 &LoadMet,               // Load met by unit (W)
                   Real64 &OnOffAirFlowRatio,     // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                   bool const HXUnitOn            // flag to enable heat exchanger
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate the components making up the changeover-bypass VAV system.

        // METHODOLOGY EMPLOYED:
        // Simulates the unit components sequentially in the air flow direction.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int const MaxIte(500); // Maximum number of iterations

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 MinHumRat;       // Minimum humidity ratio for sensible capacity calculation (kg/kg)
        Array1D<Real64> Par(6); // RegulaFalsi parameters
        int SolFla;             // Flag of RegulaFalsi solver
        Real64 QHeater;         // Load to be met by heater [W]
        Real64 QHeaterActual;   // actual heating load met [W]
        Real64 CpAir;           // Specific heat of air [J/kg-K]
        int DehumidMode;        // Dehumidification mode (0=normal, 1=enhanced)
        Real64 ApproachTemp;
        Real64 DesiredDewPoint;
        Real64 OutdoorDryBulbTemp; // Dry-bulb temperature at outdoor condenser
        Real64 OutdoorBaroPress;   // Barometric pressure at outdoor condenser
        // FLOW

        int OutletNode = CBVAV(CBVAVNum).AirOutNode;
        int InletNode = CBVAV(CBVAVNum).AirInNode;
        if (CBVAV(CBVAVNum).CondenserNodeNum > 0) {
            OutdoorDryBulbTemp = DataLoopNode::Node(CBVAV(CBVAVNum).CondenserNodeNum).Temp;
            OutdoorBaroPress = DataLoopNode::Node(CBVAV(CBVAVNum).CondenserNodeNum).Press;
        } else {
            OutdoorDryBulbTemp = DataEnvironment::OutDryBulbTemp;
            OutdoorBaroPress = DataEnvironment::OutBaroPress;
        }

        SaveCompressorPLR = 0.0;

        // Bypass excess system air through bypass duct and calculate new mixed air conditions at OA mixer inlet node
        if (CBVAV(CBVAVNum).plenumIndex > 0 || CBVAV(CBVAVNum).mixerIndex > 0) {
            Real64 saveMixerInletAirNodeFlow = DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).MassFlowRate;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode) = DataLoopNode::Node(InletNode);
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).MassFlowRate = saveMixerInletAirNodeFlow;
        } else {
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).Temp =
                (1.0 - BypassDuctFlowFraction) * DataLoopNode::Node(InletNode).Temp + BypassDuctFlowFraction * DataLoopNode::Node(OutletNode).Temp;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).HumRat = (1.0 - BypassDuctFlowFraction) * DataLoopNode::Node(InletNode).HumRat +
                                                                           BypassDuctFlowFraction * DataLoopNode::Node(OutletNode).HumRat;
            DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
                DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).Temp, DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).HumRat);
        }
        MixedAir::SimOAMixer(state, CBVAV(CBVAVNum).OAMixName, FirstHVACIteration, CBVAV(CBVAVNum).OAMixIndex);

        if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::BlowThru) {
            if (CBVAV(CBVAVNum).FanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                HVACFan::fanObjs[CBVAV(CBVAVNum).FanIndex]->simulate(state, 1.0 / OnOffAirFlowRatio, _, _, _);
            } else {
                Fans::SimulateFanComponents(state, CBVAV(CBVAVNum).FanName, FirstHVACIteration, CBVAV(CBVAVNum).FanIndex, FanSpeedRatio);
            }
        }
        // Simulate cooling coil if zone load is negative (cooling load)
        if (CBVAV(CBVAVNum).HeatCoolMode == CoolingMode) {
            if (OutdoorDryBulbTemp >= CBVAV(CBVAVNum).MinOATCompressor) {

                {
                    auto const SELECT_CASE_var(CBVAV(CBVAVNum).DXCoolCoilType_Num);

                    if (SELECT_CASE_var == DataHVACGlobals::CoilDX_CoolingHXAssisted) {
                        HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                            CBVAV(CBVAVNum).DXCoolCoilName,
                                                                            FirstHVACIteration,
                                                                            On,
                                                                            PartLoadFrac,
                                                                            CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                            DataHVACGlobals::ContFanCycCoil,
                                                                            HXUnitOn);
                        if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp <= CBVAV(CBVAVNum).CoilTempSetPoint) {
                            //         If coil inlet temp is already below the setpoint, simulated with coil off
                            PartLoadFrac = 0.0;
                            HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                                CBVAV(CBVAVNum).DXCoolCoilName,
                                                                                FirstHVACIteration,
                                                                                Off,
                                                                                PartLoadFrac,
                                                                                CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                                DataHVACGlobals::ContFanCycCoil,
                                                                                HXUnitOn);
                        } else if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp < CBVAV(CBVAVNum).CoilTempSetPoint) {
                            Par(1) = double(CBVAV(CBVAVNum).CoolCoilCompIndex);
                            Par(2) = CBVAV(CBVAVNum).CoilTempSetPoint;
                            Par(3) = OnOffAirFlowRatio;
                            Par(4) = double(CBVAVNum);
                            if (FirstHVACIteration) {
                                Par(5) = 1.0;
                            } else {
                                Par(5) = 0.0;
                            }
                            if (HXUnitOn) {
                                Par(6) = 1.0;
                            } else {
                                Par(6) = 0.0;
                            }
                            TempSolveRoot::SolveRoot(
                                state, DataHVACGlobals::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, HXAssistDXCoilResidual, 0.0, 1.0, Par);
                            HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                                CBVAV(CBVAVNum).DXCoolCoilName,
                                                                                FirstHVACIteration,
                                                                                On,
                                                                                PartLoadFrac,
                                                                                CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                                DataHVACGlobals::ContFanCycCoil,
                                                                                HXUnitOn);
                            if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                                if (CBVAV(CBVAVNum).HXDXIterationExceeded < 1) {
                                    ++CBVAV(CBVAVNum).HXDXIterationExceeded;
                                    ShowWarningError(state, "Iteration limit exceeded calculating HX assisted DX unit part-load ratio, for unit = " +
                                                     CBVAV(CBVAVNum).DXCoolCoilName);
                                    ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(state,
                                        "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state,
                                        CBVAV(CBVAVNum).Name + ", Iteration limit exceeded for HX assisted DX unit part-load ratio error continues.",
                                        CBVAV(CBVAVNum).HXDXIterationExceededIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                                PartLoadFrac = max(0.0,
                                                   min(1.0,
                                                       (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp - CBVAV(CBVAVNum).CoilTempSetPoint) /
                                                           (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp -
                                                            DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp)));
                                if (CBVAV(CBVAVNum).HXDXIterationFailed < 1) {
                                    ++CBVAV(CBVAVNum).HXDXIterationFailed;
                                    ShowSevereError(state,
                                        "HX assisted DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = " +
                                        CBVAV(CBVAVNum).DXCoolCoilName);
                                    ShowContinueErrorTimeStamp(
                                        state,
                                        format("An estimated part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:",
                                               PartLoadFrac));
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state,
                                        CBVAV(CBVAVNum).Name + ", Part-load ratio calculation failed for HX assisted DX unit error continues.",
                                        CBVAV(CBVAVNum).HXDXIterationFailedIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            }
                        }
                    } else if (SELECT_CASE_var == DataHVACGlobals::CoilDX_CoolingSingleSpeed) {
                        DXCoils::SimDXCoil(state,
                                           CBVAV(CBVAVNum).DXCoolCoilName,
                                           On,
                                           FirstHVACIteration,
                                           CBVAV(CBVAVNum).CoolCoilCompIndex,
                                           DataHVACGlobals::ContFanCycCoil,
                                           PartLoadFrac,
                                           OnOffAirFlowRatio);
                        if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp <= CBVAV(CBVAVNum).CoilTempSetPoint) {
                            //         If coil inlet temp is already below the setpoint, simulated with coil off
                            PartLoadFrac = 0.0;
                            DXCoils::SimDXCoil(state,
                                               CBVAV(CBVAVNum).DXCoolCoilName,
                                               On,
                                               FirstHVACIteration,
                                               CBVAV(CBVAVNum).CoolCoilCompIndex,
                                               DataHVACGlobals::ContFanCycCoil,
                                               PartLoadFrac,
                                               OnOffAirFlowRatio);
                        } else if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp < CBVAV(CBVAVNum).CoilTempSetPoint) {
                            Par(1) = double(CBVAV(CBVAVNum).CoolCoilCompIndex);
                            Par(2) = CBVAV(CBVAVNum).CoilTempSetPoint;
                            Par(3) = OnOffAirFlowRatio;
                            TempSolveRoot::SolveRoot(state, DataHVACGlobals::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, DOE2DXCoilResidual, 0.0, 1.0, Par);
                            DXCoils::SimDXCoil(state,
                                               CBVAV(CBVAVNum).DXCoolCoilName,
                                               On,
                                               FirstHVACIteration,
                                               CBVAV(CBVAVNum).CoolCoilCompIndex,
                                               DataHVACGlobals::ContFanCycCoil,
                                               PartLoadFrac,
                                               OnOffAirFlowRatio);
                            if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                                if (CBVAV(CBVAVNum).DXIterationExceeded < 1) {
                                    ++CBVAV(CBVAVNum).DXIterationExceeded;
                                    ShowWarningError(state, "Iteration limit exceeded calculating DX unit part-load ratio, for unit = " +
                                                     CBVAV(CBVAVNum).DXCoolCoilName);
                                    ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(state,
                                        "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state,
                                        CBVAV(CBVAVNum).Name + ", Iteration limit exceeded for DX unit part-load ratio calculation error continues.",
                                        CBVAV(CBVAVNum).DXIterationExceededIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                                PartLoadFrac = max(0.0,
                                                   min(1.0,
                                                       (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp - CBVAV(CBVAVNum).CoilTempSetPoint) /
                                                           (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp -
                                                            DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp)));
                                if (CBVAV(CBVAVNum).DXIterationFailed < 1) {
                                    ++CBVAV(CBVAVNum).DXIterationFailed;
                                    ShowSevereError(state, "DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = " +
                                                    CBVAV(CBVAVNum).DXCoolCoilName);
                                    ShowContinueErrorTimeStamp(
                                        state,
                                        format("An estimated part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:",
                                               PartLoadFrac));
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state, CBVAV(CBVAVNum).Name +
                                                                       ", Part-load ratio calculation failed for DX unit error continues.",
                                                                   CBVAV(CBVAVNum).DXIterationFailedIndex,
                                                                   PartLoadFrac,
                                                                   PartLoadFrac);
                                }
                            }
                        }
                        SaveCompressorPLR = DXCoils::DXCoilPartLoadRatio(CBVAV(CBVAVNum).DXCoolCoilIndexNum);
                    } else if (SELECT_CASE_var == DataHVACGlobals::Coil_CoolingAirToAirVariableSpeed) {
                        Real64 QZnReq(0.0);                // Zone load (W), input to variable-speed DX coil
                        Real64 QLatReq(0.0);               // Zone latent load, input to variable-speed DX coil
                        Real64 MaxONOFFCyclesperHour(4.0); // Maximum cycling rate of heat pump [cycles/hr]
                        Real64 HPTimeConstant(0.0);        // Heat pump time constant [s]
                        Real64 FanDelayTime(0.0);          // Fan delay time, time delay for the HP's fan to
                        Real64 OnOffAirFlowRatio(1.0);     // ratio of compressor on flow to average flow over time step
                        Real64 PartLoadFrac(0.0);
                        Real64 SpeedRatio(0.0);
                        int SpeedNum(1);
                        bool errorFlag(false);
                        int maxNumSpeeds = VariableSpeedCoils::GetVSCoilNumOfSpeeds(state, CBVAV(CBVAVNum).DXCoolCoilName, errorFlag);
                        Real64 DesOutTemp = CBVAV(CBVAVNum).CoilTempSetPoint;
                        // Get no load result
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  CBVAV(CBVAVNum).DXCoolCoilName,
                                                                  CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                  DataHVACGlobals::ContFanCycCoil,
                                                                  MaxONOFFCyclesperHour,
                                                                  HPTimeConstant,
                                                                  FanDelayTime,
                                                                  Off,
                                                                  PartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq);

                        Real64 NoOutput = DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).MassFlowRate *
                                          (Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp,
                                                                      DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat) -
                                           Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp,
                                                                      DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat));

                        // Get full load result
                        PartLoadFrac = 1.0;
                        SpeedNum = maxNumSpeeds;
                        SpeedRatio = 1.0;
                        QZnReq = 0.001; // to indicate the coil is running
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  CBVAV(CBVAVNum).DXCoolCoilName,
                                                                  CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                  DataHVACGlobals::ContFanCycCoil,
                                                                  MaxONOFFCyclesperHour,
                                                                  HPTimeConstant,
                                                                  FanDelayTime,
                                                                  On,
                                                                  PartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq);

                        Real64 FullOutput = DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).MassFlowRate *
                                            (Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp,
                                                                        DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat) -
                                             Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp,
                                                                        DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat));
                        Real64 ReqOutput = DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).MassFlowRate *
                                           (Psychrometrics::PsyHFnTdbW(DesOutTemp, DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat) -
                                            Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp,
                                                                       DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat));

                        Real64 loadAccuracy(0.001);                  // Watts, power
                        Real64 tempAccuracy(0.001);                  // delta C, temperature
                        if ((NoOutput - ReqOutput) < loadAccuracy) { //         IF NoOutput is lower than (more cooling than required) or very near
                                                                     //         the ReqOutput, do not run the compressor
                            PartLoadFrac = 0.0;
                            SpeedNum = 1;
                            SpeedRatio = 0.0;
                            QZnReq = 0.0;
                            // Get no load result
                            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                      CBVAV(CBVAVNum).DXCoolCoilName,
                                                                      CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                      DataHVACGlobals::ContFanCycCoil,
                                                                      MaxONOFFCyclesperHour,
                                                                      HPTimeConstant,
                                                                      FanDelayTime,
                                                                      Off,
                                                                      PartLoadFrac,
                                                                      SpeedNum,
                                                                      SpeedRatio,
                                                                      QZnReq,
                                                                      QLatReq);

                        } else if ((FullOutput - ReqOutput) > loadAccuracy) {
                            //         If the FullOutput is greater than (insufficient cooling) or very near the ReqOutput,
                            //         run the compressor at PartLoadFrac = 1.
                            PartLoadFrac = 1.0;
                            SpeedNum = maxNumSpeeds;
                            SpeedRatio = 1.0;
                            //         Else find the PLR to meet the load
                        } else {
                            //           OutletTempDXCoil is the full capacity outlet temperature at PartLoadFrac = 1 from the CALL above. If this
                            //           temp is greater than the desired outlet temp, then run the compressor at PartLoadFrac = 1, otherwise find the
                            //           operating PLR.
                            Real64 OutletTempDXCoil = state.dataVariableSpeedCoils->VarSpeedCoil(CBVAV(CBVAVNum).CoolCoilCompIndex).OutletAirDBTemp;
                            if (OutletTempDXCoil > DesOutTemp) {
                                PartLoadFrac = 1.0;
                                SpeedNum = maxNumSpeeds;
                                SpeedRatio = 1.0;
                            } else {
                                // run at lowest speed
                                PartLoadFrac = 1.0;
                                SpeedNum = 1;
                                SpeedRatio = 1.0;
                                QZnReq = 0.001; // to indicate the coil is running
                                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                          CBVAV(CBVAVNum).DXCoolCoilName,
                                                                          CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                          DataHVACGlobals::ContFanCycCoil,
                                                                          MaxONOFFCyclesperHour,
                                                                          HPTimeConstant,
                                                                          FanDelayTime,
                                                                          On,
                                                                          PartLoadFrac,
                                                                          SpeedNum,
                                                                          SpeedRatio,
                                                                          QZnReq,
                                                                          QLatReq,
                                                                          OnOffAirFlowRatio);

                                Real64 TempSpeedOut = DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).MassFlowRate *
                                                      (Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp,
                                                                                  DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat) -
                                                       Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp,
                                                                                  DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat));
                                Real64 TempSpeedReqst =
                                    DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).MassFlowRate *
                                    (Psychrometrics::PsyHFnTdbW(DesOutTemp, DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat) -
                                     Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp,
                                                                DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat));

                                if ((TempSpeedOut - TempSpeedReqst) > tempAccuracy) {
                                    // Check to see which speed to meet the load
                                    PartLoadFrac = 1.0;
                                    SpeedRatio = 1.0;
                                    for (int I = 2; I <= maxNumSpeeds; ++I) {
                                        SpeedNum = I;
                                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                                  CBVAV(CBVAVNum).DXCoolCoilName,
                                                                                  CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                                  DataHVACGlobals::ContFanCycCoil,
                                                                                  MaxONOFFCyclesperHour,
                                                                                  HPTimeConstant,
                                                                                  FanDelayTime,
                                                                                  On,
                                                                                  PartLoadFrac,
                                                                                  SpeedNum,
                                                                                  SpeedRatio,
                                                                                  QZnReq,
                                                                                  QLatReq,
                                                                                  OnOffAirFlowRatio);

                                        TempSpeedOut = DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).MassFlowRate *
                                                       (Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp,
                                                                                   DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat) -
                                                        Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp,
                                                                                   DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat));
                                        TempSpeedReqst =
                                            DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).MassFlowRate *
                                            (Psychrometrics::PsyHFnTdbW(DesOutTemp, DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat) -
                                             Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp,
                                                                        DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat));

                                        if ((TempSpeedOut - TempSpeedReqst) < tempAccuracy) {
                                            SpeedNum = I;
                                            break;
                                        }
                                    }
                                    // now find the speed ratio for the found speednum
                                    Par(1) = double(CBVAV(CBVAVNum).CoolCoilCompIndex);
                                    Par(2) = DesOutTemp;
                                    Par(5) = double(DataHVACGlobals::ContFanCycCoil);
                                    Par(3) = double(SpeedNum);
                                    TempSolveRoot::SolveRoot(
                                        state, tempAccuracy, MaxIte, SolFla, SpeedRatio, HVACDXSystem::VSCoilSpeedResidual, 1.0e-10, 1.0, Par);

                                    if (SolFla == -1) {
                                        if (!state.dataGlobal->WarmupFlag) {
                                            if (CBVAV(CBVAVNum).DXIterationExceeded < 4) {
                                                ++CBVAV(CBVAVNum).DXIterationExceeded;
                                                ShowWarningError(state, CBVAV(CBVAVNum).DXCoolCoilType +
                                                                 " - Iteration limit exceeded calculating VS DX coil speed ratio for coil named " +
                                                                 CBVAV(CBVAVNum).DXCoolCoilName + ", in Unitary system named" + CBVAV(CBVAVNum).Name);
                                                ShowContinueError(state, format("Calculated speed ratio = {:.4R}", SpeedRatio));
                                                ShowContinueErrorTimeStamp(state,
                                                    "The calculated speed ratio will be used and the simulation continues. Occurrence info:");
                                            }
                                            ShowRecurringWarningErrorAtEnd(state, CBVAV(CBVAVNum).DXCoolCoilType + " \"" + CBVAV(CBVAVNum).DXCoolCoilName +
                                                                               "\" - Iteration limit exceeded calculating speed ratio error "
                                                                               "continues. Speed Ratio statistics follow.",
                                                                           CBVAV(CBVAVNum).DXIterationExceededIndex,
                                                                           PartLoadFrac,
                                                                           PartLoadFrac);
                                        }
                                    } else if (SolFla == -2) {
                                        if (!state.dataGlobal->WarmupFlag) {
                                            if (CBVAV(CBVAVNum).DXIterationFailed < 4) {
                                                ++CBVAV(CBVAVNum).DXIterationFailed;
                                                ShowWarningError(state,
                                                    CBVAV(CBVAVNum).DXCoolCoilType +
                                                    " - DX unit speed ratio calculation failed: solver limits exceeded, for coil named " +
                                                    CBVAV(CBVAVNum).DXCoolCoilName + ", in Unitary system named" + CBVAV(CBVAVNum).Name);
                                                ShowContinueError(state, format("Estimated speed ratio = {:.3R}", TempSpeedReqst / TempSpeedOut));
                                                ShowContinueErrorTimeStamp(state,
                                                    "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                            }
                                            ShowRecurringWarningErrorAtEnd(state,
                                                CBVAV(CBVAVNum).DXCoolCoilType + " \"" + CBVAV(CBVAVNum).DXCoolCoilName +
                                                    "\" - DX unit speed ratio calculation failed error continues. speed ratio statistics follow.",
                                                CBVAV(CBVAVNum).DXIterationFailedIndex,
                                                SpeedRatio,
                                                SpeedRatio);
                                        }
                                        SpeedRatio = TempSpeedReqst / TempSpeedOut;
                                    }
                                } else {
                                    // cycling compressor at lowest speed number, find part load fraction
                                    Par(1) = double(CBVAV(CBVAVNum).CoolCoilCompIndex);
                                    Par(2) = DesOutTemp;
                                    Par(5) = double(DataHVACGlobals::ContFanCycCoil);
                                    TempSolveRoot::SolveRoot(
                                        state, tempAccuracy, MaxIte, SolFla, PartLoadFrac, HVACDXSystem::VSCoilCyclingResidual, 1.0e-10, 1.0, Par);
                                    if (SolFla == -1) {
                                        if (!state.dataGlobal->WarmupFlag) {
                                            if (CBVAV(CBVAVNum).DXCyclingIterationExceeded < 4) {
                                                ++CBVAV(CBVAVNum).DXCyclingIterationExceeded;
                                                ShowWarningError(state,
                                                    CBVAV(CBVAVNum).DXCoolCoilType +
                                                    " - Iteration limit exceeded calculating VS DX unit low speed cycling ratio, for coil named " +
                                                    CBVAV(CBVAVNum).DXCoolCoilName + ", in Unitary system named" + CBVAV(CBVAVNum).Name);
                                                ShowContinueError(state,
                                                                  format("Estimated cycling ratio  = {:.3R}", (TempSpeedReqst / TempSpeedOut)));
                                                ShowContinueError(state, format("Calculated cycling ratio = {:.3R}", PartLoadFrac));
                                                ShowContinueErrorTimeStamp(state,
                                                    "The calculated cycling ratio will be used and the simulation continues. Occurrence info:");
                                            }
                                            ShowRecurringWarningErrorAtEnd(state, CBVAV(CBVAVNum).DXCoolCoilType + " \"" + CBVAV(CBVAVNum).DXCoolCoilName +
                                                                               "\" - Iteration limit exceeded calculating low speed cycling ratio "
                                                                               "error continues. Sensible PLR statistics follow.",
                                                                           CBVAV(CBVAVNum).DXCyclingIterationExceededIndex,
                                                                           PartLoadFrac,
                                                                           PartLoadFrac);
                                        }
                                    } else if (SolFla == -2) {

                                        if (!state.dataGlobal->WarmupFlag) {
                                            if (CBVAV(CBVAVNum).DXCyclingIterationFailed < 4) {
                                                ++CBVAV(CBVAVNum).DXCyclingIterationFailed;
                                                ShowWarningError(state,
                                                    CBVAV(CBVAVNum).DXCoolCoilType +
                                                    " - DX unit low speed cycling ratio calculation failed: limits exceeded, for unit = " +
                                                    CBVAV(CBVAVNum).Name);
                                                ShowContinueError(
                                                    state, format("Estimated low speed cycling ratio = {:.3R}", TempSpeedReqst / TempSpeedOut));
                                                ShowContinueErrorTimeStamp(state, "The estimated low speed cycling ratio will be used and the simulation "
                                                                           "continues. Occurrence info:");
                                            }
                                            ShowRecurringWarningErrorAtEnd(state, CBVAV(CBVAVNum).DXCoolCoilType + " \"" + CBVAV(CBVAVNum).DXCoolCoilName +
                                                                               "\" - DX unit low speed cycling ratio calculation failed error "
                                                                               "continues. cycling ratio statistics follow.",
                                                                           CBVAV(CBVAVNum).DXCyclingIterationFailedIndex,
                                                                           PartLoadFrac,
                                                                           PartLoadFrac);
                                        }
                                        PartLoadFrac = TempSpeedReqst / TempSpeedOut;
                                    }
                                }
                            }
                        }

                        if (PartLoadFrac > 1.0) {
                            PartLoadFrac = 1.0;
                        } else if (PartLoadFrac < 0.0) {
                            PartLoadFrac = 0.0;
                        }
                        SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, CBVAV(CBVAVNum).CoolCoilCompIndex);
                        // variable-speed air-to-air cooling coil, end -------------------------

                    } else if (SELECT_CASE_var == DataHVACGlobals::CoilDX_CoolingTwoStageWHumControl) {
                        // Coil:Cooling:DX:TwoStageWithHumidityControlMode
                        // formerly (v3 and beyond) Coil:DX:MultiMode:CoolingEmpirical

                        // If DXCoolingSystem runs with a cooling load then set PartLoadFrac on Cooling System and the Mass Flow
                        // Multimode coil will switch to enhanced dehumidification if available and needed, but it
                        // still runs to meet the sensible load

                        // Determine required part load for normal mode

                        // Get full load result
                        DehumidMode = 0;
                        CBVAV(CBVAVNum).DehumidificationMode = DehumidMode;
                        DXCoils::SimDXCoilMultiMode(state,
                                                    CBVAV(CBVAVNum).DXCoolCoilName,
                                                    On,
                                                    FirstHVACIteration,
                                                    PartLoadFrac,
                                                    DehumidMode,
                                                    CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                    DataHVACGlobals::ContFanCycCoil);
                        if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp <= CBVAV(CBVAVNum).CoilTempSetPoint) {
                            PartLoadFrac = 0.0;
                            DXCoils::SimDXCoilMultiMode(state,
                                                        CBVAV(CBVAVNum).DXCoolCoilName,
                                                        On,
                                                        FirstHVACIteration,
                                                        PartLoadFrac,
                                                        DehumidMode,
                                                        CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                        DataHVACGlobals::ContFanCycCoil);
                        } else if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp > CBVAV(CBVAVNum).CoilTempSetPoint) {
                            PartLoadFrac = 1.0;
                        } else {
                            Par(1) = double(CBVAV(CBVAVNum).CoolCoilCompIndex);
                            Par(2) = CBVAV(CBVAVNum).CoilTempSetPoint;
                            // Dehumidification mode = 0 for normal mode, 1+ for enhanced mode
                            Par(3) = double(DehumidMode);
                            TempSolveRoot::SolveRoot(
                                state, DataHVACGlobals::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, MultiModeDXCoilResidual, 0.0, 1.0, Par);
                            if (SolFla == -1) {
                                if (CBVAV(CBVAVNum).MMDXIterationExceeded < 1) {
                                    ++CBVAV(CBVAVNum).MMDXIterationExceeded;
                                    ShowWarningError(state, "Iteration limit exceeded calculating DX unit part-load ratio, for unit=" +
                                                     CBVAV(CBVAVNum).Name);
                                    ShowContinueErrorTimeStamp(state, format("Part-load ratio returned = {:.2R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(state,
                                        "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state,
                                        CBVAV(CBVAVNum).Name + ", Iteration limit exceeded calculating DX unit part-load ratio error continues.",
                                        CBVAV(CBVAVNum).MMDXIterationExceededIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            } else if (SolFla == -2) {
                                PartLoadFrac = max(0.0,
                                                   min(1.0,
                                                       (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp - CBVAV(CBVAVNum).CoilTempSetPoint) /
                                                           (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp -
                                                            DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp)));
                                if (CBVAV(CBVAVNum).MMDXIterationFailed < 1) {
                                    ++CBVAV(CBVAVNum).MMDXIterationFailed;
                                    ShowSevereError(state, "DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit=" +
                                                    CBVAV(CBVAVNum).Name);
                                    ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(state,
                                        "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state, CBVAV(CBVAVNum).Name +
                                                                       ", Part-load ratio calculation failed for DX unit error continues.",
                                                                   CBVAV(CBVAVNum).MMDXIterationFailedIndex,
                                                                   PartLoadFrac,
                                                                   PartLoadFrac);
                                }
                            }
                        }

                        // If humidity setpoint is not satisfied and humidity control type is Multimode,
                        // then turn on enhanced dehumidification mode 1

                        if ((DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat > DataLoopNode::Node(OutletNode).HumRatMax) &&
                            (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).HumRat > DataLoopNode::Node(OutletNode).HumRatMax) &&
                            (CBVAV(CBVAVNum).DehumidControlType == DehumidControl_Multimode) && DataLoopNode::Node(OutletNode).HumRatMax > 0.0) {

                            // Determine required part load for enhanced dehumidification mode 1

                            // Get full load result
                            PartLoadFrac = 1.0;
                            DehumidMode = 1;
                            CBVAV(CBVAVNum).DehumidificationMode = DehumidMode;
                            DXCoils::SimDXCoilMultiMode(state,
                                                        CBVAV(CBVAVNum).DXCoolCoilName,
                                                        On,
                                                        FirstHVACIteration,
                                                        PartLoadFrac,
                                                        DehumidMode,
                                                        CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                        DataHVACGlobals::ContFanCycCoil);
                            if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp <= CBVAV(CBVAVNum).CoilTempSetPoint) {
                                PartLoadFrac = 0.0;
                            } else if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp > CBVAV(CBVAVNum).CoilTempSetPoint) {
                                PartLoadFrac = 1.0;
                            } else {
                                Par(1) = double(CBVAV(CBVAVNum).CoolCoilCompIndex);
                                Par(2) = CBVAV(CBVAVNum).CoilTempSetPoint;
                                // Dehumidification mode = 0 for normal mode, 1+ for enhanced mode
                                Par(3) = double(DehumidMode);
                                TempSolveRoot::SolveRoot(
                                    state, DataHVACGlobals::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, MultiModeDXCoilResidual, 0.0, 1.0, Par);
                                if (SolFla == -1) {
                                    if (CBVAV(CBVAVNum).DMDXIterationExceeded < 1) {
                                        ++CBVAV(CBVAVNum).DMDXIterationExceeded;
                                        ShowWarningError(state, "Iteration limit exceeded calculating DX unit dehumidifying part-load ratio, for unit = " +
                                                         CBVAV(CBVAVNum).Name);
                                        ShowContinueErrorTimeStamp(state, format("Part-load ratio returned={:.2R}", PartLoadFrac));
                                        ShowContinueErrorTimeStamp(state,
                                            "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                    } else {
                                        ShowRecurringWarningErrorAtEnd(state,
                                            CBVAV(CBVAVNum).Name +
                                                ", Iteration limit exceeded calculating DX unit dehumidifying part-load ratio error continues.",
                                            CBVAV(CBVAVNum).DMDXIterationExceededIndex,
                                            PartLoadFrac,
                                            PartLoadFrac);
                                    }
                                } else if (SolFla == -2) {
                                    PartLoadFrac =
                                        max(0.0,
                                            min(1.0,
                                                (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp - CBVAV(CBVAVNum).CoilTempSetPoint) /
                                                    (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp -
                                                     DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp)));
                                    if (CBVAV(CBVAVNum).DMDXIterationFailed < 1) {
                                        ++CBVAV(CBVAVNum).DMDXIterationFailed;
                                        ShowSevereError(state,
                                            "DX unit dehumidifying part-load ratio calculation failed: part-load ratio limits exceeded, for unit = " +
                                            CBVAV(CBVAVNum).Name);
                                        ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                        ShowContinueErrorTimeStamp(state,
                                            "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                    } else {
                                        ShowRecurringWarningErrorAtEnd(state,
                                            CBVAV(CBVAVNum).Name + ", Dehumidifying part-load ratio calculation failed for DX unit error continues.",
                                            CBVAV(CBVAVNum).DMDXIterationFailedIndex,
                                            PartLoadFrac,
                                            PartLoadFrac);
                                    }
                                }
                            }
                        } // End if humidity ratio setpoint not met - multimode humidity control

                        // If humidity setpoint is not satisfied and humidity control type is CoolReheat,
                        // then run to meet latent load

                        if ((DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).HumRat > DataLoopNode::Node(OutletNode).HumRatMax) &&
                            (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).HumRat > DataLoopNode::Node(OutletNode).HumRatMax) &&
                            (CBVAV(CBVAVNum).DehumidControlType == DehumidControl_CoolReheat) && DataLoopNode::Node(OutletNode).HumRatMax > 0.0) {

                            // Determine revised desired outlet temperature  - use approach temperature control strategy
                            // based on CONTROLLER:SIMPLE TEMPANDHUMRAT control type.

                            // Calculate the approach temperature (difference between SA dry-bulb temp and SA dew point temp)
                            ApproachTemp = DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp -
                                           Psychrometrics::PsyTdpFnWPb(state, DataLoopNode::Node(OutletNode).HumRat, OutdoorBaroPress);
                            // Calculate the dew point temperature at the SA humidity ratio setpoint
                            DesiredDewPoint = Psychrometrics::PsyTdpFnWPb(state, DataLoopNode::Node(OutletNode).HumRatMax, OutdoorBaroPress);
                            // Adjust the calculated dew point temperature by the approach temp
                            CBVAV(CBVAVNum).CoilTempSetPoint = min(CBVAV(CBVAVNum).CoilTempSetPoint, (DesiredDewPoint + ApproachTemp));

                            // Determine required part load for cool reheat at adjusted DesiredOutletTemp

                            // Get full load result
                            PartLoadFrac = 1.0;
                            DehumidMode = 0;
                            CBVAV(CBVAVNum).DehumidificationMode = DehumidMode;
                            DXCoils::SimDXCoilMultiMode(state,
                                                        CBVAV(CBVAVNum).DXCoolCoilName,
                                                        On,
                                                        FirstHVACIteration,
                                                        PartLoadFrac,
                                                        DehumidMode,
                                                        CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                        DataHVACGlobals::ContFanCycCoil);
                            if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp <= CBVAV(CBVAVNum).CoilTempSetPoint) {
                                PartLoadFrac = 0.0;
                            } else if (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp > CBVAV(CBVAVNum).CoilTempSetPoint) {
                                PartLoadFrac = 1.0;
                            } else {
                                Par(1) = double(CBVAV(CBVAVNum).CoolCoilCompIndex);
                                Par(2) = CBVAV(CBVAVNum).CoilTempSetPoint;
                                // Dehumidification mode = 0 for normal mode, 1+ for enhanced mode
                                Par(3) = double(DehumidMode);
                                TempSolveRoot::SolveRoot(
                                    state, DataHVACGlobals::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, MultiModeDXCoilResidual, 0.0, 1.0, Par);
                                if (SolFla == -1) {
                                    if (CBVAV(CBVAVNum).CRDXIterationExceeded < 1) {
                                        ++CBVAV(CBVAVNum).CRDXIterationExceeded;
                                        ShowWarningError(state, "Iteration limit exceeded calculating DX unit cool reheat part-load ratio, for unit = " +
                                                         CBVAV(CBVAVNum).Name);
                                        ShowContinueErrorTimeStamp(state, format("Part-load ratio returned = {:.2R}", PartLoadFrac));
                                        ShowContinueErrorTimeStamp(state,
                                            "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                    } else {
                                        ShowRecurringWarningErrorAtEnd(state,
                                            CBVAV(CBVAVNum).Name +
                                                ", Iteration limit exceeded calculating cool reheat part-load ratio DX unit error continues.",
                                            CBVAV(CBVAVNum).CRDXIterationExceededIndex,
                                            PartLoadFrac,
                                            PartLoadFrac);
                                    }
                                } else if (SolFla == -2) {
                                    PartLoadFrac =
                                        max(0.0,
                                            min(1.0,
                                                (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp - CBVAV(CBVAVNum).CoilTempSetPoint) /
                                                    (DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilInletNode).Temp -
                                                     DataLoopNode::Node(CBVAV(CBVAVNum).DXCoilOutletNode).Temp)));
                                    if (CBVAV(CBVAVNum).CRDXIterationFailed < 1) {
                                        ++CBVAV(CBVAVNum).CRDXIterationFailed;
                                        ShowSevereError(state,
                                            "DX unit cool reheat part-load ratio calculation failed: part-load ratio limits exceeded, for unit = " +
                                            CBVAV(CBVAVNum).Name);
                                        ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                        ShowContinueErrorTimeStamp(state,
                                            "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                    } else {
                                        ShowRecurringWarningErrorAtEnd(state,
                                            CBVAV(CBVAVNum).Name + ", Dehumidifying part-load ratio calculation failed for DX unit error continues.",
                                            CBVAV(CBVAVNum).DMDXIterationFailedIndex,
                                            PartLoadFrac,
                                            PartLoadFrac);
                                    }
                                }
                            }
                        } // End if humidity ratio setpoint not met - CoolReheat humidity control

                        if (PartLoadFrac > 1.0) {
                            PartLoadFrac = 1.0;
                        } else if (PartLoadFrac < 0.0) {
                            PartLoadFrac = 0.0;
                        }
                        SaveCompressorPLR = DXCoils::DXCoilPartLoadRatio(CBVAV(CBVAVNum).DXCoolCoilIndexNum);

                    } else {
                        ShowFatalError(state, "SimCBVAV System: Invalid DX Cooling Coil=" + CBVAV(CBVAVNum).DXCoolCoilType);
                    }
                }
            } else { // IF(OutdoorDryBulbTemp .GE. CBVAV(CBVAVNum)%MinOATCompressor)THEN
                //     Simulate DX cooling coil with compressor off
                if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::CoilDX_CoolingHXAssisted) {
                    HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                        CBVAV(CBVAVNum).DXCoolCoilName,
                                                                        FirstHVACIteration,
                                                                        Off,
                                                                        0.0,
                                                                        CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                        DataHVACGlobals::ContFanCycCoil,
                                                                        HXUnitOn);
                    SaveCompressorPLR = DXCoils::DXCoilPartLoadRatio(CBVAV(CBVAVNum).DXCoolCoilIndexNum);
                } else if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::CoilDX_CoolingSingleSpeed) {
                    DXCoils::SimDXCoil(state,
                                       CBVAV(CBVAVNum).DXCoolCoilName,
                                       Off,
                                       FirstHVACIteration,
                                       CBVAV(CBVAVNum).CoolCoilCompIndex,
                                       DataHVACGlobals::ContFanCycCoil,
                                       0.0,
                                       OnOffAirFlowRatio);
                    SaveCompressorPLR = DXCoils::DXCoilPartLoadRatio(CBVAV(CBVAVNum).DXCoolCoilIndexNum);
                } else if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::CoilDX_CoolingTwoStageWHumControl) {
                    DXCoils::SimDXCoilMultiMode(state,
                                                CBVAV(CBVAVNum).DXCoolCoilName,
                                                Off,
                                                FirstHVACIteration,
                                                0.0,
                                                0,
                                                CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                DataHVACGlobals::ContFanCycCoil);
                    SaveCompressorPLR = DXCoils::DXCoilPartLoadRatio(CBVAV(CBVAVNum).DXCoolCoilIndexNum);
                } else if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::Coil_CoolingAirToAirVariableSpeed) {
                    Real64 QZnReq(0.0);                // Zone load (W), input to variable-speed DX coil
                    Real64 QLatReq(0.0);               // Zone latent load, input to variable-speed DX coil
                    Real64 MaxONOFFCyclesperHour(4.0); // Maximum cycling rate of heat pump [cycles/hr]
                    Real64 HPTimeConstant(0.0);        // Heat pump time constant [s]
                    Real64 FanDelayTime(0.0);          // Fan delay time, time delay for the HP's fan to
                    Real64 PartLoadFrac(0.0);
                    Real64 SpeedRatio(0.0);
                    int SpeedNum(1);
                    // Get no load result
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              CBVAV(CBVAVNum).DXCoolCoilName,
                                                              CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                              DataHVACGlobals::ContFanCycCoil,
                                                              MaxONOFFCyclesperHour,
                                                              HPTimeConstant,
                                                              FanDelayTime,
                                                              Off,
                                                              PartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq);
                    SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, CBVAV(CBVAVNum).CoolCoilCompIndex);
                }
            }

            // Simulate cooling coil with compressor off if zone requires heating
        } else { // HeatCoolMode == HeatingMode and no cooling is required, set PLR to 0
            if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::CoilDX_CoolingHXAssisted) {
                HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                    CBVAV(CBVAVNum).DXCoolCoilName,
                                                                    FirstHVACIteration,
                                                                    Off,
                                                                    0.0,
                                                                    CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                                    DataHVACGlobals::ContFanCycCoil,
                                                                    HXUnitOn);
            } else if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::CoilDX_CoolingSingleSpeed) {
                DXCoils::SimDXCoil(state,
                                   CBVAV(CBVAVNum).DXCoolCoilName,
                                   Off,
                                   FirstHVACIteration,
                                   CBVAV(CBVAVNum).CoolCoilCompIndex,
                                   DataHVACGlobals::ContFanCycCoil,
                                   0.0,
                                   OnOffAirFlowRatio);
            } else if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::Coil_CoolingAirToAirVariableSpeed) {
                Real64 QZnReq(0.0);                // Zone load (W), input to variable-speed DX coil
                Real64 QLatReq(0.0);               // Zone latent load, input to variable-speed DX coil
                Real64 MaxONOFFCyclesperHour(4.0); // Maximum cycling rate of heat pump [cycles/hr]
                Real64 HPTimeConstant(0.0);        // Heat pump time constant [s]
                Real64 FanDelayTime(0.0);          // Fan delay time, time delay for the HP's fan to
                Real64 PartLoadFrac(0.0);
                Real64 SpeedRatio(0.0);
                int SpeedNum(1);
                // run model with no load
                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                          CBVAV(CBVAVNum).DXCoolCoilName,
                                                          CBVAV(CBVAVNum).CoolCoilCompIndex,
                                                          DataHVACGlobals::ContFanCycCoil,
                                                          MaxONOFFCyclesperHour,
                                                          HPTimeConstant,
                                                          FanDelayTime,
                                                          Off,
                                                          PartLoadFrac,
                                                          SpeedNum,
                                                          SpeedRatio,
                                                          QZnReq,
                                                          QLatReq);

            } else if (CBVAV(CBVAVNum).DXCoolCoilType_Num == DataHVACGlobals::CoilDX_CoolingTwoStageWHumControl) {
                DXCoils::SimDXCoilMultiMode(state,
                                            CBVAV(CBVAVNum).DXCoolCoilName,
                                            Off,
                                            FirstHVACIteration,
                                            0.0,
                                            0,
                                            CBVAV(CBVAVNum).CoolCoilCompIndex,
                                            DataHVACGlobals::ContFanCycCoil);
            }
        }

        // Simulate the heating coil based on coil type
        {
            auto const SELECT_CASE_var(CBVAV(CBVAVNum).HeatCoilType_Num);

            if (SELECT_CASE_var == DataHVACGlobals::CoilDX_HeatingEmpirical) {
                //   Simulate DX heating coil if zone load is positive (heating load)
                if (CBVAV(CBVAVNum).HeatCoolMode == HeatingMode) {
                    if (OutdoorDryBulbTemp > CBVAV(CBVAVNum).MinOATCompressor) {
                        //       simulate the DX heating coil
                        // vs coil issue

                        DXCoils::SimDXCoil(state,
                                           CBVAV(CBVAVNum).HeatCoilName,
                                           On,
                                           FirstHVACIteration,
                                           CBVAV(CBVAVNum).HeatCoilIndex,
                                           DataHVACGlobals::ContFanCycCoil,
                                           PartLoadFrac,
                                           OnOffAirFlowRatio);
                        if (DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).Temp > CBVAV(CBVAVNum).CoilTempSetPoint &&
                            DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).Temp < CBVAV(CBVAVNum).CoilTempSetPoint) {
                            // iterate to find PLR at CoilTempSetPoint
                            Par(1) = double(CBVAV(CBVAVNum).HeatCoilIndex);
                            Par(2) = min(CBVAV(CBVAVNum).CoilTempSetPoint, CBVAV(CBVAVNum).MaxLATHeating);
                            Par(3) = OnOffAirFlowRatio;
                            TempSolveRoot::SolveRoot(state, DataHVACGlobals::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, DXHeatingCoilResidual, 0.0, 1.0, Par);
                            DXCoils::SimDXCoil(state,
                                               CBVAV(CBVAVNum).HeatCoilName,
                                               On,
                                               FirstHVACIteration,
                                               CBVAV(CBVAVNum).HeatCoilIndex,
                                               DataHVACGlobals::ContFanCycCoil,
                                               PartLoadFrac,
                                               OnOffAirFlowRatio);
                            if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                                ShowWarningError(state, "Iteration limit exceeded calculating DX unit part-load ratio, for unit = " +
                                                 CBVAV(CBVAVNum).HeatCoilName);
                                ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(state,
                                    "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                                ShowSevereError(state, "DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = " +
                                                CBVAV(CBVAVNum).HeatCoilName);
                                ShowContinueErrorTimeStamp(
                                    state,
                                    format("A part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:", PartLoadFrac));
                                ShowContinueError(state, "Please send this information to the EnergyPlus support group.");
                            }
                        }
                    } else { // OAT .LT. MinOATCompressor
                        //       simulate DX heating coil with compressor off
                        DXCoils::SimDXCoil(state,
                                           CBVAV(CBVAVNum).HeatCoilName,
                                           Off,
                                           FirstHVACIteration,
                                           CBVAV(CBVAVNum).HeatCoilIndex,
                                           DataHVACGlobals::ContFanCycCoil,
                                           0.0,
                                           OnOffAirFlowRatio);
                    }
                    SaveCompressorPLR = DXCoils::DXCoilPartLoadRatio(CBVAV(CBVAVNum).DXHeatCoilIndexNum);
                } else { // HeatCoolMode = CoolingMode
                    //     simulate DX heating coil with compressor off when cooling load is required
                    DXCoils::SimDXCoil(state,
                                       CBVAV(CBVAVNum).HeatCoilName,
                                       Off,
                                       FirstHVACIteration,
                                       CBVAV(CBVAVNum).HeatCoilIndex,
                                       DataHVACGlobals::ContFanCycCoil,
                                       0.0,
                                       OnOffAirFlowRatio);
                }
            } else if (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingAirToAirVariableSpeed) {
                Real64 QZnReq(0.0);                // Zone load (W), input to variable-speed DX coil
                Real64 QLatReq(0.0);               // Zone latent load, input to variable-speed DX coil
                Real64 MaxONOFFCyclesperHour(4.0); // Maximum cycling rate of heat pump [cycles/hr]
                Real64 HPTimeConstant(0.0);        // Heat pump time constant [s]
                Real64 FanDelayTime(0.0);          // Fan delay time, time delay for the HP's fan to
                Real64 OnOffAirFlowRatio(1.0);     // ratio of compressor on flow to average flow over time step
                Real64 PartLoadFrac(0.0);
                Real64 SpeedRatio(0.0);
                int SpeedNum(1);
                bool errorFlag(false);
                int maxNumSpeeds = VariableSpeedCoils::GetVSCoilNumOfSpeeds(state, CBVAV(CBVAVNum).HeatCoilName, errorFlag);
                Real64 DesOutTemp = CBVAV(CBVAVNum).CoilTempSetPoint;
                // Get no load result
                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                          CBVAV(CBVAVNum).HeatCoilName,
                                                          CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                                          DataHVACGlobals::ContFanCycCoil,
                                                          MaxONOFFCyclesperHour,
                                                          HPTimeConstant,
                                                          FanDelayTime,
                                                          Off,
                                                          PartLoadFrac,
                                                          SpeedNum,
                                                          SpeedRatio,
                                                          QZnReq,
                                                          QLatReq);

                Real64 NoOutput = DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).MassFlowRate *
                                  (Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).Temp,
                                                              DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).HumRat) -
                                   Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).Temp,
                                                              DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).HumRat));
                Real64 TempNoOutput = DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).Temp;
                // Real64 NoLoadHumRatOut = VariableSpeedCoils::VarSpeedCoil( CBVAV( CBVAVNum ).CoolCoilCompIndex ).OutletAirHumRat;

                // Get full load result
                PartLoadFrac = 1.0;
                SpeedNum = maxNumSpeeds;
                SpeedRatio = 1.0;
                QZnReq = 0.001; // to indicate the coil is running
                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                          CBVAV(CBVAVNum).HeatCoilName,
                                                          CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                                          DataHVACGlobals::ContFanCycCoil,
                                                          MaxONOFFCyclesperHour,
                                                          HPTimeConstant,
                                                          FanDelayTime,
                                                          On,
                                                          PartLoadFrac,
                                                          SpeedNum,
                                                          SpeedRatio,
                                                          QZnReq,
                                                          QLatReq);

                // Real64 FullLoadHumRatOut = VariableSpeedCoils::VarSpeedCoil( CBVAV( CBVAVNum ).CoolCoilCompIndex ).OutletAirHumRat;
                Real64 FullOutput = DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).MassFlowRate *
                                    (Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).Temp,
                                                                DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).HumRat) -
                                     Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).Temp,
                                                                DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).HumRat));
                Real64 ReqOutput = DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).MassFlowRate *
                                   (Psychrometrics::PsyHFnTdbW(DesOutTemp, DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).HumRat) -
                                    Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).Temp,
                                                               DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).HumRat));

                Real64 loadAccuracy(0.001);                   // Watts, power
                Real64 tempAccuracy(0.001);                   // delta C, temperature
                if ((NoOutput - ReqOutput) > -loadAccuracy) { //         IF NoOutput is higher than (more heating than required) or very near the
                                                              //         ReqOutput, do not run the compressor
                    PartLoadFrac = 0.0;
                    SpeedNum = 1;
                    SpeedRatio = 0.0;
                    QZnReq = 0.0;
                    // call again with coil off
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              CBVAV(CBVAVNum).HeatCoilName,
                                                              CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                                              DataHVACGlobals::ContFanCycCoil,
                                                              MaxONOFFCyclesperHour,
                                                              HPTimeConstant,
                                                              FanDelayTime,
                                                              Off,
                                                              PartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq);

                } else if ((FullOutput - ReqOutput) < loadAccuracy) { //         If the FullOutput is less than (insufficient cooling) or very near
                                                                      //         the ReqOutput, run the compressor at PartLoadFrac = 1.
                                                                      // which we just did so nothing to be done

                } else { //  Else find how the coil is modulating (speed level and speed ratio or part load between off and speed 1) to meet the load
                    //           OutletTempDXCoil is the full capacity outlet temperature at PartLoadFrac = 1 from the CALL above. If this temp is
                    //           greater than the desired outlet temp, then run the compressor at PartLoadFrac = 1, otherwise find the operating PLR.
                    Real64 OutletTempDXCoil = state.dataVariableSpeedCoils->VarSpeedCoil(CBVAV(CBVAVNum).DXHeatCoilIndexNum).OutletAirDBTemp;
                    if (OutletTempDXCoil < DesOutTemp) {
                        PartLoadFrac = 1.0;
                        SpeedNum = maxNumSpeeds;
                        SpeedRatio = 1.0;
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  CBVAV(CBVAVNum).HeatCoilName,
                                                                  CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                                                  DataHVACGlobals::ContFanCycCoil,
                                                                  MaxONOFFCyclesperHour,
                                                                  HPTimeConstant,
                                                                  FanDelayTime,
                                                                  On,
                                                                  PartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq,
                                                                  OnOffAirFlowRatio);
                    } else {
                        // run at lowest speed
                        PartLoadFrac = 1.0;
                        SpeedNum = 1;
                        SpeedRatio = 1.0;
                        QZnReq = 0.001; // to indicate the coil is running
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  CBVAV(CBVAVNum).HeatCoilName,
                                                                  CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                                                  DataHVACGlobals::ContFanCycCoil,
                                                                  MaxONOFFCyclesperHour,
                                                                  HPTimeConstant,
                                                                  FanDelayTime,
                                                                  On,
                                                                  PartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq,
                                                                  OnOffAirFlowRatio);

                        Real64 TempSpeedOut = DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).Temp;
                        Real64 TempSpeedOutSpeed1 = TempSpeedOut;

                        if ((TempSpeedOut - DesOutTemp) < tempAccuracy) {
                            // Check to see which speed to meet the load
                            PartLoadFrac = 1.0;
                            SpeedRatio = 1.0;
                            for (int I = 2; I <= maxNumSpeeds; ++I) {
                                SpeedNum = I;
                                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                          CBVAV(CBVAVNum).HeatCoilName,
                                                                          CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                                                          DataHVACGlobals::ContFanCycCoil,
                                                                          MaxONOFFCyclesperHour,
                                                                          HPTimeConstant,
                                                                          FanDelayTime,
                                                                          On,
                                                                          PartLoadFrac,
                                                                          SpeedNum,
                                                                          SpeedRatio,
                                                                          QZnReq,
                                                                          QLatReq,
                                                                          OnOffAirFlowRatio);

                                TempSpeedOut = DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).Temp;

                                if ((TempSpeedOut - DesOutTemp) > tempAccuracy) {
                                    SpeedNum = I;
                                    break;
                                }
                            }
                            // now find the speed ratio for the found speednum
                            Par(1) = double(CBVAV(CBVAVNum).DXHeatCoilIndexNum);
                            Par(2) = DesOutTemp;
                            Par(5) = double(DataHVACGlobals::ContFanCycCoil);
                            Par(3) = double(SpeedNum);
                            TempSolveRoot::SolveRoot(
                                state, tempAccuracy, MaxIte, SolFla, SpeedRatio, HVACDXHeatPumpSystem::VSCoilSpeedResidual, 1.0e-10, 1.0, Par);

                            if (SolFla == -1) {
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (CBVAV(CBVAVNum).DXHeatIterationExceeded < 4) {
                                        ++CBVAV(CBVAVNum).DXHeatIterationExceeded;
                                        ShowWarningError(state, CBVAV(CBVAVNum).HeatCoilType +
                                                         " - Iteration limit exceeded calculating VS DX coil speed ratio for coil named " +
                                                         CBVAV(CBVAVNum).HeatCoilName + ", in Unitary system named" + CBVAV(CBVAVNum).Name);
                                        ShowContinueError(state, format("Calculated speed ratio = {:.4R}", SpeedRatio));
                                        ShowContinueErrorTimeStamp(state,
                                            "The calculated speed ratio will be used and the simulation continues. Occurrence info:");
                                    }
                                    ShowRecurringWarningErrorAtEnd(state,
                                        CBVAV(CBVAVNum).HeatCoilType + " \"" + CBVAV(CBVAVNum).HeatCoilName +
                                            "\" - Iteration limit exceeded calculating speed ratio error continues. Speed Ratio statistics follow.",
                                        CBVAV(CBVAVNum).DXHeatIterationExceededIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            } else if (SolFla == -2) {

                                if (!state.dataGlobal->WarmupFlag) {
                                    if (CBVAV(CBVAVNum).DXHeatIterationFailed < 4) {
                                        ++CBVAV(CBVAVNum).DXHeatIterationFailed;
                                        ShowWarningError(state, CBVAV(CBVAVNum).HeatCoilType +
                                                         " - DX unit speed ratio calculation failed: solver limits exceeded, for coil named " +
                                                         CBVAV(CBVAVNum).HeatCoilName + ", in Unitary system named" + CBVAV(CBVAVNum).Name);
                                        ShowContinueErrorTimeStamp(state, " Speed ratio will be set to 0.5, and the simulation continues. Occurrence info:");
                                    }
                                    ShowRecurringWarningErrorAtEnd(state,
                                        CBVAV(CBVAVNum).HeatCoilType + " \"" + CBVAV(CBVAVNum).HeatCoilName +
                                            "\" - DX unit speed ratio calculation failed error continues. speed ratio statistics follow.",
                                        CBVAV(CBVAVNum).DXHeatIterationFailedIndex,
                                        SpeedRatio,
                                        SpeedRatio);
                                }
                                SpeedRatio = 0.5;
                            }
                            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                      CBVAV(CBVAVNum).HeatCoilName,
                                                                      CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                                                      DataHVACGlobals::ContFanCycCoil,
                                                                      MaxONOFFCyclesperHour,
                                                                      HPTimeConstant,
                                                                      FanDelayTime,
                                                                      On,
                                                                      PartLoadFrac,
                                                                      SpeedNum,
                                                                      SpeedRatio,
                                                                      QZnReq,
                                                                      QLatReq,
                                                                      OnOffAirFlowRatio);
                        } else {
                            // cycling compressor at lowest speed number, find part load fraction
                            Par(1) = double(CBVAV(CBVAVNum).DXHeatCoilIndexNum);
                            Par(2) = DesOutTemp;
                            Par(5) = double(DataHVACGlobals::ContFanCycCoil);
                            TempSolveRoot::SolveRoot(
                                state, tempAccuracy, MaxIte, SolFla, PartLoadFrac, HVACDXHeatPumpSystem::VSCoilCyclingResidual, 1.0e-10, 1.0, Par);
                            if (SolFla == -1) {
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (CBVAV(CBVAVNum).DXHeatCyclingIterationExceeded < 4) {
                                        ++CBVAV(CBVAVNum).DXHeatCyclingIterationExceeded;
                                        ShowWarningError(state,
                                            CBVAV(CBVAVNum).HeatCoilType +
                                            " - Iteration limit exceeded calculating VS DX unit low speed cycling ratio, for coil named " +
                                            CBVAV(CBVAVNum).HeatCoilName + ", in Unitary system named" + CBVAV(CBVAVNum).Name);
                                        ShowContinueError(state, format("Estimated cycling ratio  = {:.3R}", (DesOutTemp / TempSpeedOut)));
                                        ShowContinueError(state, format("Calculated cycling ratio = {:.3R}", PartLoadFrac));
                                        ShowContinueErrorTimeStamp(state,
                                            "The calculated cycling ratio will be used and the simulation continues. Occurrence info:");
                                    }
                                    ShowRecurringWarningErrorAtEnd(state, CBVAV(CBVAVNum).HeatCoilType + " \"" + CBVAV(CBVAVNum).HeatCoilName +
                                                                       "\" - Iteration limit exceeded calculating low speed cycling ratio error "
                                                                       "continues. Sensible PLR statistics follow.",
                                                                   CBVAV(CBVAVNum).DXHeatCyclingIterationExceededIndex,
                                                                   PartLoadFrac,
                                                                   PartLoadFrac);
                                }
                            } else if (SolFla == -2) {

                                if (!state.dataGlobal->WarmupFlag) {
                                    if (CBVAV(CBVAVNum).DXHeatCyclingIterationFailed < 4) {
                                        ++CBVAV(CBVAVNum).DXHeatCyclingIterationFailed;
                                        ShowWarningError(state, CBVAV(CBVAVNum).HeatCoilType +
                                                         " - DX unit low speed cycling ratio calculation failed: limits exceeded, for unit = " +
                                                         CBVAV(CBVAVNum).Name);
                                        ShowContinueError(state,
                                                          format("Estimated low speed cycling ratio = {:.3R}",
                                                                 (DesOutTemp - TempNoOutput) / (TempSpeedOutSpeed1 - TempNoOutput)));
                                        ShowContinueErrorTimeStamp(state,
                                            "The estimated low speed cycling ratio will be used and the simulation continues. Occurrence info:");
                                    }
                                    ShowRecurringWarningErrorAtEnd(state, CBVAV(CBVAVNum).HeatCoilType + " \"" + CBVAV(CBVAVNum).HeatCoilName +
                                                                       "\" - DX unit low speed cycling ratio calculation failed error continues. "
                                                                       "cycling ratio statistics follow.",
                                                                   CBVAV(CBVAVNum).DXHeatCyclingIterationFailedIndex,
                                                                   PartLoadFrac,
                                                                   PartLoadFrac);
                                }
                                PartLoadFrac = (DesOutTemp - TempNoOutput) / (TempSpeedOutSpeed1 - TempNoOutput);
                            }
                            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                      CBVAV(CBVAVNum).HeatCoilName,
                                                                      CBVAV(CBVAVNum).DXHeatCoilIndexNum,
                                                                      DataHVACGlobals::ContFanCycCoil,
                                                                      MaxONOFFCyclesperHour,
                                                                      HPTimeConstant,
                                                                      FanDelayTime,
                                                                      On,
                                                                      PartLoadFrac,
                                                                      SpeedNum,
                                                                      SpeedRatio,
                                                                      QZnReq,
                                                                      QLatReq,
                                                                      OnOffAirFlowRatio);
                        }
                    }
                }

                if (PartLoadFrac > 1.0) {
                    PartLoadFrac = 1.0;
                } else if (PartLoadFrac < 0.0) {
                    PartLoadFrac = 0.0;
                }
                SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, CBVAV(CBVAVNum).DXHeatCoilIndexNum);
            } else if ((SELECT_CASE_var == DataHVACGlobals::Coil_HeatingGasOrOtherFuel) ||
                       (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingElectric) || (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingWater) ||
                       (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingSteam)) { // not a DX heating coil
                if (CBVAV(CBVAVNum).HeatCoolMode == HeatingMode) {
                    CpAir = Psychrometrics::PsyCpAirFnW(DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).HumRat);
                    QHeater = DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).MassFlowRate * CpAir *
                              (CBVAV(CBVAVNum).CoilTempSetPoint - DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilInletNode).Temp);
                } else {
                    QHeater = 0.0;
                }
                // Added None DX heating coils calling point
                DataLoopNode::Node(CBVAV(CBVAVNum).HeatingCoilOutletNode).TempSetPoint = CBVAV(CBVAVNum).CoilTempSetPoint;
                CalcNonDXHeatingCoils(state, CBVAVNum, FirstHVACIteration, QHeater, CBVAV(CBVAVNum).OpMode, QHeaterActual);
            } else {
                ShowFatalError(state, "SimCBVAV System: Invalid Heating Coil=" + CBVAV(CBVAVNum).HeatCoilType);
            }
        }

        if (CBVAV(CBVAVNum).FanPlace == DataHVACGlobals::DrawThru) {
            if (CBVAV(CBVAVNum).FanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
                HVACFan::fanObjs[CBVAV(CBVAVNum).FanIndex]->simulate(state, 1.0 / OnOffAirFlowRatio, _, _, _);
            } else {
                Fans::SimulateFanComponents(state, CBVAV(CBVAVNum).FanName, FirstHVACIteration, CBVAV(CBVAVNum).FanIndex, FanSpeedRatio);
            }
        }
        int splitterOutNode = CBVAV(CBVAVNum).SplitterOutletAirNode;
        DataLoopNode::Node(splitterOutNode).MassFlowRateSetPoint = DataLoopNode::Node(OutletNode).MassFlowRateSetPoint;
        DataLoopNode::Node(OutletNode) = DataLoopNode::Node(splitterOutNode);
        DataLoopNode::Node(OutletNode).TempSetPoint = CBVAV(CBVAVNum).OutletTempSetPoint;
        DataLoopNode::Node(OutletNode).MassFlowRate =
            (1.0 - BypassDuctFlowFraction) * DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).MassFlowRate;
        // report variable
        CBVAV(CBVAVNum).BypassMassFlowRate = BypassDuctFlowFraction * DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).MassFlowRate;
        // initialize bypass duct connected to mixer or plenum with flow rate and conditions
        if (CBVAV(CBVAVNum).plenumIndex > 0 || CBVAV(CBVAVNum).mixerIndex > 0) {
            int plenumOrMixerInletNode = CBVAV(CBVAVNum).PlenumMixerInletAirNode;
            DataLoopNode::Node(plenumOrMixerInletNode) = DataLoopNode::Node(splitterOutNode);
            DataLoopNode::Node(plenumOrMixerInletNode).MassFlowRate =
                BypassDuctFlowFraction * DataLoopNode::Node(CBVAV(CBVAVNum).MixerInletAirNode).MassFlowRate;
            DataLoopNode::Node(plenumOrMixerInletNode).MassFlowRateMaxAvail = DataLoopNode::Node(plenumOrMixerInletNode).MassFlowRate;
            state.dataAirLoop->AirLoopFlow(CBVAV(CBVAVNum).AirLoopNumber).BypassMassFlow = DataLoopNode::Node(plenumOrMixerInletNode).MassFlowRate;
        }

        // calculate sensible load met using delta enthalpy at a constant (minimum) humidity ratio)
        MinHumRat = min(DataLoopNode::Node(InletNode).HumRat, DataLoopNode::Node(OutletNode).HumRat);
        LoadMet = DataLoopNode::Node(OutletNode).MassFlowRate * (Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(OutletNode).Temp, MinHumRat) -
                                                                 Psychrometrics::PsyHFnTdbW(DataLoopNode::Node(InletNode).Temp, MinHumRat));

        // calculate OA fraction used for zone OA volume flow rate calc
        state.dataAirLoop->AirLoopFlow(CBVAV(CBVAVNum).AirLoopNumber).OAFrac = 0.0;
        if (DataLoopNode::Node(CBVAV(CBVAVNum).AirOutNode).MassFlowRate > 0.0) {
            state.dataAirLoop->AirLoopFlow(CBVAV(CBVAVNum).AirLoopNumber).OAFrac =
                DataLoopNode::Node(CBVAV(CBVAVNum).MixerOutsideAirNode).MassFlowRate / DataLoopNode::Node(CBVAV(CBVAVNum).AirOutNode).MassFlowRate;
        }
    }

    void GetZoneLoads(EnergyPlusData &state,
                      int const CBVAVNum // Index to CBVAV unit being simulated
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is used to poll the thermostats in each zone and determine the
        // mode of operation, either cooling, heating, or none.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int lastDayOfSim(0);   // used during warmup to reset changeOverTimer since need to do same thing next warmup day
        Real64 ZoneLoad = 0.0; // Total load in controlled zone [W]

        int dayOfSim = state.dataGlobal->DayOfSim; // DayOfSim increments during Warmup when it actually simulates the same day
        if (state.dataGlobal->WarmupFlag) {
            // when warmupday increments then reset timer
            if (lastDayOfSim != dayOfSim) CBVAV(CBVAVNum).changeOverTimer = -1.0; // reset to default (thisTime always > -1)
            lastDayOfSim = dayOfSim;
            dayOfSim = 1; // reset so that thisTime is <= 24 during warmup
        }
        Real64 thisTime = (dayOfSim - 1) * 24 + state.dataGlobal->HourOfDay - 1 + (state.dataGlobal->TimeStep - 1) * state.dataGlobal->TimeStepZone +
                          DataHVACGlobals::SysTimeElapsed;

        if (thisTime <= CBVAV(CBVAVNum).changeOverTimer) {
            CBVAV(CBVAVNum).modeChanged = true;
            return;
        }

        Real64 QZoneReqCool = 0.0; // Total cooling load in all controlled zones [W]
        Real64 QZoneReqHeat = 0.0; // Total heating load in all controlled zones [W]
        CBVAV(CBVAVNum).NumZonesCooled = 0;
        CBVAV(CBVAVNum).NumZonesHeated = 0;
        CBVAV(CBVAVNum).HeatCoolMode = 0;

        for (int ZoneNum = 1; ZoneNum <= CBVAV(CBVAVNum).NumControlledZones; ++ZoneNum) {
            int actualZoneNum = CBVAV(CBVAVNum).ControlledZoneNum(ZoneNum);
            int coolSeqNum = CBVAV(CBVAVNum).ZoneSequenceCoolingNum(ZoneNum);
            int heatSeqNum = CBVAV(CBVAVNum).ZoneSequenceHeatingNum(ZoneNum);
            if (coolSeqNum > 0 && heatSeqNum > 0) {
                Real64 ZoneLoadToCoolSPSequenced =
                    DataZoneEnergyDemands::ZoneSysEnergyDemand(actualZoneNum).SequencedOutputRequiredToCoolingSP(coolSeqNum);
                Real64 ZoneLoadToHeatSPSequenced =
                    DataZoneEnergyDemands::ZoneSysEnergyDemand(actualZoneNum).SequencedOutputRequiredToHeatingSP(heatSeqNum);
                if (ZoneLoadToHeatSPSequenced > 0.0 && ZoneLoadToCoolSPSequenced > 0.0) {
                    ZoneLoad = ZoneLoadToHeatSPSequenced;
                } else if (ZoneLoadToHeatSPSequenced < 0.0 && ZoneLoadToCoolSPSequenced < 0.0) {
                    ZoneLoad = ZoneLoadToCoolSPSequenced;
                } else if (ZoneLoadToHeatSPSequenced <= 0.0 && ZoneLoadToCoolSPSequenced >= 0.0) {
                    ZoneLoad = 0.0;
                }
            } else {
                ZoneLoad = DataZoneEnergyDemands::ZoneSysEnergyDemand(actualZoneNum).RemainingOutputRequired;
            }

            if (!DataZoneEnergyDemands::CurDeadBandOrSetback(actualZoneNum)) {
                if (ZoneLoad > DataHVACGlobals::SmallLoad) {
                    QZoneReqHeat += ZoneLoad;
                    ++CBVAV(CBVAVNum).NumZonesHeated;
                } else if (ZoneLoad < -DataHVACGlobals::SmallLoad) {
                    QZoneReqCool += ZoneLoad;
                    ++CBVAV(CBVAVNum).NumZonesCooled;
                }
            }
        }

        {
            auto const SELECT_CASE_var(CBVAV(CBVAVNum).PriorityControl);
            if (SELECT_CASE_var == CoolingPriority) {
                if (QZoneReqCool < 0.0) {
                    CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                } else if (QZoneReqHeat > 0.0) {
                    CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                }
            } else if (SELECT_CASE_var == HeatingPriority) {
                if (QZoneReqHeat > 0.0) {
                    CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                } else if (QZoneReqCool < 0.0) {
                    CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                }
            } else if (SELECT_CASE_var == ZonePriority) {
                if (CBVAV(CBVAVNum).NumZonesHeated > CBVAV(CBVAVNum).NumZonesCooled) {
                    if (QZoneReqHeat > 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                    } else if (QZoneReqCool < 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                    }
                } else if (CBVAV(CBVAVNum).NumZonesCooled > CBVAV(CBVAVNum).NumZonesHeated) {
                    if (QZoneReqCool < 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                    } else if (QZoneReqHeat > 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                    }
                } else {
                    if (std::abs(QZoneReqCool) > std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                    } else if (std::abs(QZoneReqCool) < std::abs(QZoneReqHeat) && QZoneReqHeat != 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                    } else if (std::abs(QZoneReqCool) == std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                    }
                }
            } else if (SELECT_CASE_var == LoadPriority) {
                if (std::abs(QZoneReqCool) > std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                    CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                } else if (std::abs(QZoneReqCool) < std::abs(QZoneReqHeat) && QZoneReqHeat != 0.0) {
                    CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                } else if (CBVAV(CBVAVNum).NumZonesHeated > CBVAV(CBVAVNum).NumZonesCooled) {
                    if (QZoneReqHeat > 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                    } else if (QZoneReqCool < 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                    }
                } else if (CBVAV(CBVAVNum).NumZonesHeated < CBVAV(CBVAVNum).NumZonesCooled) {
                    if (QZoneReqCool < 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                    } else if (QZoneReqHeat > 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                    }
                } else {
                    if (QZoneReqCool < 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = CoolingMode;
                    } else if (QZoneReqHeat > 0.0) {
                        CBVAV(CBVAVNum).HeatCoolMode = HeatingMode;
                    }
                }
            }
        }

        if (CBVAV(CBVAVNum).LastMode != CBVAV(CBVAVNum).HeatCoolMode) {
            CBVAV(CBVAVNum).changeOverTimer = thisTime + CBVAV(CBVAVNum).minModeChangeTime;
            CBVAV(CBVAVNum).LastMode = CBVAV(CBVAVNum).HeatCoolMode;
            CBVAV(CBVAVNum).modeChanged = true;
        }
    }

    Real64 CalcSetPointTempTarget(int const CBVAVNumber) // Index to changeover-bypass VAV system
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS FUNCTION:
        //  Calculate outlet air node temperature setpoint

        // METHODOLOGY EMPLOYED:
        //  Calculate an outlet temperature to satisfy zone loads. This temperature is calculated
        //  based on 1 zone's VAV box fully opened. The other VAV boxes are partially open (modulated).

        // Return value
        Real64 CalcSetPointTempTarget;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneLoad;                 // Zone load sensed by thermostat [W]
        Real64 QToCoolSetPt;             // Zone load to cooling setpoint [W]
        Real64 QToHeatSetPt;             // Zone load to heating setpoint [W]
        Real64 SupplyAirTemp;            // Supply air temperature required to meet load [C]
        Real64 SupplyAirTempToHeatSetPt; // Supply air temperature required to reach the heating setpoint [C]
        Real64 SupplyAirTempToCoolSetPt; // Supply air temperature required to reach the cooling setpoint [C]

        Real64 DXCoolCoilInletTemp = DataLoopNode::Node(CBVAV(CBVAVNumber).DXCoilInletNode).Temp;
        Real64 OutAirTemp = DataLoopNode::Node(CBVAV(CBVAVNumber).AirOutNode).Temp;
        Real64 OutAirHumRat = DataLoopNode::Node(CBVAV(CBVAVNumber).AirOutNode).HumRat;

        if (CBVAV(CBVAVNumber).HeatCoolMode == CoolingMode) { // Cooling required
            CalcSetPointTempTarget = 99999.0;
        } else if (CBVAV(CBVAVNumber).HeatCoolMode == HeatingMode) { // Heating required
            CalcSetPointTempTarget = -99999.0;
        }
        Real64 TSupplyToHeatSetPtMax = -99999.0; // Maximum of the supply air temperatures required to reach the heating setpoint [C]
        Real64 TSupplyToCoolSetPtMin = 99999.0;  // Minimum of the supply air temperatures required to reach the cooling setpoint [C]

        for (int ZoneNum = 1; ZoneNum <= CBVAV(CBVAVNumber).NumControlledZones; ++ZoneNum) {
            int ZoneNodeNum = CBVAV(CBVAVNumber).ActualZoneNodeNum(ZoneNum);
            int BoxOutletNodeNum = CBVAV(CBVAVNumber).CBVAVBoxOutletNode(ZoneNum);
            if ((CBVAV(CBVAVNumber).ZoneSequenceCoolingNum(ZoneNum) > 0) && (CBVAV(CBVAVNumber).ZoneSequenceHeatingNum(ZoneNum) > 0)) {
                QToCoolSetPt = DataZoneEnergyDemands::ZoneSysEnergyDemand(CBVAV(CBVAVNumber).ControlledZoneNum(ZoneNum))
                                   .SequencedOutputRequiredToCoolingSP(CBVAV(CBVAVNumber).ZoneSequenceCoolingNum(ZoneNum));
                QToHeatSetPt = DataZoneEnergyDemands::ZoneSysEnergyDemand(CBVAV(CBVAVNumber).ControlledZoneNum(ZoneNum))
                                   .SequencedOutputRequiredToHeatingSP(CBVAV(CBVAVNumber).ZoneSequenceHeatingNum(ZoneNum));
                if (QToHeatSetPt > 0.0 && QToCoolSetPt > 0.0) {
                    ZoneLoad = QToHeatSetPt;
                } else if (QToHeatSetPt < 0.0 && QToCoolSetPt < 0.0) {
                    ZoneLoad = QToCoolSetPt;
                } else if (QToHeatSetPt <= 0.0 && QToCoolSetPt >= 0.0) {
                    ZoneLoad = 0.0;
                }
            } else {
                ZoneLoad = DataZoneEnergyDemands::ZoneSysEnergyDemand(CBVAV(CBVAVNumber).ControlledZoneNum(ZoneNum)).RemainingOutputRequired;
                QToCoolSetPt = DataZoneEnergyDemands::ZoneSysEnergyDemand(CBVAV(CBVAVNumber).ControlledZoneNum(ZoneNum)).OutputRequiredToCoolingSP;
                QToHeatSetPt = DataZoneEnergyDemands::ZoneSysEnergyDemand(CBVAV(CBVAVNumber).ControlledZoneNum(ZoneNum)).OutputRequiredToHeatingSP;
            }

            Real64 CpSupplyAir = Psychrometrics::PsyCpAirFnW(OutAirHumRat);

            // Find the supply air temperature that will force the box to full flow
            if (BoxOutletNodeNum > 0) {
                if (DataLoopNode::Node(BoxOutletNodeNum).MassFlowRateMax == 0.0) {
                    SupplyAirTemp = DataLoopNode::Node(ZoneNodeNum).Temp;
                } else {
                    // The target supply air temperature is based on current zone temp and load and max box flow rate
                    SupplyAirTemp =
                        DataLoopNode::Node(ZoneNodeNum).Temp + ZoneLoad / (CpSupplyAir * DataLoopNode::Node(BoxOutletNodeNum).MassFlowRateMax);
                }
            } else {
                SupplyAirTemp = DataLoopNode::Node(ZoneNodeNum).Temp;
            }

            //     Save the MIN (cooling) or MAX (heating) temperature for coil control
            //     One box will always operate at maximum damper position minimizing overall system energy use
            if (CBVAV(CBVAVNumber).HeatCoolMode == CoolingMode) {
                CalcSetPointTempTarget = min(SupplyAirTemp, CalcSetPointTempTarget);
            } else if (CBVAV(CBVAVNumber).HeatCoolMode == HeatingMode) {
                CalcSetPointTempTarget = max(SupplyAirTemp, CalcSetPointTempTarget);
            } else {
                //       Should use CpAirAtCoolSetPoint or CpAirAtHeatSetPoint here?
                //       If so, use ZoneThermostatSetPointLo(ZoneNum) and ZoneThermostatSetPointHi(ZoneNum)
                //       along with the zone humidity ratio
                if (DataLoopNode::Node(BoxOutletNodeNum).MassFlowRateMax == 0.0) {
                    SupplyAirTempToHeatSetPt = DataLoopNode::Node(ZoneNodeNum).Temp;
                    SupplyAirTempToCoolSetPt = DataLoopNode::Node(ZoneNodeNum).Temp;
                } else {
                    SupplyAirTempToHeatSetPt =
                        DataLoopNode::Node(ZoneNodeNum).Temp + QToHeatSetPt / (CpSupplyAir * DataLoopNode::Node(BoxOutletNodeNum).MassFlowRateMax);
                    SupplyAirTempToCoolSetPt =
                        DataLoopNode::Node(ZoneNodeNum).Temp + QToCoolSetPt / (CpSupplyAir * DataLoopNode::Node(BoxOutletNodeNum).MassFlowRateMax);
                }
                TSupplyToHeatSetPtMax = max(SupplyAirTempToHeatSetPt, TSupplyToHeatSetPtMax);
                TSupplyToCoolSetPtMin = min(SupplyAirTempToCoolSetPt, TSupplyToCoolSetPtMin);
            }
        }

        //   Account for floating condition where cooling/heating is required to avoid overshooting setpoint
        if (CBVAV(CBVAVNumber).HeatCoolMode == 0) {
            if (CBVAV(CBVAVNumber).OpMode == DataHVACGlobals::ContFanCycCoil) {
                if (OutAirTemp > TSupplyToCoolSetPtMin) {
                    CalcSetPointTempTarget = TSupplyToCoolSetPtMin;
                } else if (OutAirTemp < TSupplyToHeatSetPtMax) {
                    CalcSetPointTempTarget = TSupplyToHeatSetPtMax;
                } else {
                    CalcSetPointTempTarget = OutAirTemp;
                }
            } else { // Reset setpoint to inlet air temp if unit is OFF and in cycling fan mode
                CalcSetPointTempTarget = DataLoopNode::Node(CBVAV(CBVAVNumber).AirInNode).Temp;
            }
            //   Reset cooling/heating mode to OFF if mixed air inlet temperature is below/above setpoint temperature.
            //   HeatCoolMode = 0 for OFF, 1 for cooling, 2 for heating
        } else if (CBVAV(CBVAVNumber).HeatCoolMode == CoolingMode) {
            if (DXCoolCoilInletTemp < CalcSetPointTempTarget) CalcSetPointTempTarget = DXCoolCoilInletTemp;
        } else if (CBVAV(CBVAVNumber).HeatCoolMode == HeatingMode) {
            if (DXCoolCoilInletTemp > CalcSetPointTempTarget) CalcSetPointTempTarget = DXCoolCoilInletTemp;
        }

        //   Limit outlet node temperature to MAX/MIN specified in input
        if (CalcSetPointTempTarget < CBVAV(CBVAVNumber).MinLATCooling) CalcSetPointTempTarget = CBVAV(CBVAVNumber).MinLATCooling;
        if (CalcSetPointTempTarget > CBVAV(CBVAVNumber).MaxLATHeating) CalcSetPointTempTarget = CBVAV(CBVAVNumber).MaxLATHeating;

        return CalcSetPointTempTarget;
    }

    Real64 DOE2DXCoilResidual(EnergyPlusData &state,
                              Real64 const PartLoadFrac, // Compressor cycling ratio (1.0 is continuous, 0.0 is off)
                              Array1D<Real64> const &Par // Par(1) = DX coil number
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   June 2006

        // PURPOSE OF THIS FUNCTION:
        // Calculates residual function (desired outlet temp - actual outlet temp)
        // DX Coil output depends on the part load ratio which is being varied to zero the residual.

        // METHODOLOGY EMPLOYED:
        // Calls CalcDoe2DXCoil to get outlet temperature at the given cycling ratio
        // and calculates the residual as defined above

        // Argument array dimensioning
        // Par(2) = desired air outlet temperature [C]

        int CoilIndex = int(Par(1));
        Real64 OnOffAirFlowFrac = Par(3); // Ratio of compressor ON to compressor OFF air mass flow rate

        DXCoils::CalcDoe2DXCoil(state, CoilIndex, On, false, PartLoadFrac, DataHVACGlobals::ContFanCycCoil, _, OnOffAirFlowFrac);

        Real64 OutletAirTemp = DXCoils::DXCoilOutletTemp(CoilIndex);
        Real64 Residuum = Par(2) - OutletAirTemp;

        return Residuum;
    }

    Real64 HXAssistDXCoilResidual(EnergyPlusData &state,
                                  Real64 const PartLoadFrac, // Compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                  Array1D<Real64> const &Par // Par(1) = DX coil number
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   June 2006

        // PURPOSE OF THIS FUNCTION:
        // Calculates residual function (desired DX coil outlet temp - actual DX coil outlet temp)
        // HX Assisted DX Coil output depends on the part load ratio which is being varied to zero the residual.

        // METHODOLOGY EMPLOYED:
        // Calls CalcDoe2DXCoil to get outlet temperature at the given cycling ratio
        // and calculates the residual as defined above

        // Argument array dimensioning
        // Par(2) = desired air outlet temperature [C]

        int CoilIndex = int(Par(1));
        // Real64 OnOffAirFlowFrac = Par(3); // not used
        int CBVAVNumTemp = int(Par(4));
        bool FirstHVACIter = (Par(5) == 1.0);
        bool HXUnitOn = (Par(6) == 1.0); // flag to enable heat exchanger

        HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(
            state, CBVAV(CBVAVNumTemp).DXCoolCoilName, FirstHVACIter, On, PartLoadFrac, CoilIndex, DataHVACGlobals::ContFanCycCoil, HXUnitOn);

        Real64 OutletAirTemp = DataLoopNode::Node(CBVAV(CBVAVNumTemp).DXCoilOutletNode).Temp;
        Real64 Residuum = Par(2) - OutletAirTemp;

        return Residuum;
    }

    Real64 DXHeatingCoilResidual(EnergyPlusData &state,
                                 Real64 const PartLoadFrac, // Compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                 Array1D<Real64> const &Par // Par(1) = DX coil number
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   June 2006

        // PURPOSE OF THIS FUNCTION:
        // Calculates residual function (desired outlet temp - actual outlet temp)
        // DX Coil output depends on the part load ratio which is being varied to zero the residual.

        // METHODOLOGY EMPLOYED:
        // Calls CalcDoe2DXCoil to get outlet temperature at the given cycling ratio
        // and calculates the residual as defined above

        // Argument array dimensioning
        // Par(2) = desired air outlet temperature [C]

        int CoilIndex = int(Par(1));
        Real64 OnOffAirFlowFrac = Par(3); // Ratio of compressor ON to compressor OFF air mass flow rate

        DXCoils::CalcDXHeatingCoil(state, CoilIndex, PartLoadFrac, DataHVACGlobals::ContFanCycCoil, OnOffAirFlowFrac);

        Real64 OutletAirTemp = DXCoils::DXCoilOutletTemp(CoilIndex);
        Real64 Residuum = Par(2) - OutletAirTemp;

        return Residuum;
    }

    Real64 MultiModeDXCoilResidual(EnergyPlusData &state,
                                   Real64 const PartLoadRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                   Array1D<Real64> const &Par  // Par(1) = DX coil number
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         M. J. Witte, GARD Analytics, Inc.
        //       DATE WRITTEN   February 2005
        //                      (based on DOE2DXCoilResidual by Richard Raustad, FSEC)

        // PURPOSE OF THIS FUNCTION:
        // Calculates residual function (desired outlet temp - actual outlet temp)
        // DX Coil output depends on the part load ratio which is being varied to zero the residual.

        // METHODOLOGY EMPLOYED:
        // Calls SimDXCoilMultiMode to get outlet temperature at the given cycling ratio
        // and calculates the residual as defined above

        // Argument array dimensioning
        // par(2) = desired air outlet temperature [C]
        // par(3) = dehumidification mode (0=normal, 1=enhanced)

        int CoilIndex = int(Par(1));
        int DehumidMode = int(Par(3));
        int FanOpMode = 2;
        DXCoils::SimDXCoilMultiMode(state, "", On, false, PartLoadRatio, DehumidMode, CoilIndex, FanOpMode);
        Real64 OutletAirTemp = DXCoils::DXCoilOutletTemp(CoilIndex);
        Real64 Residuum = Par(2) - OutletAirTemp;

        return Residuum;
    }

    void SetAverageAirFlow(EnergyPlusData &state,
                           int const CBVAVNum,       // Index to CBVAV system
                           Real64 &OnOffAirFlowRatio // Ratio of compressor ON airflow to average airflow over timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Set the average air mass flow rates for this time step
        // Set OnOffAirFlowRatio to be used by DX coils

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneMassFlow; // Zone mass flow rate required to meet zone load [kg/s]
        Real64 ZoneLoad;     // Zone load calculated by ZoneTempPredictor [W]

        int InletNode = CBVAV(CBVAVNum).AirInNode;                     // Inlet node number for CBVAVNum
        int OutletNode = CBVAV(CBVAVNum).AirOutNode;                   // Outlet node number for CBVAVNum
        int MixerMixedAirNode = CBVAV(CBVAVNum).MixerMixedAirNode;     // Mixed air node number in OA mixer
        int MixerOutsideAirNode = CBVAV(CBVAVNum).MixerOutsideAirNode; // Outside air node number in OA mixer
        int MixerReliefAirNode = CBVAV(CBVAVNum).MixerReliefAirNode;   // Relief air node number in OA mixer
        int MixerInletAirNode = CBVAV(CBVAVNum).MixerInletAirNode;     // Inlet air node number in OA mixer

        Real64 SystemMassFlow = 0.0; // System mass flow rate required for all zones [kg/s]
        Real64 CpSupplyAir = Psychrometrics::PsyCpAirFnW(DataLoopNode::Node(OutletNode).HumRat); // Specific heat of outlet air [J/kg-K]
        // Determine zone air flow
        for (int ZoneNum = 1; ZoneNum <= CBVAV(CBVAVNum).NumControlledZones; ++ZoneNum) {
            int ZoneNodeNum = CBVAV(CBVAVNum).ActualZoneNodeNum(ZoneNum);
            int BoxOutletNodeNum = CBVAV(CBVAVNum).CBVAVBoxOutletNode(ZoneNum); // Zone supply air inlet node number
            if ((CBVAV(CBVAVNum).ZoneSequenceCoolingNum(ZoneNum) > 0) && (CBVAV(CBVAVNum).ZoneSequenceHeatingNum(ZoneNum) > 0)) {
                Real64 QToCoolSetPt = DataZoneEnergyDemands::ZoneSysEnergyDemand(CBVAV(CBVAVNum).ControlledZoneNum(ZoneNum))
                                          .SequencedOutputRequiredToCoolingSP(CBVAV(CBVAVNum).ZoneSequenceCoolingNum(ZoneNum));
                Real64 QToHeatSetPt = DataZoneEnergyDemands::ZoneSysEnergyDemand(CBVAV(CBVAVNum).ControlledZoneNum(ZoneNum))
                                          .SequencedOutputRequiredToHeatingSP(CBVAV(CBVAVNum).ZoneSequenceHeatingNum(ZoneNum));
                if (QToHeatSetPt > 0.0 && QToCoolSetPt > 0.0) {
                    ZoneLoad = QToHeatSetPt;
                } else if (QToHeatSetPt < 0.0 && QToCoolSetPt < 0.0) {
                    ZoneLoad = QToCoolSetPt;
                } else if (QToHeatSetPt <= 0.0 && QToCoolSetPt >= 0.0) {
                    ZoneLoad = 0.0;
                }
            } else {
                ZoneLoad = DataZoneEnergyDemands::ZoneSysEnergyDemand(CBVAV(CBVAVNum).ControlledZoneNum(ZoneNum)).RemainingOutputRequired;
            }
            Real64 CpZoneAir = Psychrometrics::PsyCpAirFnW(DataLoopNode::Node(ZoneNodeNum).HumRat);
            Real64 DeltaCpTemp = CpSupplyAir * DataLoopNode::Node(OutletNode).Temp - CpZoneAir * DataLoopNode::Node(ZoneNodeNum).Temp;

            // Need to check DeltaCpTemp and ensure that it is not zero
            if (DeltaCpTemp != 0.0) { // .AND. .NOT. CurDeadBandOrSetback(ZoneNum))THEN
                ZoneMassFlow = ZoneLoad / DeltaCpTemp;
            } else {
                //     reset to 0 so we don't add in the last zone's mass flow rate
                ZoneMassFlow = 0.0;
            }
            SystemMassFlow +=
                max(DataLoopNode::Node(BoxOutletNodeNum).MassFlowRateMin, min(ZoneMassFlow, DataLoopNode::Node(BoxOutletNodeNum).MassFlowRateMax));
        }

        Real64 AverageUnitMassFlow = CompOnMassFlow;
        Real64 AverageOAMassFlow = OACompOnMassFlow;
        FanSpeedRatio = CompOnFlowRatio;

        DataLoopNode::Node(MixerInletAirNode) = DataLoopNode::Node(InletNode);

        DataLoopNode::Node(MixerMixedAirNode).MassFlowRateMin = 0.0;

        if (ScheduleManager::GetCurrentScheduleValue(state, CBVAV(CBVAVNum).SchedPtr) == 0.0 || AverageUnitMassFlow == 0.0) {
            DataLoopNode::Node(InletNode).MassFlowRate = 0.0;
            DataLoopNode::Node(MixerOutsideAirNode).MassFlowRate = 0.0;
            DataLoopNode::Node(MixerReliefAirNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 0.0;
            BypassDuctFlowFraction = 0.0;
        } else {
            DataLoopNode::Node(MixerInletAirNode).MassFlowRate = AverageUnitMassFlow;
            DataLoopNode::Node(MixerOutsideAirNode).MassFlowRate = AverageOAMassFlow;
            DataLoopNode::Node(MixerReliefAirNode).MassFlowRate = AverageOAMassFlow;
            OnOffAirFlowRatio = 1.0;
            auto &cbVAVBoxOut(CBVAV(CBVAVNum).CBVAVBoxOutletNode);
            Real64 boxOutletNodeFlow = 0.0;
            for (int i = 1; i <= CBVAV(CBVAVNum).NumControlledZones; ++i) {
                boxOutletNodeFlow += DataLoopNode::Node(cbVAVBoxOut(i)).MassFlowRate;
            }
            BypassDuctFlowFraction = max(0.0, 1.0 - (boxOutletNodeFlow / AverageUnitMassFlow));
        }
    }

    void ReportCBVAV(EnergyPlusData &state, int const CBVAVNum) // Index of the current CBVAV unit being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Fills some of the report variables for the changeover-bypass VAV system

        Real64 ReportingConstant = DataHVACGlobals::TimeStepSys * DataGlobalConstants::SecInHour();

        CBVAV(CBVAVNum).TotCoolEnergy = CBVAV(CBVAVNum).TotCoolEnergyRate * ReportingConstant;
        CBVAV(CBVAVNum).TotHeatEnergy = CBVAV(CBVAVNum).TotHeatEnergyRate * ReportingConstant;
        CBVAV(CBVAVNum).SensCoolEnergy = CBVAV(CBVAVNum).SensCoolEnergyRate * ReportingConstant;
        CBVAV(CBVAVNum).SensHeatEnergy = CBVAV(CBVAVNum).SensHeatEnergyRate * ReportingConstant;
        CBVAV(CBVAVNum).LatCoolEnergy = CBVAV(CBVAVNum).LatCoolEnergyRate * ReportingConstant;
        CBVAV(CBVAVNum).LatHeatEnergy = CBVAV(CBVAVNum).LatHeatEnergyRate * ReportingConstant;
        CBVAV(CBVAVNum).ElecConsumption = CBVAV(CBVAVNum).ElecPower * ReportingConstant;

        if (CBVAV(CBVAVNum).FirstPass) {
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(DataSizing::CurZoneEqNum, DataSizing::CurSysNum, CBVAV(CBVAVNum).FirstPass);
            }
        }

        // reset to 1 in case blow through fan configuration (fan resets to 1, but for blow thru fans coil sets back down < 1)
        DataHVACGlobals::OnOffFanPartLoadFraction = 1.0;
    }

    void CalcNonDXHeatingCoils(EnergyPlusData &state,
                               int const CBVAVNum,            // Changeover bypass VAV unit index
                               bool const FirstHVACIteration, // flag for first HVAC iteration in the time step
                               Real64 &HeatCoilLoad,          // heating coil load to be met (Watts)
                               int const FanMode,             // fan operation mode
                               Real64 &HeatCoilLoadmet        // coil heating load met
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   January 2012

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the four non dx heating coil types: Gas, Electric, hot water and steam.

        // METHODOLOGY EMPLOYED:
        // Simply calls the different heating coil component.  The hot water flow rate matching the coil load
        // is calculated iteratively.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 const ErrTolerance(0.001); // convergence limit for hotwater coil
        int const SolveMaxIter(50);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 mdot;            // heating coil steam or hot water mass flow rate
        Real64 MinWaterFlow;    // minimum water mass flow rate
        Real64 MaxHotWaterFlow; // maximum hot water mass flow rate, kg/s
        Real64 HotWaterMdot;    // actual hot water mass flow rate
        Array1D<Real64> Par(3);
        int SolFlag; // error flag

        Real64 QCoilActual = 0.0; // actual heating load met
        if (HeatCoilLoad > DataHVACGlobals::SmallLoad) {
            {
                auto const SELECT_CASE_var(CBVAV(CBVAVNum).HeatCoilType_Num);
                if ((SELECT_CASE_var == DataHVACGlobals::Coil_HeatingGasOrOtherFuel) || (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingElectric)) {
                    HeatingCoils::SimulateHeatingCoilComponents(state,
                                                                CBVAV(CBVAVNum).HeatCoilName,
                                                                FirstHVACIteration,
                                                                HeatCoilLoad,
                                                                CBVAV(CBVAVNum).HeatCoilIndex,
                                                                QCoilActual,
                                                                true,
                                                                FanMode);
                } else if (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingWater) {
                    // simulate the heating coil at maximum hot water flow rate
                    MaxHotWaterFlow = CBVAV(CBVAVNum).MaxHeatCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow,
                                                         CBVAV(CBVAVNum).CoilControlNode,
                                                         CBVAV(CBVAVNum).CoilOutletNode,
                                                         CBVAV(CBVAVNum).LoopNum,
                                                         CBVAV(CBVAVNum).LoopSide,
                                                         CBVAV(CBVAVNum).BranchNum,
                                                         CBVAV(CBVAVNum).CompNum);
                    WaterCoils::SimulateWaterCoilComponents(
                        state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACIteration, CBVAV(CBVAVNum).HeatCoilIndex, QCoilActual, FanMode);
                    if (QCoilActual > (HeatCoilLoad + DataHVACGlobals::SmallLoad)) {
                        // control water flow to obtain output matching HeatCoilLoad
                        SolFlag = 0;
                        MinWaterFlow = 0.0;
                        Par(1) = double(CBVAVNum);
                        if (FirstHVACIteration) {
                            Par(2) = 1.0;
                        } else {
                            Par(2) = 0.0;
                        }
                        Par(3) = HeatCoilLoad;
                        TempSolveRoot::SolveRoot(
                            state, ErrTolerance, SolveMaxIter, SolFlag, HotWaterMdot, HotWaterCoilResidual, MinWaterFlow, MaxHotWaterFlow, Par);
                        if (SolFlag == -1) {
                            if (CBVAV(CBVAVNum).HotWaterCoilMaxIterIndex == 0) {
                                ShowWarningMessage(state, "CalcNonDXHeatingCoils: Hot water coil control failed for " + CBVAV(CBVAVNum).UnitType + "=\"" +
                                                   CBVAV(CBVAVNum).Name + "\"");
                                ShowContinueErrorTimeStamp(state, "");
                                ShowContinueError(state,
                                                  format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}",
                                       SolveMaxIter,
                                       CBVAV(CBVAVNum).UnitType,
                                       CBVAV(CBVAVNum).Name),
                                CBVAV(CBVAVNum).HotWaterCoilMaxIterIndex);
                        } else if (SolFlag == -2) {
                            if (CBVAV(CBVAVNum).HotWaterCoilMaxIterIndex2 == 0) {
                                ShowWarningMessage(state, "CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for " +
                                                   CBVAV(CBVAVNum).UnitType + "=\"" + CBVAV(CBVAVNum).Name + "\"");
                                ShowContinueErrorTimeStamp(state, "");
                                ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                                ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinWaterFlow));
                                ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                            }
                            ShowRecurringWarningErrorAtEnd(state, "CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for " +
                                                               CBVAV(CBVAVNum).UnitType + "=\"" + CBVAV(CBVAVNum).Name + "\"",
                                                           CBVAV(CBVAVNum).HotWaterCoilMaxIterIndex2,
                                                           MaxHotWaterFlow,
                                                           MinWaterFlow,
                                                           _,
                                                           "[kg/s]",
                                                           "[kg/s]");
                        }
                        // simulate the hot water heating coil
                        QCoilActual = HeatCoilLoad;
                        // simulate the hot water heating coil
                        WaterCoils::SimulateWaterCoilComponents(
                            state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACIteration, CBVAV(CBVAVNum).HeatCoilIndex, QCoilActual, FanMode);
                    }
                } else if (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingSteam) {
                    mdot = CBVAV(CBVAVNum).MaxHeatCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(state, mdot,
                                                         CBVAV(CBVAVNum).CoilControlNode,
                                                         CBVAV(CBVAVNum).CoilOutletNode,
                                                         CBVAV(CBVAVNum).LoopNum,
                                                         CBVAV(CBVAVNum).LoopSide,
                                                         CBVAV(CBVAVNum).BranchNum,
                                                         CBVAV(CBVAVNum).CompNum);

                    // simulate the steam heating coil
                    SteamCoils::SimulateSteamCoilComponents(
                        state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACIteration, CBVAV(CBVAVNum).HeatCoilIndex, HeatCoilLoad, QCoilActual, FanMode);
                }
            }
        } else {
            {
                auto const SELECT_CASE_var(CBVAV(CBVAVNum).HeatCoilType_Num);
                if ((SELECT_CASE_var == DataHVACGlobals::Coil_HeatingGasOrOtherFuel) || (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingElectric)) {
                    HeatingCoils::SimulateHeatingCoilComponents(state,
                                                                CBVAV(CBVAVNum).HeatCoilName,
                                                                FirstHVACIteration,
                                                                HeatCoilLoad,
                                                                CBVAV(CBVAVNum).HeatCoilIndex,
                                                                QCoilActual,
                                                                true,
                                                                FanMode);
                } else if (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingWater) {
                    mdot = 0.0;
                    PlantUtilities::SetComponentFlowRate(state, mdot,
                                                         CBVAV(CBVAVNum).CoilControlNode,
                                                         CBVAV(CBVAVNum).CoilOutletNode,
                                                         CBVAV(CBVAVNum).LoopNum,
                                                         CBVAV(CBVAVNum).LoopSide,
                                                         CBVAV(CBVAVNum).BranchNum,
                                                         CBVAV(CBVAVNum).CompNum);
                    QCoilActual = HeatCoilLoad;
                    // simulate the hot water heating coil
                    WaterCoils::SimulateWaterCoilComponents(
                        state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACIteration, CBVAV(CBVAVNum).HeatCoilIndex, QCoilActual, FanMode);
                } else if (SELECT_CASE_var == DataHVACGlobals::Coil_HeatingSteam) {
                    mdot = 0.0;
                    PlantUtilities::SetComponentFlowRate(state, mdot,
                                                         CBVAV(CBVAVNum).CoilControlNode,
                                                         CBVAV(CBVAVNum).CoilOutletNode,
                                                         CBVAV(CBVAVNum).LoopNum,
                                                         CBVAV(CBVAVNum).LoopSide,
                                                         CBVAV(CBVAVNum).BranchNum,
                                                         CBVAV(CBVAVNum).CompNum);
                    // simulate the steam heating coil
                    SteamCoils::SimulateSteamCoilComponents(
                        state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACIteration, CBVAV(CBVAVNum).HeatCoilIndex, HeatCoilLoad, QCoilActual, FanMode);
                }
            }
        }
        HeatCoilLoadmet = QCoilActual;
    }

    Real64 HotWaterCoilResidual(EnergyPlusData &state,
                                Real64 const HWFlow,       // hot water flow rate in kg/s
                                Array1D<Real64> const &Par // Par(1) = DX coil number
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   January 2012

        // PURPOSE OF THIS FUNCTION:
        // Calculates residual function (Actual Coil Output - Requested Coil Load) / Requested Coil Load
        // the actual coil output depends on the hot water flow rate which is varied to minimize the residual.

        // METHODOLOGY EMPLOYED:
        // Calls HotWaterCoilResidual, and calculates the residual as defined above.

        // Return value
        Real64 Residuum; // residual to be minimized to zero

        int CBVAVNum = int(Par(1));
        bool FirstHVACSoln = (Par(2) > 0.0);
        Real64 HeatCoilLoad = Par(3);
        Real64 QCoilActual = HeatCoilLoad;
        Real64 mdot = HWFlow;
        PlantUtilities::SetComponentFlowRate(state, mdot,
                                             CBVAV(CBVAVNum).CoilControlNode,
                                             CBVAV(CBVAVNum).CoilOutletNode,
                                             CBVAV(CBVAVNum).LoopNum,
                                             CBVAV(CBVAVNum).LoopSide,
                                             CBVAV(CBVAVNum).BranchNum,
                                             CBVAV(CBVAVNum).CompNum);

        // simulate the hot water supplemental heating coil
        WaterCoils::SimulateWaterCoilComponents(
            state, CBVAV(CBVAVNum).HeatCoilName, FirstHVACSoln, CBVAV(CBVAVNum).HeatCoilIndex, QCoilActual, CBVAV(CBVAVNum).OpMode);
        if (HeatCoilLoad != 0.0) {
            Residuum = (QCoilActual - HeatCoilLoad) / HeatCoilLoad;
        } else { // Autodesk:Return Condition added to assure return value is set
            Residuum = 0.0;
        }
        return Residuum;
    }

} // namespace HVACUnitaryBypassVAV

} // namespace EnergyPlus
