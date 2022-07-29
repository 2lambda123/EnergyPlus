// EnergyPlus, Copyright (c) 1996-2022, The Board of Trustees of the University of Illinois,
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
#include <cassert>
#include <cmath>
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include "RS0001.h"
#include "RS0001_factory.h"
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/ChillerElectricASHRAE205.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/StandardRatings.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::ChillerElectricASHRAE205 {

void getChillerASHRAE205Input(EnergyPlusData &state)
{
    static constexpr std::string_view RoutineName("getChillerASHRAE205Input: "); // include trailing blank space

    using namespace tk205;
    rs_instance_factory::Register_factory("RS0001", std::make_shared<RS0001_factory>());

    bool ErrorsFound{false};

    state.dataIPShortCut->cCurrentModuleObject = "Chiller:Electric:ASHRAE205";
    auto &ip = state.dataInputProcessing->inputProcessor;
    int numElectric205Chillers = ip->getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);

    if (numElectric205Chillers <= 0) {
        ShowSevereError(state, "No " + state.dataIPShortCut->cCurrentModuleObject + " equipment specified in input file");
        ErrorsFound = true;
    }

    state.dataChillerElectricASHRAE205->Electric205Chiller.allocate(numElectric205Chillers);

    auto const ChillerInstances = ip->epJSON.find(state.dataIPShortCut->cCurrentModuleObject).value();
    auto ChillerNum{0};
    auto const &objectSchemaProps = ip->getObjectSchemaProps(state, state.dataIPShortCut->cCurrentModuleObject);
    for (auto &instance : ChillerInstances.items()) {
        auto const &fields = instance.value();
        auto const &thisObjectName = instance.key();
        GlobalNames::VerifyUniqueChillerName(
            state, state.dataIPShortCut->cCurrentModuleObject, thisObjectName, ErrorsFound, state.dataIPShortCut->cCurrentModuleObject + " Name");

        ++ChillerNum;
        auto &thisChiller = state.dataChillerElectricASHRAE205->Electric205Chiller(ChillerNum);
        thisChiller.Name = UtilityRoutines::MakeUPPERCase(thisObjectName);
        ip->markObjectAsUsed(state.dataIPShortCut->cCurrentModuleObject, thisObjectName);

        auto rep_file_name = ip->getAlphaFieldValue(fields, objectSchemaProps, "representation_file_name");
        fs::path rep_file_path = DataSystemVariables::CheckForActualFilePath(state, fs::path(rep_file_name), std::string(RoutineName));
        thisChiller.Representation = rs_instance_factory::Create("RS0001", rep_file_path.string().c_str());

        auto const evap_inlet_node_name = ip->getAlphaFieldValue(fields, objectSchemaProps, "chilled_water_inlet_node_name");
        auto const evap_outlet_node_name = ip->getAlphaFieldValue(fields, objectSchemaProps, "chilled_water_outlet_node_name");
        if (evap_inlet_node_name.empty() || evap_outlet_node_name.empty()) {
            ShowSevereError(state, std::string{RoutineName} + state.dataIPShortCut->cCurrentModuleObject + "=\"" + thisChiller.Name + "\"");
            ShowContinueError(state, "Evaporator Inlet or Outlet Node Name is blank.");
            ErrorsFound = true;
        }
        thisChiller.EvapInletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                           evap_inlet_node_name,
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                           thisChiller.Name,
                                                                           DataLoopNode::NodeFluidType::Water,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);
        thisChiller.EvapOutletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            evap_outlet_node_name,
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                            thisChiller.Name,
                                                                            DataLoopNode::NodeFluidType::Water,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);
        BranchNodeConnections::TestCompSet(
            state, state.dataIPShortCut->cCurrentModuleObject, thisChiller.Name, evap_inlet_node_name, evap_outlet_node_name, "Chilled Water Nodes");

        thisChiller.CondenserType = DataPlant::CondenserType::WaterCooled;

        const auto is_water_cooled = thisChiller.CondenserType == DataPlant::CondenserType::WaterCooled;

        auto const cond_inlet_node_name = ip->getAlphaFieldValue(fields, objectSchemaProps, "condenser_inlet_node_name");
        auto const cond_outlet_node_name = ip->getAlphaFieldValue(fields, objectSchemaProps, "condenser_outlet_node_name");
        if (cond_inlet_node_name.empty() || cond_outlet_node_name.empty()) {
            ShowSevereError(state, std::string{RoutineName} + state.dataIPShortCut->cCurrentModuleObject + "=\"" + thisChiller.Name + "\"");
            ShowContinueError(state, "Condenser Inlet or Outlet Node Name is blank.");
            ErrorsFound = true;
        }
        thisChiller.CondInletNodeNum =
            NodeInputManager::GetOnlySingleNode(state,
                                                cond_inlet_node_name,
                                                ErrorsFound,
                                                DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                thisChiller.Name,
                                                is_water_cooled ? DataLoopNode::NodeFluidType::Water : DataLoopNode::NodeFluidType::Blank,
                                                DataLoopNode::ConnectionType::Inlet,
                                                NodeInputManager::CompFluidStream::Secondary,
                                                DataLoopNode::ObjectIsNotParent);

        thisChiller.CondOutletNodeNum =
            NodeInputManager::GetOnlySingleNode(state,
                                                cond_outlet_node_name,
                                                ErrorsFound,
                                                DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                thisChiller.Name,
                                                is_water_cooled ? DataLoopNode::NodeFluidType::Water : DataLoopNode::NodeFluidType::Blank,
                                                DataLoopNode::ConnectionType::Outlet,
                                                NodeInputManager::CompFluidStream::Secondary,
                                                DataLoopNode::ObjectIsNotParent);

        BranchNodeConnections::TestCompSet(state,
                                           state.dataIPShortCut->cCurrentModuleObject,
                                           thisChiller.Name,
                                           cond_inlet_node_name,
                                           cond_outlet_node_name,
                                           is_water_cooled ? "Condenser Water Nodes" : "Condenser (unknown fluid) Nodes");

        thisChiller.FlowMode = static_cast<DataPlant::FlowMode>(
            getEnumerationValue(DataPlant::FlowModeNamesUC, ip->getAlphaFieldValue(fields, objectSchemaProps, "chiller_flow_mode")));

        if (thisChiller.FlowMode == DataPlant::FlowMode::Invalid) {
            ShowSevereError(state, std::string{RoutineName} + state.dataIPShortCut->cCurrentModuleObject + "=\"" + thisObjectName + "\",");
            ShowContinueError(state, "Invalid Chiller Flow Mode = " + fields.at("chiller_flow_mode").get<std::string>());
            ShowContinueError(state, "Available choices are ConstantFlow, NotModulated, or LeavingSetpointModulated");
            ShowContinueError(state, "Flow mode NotModulated is assumed and the simulation continues.");
            thisChiller.FlowMode = DataPlant::FlowMode::NotModulated;
        };

        thisChiller.SizFac = fields.at("sizing_factor").get<Real64>();
        if (thisChiller.SizFac <= 0.0) thisChiller.SizFac = 1.0;

        thisChiller.EvapVolFlowRate = fields.at("chilled_water_maximum_requested_flow_rate").get<Real64>();
        if (thisChiller.EvapVolFlowRate == DataSizing::AutoSize) {
            thisChiller.EvapVolFlowRateWasAutoSized = true;
        }
        thisChiller.CondVolFlowRate = fields.at("condenser_maximum_requested_flow_rate").get<Real64>();
        if (thisChiller.CondVolFlowRate == DataSizing::AutoSize) {
            thisChiller.CondVolFlowRateWasAutoSized = true;
        }

        thisChiller.AmbientTempType = static_cast<AmbientTempIndicator>(getEnumerationValue(
            AmbientTempNamesUC, UtilityRoutines::MakeUPPERCase(ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_indicator"))));
        switch (thisChiller.AmbientTempType) {
        case AmbientTempIndicator::Schedule: {
            const auto ambient_temp_schedule = ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_schedule");
            thisChiller.AmbientTempSchedule = ScheduleManager::GetScheduleIndex(state, ambient_temp_schedule);
            if (thisChiller.AmbientTempSchedule == 0) {
                ShowSevereError(state,
                                state.dataIPShortCut->cCurrentModuleObject + " = " + thisObjectName +
                                    ":  Ambient Temperature Schedule not found = " + ambient_temp_schedule);
                ErrorsFound = true;
            }

            break;
        }
        case AmbientTempIndicator::TempZone: {
            const auto ambient_temp_zone_name = ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_zone_name");
            thisChiller.AmbientTempZone = UtilityRoutines::FindItemInList(ambient_temp_zone_name, state.dataHeatBal->Zone);
            if (thisChiller.AmbientTempZone == 0) {
                ShowSevereError(state,
                                state.dataIPShortCut->cCurrentModuleObject + " = " + thisObjectName +
                                    ":  Ambient Temperature Zone not found = " + ambient_temp_zone_name);
                ErrorsFound = true;
            } else {
                SetupZoneInternalGain(state,
                                      thisChiller.AmbientTempZone,
                                      thisChiller.Name,
                                      DataHeatBalance::IntGainType::ElectricEquipment,
                                      &thisChiller.AmbientZoneGain);
            }
            break;
        }
        case AmbientTempIndicator::OutsideAir: {
            const auto ambient_temp_outdoor_node = ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_outdoor_air_node_name");
            thisChiller.AmbientTempOutsideAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                        ambient_temp_outdoor_node,
                                                                                        ErrorsFound,
                                                                                        DataLoopNode::ConnectionObjectType::WaterHeaterMixed,
                                                                                        thisChiller.Name,
                                                                                        DataLoopNode::NodeFluidType::Air,
                                                                                        DataLoopNode::ConnectionType::OutsideAirReference,
                                                                                        NodeInputManager::CompFluidStream::Primary,
                                                                                        DataLoopNode::ObjectIsNotParent);
            if (fields.count("ambient_temperature_outdoor_air_node_name")) {
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, thisChiller.AmbientTempOutsideAirNode)) {
                    ShowSevereError(state,
                                    state.dataIPShortCut->cCurrentModuleObject + " = " + thisObjectName +
                                        ": Outdoor Air Node not on OutdoorAir:NodeList or OutdoorAir:Node");
                    ShowContinueError(state, "...Referenced Node Name=" + ambient_temp_outdoor_node);
                    ErrorsFound = true;
                }
            } else {
                ShowSevereError(state, state.dataIPShortCut->cCurrentModuleObject + " = " + ambient_temp_outdoor_node);
                ShowContinueError(state, "An Ambient Outdoor Air Node name must be used when the Ambient Temperature Indicator is Outdoors.");
                ErrorsFound = true;
            }

            break;
        }
        default: {
            ShowSevereError(state,
                            state.dataIPShortCut->cCurrentModuleObject + " = " + thisObjectName +
                                ":  Invalid Ambient Temperature Indicator entered=" +
                                ip->getAlphaFieldValue(fields, objectSchemaProps, "ambient_temperature_indicator"));
            ShowContinueError(state, " Valid entries are SCHEDULE, ZONE, and OUTDOORS.");
            ErrorsFound = true;
            break;
        }
        }
        // end Ambient temperature
        const auto oil_cooler_inlet_node = ip->getAlphaFieldValue(fields, objectSchemaProps, "oil_cooler_inlet_node_name");
        const auto oil_cooler_outlet_node = ip->getAlphaFieldValue(fields, objectSchemaProps, "oil_cooler_outlet_node_name");
        if (!oil_cooler_inlet_node.empty() && !oil_cooler_outlet_node.empty()) {
            thisChiller.OilCoolerInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                 oil_cooler_inlet_node,
                                                                                 ErrorsFound,
                                                                                 DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                 thisChiller.Name,
                                                                                 DataLoopNode::NodeFluidType::Water,
                                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                                 DataLoopNode::ObjectIsNotParent);
            thisChiller.OilCoolerOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                  oil_cooler_outlet_node,
                                                                                  ErrorsFound,
                                                                                  DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                  thisChiller.Name,
                                                                                  DataLoopNode::NodeFluidType::Water,
                                                                                  DataLoopNode::ConnectionType::Outlet,
                                                                                  NodeInputManager::CompFluidStream::Primary,
                                                                                  DataLoopNode::ObjectIsNotParent);
        }
        const auto aux_heat_inlet_node = ip->getAlphaFieldValue(fields, objectSchemaProps, "auxiliary_inlet_node_name");
        const auto aux_heat_outlet_node = ip->getAlphaFieldValue(fields, objectSchemaProps, "auxiliary_outlet_node_name");
        if (!aux_heat_inlet_node.empty() && !aux_heat_outlet_node.empty()) {

            thisChiller.AuxiliaryHeatInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                     aux_heat_inlet_node,
                                                                                     ErrorsFound,
                                                                                     DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                     thisChiller.Name,
                                                                                     DataLoopNode::NodeFluidType::Water,
                                                                                     DataLoopNode::ConnectionType::Outlet,
                                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                                     DataLoopNode::ObjectIsNotParent);
            thisChiller.AuxiliaryHeatOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                      aux_heat_outlet_node,
                                                                                      ErrorsFound,
                                                                                      DataLoopNode::ConnectionObjectType::ChillerElectricASHRAE205,
                                                                                      thisChiller.Name,
                                                                                      DataLoopNode::NodeFluidType::Water,
                                                                                      DataLoopNode::ConnectionType::Outlet,
                                                                                      NodeInputManager::CompFluidStream::Primary,
                                                                                      DataLoopNode::ObjectIsNotParent);
        }

        // TODO: When implemented, add ...WasAutoSized variables
        if (fields.count("oil_cooler_design_flow_rate")) {
            thisChiller.OilCoolerVolFlowRate = fields.at("oil_cooler_design_flow_rate").get<Real64>();
        }
        if (fields.count("auxiliary_equipment_design_flow_rate")) {
            thisChiller.AuxiliaryVolFlowRate = fields.at("auxiliary_equipment_design_flow_rate").get<Real64>();
        }

        if (fields.count("end_use_subcategory")) {
            thisChiller.EndUseSubcategory = ip->getAlphaFieldValue(fields, objectSchemaProps, "end_use_subcategory");
        } else {
            thisChiller.EndUseSubcategory = "General";
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Errors found in processing input for " + state.dataIPShortCut->cCurrentModuleObject);
    }
}

