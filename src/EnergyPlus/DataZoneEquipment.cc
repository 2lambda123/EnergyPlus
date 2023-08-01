// EnergyPlus, Copyright (c) 1996-2023, The Board of Trustees of the University of Illinois,
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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::DataZoneEquipment {

// MODULE INFORMATION
//             AUTHOR:  Russ Taylor
//       DATE WRITTEN:  June 1998

// PURPOSE OF THIS MODULE:
// This module contains variable declarations for zone equipment configuration data

Array1D_string const cValidSysAvailManagerCompTypes(NumValidSysAvailZoneComponents,
                                                    {"ZoneHVAC:FourPipeFanCoil",
                                                     "ZoneHVAC:PackagedTerminalHeatPump",
                                                     "ZoneHVAC:PackagedTerminalAirConditioner",
                                                     "ZoneHVAC:WaterToAirHeatPump",
                                                     "ZoneHVAC:WindowAirConditioner",
                                                     "ZoneHVAC:UnitHeater",
                                                     "ZoneHVAC:UnitVentilator",
                                                     "ZoneHVAC:EnergyRecoveryVentilator",
                                                     "ZoneHVAC:VentilatedSlab",
                                                     "ZoneHVAC:OutdoorAirUnit",
                                                     "ZoneHVAC:TerminalUnit:VariableRefrigerantFlow",
                                                     "ZoneHVAC:IdealLoadsAirSystem",
                                                     "ZoneHVAC:EvaporativeCoolerUnit",
                                                     "ZoneHVAC:HybridUnitaryHVAC"});

constexpr std::array<std::string_view, static_cast<int>(ZoneEquipType::Num)> zoneEquipTypeNamesUC = {
    "DUMMY", // DUMMY,

    "ZONEHVAC:FOURPIPEFANCOIL",                      // FanCoilFourPipe
    "ZONEHVAC:PACKAGEDTERMINALHEATPUMP",             //   PackagedTerminalHeatPump
    "ZONEHVAC:PACKAGEDTERMINALAIRCONDITIONER",       //  PackagedTerminalAirConditioner
    "ZONEHVAC:WATERTOAIRHEATPUMP",                   //   PackagedTerminalHeatPumpWaterToAir
    "ZONEHVAC:WINDOWAIRCONDITIONER",                 //  WindowAirConditioner
    "ZONEHVAC:UNITHEATER",                           //  UnitHeater
    "ZONEHVAC:UNITVENTILATOR",                       //  UnitVentilator
    "ZONEHVAC:ENERGYRECOVERYVENTILATOR",             // EnergyRecoveryVentilator
    "ZONEHVAC:VENTILATEDSLAB",                       // VentilatedSlab
    "ZONEHVAC:OUTDOORAIRUNIT",                       // OutdoorAirUnit
    "ZONEHVAC:TERMINALUNIT:VARIABLEREFRIGERANTFLOW", //  VariableRefrigerantFlowTerminal
    "ZONEHVAC:IDEALLOADSAIRSYSTEM",                  // IdealLoadsAirSystem
    "ZONEHVAC:EVAPORATIVECOOLERUNIT",                // EvaporativeCooler
    "ZONEHVAC:HYBRIDUNITARYHVAC",                    // HybridEvaporativeCooler,

    // last zone equipment type to use zone availability manager. The above list must not change or
    // NumValidSysAvailZoneComponents must also change.

    "ZONEHVAC:AIRDISTRIBUTIONUNIT",                            // AirDistributionUnit
    "ZONEHVAC:BASEBOARD:CONVECTIVE:WATER",                     // BaseboardWaterConvective
    "ZONEHVAC:BASEBOARD:CONVECTIVE:ELECTRIC",                  // BaseboardElectricConvective
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:STEAM",              // BaseboardSteam
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:WATER",              // BaseboardWater
    "ZONEHVAC:BASEBOARD:RADIANTCONVECTIVE:ELECTRIC",           // BaseboardElectric
    "ZONEHVAC:HIGHTEMPERATURERADIANT",                         // HighTempRadiant
    "ZONEHVAC:LOWTEMPERATURERADIANT:VARIABLEFLOW",             //  LowTempRadiant
    "FAN:ZONEEXHAUST",                                         // ExhaustFan
    "HEATEXCHANGER:AIRTOAIR:FLATPLATE",                        // HeatExchanger
    "WATERHEATER:HEATPUMP:PUMPEDCONDENSER",                    //  HeatPumpWaterHeater
    "ZONEHVAC:DEHUMIDIFIER:DX",                                //  DXDehumidifier
    "ZONEHVAC:REFRIGERATIONCHILLERSET",                        // RefrigerationAirChillerSet
    "ZONEHVAC:FORCEDAIR:USERDEFINED",                          // UserDefinedVACForcedAir
    "ZONEHVAC:COOLINGPANEL:RADIANTCONVECTIVE:WATER",           // CoolingPanel
    "AIRLOOPHVAC:UNITARYSYSTEM",                               // UnitarySystem
    "AIRTERMINAL:DUALDUCT:CONSTANTVOLUME",                     // AirTerminalDualDuctConstantVolume
    "AIRTERMINAL:DUALDUCT:VAV",                                // AirTerminalDualDuctVAV
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:REHEAT",            // AirTerminalSingleDuctConstantVolumeReheat
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:NOREHEAT",          // AirTerminalSingleDuctConstantVolumeNoReheat
    "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT",                       // AirTerminalSingleDuctVAVReheat
    "AIRTERMINAL:SINGLEDUCT:VAV:NOREHEAT",                     // AirTerminalSingleDuctVAVNoReheat
    "AIRTERMINAL:SINGLEDUCT:SERIESPIU:REHEAT",                 // AirTerminalSingleDuctSeriesPIUReheat
    "AIRTERMINAL:SINGLEDUCT:PARALLELPIU:REHEAT",               // AirTerminalSingleDuctParallelPIUReheat
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:FOURPIPEINDUCTION", // AirTerminalSingleDuctCAVFourPipeInduction
    "AIRTERMINAL:SINGLEDUCT:VAV:REHEAT:VARIABLESPEEDFAN",      // AirTerminalSingleDuctVAVReheatVariableSpeedFan
    "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:REHEAT",           // AirTerminalSingleDuctVAVHeatAndCoolReheat
    "AIRTERMINAL:SINGLEDUCT:VAV:HEATANDCOOL:NOREHEAT",         // AirTerminalSingleDuctVAVHeatAndCoolNoReheat
    "AIRTERMINAL:SINGLEDUCT:CONSTANTVOLUME:COOLEDBEAM",        // AirTerminalSingleDuctConstantVolumeCooledBeam
    "AIRTERMINAL:DUALDUCT:VAV:OUTDOORAIR",                     // AirTerminalDualDuctVAVOutdoorAir
    "AIRLOOPHVACRETURNAIR"                                     // AirLoopHVACReturnAir
};

static constexpr std::array<std::string_view, static_cast<int>(LoadDist::Num)> LoadDistNamesUC = {
    "SEQUENTIALLOAD", "UNIFORMLOAD", "UNIFORMPLR", "SEQUENTIALUNIFORMPLR"};

void GetZoneEquipmentData(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   June 1997
    //       MODIFIED       Aug 2003, FCW: set ZoneEquipConfig number for each zone

    // PURPOSE OF THIS SUBROUTINE:
    // Get all the system related equipment which may be attached to
    // a zone

    // Using/Aliasing
    using NodeInputManager::CheckUniqueNodeNames;
    using NodeInputManager::CheckUniqueNodeNumbers;
    using NodeInputManager::EndUniqueNodeCheck;
    using NodeInputManager::GetNodeNums;
    using NodeInputManager::GetOnlySingleNode;
    using NodeInputManager::InitUniqueNodeCheck;
    using namespace DataHVACGlobals;
    using namespace DataLoopNode;
    using namespace ScheduleManager;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetZoneEquipmentData: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;
    int NumNums;
    int NodeNum;
    int IOStat;
    Array1D_string AlphArray;
    Array1D<Real64> NumArray;
    int MaxAlphas;
    int MaxNums;
    int NumParams;
    int NumNodes;
    Array1D_int NodeNums;
    bool IsNotOK; // Flag to verify nam
    bool NodeListError;
    bool UniqueNodeError;
    std::string CurrentModuleObject; // Object type for getting and error messages
    Array1D_string cAlphaFields;     // Alpha field names
    Array1D_string cNumericFields;   // Numeric field names
    Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.

    struct EquipListAudit
    {
        // Members
        std::string ObjectType;
        std::string ObjectName;
        int OnListNum;

        // Default Constructor
        EquipListAudit() : OnListNum(0)
        {
        }
    };
    // Object Data
    Array1D<EquipListAudit> ZoneEquipListAcct;

    // Look in the input file for zones with air loop and zone equipment attached

    int numControlledZones = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneHVAC:EquipmentConnections");
    int numControlledSpaces = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "SpaceHVAC:EquipmentConnections");
    state.dataZoneEquip->NumOfZoneEquipLists = state.dataInputProcessing->inputProcessor->getNumObjectsFound(
        state, "ZoneHVAC:EquipmentList"); // Look for lists of equipment data - there should
    // be as many of these as there are controlled zones
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "NodeList", NumParams, NumAlphas, NumNums);
    NodeNums.dimension(NumParams, 0);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "ZoneHVAC:EquipmentList", NumParams, NumAlphas, NumNums);
    MaxAlphas = NumAlphas;
    MaxNums = NumNums;
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "ZoneHVAC:EquipmentConnections", NumParams, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirLoopHVAC:SupplyPath", NumParams, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "AirLoopHVAC:ReturnPath", NumParams, NumAlphas, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);
    MaxNums = max(MaxNums, NumNums);
    AlphArray.allocate(MaxAlphas);
    NumArray.dimension(MaxNums, 0.0);
    cAlphaFields.allocate(MaxAlphas);
    cNumericFields.allocate(MaxNums);
    lAlphaBlanks.dimension(MaxAlphas, true);
    lNumericBlanks.dimension(MaxNums, true);

    if (!allocated(state.dataZoneEquip->SupplyAirPath)) {
        // Look for and read in the air supply path
        // component (splitters) information for each zone
        state.dataZoneEquip->NumSupplyAirPaths = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:SupplyPath");
        state.dataZoneEquip->SupplyAirPath.allocate(state.dataZoneEquip->NumSupplyAirPaths);
    }

    if (!allocated(state.dataZoneEquip->ReturnAirPath)) {
        // Look for and read in the air return path
        // component (mixers & plenums) information for each zone
        state.dataZoneEquip->NumReturnAirPaths = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirLoopHVAC:ReturnPath");
        state.dataZoneEquip->ReturnAirPath.allocate(state.dataZoneEquip->NumReturnAirPaths);
    }

    state.dataZoneEquip->ZoneEquipConfig.allocate(state.dataGlobal->NumOfZones); // Allocate the array containing the configuration
    if (state.dataHeatBal->doSpaceHeatBalanceSizing || state.dataHeatBal->doSpaceHeatBalanceSimulation) {
        state.dataZoneEquip->spaceEquipConfig.allocate(state.dataGlobal->numSpaces); // Allocate the array containing the configuration
    }
    // data for each zone to the number of controlled zones
    // found in the input file.  This may or may not
    // be the same as the number of zones in the building
    state.dataZoneEquip->ZoneEquipList.allocate(state.dataGlobal->NumOfZones);
    state.dataZoneEquip->ZoneEquipAvail.dimension(state.dataGlobal->NumOfZones, NoAction);
    state.dataZoneEquip->UniqueZoneEquipListNames.reserve(state.dataGlobal->NumOfZones);

    if (state.dataZoneEquip->NumOfZoneEquipLists != numControlledZones) {
        ShowSevereError(state,
                        format("{}Number of Zone Equipment lists [{}] not equal Number of Controlled Zones [{}]",
                               RoutineName,
                               state.dataZoneEquip->NumOfZoneEquipLists,
                               numControlledZones));
        ShowContinueError(state, "..Each Controlled Zone [ZoneHVAC:EquipmentConnections] must have a corresponding (unique) ZoneHVAC:EquipmentList");
        ShowFatalError(state, "GetZoneEquipment: Incorrect number of zone equipment lists");
    }

    if (numControlledZones > state.dataGlobal->NumOfZones) {
        ShowSevereError(state,
                        format("{}Number of Controlled Zone objects [{}] greater than Number of Zones [{}]",
                               RoutineName,
                               numControlledZones,
                               state.dataGlobal->NumOfZones));
        ShowFatalError(state, format("{}Too many ZoneHVAC:EquipmentConnections objects.", RoutineName));
    }

    InitUniqueNodeCheck(state, "ZoneHVAC:EquipmentConnections");

    int overallEquipCount = 0;
    int locTermUnitSizingCounter = 0; // will increment for every zone inlet node

    // auto &Zone(state.dataHeatBal->Zone);

    for (int controlledZoneLoop = 1; controlledZoneLoop <= numControlledZones; ++controlledZoneLoop) {
        CurrentModuleObject = "ZoneHVAC:EquipmentConnections";
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 controlledZoneLoop,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); // Get Equipment | data for one zone

        int zoneOrSpaceNum = UtilityRoutines::FindItemInList(AlphArray(1), state.dataHeatBal->Zone);
        std::string_view zsString = "Zone";

        if (zoneOrSpaceNum == 0) {
            ShowSevereError(state, format("{}{}: {}=\"{}\"", RoutineName, CurrentModuleObject, cAlphaFields(1), AlphArray(1)));
            ShowContinueError(state,
                              format("..Requested Controlled {} not among {}s, remaining items for this object not processed.", zsString, zsString));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        bool isControlledAlready = state.dataHeatBal->Zone(zoneOrSpaceNum).IsControlled;
        if (isControlledAlready) {
            ShowSevereError(state, format("{}{}: {}=\"{}\"", RoutineName, CurrentModuleObject, cAlphaFields(1), AlphArray(1)));
            ShowContinueError(state,
                              format("..Duplicate Controlled {} entered, only one {} per {} is allowed.", zsString, CurrentModuleObject, zsString));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        state.dataHeatBal->Zone(zoneOrSpaceNum).IsControlled = true;
        bool isSpace = false;
        processZoneEquipmentInput(state,
                                  CurrentModuleObject,
                                  zoneOrSpaceNum,
                                  isSpace,
                                  locTermUnitSizingCounter,
                                  overallEquipCount,
                                  state.dataZoneEquip->ZoneEquipConfig(zoneOrSpaceNum),
                                  AlphArray,
                                  NumArray,
                                  cAlphaFields,
                                  cNumericFields,
                                  lAlphaBlanks,
                                  lNumericBlanks,
                                  NodeNums);
        state.dataHeatBal->Zone(zoneOrSpaceNum).SystemZoneNodeNumber = state.dataZoneEquip->ZoneEquipConfig(zoneOrSpaceNum).ZoneNode;
    } // end loop over controlled zones
    for (int controlledSpaceLoop = 1; controlledSpaceLoop <= numControlledSpaces; ++controlledSpaceLoop) {
        CurrentModuleObject = "SpaceHVAC:EquipmentConnections";
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 controlledSpaceLoop,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); // Get Equipment | data for one zone

        int zoneOrSpaceNum = UtilityRoutines::FindItemInList(AlphArray(1), state.dataHeatBal->Zone);
        std::string_view zsString = "Space";

        if (zoneOrSpaceNum == 0) {
            ShowSevereError(state, format("{}{}: {}=\"{}\"", RoutineName, CurrentModuleObject, cAlphaFields(1), AlphArray(1)));
            ShowContinueError(state,
                              format("..Requested Controlled {} not among {}s, remaining items for this object not processed.", zsString, zsString));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        bool isControlledAlready = state.dataHeatBal->space(zoneOrSpaceNum).IsControlled;
        if (isControlledAlready) {
            ShowSevereError(state, format("{}{}: {}=\"{}\"", RoutineName, CurrentModuleObject, cAlphaFields(1), AlphArray(1)));
            ShowContinueError(state,
                              format("..Duplicate Controlled {} entered, only one {} per {} is allowed.", zsString, CurrentModuleObject, zsString));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            continue;
        }
        state.dataHeatBal->space(zoneOrSpaceNum).IsControlled = true;
        bool isSpace = true;
        processZoneEquipmentInput(state,
                                  CurrentModuleObject,
                                  zoneOrSpaceNum,
                                  isSpace,
                                  locTermUnitSizingCounter,
                                  overallEquipCount,
                                  state.dataZoneEquip->spaceEquipConfig(zoneOrSpaceNum),
                                  AlphArray,
                                  NumArray,
                                  cAlphaFields,
                                  cNumericFields,
                                  lAlphaBlanks,
                                  lNumericBlanks,
                                  NodeNums);
        state.dataHeatBal->space(zoneOrSpaceNum).SystemZoneNodeNumber = state.dataZoneEquip->spaceEquipConfig(zoneOrSpaceNum).ZoneNode;
    } // end loop over controlled spaces

    if (state.dataHeatBal->doSpaceHeatBalanceSizing || state.dataHeatBal->doSpaceHeatBalanceSimulation) {
        // Auto-assign the system node name for spaces that do not have a SpaceHVAC:EquipmentConnections input
        for (auto &thisZone : state.dataHeatBal->Zone) {
            int spaceCount = 0;
            for (int spaceNum : thisZone.spaceIndexes) {
                ++spaceCount;
                if (state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber == 0) {
                }
                std::string spaceNodeName = format("{}-Space {}", AlphArray(5), spaceCount);
                int spaceNodeNum = GetOnlySingleNode(state,
                                                     spaceNodeName,
                                                     state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                     DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections,
                                                     AlphArray(1),
                                                     DataLoopNode::NodeFluidType::Air,
                                                     DataLoopNode::ConnectionType::ZoneNode,
                                                     NodeInputManager::CompFluidStream::Primary,
                                                     DataLoopNode::ObjectIsNotParent);
                state.dataHeatBal->space(spaceNum).SystemZoneNodeNumber = spaceNodeNum;
            }
        }
    }
    // Allocate TermUnitSizing array and set zone number
    if (locTermUnitSizingCounter > 0) {
        state.dataSize->NumAirTerminalUnits = locTermUnitSizingCounter;
        state.dataSize->TermUnitSizing.allocate(state.dataSize->NumAirTerminalUnits);
        for (int loopZoneNum = 1; loopZoneNum <= state.dataGlobal->NumOfZones; ++loopZoneNum) {
            {
                auto &thisZoneEqConfig = state.dataZoneEquip->ZoneEquipConfig(loopZoneNum);
                for (int loopNodeNum = 1; loopNodeNum <= thisZoneEqConfig.NumInletNodes; ++loopNodeNum) {
                    state.dataSize->TermUnitSizing(thisZoneEqConfig.AirDistUnitCool(loopNodeNum).TermUnitSizingIndex).CtrlZoneNum = loopZoneNum;
                }
            }
        }
    }
    if (state.dataZoneEquip->GetZoneEquipmentDataErrorsFound) {
        ShowWarningError(state, format("{}{}, duplicate items NOT CHECKED due to previous errors.", RoutineName, CurrentModuleObject));
        overallEquipCount = 0;
    }
    if (overallEquipCount > 0) {
        ZoneEquipListAcct.allocate(overallEquipCount);
        overallEquipCount = 0;
        for (int Loop1 = 1; Loop1 <= numControlledZones; ++Loop1) {
            for (int Loop2 = 1; Loop2 <= state.dataZoneEquip->ZoneEquipList(Loop1).NumOfEquipTypes; ++Loop2) {
                ++overallEquipCount;
                ZoneEquipListAcct(overallEquipCount).ObjectType = state.dataZoneEquip->ZoneEquipList(Loop1).EquipType(Loop2);
                ZoneEquipListAcct(overallEquipCount).ObjectName = state.dataZoneEquip->ZoneEquipList(Loop1).EquipName(Loop2);
                ZoneEquipListAcct(overallEquipCount).OnListNum = Loop1;
            }
        }
        // Now check for uniqueness
        for (int Loop1 = 1; Loop1 <= overallEquipCount; ++Loop1) {
            for (int Loop2 = Loop1 + 1; Loop2 <= overallEquipCount; ++Loop2) {
                if (ZoneEquipListAcct(Loop1).ObjectType != ZoneEquipListAcct(Loop2).ObjectType ||
                    ZoneEquipListAcct(Loop1).ObjectName != ZoneEquipListAcct(Loop2).ObjectName)
                    continue;
                // Duplicated -- not allowed
                ShowSevereError(state, format("{}{}, duplicate items in ZoneHVAC:EquipmentList.", RoutineName, CurrentModuleObject));
                ShowContinueError(state,
                                  format("Equipment: Type={}, Name={}", ZoneEquipListAcct(Loop1).ObjectType, ZoneEquipListAcct(Loop1).ObjectName));
                ShowContinueError(state,
                                  format("Found on List=\"{}\".", state.dataZoneEquip->ZoneEquipList(ZoneEquipListAcct(Loop1).OnListNum).Name));
                ShowContinueError(
                    state,
                    format("Equipment Duplicated on List=\"{}\".", state.dataZoneEquip->ZoneEquipList(ZoneEquipListAcct(Loop2).OnListNum).Name));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }
        }
        ZoneEquipListAcct.deallocate();
    }

    // map ZoneEquipConfig%EquipListIndex to ZoneEquipList%Name

    for (int ControlledZoneLoop = 1; ControlledZoneLoop <= state.dataGlobal->NumOfZones; ++ControlledZoneLoop) {
        state.dataZoneEquip->GetZoneEquipmentDataFound = UtilityRoutines::FindItemInList(
            state.dataZoneEquip->ZoneEquipList(ControlledZoneLoop).Name, state.dataZoneEquip->ZoneEquipConfig, &EquipConfiguration::EquipListName);
        if (state.dataZoneEquip->GetZoneEquipmentDataFound > 0)
            state.dataZoneEquip->ZoneEquipConfig(state.dataZoneEquip->GetZoneEquipmentDataFound).EquipListIndex = ControlledZoneLoop;
    } // end loop over controlled zones

    EndUniqueNodeCheck(state, "ZoneHVAC:EquipmentConnections");

    CurrentModuleObject = "AirLoopHVAC:SupplyPath";
    for (int PathNum = 1; PathNum <= state.dataZoneEquip->NumSupplyAirPaths; ++PathNum) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 PathNum,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); //  data for one zone
        state.dataZoneEquip->SupplyAirPath(PathNum).Name = AlphArray(1);
        state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents = nint((double(NumAlphas) - 2.0) / 2.0);

        state.dataZoneEquip->SupplyAirPath(PathNum).InletNodeNum = GetOnlySingleNode(state,
                                                                                     AlphArray(2),
                                                                                     state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                                                     DataLoopNode::ConnectionObjectType::AirLoopHVACSupplyPath,
                                                                                     AlphArray(1),
                                                                                     DataLoopNode::NodeFluidType::Air,
                                                                                     DataLoopNode::ConnectionType::Inlet,
                                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                                     ObjectIsParent);

        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentType.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentTypeEnum.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentTypeEnum = DataZoneEquipment::AirLoopHVACZone::Invalid;
        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentName.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).ComponentIndex.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).SplitterIndex.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->SupplyAirPath(PathNum).PlenumIndex.allocate(state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents);

        int Counter = 3;

        for (int CompNum = 1; CompNum <= state.dataZoneEquip->SupplyAirPath(PathNum).NumOfComponents; ++CompNum) {

            if ((AlphArray(Counter) == "AIRLOOPHVAC:ZONESPLITTER") || (AlphArray(Counter) == "AIRLOOPHVAC:SUPPLYPLENUM")) {

                state.dataZoneEquip->SupplyAirPath(PathNum).ComponentType(CompNum) = AlphArray(Counter);
                state.dataZoneEquip->SupplyAirPath(PathNum).ComponentName(CompNum) = AlphArray(Counter + 1);
                ValidateComponent(state,
                                  state.dataZoneEquip->SupplyAirPath(PathNum).ComponentType(CompNum),
                                  state.dataZoneEquip->SupplyAirPath(PathNum).ComponentName(CompNum),
                                  IsNotOK,
                                  CurrentModuleObject);
                state.dataZoneEquip->SupplyAirPath(PathNum).ComponentIndex(CompNum) = 0;
                state.dataZoneEquip->SupplyAirPath(PathNum).SplitterIndex(CompNum) = 0;
                state.dataZoneEquip->SupplyAirPath(PathNum).PlenumIndex(CompNum) = 0;
                state.dataZoneEquip->SupplyAirPath(PathNum).ComponentTypeEnum(CompNum) =
                    (AirLoopHVACZone)getEnumValue(AirLoopHVACTypeNamesUC, AlphArray(Counter));
            } else {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cAlphaFields(1), state.dataZoneEquip->SupplyAirPath(PathNum).Name));
                ShowContinueError(state, format("Unhandled component type =\"{}\".", AlphArray(Counter)));
                ShowContinueError(state, R"(Must be "AirLoopHVAC:ZoneSplitter" or "AirLoopHVAC:SupplyPlenum")");
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }

            Counter += 2;
        }

        state.dataZoneEquip->SupplyAirPath(PathNum).NumOutletNodes = 0;
        state.dataZoneEquip->SupplyAirPath(PathNum).NumNodes = 0;

    } // end loop over supply air paths

    CurrentModuleObject = "AirLoopHVAC:ReturnPath";
    for (int PathNum = 1; PathNum <= state.dataZoneEquip->NumReturnAirPaths; ++PathNum) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 PathNum,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields); //  data for one zone
        state.dataZoneEquip->ReturnAirPath(PathNum).Name = AlphArray(1);
        state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents = nint((double(NumAlphas) - 2.0) / 2.0);

        state.dataZoneEquip->ReturnAirPath(PathNum).OutletNodeNum = GetOnlySingleNode(state,
                                                                                      AlphArray(2),
                                                                                      state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                                                      DataLoopNode::ConnectionObjectType::AirLoopHVACReturnPath,
                                                                                      AlphArray(1),
                                                                                      DataLoopNode::NodeFluidType::Air,
                                                                                      DataLoopNode::ConnectionType::Outlet,
                                                                                      NodeInputManager::CompFluidStream::Primary,
                                                                                      ObjectIsParent);

        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum = DataZoneEquipment::AirLoopHVACZone::Invalid;
        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);
        state.dataZoneEquip->ReturnAirPath(PathNum).ComponentIndex.allocate(state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents);

        int Counter = 3;

        for (int CompNum = 1; CompNum <= state.dataZoneEquip->ReturnAirPath(PathNum).NumOfComponents; ++CompNum) {

            if ((AlphArray(Counter) == "AIRLOOPHVAC:ZONEMIXER") || (AlphArray(Counter) == "AIRLOOPHVAC:RETURNPLENUM")) {

                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType(CompNum) = AlphArray(Counter);
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName(CompNum) = AlphArray(Counter + 1);
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentIndex(CompNum) = 0;
                ValidateComponent(state,
                                  state.dataZoneEquip->ReturnAirPath(PathNum).ComponentType(CompNum),
                                  state.dataZoneEquip->ReturnAirPath(PathNum).ComponentName(CompNum),
                                  IsNotOK,
                                  CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("In {} = {}", CurrentModuleObject, state.dataZoneEquip->ReturnAirPath(PathNum).Name));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                }
                state.dataZoneEquip->ReturnAirPath(PathNum).ComponentTypeEnum(CompNum) =
                    static_cast<AirLoopHVACZone>(getEnumValue(AirLoopHVACTypeNamesUC, AlphArray(Counter)));
            } else {
                ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, cAlphaFields(1), state.dataZoneEquip->ReturnAirPath(PathNum).Name));
                ShowContinueError(state, format("Unhandled component type =\"{}\".", AlphArray(Counter)));
                ShowContinueError(state, R"(Must be "AirLoopHVAC:ZoneMixer" or "AirLoopHVAC:ReturnPlenum")");
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }

            Counter += 2;
        }

    } // end loop over return air paths

    AlphArray.deallocate();
    NumArray.deallocate();
    cAlphaFields.deallocate();
    cNumericFields.deallocate();
    lAlphaBlanks.deallocate();
    lNumericBlanks.deallocate();

    if (state.dataZoneEquip->GetZoneEquipmentDataErrorsFound) {
        ShowFatalError(state, format("{}Errors found in getting Zone Equipment input.", RoutineName));
    }
}