ASHRAE205ChillerSpecs *ASHRAE205ChillerSpecs::factory(EnergyPlusData &state, std::string const &objectName)
{
    if (state.dataChillerElectricASHRAE205->getInputFlag) {
        getChillerASHRAE205Input(state);
        state.dataChillerElectricASHRAE205->getInputFlag = false;
    }
    for (auto &obj : state.dataChillerElectricASHRAE205->Electric205Chiller) {
        if (obj.Name == objectName) {
            return &obj;
        }
    }
    // If we didn't find it, fatal
    ShowFatalError(state, "ASHRAE205ChillerSpecs::factory: Error getting inputs for object named: " + objectName); // LCOV_EXCL_LINE
    return nullptr;                                                                                                // LCOV_EXCL_LINE
}

void ASHRAE205ChillerSpecs::oneTimeInit(EnergyPlusData &state)
{
    // Locate the chillers on the plant loops for later usage
    bool errFlag{false};
    PlantUtilities::ScanPlantLoopsForObject(
        state, this->Name, DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205, this->CWPlantLoc, errFlag, _, _, _, this->EvapInletNodeNum, _);
    if (this->CondenserType != DataPlant::CondenserType::AirCooled) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                this->CDPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->CondInletNodeNum,
                                                _);
        PlantUtilities::InterConnectTwoPlantLoopSides(
            state, this->CWPlantLoc, this->CDPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205, true);
    }
    if (this->OilCoolerInletNode) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                this->OCPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->OilCoolerInletNode,
                                                _);
    }
    if (this->AuxiliaryHeatInletNode) {
        PlantUtilities::ScanPlantLoopsForObject(state,
                                                this->Name,
                                                DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                this->AHPlantLoc,
                                                errFlag,
                                                _,
                                                _,
                                                _,
                                                this->AuxiliaryHeatInletNode,
                                                _);
    }
#if 0  // If and when heat recovery is implemented, uncomment
        if (this->HeatRecActive) {
            PlantUtilities::ScanPlantLoopsForObject(state,
                                                    this->Name,
                                                    DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205,
                                                    this->HRPlantLoc,
                                                    errFlag,
                                                    _,
                                                    _,
                                                    _,
                                                    this->HeatRecInletNodeNum,
                                                    _);
            PlantUtilities::InterConnectTwoPlantLoopSides(
                    state, this->CWPlantLoc, this->HRPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205, true);
        }

        if ((this->CondenserType != DataPlant::CondenserType::AirCooled) && (this->HeatRecActive)) {
            PlantUtilities::InterConnectTwoPlantLoopSides(
                    state, this->CDPlantLoc, this->HRPlantLoc, DataPlant::PlantEquipmentType::Chiller_ElectricASHRAE205, false);
        }

        if (errFlag) {
            ShowFatalError(state, "InitElecASHRAE205Chiller: Program terminated due to previous condition(s).");
        }
#endif // #if 0
    if (this->FlowMode == DataPlant::FlowMode::Constant) {
        // reset flow priority
        DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
    }

    else if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
        // reset flow priority
        DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowPriority = DataPlant::LoopFlowStatus::NeedyIfLoopOn;
        // check if setpoint on outlet node
        if ((state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint == DataLoopNode::SensedNodeFlagValue) &&
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi == DataLoopNode::SensedNodeFlagValue)) {
            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                if (!this->ModulatedFlowErrDone) {
                    ShowWarningError(state, "Missing temperature setpoint for LeavingSetpointModulated mode chiller named " + this->Name);
                    ShowContinueError(
                        state, "  A temperature setpoint is needed at the outlet node of a chiller in variable flow mode, use a SetpointManager");
                    ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                    this->ModulatedFlowErrDone = true;
                }
            } else {
                // need call to EMS to check node
                bool fatalError = false; // but not really fatal yet, but should be.
                EMSManager::CheckIfNodeSetPointManagedByEMS(
                    state, this->EvapOutletNodeNum, EMSManager::SPControlType::TemperatureSetPoint, fatalError);
                state.dataLoopNodes->NodeSetpointCheck(this->EvapOutletNodeNum).needsSetpointChecking = false;
                if (fatalError) {
                    if (!this->ModulatedFlowErrDone) {
                        ShowWarningError(state, "Missing temperature setpoint for LeavingSetpointModulated mode chiller named " + this->Name);
                        ShowContinueError(state,
                                          "  A temperature setpoint is needed at the outlet node of a chiller evaporator in variable flow mode");
                        ShowContinueError(state, "  use a Setpoint Manager to establish a setpoint at the chiller evaporator outlet node ");
                        ShowContinueError(state, "  or use an EMS actuator to establish a setpoint at the outlet node ");
                        ShowContinueError(state, "  The overall loop setpoint will be assumed for chiller. The simulation continues ... ");
                        this->ModulatedFlowErrDone = true;
                    }
                }
            }
            this->ModulatedFlowSetToLoop = true;
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint =
                state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPoint;
            state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi =
                state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPointHi;
        }
    }
}