void processZoneEquipmentInput(EnergyPlusData &state,
                               std::string_view zoneEqModuleObject,
                               int const zoneOrSpaceNum,
                               bool const isSpace,
                               int &locTermUnitSizingCounter,
                               int &overallEquipCount,
                               DataZoneEquipment::EquipConfiguration &thisEquipConfig,
                               Array1D_string &AlphArray,
                               Array1D<Real64> &NumArray,
                               Array1D_string &cAlphaFields,   // Alpha field names
                               Array1D_string &cNumericFields, // Numeric field names
                               Array1D_bool &lAlphaBlanks,     // Logical array, alpha field input BLANK = .TRUE.
                               Array1D_bool &lNumericBlanks,   // Logical array, numeric field input BLANK = .TRUE.
                               Array1D_int &NodeNums)
{
    static constexpr std::string_view RoutineName("processZoneEquipmentInput: "); // include trailing blank space
    std::string_view zsString = "Zone";
    if (isSpace) {
        zsString = "Space";
    }

    thisEquipConfig.IsControlled = true;
    thisEquipConfig.ZoneName = AlphArray(1); // for x-referencing with the geometry data

    bool IsNotOK = false;
    GlobalNames::IntraObjUniquenessCheck(
        state, AlphArray(2), zoneEqModuleObject, cAlphaFields(2), state.dataZoneEquip->UniqueZoneEquipListNames, IsNotOK);
    if (IsNotOK) {
        ShowContinueError(state, format("..another Controlled Zone has been assigned that {}.", cAlphaFields(2)));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }
    thisEquipConfig.EquipListName = AlphArray(2); // the name of the list containing all the zone eq.
    std::string const InletNodeListName = AlphArray(3);
    std::string const ExhaustNodeListName = AlphArray(4);
    thisEquipConfig.ZoneNode = GetOnlySingleNode(state,
                                                 AlphArray(5),
                                                 state.dataZoneEquip->GetZoneEquipmentDataErrorsFound,
                                                 DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections,
                                                 AlphArray(1),
                                                 DataLoopNode::NodeFluidType::Air,
                                                 DataLoopNode::ConnectionType::ZoneNode,
                                                 NodeInputManager::CompFluidStream::Primary,
                                                 DataLoopNode::ObjectIsNotParent); // all zone air state variables are
    if (thisEquipConfig.ZoneNode == 0) {
        ShowSevereError(state, format("{}{}: {}=\"{}\", invalid", RoutineName, zoneEqModuleObject, cAlphaFields(1), AlphArray(1)));
        ShowContinueError(state, format("{} must be present.", cAlphaFields(5)));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    } else {
        bool UniqueNodeError = false;
        NodeInputManager::CheckUniqueNodeNames(state, cAlphaFields(5), UniqueNodeError, AlphArray(5), AlphArray(1));
        if (UniqueNodeError) {
            // ShowContinueError(state, format("Occurs for {} = {}", trim( cAlphaFields( 1 ) ), trim( AlphArray( 1 ) )));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
        }
    }

    std::string ReturnNodeListName = AlphArray(6);
    if (lAlphaBlanks(7)) {
        thisEquipConfig.ReturnFlowSchedPtrNum = ScheduleManager::ScheduleAlwaysOn;
    } else {
        thisEquipConfig.ReturnFlowSchedPtrNum = ScheduleManager::GetScheduleIndex(state, AlphArray(7));
        if (thisEquipConfig.ReturnFlowSchedPtrNum == 0) {
            ShowSevereError(state,
                            format("{}{}: invalid {} entered ={} for {}={}",
                                   RoutineName,
                                   zoneEqModuleObject,
                                   cAlphaFields(7),
                                   AlphArray(7),
                                   cAlphaFields(1),
                                   AlphArray(1)));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
        }
    }
    std::string ReturnFlowBasisNodeListName = AlphArray(8);

    // Read in the equipment type, name and sequence information
    // for each equipment list

    std::string CurrentModuleObject = "ZoneHVAC:EquipmentList";
    auto &ip = state.dataInputProcessing->inputProcessor;

    int ZoneEquipListNum = ip->getObjectItemNum(state, CurrentModuleObject, thisEquipConfig.EquipListName);

    if (ZoneEquipListNum <= 0) {
        ShowSevereError(state, format("{}{} not found = {}", RoutineName, CurrentModuleObject, thisEquipConfig.EquipListName));
        ShowContinueError(state, format("In ZoneHVAC:EquipmentConnections object, for Zone = {}", thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    } else {
        auto const &epListFields = ip->getJSONObjectItem(state, CurrentModuleObject, thisEquipConfig.EquipListName);

        auto const &objectSchemaProps = ip->getObjectSchemaProps(state, CurrentModuleObject);

        EquipList &thisZoneEquipList = state.dataZoneEquip->ZoneEquipList(zoneOrSpaceNum);

        thisZoneEquipList.Name = thisEquipConfig.EquipListName;

        std::string loadDistName = ip->getAlphaFieldValue(epListFields, objectSchemaProps, "load_distribution_scheme");
        thisZoneEquipList.LoadDistScheme =
            static_cast<DataZoneEquipment::LoadDist>(getEnumValue(DataZoneEquipment::LoadDistNamesUC, UtilityRoutines::makeUPPER(loadDistName)));
        if (thisZoneEquipList.LoadDistScheme == DataZoneEquipment::LoadDist::Invalid) {
            ShowSevereError(state, format("{}{} = \"{}, Invalid choice\".", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
            ShowContinueError(state, format("...load_distribution_scheme=\"{}\".", loadDistName));
            state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
        }

        auto extensibles = epListFields.find("equipment");
        if (extensibles != epListFields.end()) {
            auto &extensiblesArray = extensibles.value();
            thisZoneEquipList.NumOfEquipTypes = static_cast<int>(extensiblesArray.size());

            // Increment overall count of equipment
            overallEquipCount += thisZoneEquipList.NumOfEquipTypes;

            thisZoneEquipList.EquipTypeName.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.EquipType.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.compPointer.resize(thisZoneEquipList.NumOfEquipTypes + 1);
            thisZoneEquipList.EquipName.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.EquipIndex.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.EquipData.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.CoolingPriority.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.HeatingPriority.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.CoolingCapacity.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.HeatingCapacity.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.SequentialCoolingFractionSchedPtr.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.SequentialHeatingFractionSchedPtr.allocate(thisZoneEquipList.NumOfEquipTypes);
            thisZoneEquipList.EquipTypeName = "";
            thisZoneEquipList.EquipType = DataZoneEquipment::ZoneEquipType::Invalid;
            thisZoneEquipList.EquipName = "";
            thisZoneEquipList.EquipIndex = 0;
            thisZoneEquipList.CoolingPriority = 0;
            thisZoneEquipList.HeatingPriority = 0;
            thisZoneEquipList.CoolingCapacity = 0;
            thisZoneEquipList.HeatingCapacity = 0;
            thisZoneEquipList.SequentialCoolingFractionSchedPtr = 0;
            thisZoneEquipList.SequentialHeatingFractionSchedPtr = 0;

            auto const &extensionSchemaProps = objectSchemaProps["equipment"]["items"]["properties"];

            int ZoneEquipTypeNum = 0;
            for (auto &extensibleInstance : extensiblesArray) {
                ++ZoneEquipTypeNum;

                thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum) =
                    ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_object_type");
                thisZoneEquipList.EquipName(ZoneEquipTypeNum) =
                    ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_name");

                ValidateComponent(state,
                                  thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum),
                                  thisZoneEquipList.EquipName(ZoneEquipTypeNum),
                                  IsNotOK,
                                  CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("In {}={}", CurrentModuleObject, thisZoneEquipList.Name));
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                }

                // If not present, this return 0
                thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) =
                    ip->getIntFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_cooling_sequence");

                if ((thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) < 0) ||
                    (thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) > thisZoneEquipList.NumOfEquipTypes)) {
                    ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                    ShowContinueError(state,
                                      format("invalid zone_equipment_cooling_sequence=[{}].", thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum)));
                    ShowContinueError(state, "equipment sequence must be > 0 and <= number of equipments in the list.");
                    if (thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) > 0) {
                        ShowContinueError(state, format("only {} in the list.", thisZoneEquipList.NumOfEquipTypes));
                    }
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                }

                thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) =
                    ip->getIntFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_heating_or_no_load_sequence");
                if ((thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) < 0) ||
                    (thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) > thisZoneEquipList.NumOfEquipTypes)) {
                    ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                    ShowContinueError(state,
                                      format("invalid zone_equipment_heating_sequence=[{}].", thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum)));
                    ShowContinueError(state, "equipment sequence must be > 0 and <= number of equipments in the list.");
                    if (thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) > 0) {
                        ShowContinueError(state, format("only {} in the list.", thisZoneEquipList.NumOfEquipTypes));
                    }
                    state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                }

                std::string coolingSchName =
                    ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_sequential_cooling_fraction_schedule_name");
                if (coolingSchName.empty()) {
                    thisZoneEquipList.SequentialCoolingFractionSchedPtr(ZoneEquipTypeNum) = ScheduleManager::ScheduleAlwaysOn;
                } else {
                    thisZoneEquipList.SequentialCoolingFractionSchedPtr(ZoneEquipTypeNum) = ScheduleManager::GetScheduleIndex(state, coolingSchName);
                    if (thisZoneEquipList.SequentialCoolingFractionSchedPtr(ZoneEquipTypeNum) == 0) {
                        ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                        ShowContinueError(state, format("invalid zone_equipment_sequential_cooling_fraction_schedule_name=[{}].", coolingSchName));
                        ShowContinueError(state, "Schedule does not exist.");
                        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    }
                }

                std::string heatingSchName =
                    ip->getAlphaFieldValue(extensibleInstance, extensionSchemaProps, "zone_equipment_sequential_heating_fraction_schedule_name");
                if (heatingSchName.empty()) {
                    thisZoneEquipList.SequentialHeatingFractionSchedPtr(ZoneEquipTypeNum) = ScheduleManager::ScheduleAlwaysOn;
                } else {
                    thisZoneEquipList.SequentialHeatingFractionSchedPtr(ZoneEquipTypeNum) = ScheduleManager::GetScheduleIndex(state, heatingSchName);
                    if (thisZoneEquipList.SequentialHeatingFractionSchedPtr(ZoneEquipTypeNum) == 0) {
                        ShowSevereError(state, format("{}{} = \"{}\".", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                        ShowContinueError(state, format("invalid zone_equipment_sequential_heating_fraction_schedule_name=[{}].", coolingSchName));
                        ShowContinueError(state, "Schedule does not exist.");
                        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    }
                }

                // do this here for initial prototype, but later will call all the equipment in a separate function to see who is on - maybe
                if (thisZoneEquipList.HeatingPriority(ZoneEquipTypeNum) > 0) {
                    ++thisZoneEquipList.NumAvailHeatEquip;
                }
                if (thisZoneEquipList.CoolingPriority(ZoneEquipTypeNum) > 0) {
                    ++thisZoneEquipList.NumAvailCoolEquip;
                }

                thisZoneEquipList.EquipType(ZoneEquipTypeNum) = static_cast<ZoneEquipType>(
                    getEnumValue(zoneEquipTypeNamesUC, UtilityRoutines::makeUPPER(thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum))));

                if (thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::UnitarySystem ||
                    thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::PackagedTerminalAirConditioner ||
                    thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::PackagedTerminalHeatPump ||
                    thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::PackagedTerminalHeatPumpWaterToAir) {
                    // loop index accesses correct pointer to equipment on this equipment list
                    // EquipIndex is used to access specific equipment for a single class of equipment (e.g., PTAC 1, 2 and 3)
                    thisZoneEquipList.compPointer[ZoneEquipTypeNum] = UnitarySystems::UnitarySys::factory(
                        state, DataHVACGlobals::UnitarySys_AnyCoilType, thisZoneEquipList.EquipName(ZoneEquipTypeNum), true, 0);
                    thisZoneEquipList.EquipIndex(ZoneEquipTypeNum) = thisZoneEquipList.compPointer[ZoneEquipTypeNum]->getEquipIndex();
                }

                if (thisZoneEquipList.EquipType(ZoneEquipTypeNum) == ZoneEquipType::Invalid) {
                    if (thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum) == "ZONEHVAC:LOWTEMPERATURERADIANT:CONSTANTFLOW" ||
                        thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum) == "ZONEHVAC:LOWTEMPERATURERADIANT:ELECTRIC") {
                        thisZoneEquipList.EquipType(ZoneEquipTypeNum) = ZoneEquipType::LowTemperatureRadiant;
                    } else if (thisZoneEquipList.EquipTypeName(ZoneEquipTypeNum) == "WATERHEATER:HEATPUMP:WRAPPEDCONDENSER") {
                        thisZoneEquipList.EquipType(ZoneEquipTypeNum) = DataZoneEquipment::ZoneEquipType::HeatPumpWaterHeater;
                    } else {
                        ShowSevereError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                        ShowContinueError(state, format("..Invalid Equipment Type = {}", thisZoneEquipList.EquipType(ZoneEquipTypeNum)));
                        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
                    }
                }
            }
        } // End parsing all extensible Zone Equipment info

        // Check for multiple assignments
        for (int ZoneEquipTypeNum = 1; ZoneEquipTypeNum <= thisZoneEquipList.NumOfEquipTypes; ++ZoneEquipTypeNum) {
            if (count_eq(thisZoneEquipList.CoolingPriority, ZoneEquipTypeNum) > 1) {
                ShowSevereError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                ShowContinueError(state,
                                  format("...multiple assignments for Zone Equipment Cooling Sequence={}, must be 1-1 correspondence between "
                                         "sequence assignments and number of equipments.",
                                         ZoneEquipTypeNum));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            } else if (count_eq(thisZoneEquipList.CoolingPriority, ZoneEquipTypeNum) == 0) {
                ShowWarningError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                ShowContinueError(state,
                                  format("...zero assigned to Zone Equipment Cooling Sequence={}, apparent gap in sequence assignments in "
                                         "this equipment list.",
                                         ZoneEquipTypeNum));
            }
            if (count_eq(thisZoneEquipList.HeatingPriority, ZoneEquipTypeNum) > 1) {
                ShowSevereError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                ShowContinueError(state,
                                  format("...multiple assignments for Zone Equipment Heating or No-Load Sequence={}, must be 1-1 "
                                         "correspondence between sequence assignments and number of equipments.",
                                         ZoneEquipTypeNum));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            } else if (count_eq(thisZoneEquipList.HeatingPriority, ZoneEquipTypeNum) == 0) {
                ShowWarningError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, thisZoneEquipList.Name));
                ShowContinueError(state,
                                  format("...zero assigned to Zone Equipment Heating or No-Load Sequence={}, apparent gap in sequence "
                                         "assignments in this equipment list.",
                                         ZoneEquipTypeNum));
            }
        }
    } // End ZoneHVAC:EquipmentList

    bool NodeListError = false;
    int NumNodes = 0;
    GetNodeNums(state,
                InletNodeListName,
                NumNodes,
                NodeNums,
                NodeListError,
                DataLoopNode::NodeFluidType::Air,
                DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections,
                thisEquipConfig.ZoneName,
                DataLoopNode::ConnectionType::ZoneInlet,
                NodeInputManager::CompFluidStream::Primary,
                DataLoopNode::ObjectIsNotParent);

    if (!NodeListError) {
        thisEquipConfig.NumInletNodes = NumNodes;

        thisEquipConfig.InletNode.allocate(NumNodes);
        thisEquipConfig.InletNodeAirLoopNum.allocate(NumNodes);
        thisEquipConfig.InletNodeADUNum.allocate(NumNodes);
        thisEquipConfig.AirDistUnitCool.allocate(NumNodes);
        thisEquipConfig.AirDistUnitHeat.allocate(NumNodes);

        for (int NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
            thisEquipConfig.InletNode(NodeNum) = NodeNums(NodeNum);
            bool UniqueNodeError = false;
            NodeInputManager::CheckUniqueNodeNumbers(state, "Zone Air Inlet Nodes", UniqueNodeError, NodeNums(NodeNum), thisEquipConfig.ZoneName);
            if (UniqueNodeError) {
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }
            thisEquipConfig.InletNodeAirLoopNum(NodeNum) = 0;
            thisEquipConfig.InletNodeADUNum(NodeNum) = 0;
            thisEquipConfig.AirDistUnitCool(NodeNum).InNode = 0;
            thisEquipConfig.AirDistUnitHeat(NodeNum).InNode = 0;
            thisEquipConfig.AirDistUnitCool(NodeNum).OutNode = 0;
            thisEquipConfig.AirDistUnitHeat(NodeNum).OutNode = 0;
            ++locTermUnitSizingCounter;
            thisEquipConfig.AirDistUnitCool(NodeNum).TermUnitSizingIndex = locTermUnitSizingCounter;
            thisEquipConfig.AirDistUnitHeat(NodeNum).TermUnitSizingIndex = locTermUnitSizingCounter;
        }
    } else {
        ShowContinueError(
            state,
            format("Invalid Zone Air Inlet Node or NodeList Name in ZoneHVAC:EquipmentConnections object, for Zone = {}", thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    NodeListError = false;
    GetNodeNums(state,
                ExhaustNodeListName,
                NumNodes,
                NodeNums,
                NodeListError,
                DataLoopNode::NodeFluidType::Air,
                DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections,
                thisEquipConfig.ZoneName,
                DataLoopNode::ConnectionType::ZoneExhaust,
                NodeInputManager::CompFluidStream::Primary,
                DataLoopNode::ObjectIsNotParent);

    if (!NodeListError) {
        thisEquipConfig.NumExhaustNodes = NumNodes;

        thisEquipConfig.ExhaustNode.allocate(NumNodes);

        for (int NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
            thisEquipConfig.ExhaustNode(NodeNum) = NodeNums(NodeNum);
            bool UniqueNodeError = false;
            NodeInputManager::CheckUniqueNodeNumbers(state, "Zone Air Exhaust Nodes", UniqueNodeError, NodeNums(NodeNum), thisEquipConfig.ZoneName);
            if (UniqueNodeError) {
                // ShowContinueError(state, format("Occurs for Zone = {}", trim( AlphArray( 1 ) )));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }
        }
    } else {
        ShowContinueError(
            state,
            format("Invalid Zone Air Exhaust Node or NodeList Name in ZoneHVAC:EquipmentConnections object, for Zone={}", thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    NodeListError = false;
    GetNodeNums(state,
                ReturnNodeListName,
                NumNodes,
                NodeNums,
                NodeListError,
                DataLoopNode::NodeFluidType::Air,
                DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections,
                thisEquipConfig.ZoneName,
                DataLoopNode::ConnectionType::ZoneReturn,
                NodeInputManager::CompFluidStream::Primary,
                DataLoopNode::ObjectIsNotParent);

    if (!NodeListError) {
        thisEquipConfig.NumReturnNodes = NumNodes;

        thisEquipConfig.ReturnNode.allocate(NumNodes);
        thisEquipConfig.ReturnNodeAirLoopNum.allocate(NumNodes);
        thisEquipConfig.ReturnNodeInletNum.allocate(NumNodes);
        thisEquipConfig.FixedReturnFlow.allocate(NumNodes);
        thisEquipConfig.ReturnNodePlenumNum.allocate(NumNodes);
        thisEquipConfig.ReturnNodeExhaustNodeNum.allocate(NumNodes);
        thisEquipConfig.SharedExhaustNode.allocate(NumNodes);
        thisEquipConfig.ReturnNode = 0;                                         // initialize to zero here
        thisEquipConfig.ReturnNodeAirLoopNum = 0;                               // initialize to zero here
        thisEquipConfig.ReturnNodeInletNum = 0;                                 // initialize to zero here
        thisEquipConfig.FixedReturnFlow = false;                                // initialize to false here
        thisEquipConfig.ReturnNodePlenumNum = 0;                                // initialize to zero here
        thisEquipConfig.ReturnNodeExhaustNodeNum = 0;                           // initialize to zero here
        thisEquipConfig.SharedExhaustNode = LightReturnExhaustConfig::NoExhast; // initialize to zero here

        for (int NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
            thisEquipConfig.ReturnNode(NodeNum) = NodeNums(NodeNum);
            bool UniqueNodeError = false;
            NodeInputManager::CheckUniqueNodeNumbers(state, "Zone Return Air Nodes", UniqueNodeError, NodeNums(NodeNum), thisEquipConfig.ZoneName);
            if (UniqueNodeError) {
                // ShowContinueError(state, format("Occurs for Zone = {}", trim( AlphArray( 1 ) )));
                state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
            }
        }
    } else {
        ShowContinueError(
            state,
            format("Invalid Zone Return Air Node or NodeList Name in ZoneHVAC:EquipmentConnections object, for Zone={}", thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }

    NodeListError = false;
    GetNodeNums(state,
                ReturnFlowBasisNodeListName,
                NumNodes,
                NodeNums,
                NodeListError,
                DataLoopNode::NodeFluidType::Air,
                DataLoopNode::ConnectionObjectType::ZoneHVACEquipmentConnections,
                thisEquipConfig.ZoneName,
                DataLoopNode::ConnectionType::Sensor,
                NodeInputManager::CompFluidStream::Primary,
                DataLoopNode::ObjectIsNotParent);

    if (!NodeListError) {
        thisEquipConfig.NumReturnFlowBasisNodes = NumNodes;

        thisEquipConfig.ReturnFlowBasisNode.allocate(NumNodes);

        for (int NodeNum = 1; NodeNum <= NumNodes; ++NodeNum) {
            thisEquipConfig.ReturnFlowBasisNode(NodeNum) = NodeNums(NodeNum);
        }
    } else {
        ShowContinueError(
            state,
            format("Invalid Zone Return Air Node 1 Flow Rate Basis Node or NodeList Name in ZoneHVAC:EquipmentConnections object, for Zone={}",
                   thisEquipConfig.ZoneName));
        state.dataZoneEquip->GetZoneEquipmentDataErrorsFound = true;
    }
}

bool CheckZoneEquipmentList(EnergyPlusData &state,
                            std::string_view const ComponentType, // Type of component
                            std::string_view const ComponentName, // Name of component
                            ObjexxFCL::Optional_int CtrlZoneNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   May 2007
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // Provides a way to check if a component name is listed on a zone equipment list.

    // Return value
    bool IsOnList; // True if item is on a list, false if not.

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int Loop;
    int ListLoop;
    int CtrlZoneNumLocal;

    CtrlZoneNumLocal = 0;
    IsOnList = false;
    for (Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {           // NumOfZoneEquipLists
        if (state.dataZoneEquip->ZoneEquipList(Loop).Name.empty()) continue; // dimensioned by NumOfZones.  Only valid ones have names.
        for (ListLoop = 1; ListLoop <= state.dataZoneEquip->ZoneEquipList(Loop).NumOfEquipTypes; ++ListLoop) {
            if (!UtilityRoutines::SameString(state.dataZoneEquip->ZoneEquipList(Loop).EquipTypeName(ListLoop), ComponentType)) continue;
            if (ComponentName == "*") {
                IsOnList = true;
                CtrlZoneNumLocal = Loop;
                goto EquipList_exit;
            }
            if (!UtilityRoutines::SameString(state.dataZoneEquip->ZoneEquipList(Loop).EquipName(ListLoop), ComponentName)) continue;
            IsOnList = true;
            CtrlZoneNumLocal = Loop;
            goto EquipList_exit;
        }
    }
EquipList_exit:;
    if (present(CtrlZoneNum)) {
        CtrlZoneNum = CtrlZoneNumLocal;
    }
    return IsOnList;
}

int GetControlledZoneIndex(EnergyPlusData &state, std::string const &ZoneName) // Zone name to match into Controlled Zone structure
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2008

    // PURPOSE OF THIS FUNCTION:
    // This function returns the index into the Controlled Zone Equipment structure
    // of the indicated zone.

    if (!state.dataZoneEquip->ZoneEquipInputsFilled) {
        GetZoneEquipmentData(state);
        state.dataZoneEquip->ZoneEquipInputsFilled = true;
    }

    return UtilityRoutines::FindItemInList(ZoneName, state.dataZoneEquip->ZoneEquipConfig, &EquipConfiguration::ZoneName);
}

int FindControlledZoneIndexFromSystemNodeNumberForZone(EnergyPlusData &state,
                                                       int const TrialZoneNodeNum) // Node number to match into Controlled Zone structure
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   August 2013

    // PURPOSE OF THIS FUNCTION:
    // This function returns the zone number for the indicated
    // zone node num.  Returns 0 if did not find zone node in any Zone

    int ControlledZoneIndex = 0; // Index into Controlled Zone structure

    if (!state.dataZoneEquip->ZoneEquipInputsFilled) {
        GetZoneEquipmentData(state);
        state.dataZoneEquip->ZoneEquipInputsFilled = true;
    }

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).IsControlled) {
            if (TrialZoneNodeNum == state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ZoneNode) {
                // found it.
                ControlledZoneIndex = ZoneNum;
                break;
            }
        }
    }

    return ControlledZoneIndex;
}

int GetSystemNodeNumberForZone(EnergyPlusData &state, int const zoneNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2008

    // PURPOSE OF THIS FUNCTION:
    // This function returns the system node number for the indicated
    // zone.  Returns 0 if the Zone is not a controlled zone.

    int SystemZoneNodeNumber = 0; // System node number for controlled zone

    if (!state.dataZoneEquip->ZoneEquipInputsFilled) {
        GetZoneEquipmentData(state);
        state.dataZoneEquip->ZoneEquipInputsFilled = true;
    }

    if (zoneNum > 0) {
        if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).IsControlled) {
            SystemZoneNodeNumber = state.dataZoneEquip->ZoneEquipConfig(zoneNum).ZoneNode;
        }
    }

    return SystemZoneNodeNumber;
}

int GetReturnAirNodeForZone(EnergyPlusData &state,
                            int const zoneNum,
                            std::string const &NodeName,             // Return air node name to match (may be blank)
                            std::string const &calledFromDescription // String identifying the calling function and object
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   March 2008
    //       MODIFIED       Feb 2017 expanded for multiple return nodes in a zone

    // PURPOSE OF THIS FUNCTION:
    // This function returns the return air node number for the indicated
    // zone and node name.  If NodeName is blank, return the first return node number,
    // otherwise return the node number of the matching return node name.
    // Returns 0 if the Zone is not a controlled zone or the node name does not match.

    // Return value
    int ReturnAirNodeNumber = 0; // Return Air node number for controlled zone

    if (!state.dataZoneEquip->ZoneEquipInputsFilled) {
        GetZoneEquipmentData(state);
        state.dataZoneEquip->ZoneEquipInputsFilled = true;
    }

    ReturnAirNodeNumber = 0; // default is not found
    if (zoneNum > 0) {
        {
            auto const &thisZoneEquip(state.dataZoneEquip->ZoneEquipConfig(zoneNum));
            if (thisZoneEquip.IsControlled) {
                if (NodeName.empty()) {
                    // If NodeName is blank, return first return node number, but warn if there are multiple return nodes for this zone
                    ReturnAirNodeNumber = thisZoneEquip.ReturnNode(1);
                    if (thisZoneEquip.NumReturnNodes > 1) {
                        ShowWarningError(state,
                                         format("GetReturnAirNodeForZone: {}, request for zone return node is ambiguous.", calledFromDescription));
                        ShowContinueError(state,
                                          format("Zone={} has {} return nodes. First return node will be used.",
                                                 thisZoneEquip.ZoneName,
                                                 thisZoneEquip.NumReturnNodes));
                    }
                } else {
                    for (int nodeCount = 1; nodeCount <= thisZoneEquip.NumReturnNodes; ++nodeCount) {
                        int curNodeNum = thisZoneEquip.ReturnNode(nodeCount);
                        if (NodeName == state.dataLoopNodes->NodeID(curNodeNum)) {
                            ReturnAirNodeNumber = curNodeNum;
                        }
                    }
                }
            }
        }
    }

    return ReturnAirNodeNumber;
}

int GetReturnNumForZone(EnergyPlusData &state,
                        int const zoneNum,
                        std::string const &NodeName // Return air node name to match (may be blank)
)
{

    // PURPOSE OF THIS FUNCTION:
    // This function returns the zone return number (not the node number) for the indicated
    // zone and node name.  If NodeName is blank, return 1 (the first return node)
    // otherwise return the index of the matching return node name.
    // Returns 0 if the Zone is not a controlled zone or the node name does not match.

    // Return value
    int ReturnIndex = 0; // Return number for the given zone (not the node number)

    if (!state.dataZoneEquip->ZoneEquipInputsFilled) {
        GetZoneEquipmentData(state);
        state.dataZoneEquip->ZoneEquipInputsFilled = true;
    }

    if (zoneNum > 0) {
        if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).IsControlled) {
            if (NodeName.empty()) {
                // If NodeName is blank, return first return node number
                ReturnIndex = 1;
            } else {
                for (int nodeCount = 1; nodeCount <= state.dataZoneEquip->ZoneEquipConfig(zoneNum).NumReturnNodes; ++nodeCount) {
                    int curNodeNum = state.dataZoneEquip->ZoneEquipConfig(zoneNum).ReturnNode(nodeCount);
                    if (NodeName == state.dataLoopNodes->NodeID(curNodeNum)) {
                        ReturnIndex = nodeCount;
                    }
                }
            }
        }
    }

    return ReturnIndex;
}