void ASHRAE205ChillerSpecs::initialize(EnergyPlusData &state, bool const RunFlag, Real64 const MyLoad)
{
    static constexpr std::string_view RoutineName("ASHRAE205ChillerSpecs::initialize");

    using FluidProperties::GetDensityGlycol;

    if (this->MyInitFlag) {
        this->oneTimeInit(state);
        this->setOutputVariables(state);
        // TODO: Need this?
        if (!state.dataGlobal->SysSizingCalc) {
            PlantUtilities::RegisterPlantCompDesignFlow(state, this->OilCoolerInletNode, this->OilCoolerVolFlowRate);
            PlantUtilities::RegisterPlantCompDesignFlow(state, this->AuxiliaryHeatInletNode, this->AuxiliaryVolFlowRate);
        }
        this->MyInitFlag = false;
    }

    switch (this->AmbientTempType) {
    case AmbientTempIndicator::Schedule: {
        this->AmbientTemp = ScheduleManager::GetCurrentScheduleValue(state, this->AmbientTempSchedule);
        break;
    }
    case AmbientTempIndicator::TempZone: {
        this->AmbientTemp = state.dataHeatBalFanSys->MAT(this->AmbientTempZone);
        break;
    }
    case AmbientTempIndicator::OutsideAir: {
        this->AmbientTemp = state.dataLoopNodes->Node(this->AmbientTempOutsideAirNode).Temp;
        break;
    }
    default:
        break;
    }

    this->EquipFlowCtrl = DataPlant::CompData::getPlantComponent(state, this->CWPlantLoc).FlowCtrl;

    if (this->MyEnvrnFlag && state.dataGlobal->BeginEnvrnFlag && (state.dataPlnt->PlantFirstSizesOkayToFinalize)) {
        Real64 rho = GetDensityGlycol(state,
                                      state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).FluidName,
                                      DataGlobalConstants::CWInitConvTemp,
                                      state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).FluidIndex,
                                      RoutineName);

        this->EvapMassFlowRateMax = rho * this->EvapVolFlowRate;
        PlantUtilities::InitComponentNodes(state, 0.0, this->EvapMassFlowRateMax, this->EvapInletNodeNum, this->EvapOutletNodeNum);

        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            rho = GetDensityGlycol(state,
                                   state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).FluidName,
                                   this->TempRefCondIn,
                                   state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).FluidIndex,
                                   RoutineName);
            this->CondMassFlowRateMax = rho * this->CondVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->CondMassFlowRateMax, this->CondInletNodeNum, this->CondOutletNodeNum);
            state.dataLoopNodes->Node(this->CondInletNodeNum).Temp = this->TempRefCondIn;
        }
        // Set mass flow rates at Oil Cooler and Aux Equipment nodes
        if (this->OilCoolerInletNode) {
            Real64 rho_oil_cooler = GetDensityGlycol(state,
                                                     state.dataPlnt->PlantLoop(this->OCPlantLoc.loopNum).FluidName,
                                                     DataGlobalConstants::InitConvTemp,
                                                     state.dataPlnt->PlantLoop(this->OCPlantLoc.loopNum).FluidIndex,
                                                     RoutineName);
            this->OilCoolerMassFlowRate = rho_oil_cooler * this->OilCoolerVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->OilCoolerMassFlowRate, this->OilCoolerInletNode, this->OilCoolerOutletNode);
        }
        if (this->AuxiliaryHeatInletNode) {
            Real64 rho_aux = GetDensityGlycol(state,
                                              state.dataPlnt->PlantLoop(this->AHPlantLoc.loopNum).FluidName,
                                              DataGlobalConstants::InitConvTemp,
                                              state.dataPlnt->PlantLoop(this->AHPlantLoc.loopNum).FluidIndex,
                                              RoutineName);
            this->AuxiliaryMassFlowRate = rho_aux * this->AuxiliaryVolFlowRate;
            PlantUtilities::InitComponentNodes(state, 0.0, this->AuxiliaryMassFlowRate, this->AuxiliaryHeatInletNode, this->AuxiliaryHeatOutletNode);
        }
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->MyEnvrnFlag = true;
    }

    if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) && this->ModulatedFlowSetToLoop) {
        // see ReformulatedEIR or EIR Chiller for origin of the following
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint =
            state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPoint;
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi =
            state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).TempSetPointNodeNum).TempSetPointHi;
    }

    Real64 mdot = ((std::abs(MyLoad) > 0.0) && RunFlag) ? this->EvapMassFlowRateMax : 0.0;
    Real64 mdotCond = ((std::abs(MyLoad) > 0.0) && RunFlag) ? this->CondMassFlowRateMax : 0.0;

    PlantUtilities::SetComponentFlowRate(state, mdot, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);

    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        PlantUtilities::SetComponentFlowRate(state, mdotCond, this->CondInletNodeNum, this->CondOutletNodeNum, this->CDPlantLoc);
    }

    // Set component flow rates for Oil Cooler and Aux equipment
    if (this->OilCoolerInletNode) {
        PlantUtilities::SetComponentFlowRate(
            state, this->OilCoolerMassFlowRate, this->OilCoolerInletNode, this->OilCoolerOutletNode, this->OCPlantLoc);
    }
    if (this->AuxiliaryHeatInletNode) {
        PlantUtilities::SetComponentFlowRate(
            state, this->AuxiliaryMassFlowRate, this->AuxiliaryHeatInletNode, this->AuxiliaryHeatOutletNode, this->AHPlantLoc);
    }
    // Recalculate volumetric flow rates from component mass flow rates if necessary

#if 0  // Revisit when heat recovery implemented
       // Initialize heat recovery flow rates at node
        if (this->HeatRecActive) {

            // check if inlet limit active and if exceeded.
            bool HeatRecRunFlag = RunFlag;
            if (this->HeatRecInletLimitSchedNum > 0) {
                Real64 HeatRecHighInletLimit = ScheduleManager::GetCurrentScheduleValue(state, this->HeatRecInletLimitSchedNum);
                if (state.dataLoopNodes->Node(this->HeatRecInletNodeNum).Temp > HeatRecHighInletLimit) { // shut down heat recovery
                    HeatRecRunFlag = false;
                } else {
                    HeatRecRunFlag = RunFlag;
                }
            }

            mdot = HeatRecRunFlag ? this->DesignHeatRecMassFlowRate : 0.0;

            PlantUtilities::SetComponentFlowRate(state, mdot, this->HeatRecInletNodeNum, this->HeatRecOutletNodeNum, this->HRPlantLoc);
        }
#endif // if 0
}

void ASHRAE205ChillerSpecs::size([[maybe_unused]] EnergyPlusData &state)
{
}

void ASHRAE205ChillerSpecs::setOutputVariables(EnergyPlusData &state)
{
    SetupOutputVariable(state,
                        "Chiller Part Load Ratio",
                        OutputProcessor::Unit::None,
                        this->ChillerPartLoadRatio,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Cycling Ratio",
                        OutputProcessor::Unit::None,
                        this->ChillerCyclingRatio,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Electricity Rate",
                        OutputProcessor::Unit::W,
                        this->Power,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Electricity Energy",
                        OutputProcessor::Unit::J,
                        this->Energy,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Summed,
                        this->Name,
                        _,
                        "ELECTRICITY",
                        "Cooling",
                        this->EndUseSubcategory,
                        "Plant");

    SetupOutputVariable(state,
                        "Chiller Evaporator Cooling Rate",
                        OutputProcessor::Unit::W,
                        this->QEvaporator,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Cooling Energy",
                        OutputProcessor::Unit::J,
                        this->EvapEnergy,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Summed,
                        this->Name,
                        _,
                        "ENERGYTRANSFER",
                        "CHILLERS",
                        _,
                        "Plant");

    SetupOutputVariable(state,
                        "Chiller Evaporator Inlet Temperature",
                        OutputProcessor::Unit::C,
                        this->EvapInletTemp,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Outlet Temperature",
                        OutputProcessor::Unit::C,
                        this->EvapOutletTemp,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Evaporator Mass Flow Rate",
                        OutputProcessor::Unit::kg_s,
                        this->EvapMassFlowRate,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Heat Transfer Rate",
                        OutputProcessor::Unit::W,
                        this->QCondenser,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Heat Transfer Energy",
                        OutputProcessor::Unit::J,
                        this->CondEnergy,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Summed,
                        this->Name,
                        _,
                        "ENERGYTRANSFER",
                        "HEATREJECTION",
                        _,
                        "Plant");

    SetupOutputVariable(state,
                        "Chiller COP",
                        OutputProcessor::Unit::W_W,
                        this->ActualCOP,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Inlet Temperature",
                        OutputProcessor::Unit::C,
                        this->CondInletTemp,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Outlet Temperature",
                        OutputProcessor::Unit::C,
                        this->CondOutletTemp,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Condenser Mass Flow Rate",
                        OutputProcessor::Unit::kg_s,
                        this->CondMassFlowRate,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Effective Heat Rejection Temperature",
                        OutputProcessor::Unit::C,
                        this->ChillerCondAvgTemp,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    SetupOutputVariable(state,
                        "Chiller Heat Rejected to Zone",
                        OutputProcessor::Unit::W,
                        this->AmbientZoneGain,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Average,
                        this->Name);

    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        SetupEMSInternalVariable(state, "Chiller Nominal Capacity", this->Name, "[W]", this->RefCap);
    }
}

void ASHRAE205ChillerSpecs::findEvaporatorMassFlowRate(EnergyPlusData &state, Real64 &load, Real64 Cp)
{
    static constexpr std::string_view RoutineName("ASHRAE205ChillerSpecs::findEvaporatorMassFlowRate");
    const int PlantLoopNum = this->CWPlantLoc.loopNum;
    const DataPlant::LoopSideLocation LoopSideNum = this->CWPlantLoc.loopSideNum;
    const int BranchNum = this->CWPlantLoc.branchNum;
    const int CompNum = this->CWPlantLoc.compNum;

    // If FlowLock is False (0), the chiller sets the plant loop mdot
    // If FlowLock is True (1),  the new resolved plant loop mdot is used
    if (state.dataPlnt->PlantLoop(PlantLoopNum).LoopSide(LoopSideNum).FlowLock == DataPlant::FlowLock::Unlocked) {
        this->PossibleSubcooling = !(state.dataPlnt->PlantLoop(PlantLoopNum).LoopSide(LoopSideNum).Branch(BranchNum).Comp(CompNum).CurOpSchemeType ==
                                     DataPlant::OpScheme::CompSetPtBased);

        Real64 evapDeltaTemp(0.0); // Evaporator temperature difference [C]

        // Either set the flow to the Constant value or calculate the flow for the variable volume case
        if ((this->FlowMode == DataPlant::FlowMode::Constant) || (this->FlowMode == DataPlant::FlowMode::NotModulated)) {
            // Set the evaporator mass flow rate to design
            // Start by assuming max (design) flow
            this->EvapMassFlowRate = this->EvapMassFlowRateMax;
            // Use PlantUtilities::SetComponentFlowRate to decide actual flow
            PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
            if (this->EvapMassFlowRate != 0.0) {
                evapDeltaTemp = std::abs(load) / this->EvapMassFlowRate / Cp; // MyLoad = net evaporator capacity, QEvaporator
            } else {
                evapDeltaTemp = 0.0;
            }
            this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - evapDeltaTemp;
        } else if (this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) {
            switch (state.dataPlnt->PlantLoop(PlantLoopNum).LoopDemandCalcScheme) {
            case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                // Calculate the Delta Temp from the inlet temp to the chiller outlet setpoint
                evapDeltaTemp =
                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
            } break;
            case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                evapDeltaTemp =
                    state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
            } break;
            default: {
                assert(false);
            } break;
            }

            if (evapDeltaTemp != 0) {
                this->EvapMassFlowRate = max(0.0, (std::abs(load) / Cp / evapDeltaTemp));
                if ((this->EvapMassFlowRate - this->EvapMassFlowRateMax) > DataBranchAirLoopPlant::MassFlowTolerance) this->PossibleSubcooling = true;
                // Check to see if the Maximum is exceeded, if so set to maximum
                this->EvapMassFlowRate = min(this->EvapMassFlowRateMax, this->EvapMassFlowRate);
                // Use PlantUtilities::SetComponentFlowRate to decide actual flow
                PlantUtilities::SetComponentFlowRate(
                    state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
                // Should we recalculate this with the corrected setpoint?
                switch (state.dataPlnt->PlantLoop(PlantLoopNum).LoopDemandCalcScheme) {
                case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
                    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
                } break;
                case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
                    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
                } break;
                default:
                    break;
                }
            } else {
                // Try to request zero flow
                this->EvapMassFlowRate = 0.0;
                // Use PlantUtilities::SetComponentFlowRate to decide actual flow
                PlantUtilities::SetComponentFlowRate(
                    state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
                // No deltaT since component is not running
                this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
                this->QEvaporator = 0.0;
                // PartLoadRat = 0.0;
                this->ChillerPartLoadRatio = 0.0;

                if (this->DeltaTErrCount < 1 && !state.dataGlobal->WarmupFlag) {
                    ++this->DeltaTErrCount;
                    ShowWarningError(state, "Evaporator DeltaTemp = 0 in mass flow calculation (Tevapin = Tevapout setpoint temp).");
                    ShowContinueErrorTimeStamp(state, "");
                } else if (!state.dataGlobal->WarmupFlag) {
                    ++this->ChillerCapFTError;
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "CHILLER:ELECTRIC:ASHRAE205 \"" + this->Name +
                                                       "\": Evaporator DeltaTemp = 0 in mass flow calculation warning continues...",
                                                   this->DeltaTErrCountIndex,
                                                   evapDeltaTemp,
                                                   evapDeltaTemp);
                }
            }
        }
    } else { // If FlowLock is True
        this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        PlantUtilities::SetComponentFlowRate(state, this->EvapMassFlowRate, this->EvapInletNodeNum, this->EvapOutletNodeNum, this->CWPlantLoc);
        //       Some other component set the flow to 0. No reason to continue with calculations.
        if (this->EvapMassFlowRate == 0.0) {
            load = 0.0;
            return;
        }
    } // This is the end of the FlowLock Block

    const Real64 rho = FluidProperties::GetDensityGlycol(state,
                                                         state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).FluidName,
                                                         DataGlobalConstants::CWInitConvTemp,
                                                         state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).FluidIndex,
                                                         RoutineName);

    this->EvapVolFlowRate = this->EvapMassFlowRate / rho;
}