bool VerifyLightsExhaustNodeForZone(EnergyPlusData &state, int const ZoneNum, int const ZoneExhaustNodeNum)
{
    bool exhaustNodeError = true;

    if (!state.dataZoneEquip->ZoneEquipInputsFilled) {
        GetZoneEquipmentData(state);
        state.dataZoneEquip->ZoneEquipInputsFilled = true;
    }

    for (int ExhaustNum = 1; ExhaustNum <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumExhaustNodes; ++ExhaustNum) {
        if (ZoneExhaustNodeNum == state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ExhaustNode(ExhaustNum)) {
            exhaustNodeError = false;
            break;
        }
    }

    return exhaustNodeError;
}

void EquipList::getPrioritiesForInletNode(EnergyPlusData &state,
                                          int const inletNodeNum, // Zone inlet node number to match
                                          int &coolingPriority,   // Cooling priority num for matching equipment
                                          int &heatingPriority    // Heating priority num for matching equipment
)
{
    bool equipFound = false;
    for (int equipNum = 1; equipNum <= this->NumOfEquipTypes; ++equipNum) {
        if (this->EquipType(equipNum) == DataZoneEquipment::ZoneEquipType::AirDistributionUnit) {
            if (inletNodeNum == state.dataDefineEquipment->AirDistUnit(this->EquipIndex(equipNum)).OutletNodeNum) {
                equipFound = true;
            }
        }
        if (equipFound) {
            coolingPriority = this->CoolingPriority(equipNum);
            heatingPriority = this->HeatingPriority(equipNum);
            break;
        }
    }
    // Set MinAirLoopIterationsAfterFirst for equipment that uses sequenced loads, based on zone equip load distribution scheme
    int minIterations = state.dataHVACGlobal->MinAirLoopIterationsAfterFirst;
    if (this->LoadDistScheme == DataZoneEquipment::LoadDist::Sequential) {
        // Sequential needs one extra iterations up to the highest airterminal unit equipment number
        minIterations = max(coolingPriority, heatingPriority, minIterations);
    } else if (this->LoadDistScheme == DataZoneEquipment::LoadDist::Uniform) {
        // Uniform needs one extra iteration which is the default
    } else if (this->LoadDistScheme == DataZoneEquipment::LoadDist::UniformPLR) {
        // UniformPLR needs two extra iterations, regardless of unit equipment number
        minIterations = max(2, minIterations);
    } else if (this->LoadDistScheme == DataZoneEquipment::LoadDist::SequentialUniformPLR) {
        // SequentialUniformPLR needs one extra iterations up to the highest airterminal unit equipment number plus one more
        minIterations = max((coolingPriority + 1), (heatingPriority + 1), minIterations);
    }
    state.dataHVACGlobal->MinAirLoopIterationsAfterFirst = minIterations;
}