Real64 ASHRAE205ChillerSpecs::findCapacityResidual(EnergyPlusData &, Real64 partLoadSequenceNumber, std::array<Real64, 4> const &par)
{
    static auto rep = dynamic_cast<tk205::RS0001_NS::RS0001 *>(this->Representation.get());

    this->QEvaporator = rep->performance.performance_map_cooling
                            .Calculate_performance(this->EvapVolFlowRate,
                                                   this->EvapOutletTemp + DataGlobalConstants::KelvinConv,
                                                   this->CondVolFlowRate,
                                                   this->CondInletTemp + DataGlobalConstants::KelvinConv,
                                                   partLoadSequenceNumber)
                            .net_evaporator_capacity;
    const auto load = par[0];
    return std::abs(load) - this->QEvaporator;
}

void ASHRAE205ChillerSpecs::calculate(EnergyPlusData &state, Real64 &MyLoad, bool const RunFlag)
{
    static constexpr std::string_view RoutineName("CalcElecASHRAE205ChillerModel");
    this->ChillerPartLoadRatio = 0.0;
    this->ChillerCyclingRatio = 1.0;
    this->ChillerFalseLoadRate = 0.0;
    this->EvapMassFlowRate = 0.0;
    this->CondMassFlowRate = 0.0;
    this->Power = 0.0;
    this->QCondenser = 0.0;
    this->QEvaporator = 0.0;
    this->QHeatRecovery = 0.0;
    int PlantLoopNum = this->CWPlantLoc.loopNum;
    DataPlant::LoopSideLocation LoopSideNum = this->CWPlantLoc.loopSideNum;
    int BranchNum = this->CWPlantLoc.branchNum;
    int CompNum = this->CWPlantLoc.compNum;

    // Set module-level chiller evaporator and condenser inlet temperature variables
    // using prior time step's temperature
    Real64 condInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;

    // If no loop demand or chiller OFF, return
    // If chiller load is 0 or chiller is not running then leave the subroutine. Before leaving
    //  if the component control is SERIESACTIVE we set the component flow to inlet flow so that
    //  flow resolver will not shut down the branch

    // TODO: calculate performance for standby (only used when off or cycling)
    auto rep = dynamic_cast<tk205::RS0001_NS::RS0001 *>(this->Representation.get());
    Real64 standbyPower = rep->performance.performance_map_standby.Calculate_performance(this->AmbientTemp).input_power;
    if (MyLoad >= 0 || !RunFlag) {
        if (this->EquipFlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive ||
            state.dataPlnt->PlantLoop(PlantLoopNum).LoopSide(LoopSideNum).FlowLock == DataPlant::FlowLock::Locked) {
            this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
        }
        if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
            if (DataPlant::CompData::getPlantComponent(state, this->CDPlantLoc).FlowCtrl == DataBranchAirLoopPlant::ControlType::SeriesActive) {
                this->CondMassFlowRate = state.dataLoopNodes->Node(this->CondInletNodeNum).MassFlowRate;
            }
        }
        this->AmbientZoneGain = standbyPower; // TODO: Actually calculate using heat balance
        // Create some outputs to test this
        return;
    }

// Revisit
#if 0
        //        // If there is a fault of chiller fouling
//        if (this->FaultyChillerFoulingFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
//            (!state.dataGlobal->KickOffSimulation)) {
//            int FaultIndex = this->FaultyChillerFoulingIndex;
//            Real64 NomCap_ff = ChillerRefCap;
//            Real64 ReferenceCOP_ff = ReferenceCOP;
//
//            // calculate the Faulty Chiller Fouling Factor using fault information
//            this->FaultyChillerFoulingFactor = state.dataFaultsMgr->FaultsChillerFouling(FaultIndex).CalFoulingFactor(state);
//
//            // update the Chiller nominal capacity and COP at faulty cases
//            ChillerRefCap = NomCap_ff * this->FaultyChillerFoulingFactor;
//            ReferenceCOP = ReferenceCOP_ff * this->FaultyChillerFoulingFactor;
//        }
#endif // 0

    // Set mass flow rates
    if (this->CondenserType == DataPlant::CondenserType::WaterCooled) {
        this->CondMassFlowRate = this->CondMassFlowRateMax;
        PlantUtilities::SetComponentFlowRate(state, this->CondMassFlowRate, this->CondInletNodeNum, this->CondOutletNodeNum, this->CDPlantLoc);
        PlantUtilities::PullCompInterconnectTrigger(
            state, this->CWPlantLoc, this->CondMassFlowIndex, this->CDPlantLoc, DataPlant::CriteriaType::MassFlowRate, this->CondMassFlowRate);

        if (this->CondMassFlowRate < DataBranchAirLoopPlant::MassFlowTolerance) return;
    }
    Real64 EvapOutletTempSetPoint(0.0); // Evaporator outlet temperature setpoint [C]
    switch (state.dataPlnt->PlantLoop(PlantLoopNum).LoopDemandCalcScheme) {
    case DataPlant::LoopDemandCalcScheme::SingleSetPoint: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (state.dataPlnt->PlantLoop(PlantLoopNum).LoopSide(LoopSideNum).Branch(BranchNum).Comp(CompNum).CurOpSchemeType ==
             DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPoint;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(PlantLoopNum).TempSetPointNodeNum).TempSetPoint;
        }
    } break;
    case DataPlant::LoopDemandCalcScheme::DualSetPointDeadBand: {
        if ((this->FlowMode == DataPlant::FlowMode::LeavingSetpointModulated) ||
            (state.dataPlnt->PlantLoop(PlantLoopNum).LoopSide(LoopSideNum).Branch(BranchNum).Comp(CompNum).CurOpSchemeType ==
             DataPlant::OpScheme::CompSetPtBased) ||
            (state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi != DataLoopNode::SensedNodeFlagValue)) {
            // there will be a valid setpoint on outlet
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(this->EvapOutletNodeNum).TempSetPointHi;
        } else { // use plant loop overall setpoint
            EvapOutletTempSetPoint = state.dataLoopNodes->Node(state.dataPlnt->PlantLoop(PlantLoopNum).TempSetPointNodeNum).TempSetPointHi;
        }
    } break;
    default: {
        assert(false);
    } break;
    }
// Revisit
#if 0
        //        // If there is a fault of Chiller SWT Sensor