Real64 EquipList::SequentialHeatingFraction(EnergyPlusData &state, const int equipNum)
{
    return ScheduleManager::GetCurrentScheduleValue(state, SequentialHeatingFractionSchedPtr(equipNum));
}

Real64 EquipList::SequentialCoolingFraction(EnergyPlusData &state, const int equipNum)
{
    return ScheduleManager::GetCurrentScheduleValue(state, SequentialCoolingFractionSchedPtr(equipNum));
}

int GetZoneEquipControlledZoneNum(EnergyPlusData &state, DataZoneEquipment::ZoneEquipType const zoneEquipType, std::string const &EquipmentName)
{
    static constexpr std::string_view RoutineName("GetZoneEquipControlledZoneNum: ");
    int ControlZoneNum = 0;

    for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) continue;
        for (int Num = 1; Num <= state.dataZoneEquip->ZoneEquipList(CtrlZone).NumOfEquipTypes; ++Num) {
            if (zoneEquipType == state.dataZoneEquip->ZoneEquipList(CtrlZone).EquipType(Num) &&
                UtilityRoutines::SameString(EquipmentName, state.dataZoneEquip->ZoneEquipList(CtrlZone).EquipName(Num))) {
                return ControlZoneNum = CtrlZone;
            }
        }
    }
    ShowSevereError(state,
                    fmt::format("{}{}=\"{}\" is not on any ZoneHVAC:Equipmentlist. It will not be simulated.",
                                RoutineName,
                                zoneEquipTypeNamesUC[(int)zoneEquipType],
                                EquipmentName));
    return ControlZoneNum;
}

void CheckSharedExhaust(EnergyPlusData &state)
{
    int ExhastNodeNum = 0;
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumReturnNodes < 2) continue;
        for (int nodeCount = 1; nodeCount <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumReturnNodes; ++nodeCount) {
            if (state.dataZoneEquip->ZoneEquipConfig(ZoneNum).SharedExhaustNode(nodeCount) == LightReturnExhaustConfig::Shared) continue;
            ExhastNodeNum = state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ReturnNodeExhaustNodeNum(nodeCount);
            if (ExhastNodeNum > 0) {
                state.dataZoneEquip->ZoneEquipConfig(ZoneNum).SharedExhaustNode(nodeCount) = LightReturnExhaustConfig::Single;
                for (int nodeCount1 = nodeCount + 1; nodeCount1 <= state.dataZoneEquip->ZoneEquipConfig(ZoneNum).NumReturnNodes; ++nodeCount1) {
                    if (ExhastNodeNum == state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ReturnNodeExhaustNodeNum(nodeCount1)) {
                        state.dataZoneEquip->ZoneEquipConfig(ZoneNum).SharedExhaustNode(nodeCount) = LightReturnExhaustConfig::Multi;
                        state.dataZoneEquip->ZoneEquipConfig(ZoneNum).SharedExhaustNode(nodeCount1) = LightReturnExhaustConfig::Shared;
                    }
                }
            }
        }
    }
}

} // namespace EnergyPlus::DataZoneEquipment