//        if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) && (!state.dataGlobal->KickOffSimulation)) {
//            int FaultIndex = this->FaultyChillerSWTIndex;
//            Real64 EvapOutletTempSetPoint_ff = EvapOutletTempSetPoint;
//
//            // calculate the sensor offset using fault information
//            this->FaultyChillerSWTOffset = state.dataFaultsMgr->FaultsChillerSWTSensor(FaultIndex).CalFaultOffsetAct(state);
//            // update the EvapOutletTempSetPoint
//            EvapOutletTempSetPoint =
//                    max(this->TempLowLimitEvapOut,
//                        min(state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp, EvapOutletTempSetPoint_ff - this->FaultyChillerSWTOffset));
//            this->FaultyChillerSWTOffset = EvapOutletTempSetPoint_ff - EvapOutletTempSetPoint;
//        }
#endif // 0
       // When implemented, TODO: correct temperature if using heat recovery

    this->EvapMassFlowRate = state.dataLoopNodes->Node(this->EvapInletNodeNum).MassFlowRate;
    // If some other component set the flow to 0, no reason to continue with calculations.
    if (this->EvapMassFlowRate == 0.0) {
        MyLoad = 0.0;
        return;
    }

    Real64 Cp = FluidProperties::GetSpecificHeatGlycol(state,
                                                       state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).FluidName,
                                                       state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp,
                                                       state.dataPlnt->PlantLoop(this->CWPlantLoc.loopNum).FluidIndex,
                                                       RoutineName);

    const auto compressorSequence = rep->performance.performance_map_cooling.grid_variables.compressor_sequence_number;
    // minmax_element is sound but perhaps overkill; as sequence numbers are required by A205 to be in ascending order
    const auto minmaxSequenceNum = std::minmax_element(compressorSequence.begin(), compressorSequence.end());
    const auto minSequenceNum = *(minmaxSequenceNum.first);
    const auto maxSequenceNum = *(minmaxSequenceNum.second);

    // Calculate mass flow rate based on MyLoad (TODO: then adjust it after determining if chiller can meet the load)
    this->findEvaporatorMassFlowRate(state, MyLoad, Cp);

    // Available chiller capacity is capacity at the highest sequence number; i.e. max chiller capacity
    const Real64 maximumChillerCap = rep->performance.performance_map_cooling
                                         .Calculate_performance(this->EvapVolFlowRate,
                                                                this->EvapOutletTemp + DataGlobalConstants::KelvinConv,
                                                                this->CondVolFlowRate,
                                                                this->CondInletTemp + DataGlobalConstants::KelvinConv,
                                                                maxSequenceNum)
                                         .net_evaporator_capacity;
    if (maximumChillerCap <= 0) {
        // TODO: Issue a warning
    }
    const Real64 minimumChillerCap = rep->performance.performance_map_cooling
                                         .Calculate_performance(this->EvapVolFlowRate,
                                                                this->EvapOutletTemp + DataGlobalConstants::KelvinConv,
                                                                this->CondVolFlowRate,
                                                                this->CondInletTemp + DataGlobalConstants::KelvinConv,
                                                                minSequenceNum)
                                         .net_evaporator_capacity;
    // Part load ratio based on load and available chiller capacity; cap at max P.L.R. (can be >1)
    this->ChillerPartLoadRatio = (maximumChillerCap > 0) ? max(0.0, std::abs(MyLoad) / maximumChillerCap) : 0.0;
    // Minimum capacity ratio, under which cycling occurs
    this->MinPartLoadRat = (maximumChillerCap > 0) ? minimumChillerCap / maximumChillerCap : 0.0;
    Real64 partLoadSeqNum{0.};

    // Chiller may be operating in one of three modes: cycling, modulating, or full capacity
    if (this->ChillerPartLoadRatio < this->MinPartLoadRat) // Cycling
    {
        this->ChillerCyclingRatio = this->ChillerPartLoadRatio / this->MinPartLoadRat;
        partLoadSeqNum = minSequenceNum;
    } else if (this->ChillerPartLoadRatio < 1.0) // Modulating
    {
        // Use performance map to find the fractional sequence number (which most closely matches our part load)
        auto f = std::bind(&ASHRAE205ChillerSpecs::findCapacityResidual, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        Real64 constexpr accuracy{0.0001};
        int constexpr maxIter{500};
        int solFla{0};
        std::array<Real64, 4> par{{MyLoad, RunFlag ? 1.0 : 0.0, 1.0}}; // Initialize iteration parameters for RegulaFalsi function
        // Iteratively calculate this->QEvaporator by modulating partLoadSeqNum, ending at Q_Evaporator(partLoadSeqNum)
        General::SolveRoot(state, accuracy, maxIter, solFla, partLoadSeqNum, f, minSequenceNum, maxSequenceNum, par);
    } else // Full capacity: std::abs(MyLoad) > this->QEvaporator
    {
        this->QEvaporator = maximumChillerCap;
        partLoadSeqNum = maxSequenceNum;
        // SolveRoot stuff for eventual flow rate (can always calculate Ts if you have MFR and capacity)
        // recursion? Revisit.
        findEvaporatorMassFlowRate(state, this->QEvaporator, Cp);
        // if MFR changes, recalculate chiller capacity.
        // repeat until load <= capacity
    }

    // Use performance map to get the rest of results at new sequence number
    auto lookupVariablesCooling =
        rep->performance.performance_map_cooling.Calculate_performance(this->EvapVolFlowRate,
                                                                       this->EvapOutletTemp + DataGlobalConstants::KelvinConv,
                                                                       this->CondVolFlowRate,
                                                                       this->CondInletTemp + DataGlobalConstants::KelvinConv,
                                                                       partLoadSeqNum);
    this->QEvaporator = lookupVariablesCooling.net_evaporator_capacity * this->ChillerCyclingRatio;

    // this->EvapInletTemp is not set from results, as it's overspecified
    this->CondOutletTemp = lookupVariablesCooling.condenser_liquid_leaving_temperature - DataGlobalConstants::KelvinConv;
    auto evapDeltaTemp = this->QEvaporator / this->EvapMassFlowRate / Cp;
    this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp - evapDeltaTemp;

    // TODO: Revisit fault
#if 0
        // If there is a fault of Chiller SWT Sensor
        if (this->FaultyChillerSWTFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation) && (this->EvapMassFlowRate > 0)) {
            // calculate directly affected variables at faulty case: EvapOutletTemp, EvapMassFlowRate, QEvaporator
            int FaultIndex = this->FaultyChillerSWTIndex;
            bool VarFlowFlag = false;
            state.dataFaultsMgr->FaultsChillerSWTSensor(FaultIndex)
                    .CalFaultChillerSWT(VarFlowFlag,
                                        this->FaultyChillerSWTOffset,
                                        Cp,
                                        state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp,
                                        this->EvapOutletTemp,
                                        this->EvapMassFlowRate,
                                        this->QEvaporator);
            // update corresponding variables at faulty case
        }
#endif // 0

    auto cd = rep->performance.cycling_degradation_coefficient;
    Real64 cyclingFactor{(1.0 - cd) + (cd * this->ChillerCyclingRatio)};
    Real64 runtimeFactor{this->ChillerCyclingRatio / cyclingFactor};
    this->Power = lookupVariablesCooling.input_power * runtimeFactor + ((1 - this->ChillerCyclingRatio) * standbyPower);
    this->QCondenser = lookupVariablesCooling.net_condenser_capacity * this->ChillerCyclingRatio;
    this->AmbientZoneGain = this->QEvaporator + this->Power - (this->QCondenser + this->QOilHeater + this->QAuxiliary);

    if (this->CondMassFlowRate > DataBranchAirLoopPlant::MassFlowTolerance) {
        // If Heat Recovery specified for this vapor compression chiller, then QCondenser will be adjusted by this subroutine
        // if (this->HeatRecActive) this->calcHeatRecovery(state, this->QCondenser, this->CondMassFlowRate, condInletTemp, this->QHeatRecovery);
        Cp = FluidProperties::GetSpecificHeatGlycol(state,
                                                    state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).FluidName,
                                                    condInletTemp,
                                                    state.dataPlnt->PlantLoop(this->CDPlantLoc.loopNum).FluidIndex,
                                                    RoutineName);
        this->CondOutletTemp = this->QCondenser / this->CondMassFlowRate / Cp + condInletTemp;
    } else {
        ShowSevereError(state, "ControlReformEIRChillerModel: Condenser flow = 0, for ElecReformEIRChiller=" + this->Name);
        ShowContinueErrorTimeStamp(state, "");
    }

    // Oil cooler and Auxiliary Heat delta-T calculations
    if (this->OilCoolerInletNode) {
        auto oilCoolerDeltaTemp{0.0};
        PlantUtilities::SetComponentFlowRate(
            state, this->OilCoolerMassFlowRate, this->OilCoolerInletNode, this->OilCoolerOutletNode, this->OCPlantLoc);
        if (this->OilCoolerMassFlowRate != 0.0) {
            oilCoolerDeltaTemp = lookupVariablesCooling.oil_cooler_heat / this->OilCoolerMassFlowRate / Cp;
        } else {
            oilCoolerDeltaTemp = 0.0;
        }
        state.dataLoopNodes->Node(this->OilCoolerOutletNode).Temp = state.dataLoopNodes->Node(this->OilCoolerInletNode).Temp - oilCoolerDeltaTemp;
    }
    if (this->AuxiliaryHeatInletNode) {
        auto auxiliaryDeltaTemp{0.0};
        PlantUtilities::SetComponentFlowRate(
            state, this->AuxiliaryMassFlowRate, this->AuxiliaryHeatInletNode, this->AuxiliaryHeatOutletNode, this->AHPlantLoc);
        if (this->AuxiliaryMassFlowRate != 0.0) {
            auxiliaryDeltaTemp = lookupVariablesCooling.auxiliary_heat / this->AuxiliaryMassFlowRate / Cp;
        } else {
            auxiliaryDeltaTemp = 0.0;
        }
        state.dataLoopNodes->Node(this->AuxiliaryHeatOutletNode).Temp =
            state.dataLoopNodes->Node(this->AuxiliaryHeatInletNode).Temp - auxiliaryDeltaTemp;
    }
}

void ASHRAE205ChillerSpecs::update(EnergyPlusData &state, Real64 const MyLoad, bool const RunFlag)
{
    if (MyLoad >= 0.0 || !RunFlag) { // Chiller not running so pass inlet states to outlet states
        // Set node temperatures
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;

        this->ChillerPartLoadRatio = 0.0;
        this->ChillerCyclingRatio = 0.0;
        this->ChillerFalseLoadRate = 0.0;
        this->ChillerFalseLoad = 0.0;
        this->Power = 0.0;
        this->QEvaporator = 0.0;
        this->QCondenser = 0.0;
        this->Energy = 0.0;
        this->EvapEnergy = 0.0;
        this->CondEnergy = 0.0;
        this->EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        this->EvapOutletTemp = state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp;
        this->ActualCOP = 0.0;

    } else { // Chiller is running, so pass calculated values
        // Set node temperatures
        state.dataLoopNodes->Node(this->EvapOutletNodeNum).Temp = this->EvapOutletTemp;
        state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp = this->CondOutletTemp;
        // Set node flow rates;  for these load based models
        // assume that sufficient evaporator flow rate is available
        this->Energy = this->Power * state.dataHVACGlobal->TimeStepSys * DataGlobalConstants::SecInHour;
        this->EvapEnergy = this->QEvaporator * state.dataHVACGlobal->TimeStepSys * DataGlobalConstants::SecInHour;
        this->CondEnergy = this->QCondenser * state.dataHVACGlobal->TimeStepSys * DataGlobalConstants::SecInHour;
        this->EvapInletTemp = state.dataLoopNodes->Node(this->EvapInletNodeNum).Temp;
        this->CondInletTemp = state.dataLoopNodes->Node(this->CondInletNodeNum).Temp;
        this->CondOutletTemp = state.dataLoopNodes->Node(this->CondOutletNodeNum).Temp;
        if (this->Power != 0.0) {
            this->ActualCOP = this->QEvaporator / this->Power;
        } else {
            this->ActualCOP = 0.0;
        }
    }
}

void ASHRAE205ChillerSpecs::simulate(
    EnergyPlusData &state, const PlantLocation &calledFromLocation, bool FirstHVACIteration, Real64 &CurLoad, bool RunFlag)
{
    if (calledFromLocation.loopNum == this->CWPlantLoc.loopNum) {
        this->initialize(state, RunFlag, CurLoad);
        this->calculate(state, CurLoad, RunFlag);
        this->update(state, CurLoad, RunFlag);
    } else if (calledFromLocation.loopNum == this->CDPlantLoc.loopNum) {
        DataPlant::LoopSideLocation LoopSide = this->CDPlantLoc.loopSideNum;
        PlantUtilities::UpdateChillerComponentCondenserSide(state,
                                                            calledFromLocation.loopNum,
                                                            LoopSide,
                                                            DataPlant::PlantEquipmentType::Chiller_ElectricReformEIR,
                                                            this->CondInletNodeNum,
                                                            this->CondOutletNodeNum,
                                                            this->QCondenser,
                                                            this->CondInletTemp,
                                                            this->CondOutletTemp,
                                                            this->CondMassFlowRate,
                                                            FirstHVACIteration);
    }
}

} // namespace EnergyPlus::ChillerElectricASHRAE205
