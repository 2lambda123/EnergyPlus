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

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/RuntimeLanguageProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

// note there are routines that lie outside of the Module at the end of this file

namespace EMSManager {

    // MODULE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith
    //                      May - August 2009

    // PURPOSE OF THIS MODULE:
    // This module manages the programmable energy management system(EMS).

    constexpr std::array<std::string_view, static_cast<int>(EMSCallFrom::Num)> EMSCallFromNamesUC{"ENDOFZONESIZING",
                                                                                                  "ENDOFSYSTEMSIZING",
                                                                                                  "BEGINNEWENVIRONMENT",
                                                                                                  "AFTERNEWENVIRONMENTWARMUPISCOMPLETE",
                                                                                                  "BEGINTIMESTEPBEFOREPREDICTOR",
                                                                                                  "AFTERPREDICTORBEFOREHVACMANAGERS",
                                                                                                  "AFTERPREDICTORAFTERHVACMANAGERS",
                                                                                                  "INSIDEHVACSYSTEMITERATIONLOOP",
                                                                                                  "ENDOFSYSTEMTIMESTEPBEFOREHVACREPORTING",
                                                                                                  "ENDOFSYSTEMTIMESTEPAFTERHVACREPORTING",
                                                                                                  "ENDOFZONETIMESTEPBEFOREZONEREPORTING",
                                                                                                  "ENDOFZONETIMESTEPAFTERZONEREPORTING",
                                                                                                  "SETUPSIMULATION",
                                                                                                  "EXTERNALINTERFACE",
                                                                                                  "AFTERCOMPONENTINPUTREADIN",
                                                                                                  "USERDEFINEDCOMPONENTMODEL",
                                                                                                  "UNITARYSYSTEMSIZING",
                                                                                                  "BEGINZONETIMESTEPBEFOREINITHEATBALANCE",
                                                                                                  "BEGINZONETIMESTEPAFTERINITHEATBALANCE",
                                                                                                  "BEGINZONETIMESTEPBEFORESETCURRENTWEATHER"};

    constexpr std::array<std::string_view, static_cast<int>(SPControlType::Num)> controlTypeName{"Temperature Setpoint",
                                                                                                 "Temperature Minimum Setpoint",
                                                                                                 "Temperature Maximum Setpoint",
                                                                                                 "Humidity Ratio Setpoint",
                                                                                                 "Humidity Ratio Minimum Setpoint",
                                                                                                 "Humidity Ratio Maximum Setpoint",
                                                                                                 "Mass Flow Rate Setpoint",
                                                                                                 "Mass Flow Rate Minimum Available Setpoint",
                                                                                                 "Mass Flow Rate Maximum Available Setpoint"};

    void CheckIfAnyEMS(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   April 2009
        //       MODIFIED       Rui Zhang February 2010

        // PURPOSE OF THIS SUBROUTINE:
        // Determine if EMS is used in model and set flag
        // This needs to be checked early so calls to SetupEMSActuator
        // can be avoided if there is no EMS in model.
        // We cannot do error checking during the full get input until later in the simulation.

        // METHODOLOGY EMPLOYED:
        // Get number of EMS-related input objects and set
        // global logical AnyEnergyManagementSystemInModel

        std::string cCurrentModuleObject = "EnergyManagementSystem:Sensor";
        state.dataRuntimeLang->NumSensors = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:Actuator";
        state.dataRuntimeLang->numActuatorsUsed = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:ProgramCallingManager";
        state.dataRuntimeLang->NumProgramCallManagers = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:Program";
        state.dataRuntimeLang->NumErlPrograms = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:Subroutine";
        state.dataRuntimeLang->NumErlSubroutines = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:GlobalVariable";
        state.dataRuntimeLang->NumUserGlobalVariables = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:OutputVariable";
        state.dataRuntimeLang->NumEMSOutputVariables = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:MeteredOutputVariable";
        state.dataRuntimeLang->NumEMSMeteredOutputVariables =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:CurveOrTableIndexVariable";
        state.dataRuntimeLang->NumEMSCurveIndices = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "ExternalInterface:Variable";
        state.dataRuntimeLang->NumExternalInterfaceGlobalVariables =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        // added for FMUImport
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Variable";
        state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        // added for FMUExport
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Variable";
        state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportGlobalVariables =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "ExternalInterface:Actuator";
        state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        // added for FMUImport
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Actuator";
        state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        // added for FMUExport
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Actuator";
        state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "EnergyManagementSystem:ConstructionIndexVariable";
        state.dataRuntimeLang->NumEMSConstructionIndices = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        cCurrentModuleObject = "Output:EnergyManagementSystem";
        int NumOutputEMSs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        // Python plugin instances also count since actuators need to be set up for them
        int numPythonPlugins = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "PythonPlugin:Instance");
        int numActiveCallbacks = PluginManagement::PluginManager::numActiveCallbacks(state);

        // added for FMU
        if ((state.dataRuntimeLang->NumSensors + state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumProgramCallManagers +
             state.dataRuntimeLang->NumErlPrograms + state.dataRuntimeLang->NumErlSubroutines + state.dataRuntimeLang->NumUserGlobalVariables +
             state.dataRuntimeLang->NumEMSOutputVariables + state.dataRuntimeLang->NumEMSCurveIndices +
             state.dataRuntimeLang->NumExternalInterfaceGlobalVariables + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
             state.dataRuntimeLang->NumEMSConstructionIndices + state.dataRuntimeLang->NumEMSMeteredOutputVariables +
             state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed +
             state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportGlobalVariables +
             state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed +
             state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportGlobalVariables + NumOutputEMSs + numPythonPlugins +
             numActiveCallbacks) > 0) {
            state.dataGlobal->AnyEnergyManagementSystemInModel = true;
        } else {
            state.dataGlobal->AnyEnergyManagementSystemInModel = false;
        }

        state.dataGlobal->AnyEnergyManagementSystemInModel =
            state.dataGlobal->AnyEnergyManagementSystemInModel || state.dataGlobal->externalHVACManager;

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {

            General::ScanForReports(state, "EnergyManagementSystem", state.dataRuntimeLang->OutputEDDFile);
            if (state.dataRuntimeLang->OutputEDDFile) {
                // open up output file for EMS EDD file  EMS Data and Debug
                state.files.edd.ensure_open(state, "CheckIFAnyEMS", state.files.outputControl.edd);
            }
        } else {
            General::ScanForReports(state, "EnergyManagementSystem", state.dataRuntimeLang->OutputEDDFile);
            if (state.dataRuntimeLang->OutputEDDFile) {
                ShowWarningError(state, "CheckIFAnyEMS: No EnergyManagementSystem has been set up in the input file but output is requested.");
                ShowContinueError(state,
                                  "No EDD file will be produced. Refer to EMS Application Guide and/or InputOutput Reference to set up your "
                                  "EnergyManagementSystem.");
            }
        }
    }

    // MODULE SUBROUTINES:

    void ManageEMS(EnergyPlusData &state,
                   EMSCallFrom const iCalledFrom,                    // indicates where subroutine was called from, parameters in DataGlobals.
                   bool &anyProgramRan,                              // true if any Erl programs ran for this call
                   ObjexxFCL::Optional_int_const ProgramManagerToRun // specific program manager to run
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   June 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  Brent Griffith, April 2009
        //                      added calling point argument and logic.
        //                      Collapsed SimulateEMS into this routine

        anyProgramRan = false;
        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) return; // quick return if nothing to do

        if (iCalledFrom == EMSCallFrom::BeginNewEnvironment) {
            RuntimeLanguageProcessor::BeginEnvrnInitializeRuntimeLanguage(state);
            PluginManagement::onBeginEnvironment(state);
        }

        InitEMS(state, iCalledFrom);

        // also call plugins and callbacks here for convenience
        if (iCalledFrom != EMSCallFrom::UserDefinedComponentModel) { // don't run user-defined component plugins this way
            bool anyPluginsOrCallbacksRan = false;
            PluginManagement::runAnyRegisteredCallbacks(state, iCalledFrom, anyPluginsOrCallbacksRan);
            if (anyPluginsOrCallbacksRan) {
                anyProgramRan = true;
            }
        }

        if (iCalledFrom == EMSCallFrom::SetupSimulation) {
            ProcessEMSInput(state, true);
            return;
        }

        // Run the Erl programs depending on calling point.

        if (iCalledFrom != EMSCallFrom::UserDefinedComponentModel) {
            for (int ProgramManagerNum = 1; ProgramManagerNum <= state.dataRuntimeLang->NumProgramCallManagers; ++ProgramManagerNum) {

                if (state.dataRuntimeLang->EMSProgramCallManager(ProgramManagerNum).CallingPoint == iCalledFrom) {
                    for (int ErlProgramNum = 1; ErlProgramNum <= state.dataRuntimeLang->EMSProgramCallManager(ProgramManagerNum).NumErlPrograms;
                         ++ErlProgramNum) {
                        RuntimeLanguageProcessor::EvaluateStack(
                            state, state.dataRuntimeLang->EMSProgramCallManager(ProgramManagerNum).ErlProgramARR(ErlProgramNum));
                        anyProgramRan = true;
                    }
                }
            }
        } else { // call specific program manager
            if (present(ProgramManagerToRun)) {
                for (int ErlProgramNum = 1; ErlProgramNum <= state.dataRuntimeLang->EMSProgramCallManager(ProgramManagerToRun).NumErlPrograms;
                     ++ErlProgramNum) {
                    RuntimeLanguageProcessor::EvaluateStack(
                        state, state.dataRuntimeLang->EMSProgramCallManager(ProgramManagerToRun).ErlProgramARR(ErlProgramNum));
                    anyProgramRan = true;
                }
            }
        }

        if (iCalledFrom == EMSCallFrom::ExternalInterface) {
            anyProgramRan = true;
        }

        if (!anyProgramRan) return;

        // Set actuated variables with new values
        for (int ActuatorUsedLoop = 1;
             ActuatorUsedLoop <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                                     state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed +
                                     state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed;
             ++ActuatorUsedLoop) {
            int ErlVariableNum = state.dataRuntimeLang->EMSActuatorUsed(ActuatorUsedLoop).ErlVariableNum;
            if (ErlVariableNum <= 0) continue; // this can happen for good reason during sizing

            int EMSActuatorVariableNum = state.dataRuntimeLang->EMSActuatorUsed(ActuatorUsedLoop).ActuatorVariableNum;
            if (EMSActuatorVariableNum <= 0) continue; // this can happen for good reason during sizing

            if (state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value.Type == DataRuntimeLanguage::Value::Null) {
                *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).Actuated = false;
            } else {
                // Set the value and the actuated flag remotely on the actuated object via the pointer
                switch (state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).PntrVarTypeUsed) {
                case DataRuntimeLanguage::PtrDataType::Real: {
                    *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).Actuated = true;
                    *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).RealValue =
                        state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value.Number;
                } break;
                case DataRuntimeLanguage::PtrDataType::Integer: {
                    *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).Actuated = true;
                    int tmpInteger = std::floor(state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value.Number);
                    *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).IntValue = tmpInteger;
                } break;
                case DataRuntimeLanguage::PtrDataType::Logical: {
                    *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).Actuated = true;
                    if (state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value.Number == 0.0) {
                        *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).LogValue = false;
                    } else if (state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value.Number == 1.0) {
                        *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).LogValue = true;
                    } else {
                        *state.dataRuntimeLang->EMSActuatorAvailable(EMSActuatorVariableNum).LogValue = false;
                    }
                } break;
                default:
                    break;
                }
            }
        }

        ReportEMS(state);
    }

    void InitEMS(EnergyPlusData &state, EMSCallFrom const iCalledFrom) // indicates where subroutine was called from, parameters in DataGlobals.
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // collect routines needed to initialize EMS

        if (state.dataEMSMgr->GetEMSUserInput) {
            SetupZoneInfoAsInternalDataAvail(state);
            SetupWindowShadingControlActuators(state);
            SetupSurfaceConvectionActuators(state);
            SetupSurfaceConstructionActuators(state);
            SetupSurfaceOutdoorBoundaryConditionActuators(state);
            SetupZoneOutdoorBoundaryConditionActuators(state);
            GetEMSInput(state);
            state.dataEMSMgr->GetEMSUserInput = false;
        }

        if (!state.dataZoneCtrls->GetZoneAirStatsInputFlag && !state.dataEMSMgr->ZoneThermostatActuatorsHaveBeenSetup) {
            SetupThermostatActuators(state);
            state.dataEMSMgr->ZoneThermostatActuatorsHaveBeenSetup = true;
        }

        // need to delay setup of HVAC actuator until after the systems input has been processed (if present)
        if (state.dataEMSMgr->FinishProcessingUserInput && !state.dataGlobal->DoingSizing && !state.dataGlobal->KickOffSimulation) {
            SetupNodeSetPointsAsActuators(state);
            SetupPrimaryAirSystemAvailMgrAsActuators(state);
            //    CALL SetupWindowShadingControlActuators !this is too late for including in sizing, moved to GetEMSUserInput
            //    CALL SetupThermostatActuators !this is too late for including in sizing, moved to GetEMSUserInput
            //    CALL SetupSurfaceConvectionActuators !this is too late for including in sizing, moved to GetEMSUserInput
            state.dataEMSMgr->FinishProcessingUserInput = false;
        }

        RuntimeLanguageProcessor::InitializeRuntimeLanguage(state);

        if ((state.dataGlobal->BeginEnvrnFlag) || (iCalledFrom == EMSCallFrom::ZoneSizing) || (iCalledFrom == EMSCallFrom::SystemSizing) ||
            (iCalledFrom == EMSCallFrom::UserDefinedComponentModel)) {

            // another pass at trying to setup input data.
            if (state.dataEMSMgr->FinishProcessingUserInput) {
                ProcessEMSInput(state, false);
            }

            // update internal data variables being used by Erl
            for (int InternalVarUsedNum = 1; InternalVarUsedNum <= state.dataRuntimeLang->NumInternalVariablesUsed; ++InternalVarUsedNum) {
                int ErlVariableNum = state.dataRuntimeLang->EMSInternalVarsUsed(InternalVarUsedNum).ErlVariableNum;
                int InternVarAvailNum = state.dataRuntimeLang->EMSInternalVarsUsed(InternalVarUsedNum).InternVarNum;
                if (InternVarAvailNum <= 0) continue; // sometimes executes before completely finished setting up.
                if (ErlVariableNum <= 0) continue;

                switch (state.dataRuntimeLang->EMSInternalVarsAvailable(InternVarAvailNum).PntrVarTypeUsed) {
                case DataRuntimeLanguage::PtrDataType::Real: {
                    state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value =
                        RuntimeLanguageProcessor::SetErlValueNumber(*state.dataRuntimeLang->EMSInternalVarsAvailable(InternVarAvailNum).RealValue);
                } break;
                case DataRuntimeLanguage::PtrDataType::Integer: {
                    Real64 tmpReal = double(*state.dataRuntimeLang->EMSInternalVarsAvailable(InternVarAvailNum).IntValue);
                    state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value = RuntimeLanguageProcessor::SetErlValueNumber(tmpReal);
                } break;
                default:
                    break;
                }
            }
        }

        // Update sensors with current data
        for (int SensorNum = 1; SensorNum <= state.dataRuntimeLang->NumSensors; ++SensorNum) {
            int ErlVariableNum = state.dataRuntimeLang->Sensor(SensorNum).VariableNum;
            if ((ErlVariableNum > 0) && (state.dataRuntimeLang->Sensor(SensorNum).Index > 0)) {
                if (state.dataRuntimeLang->Sensor(SensorNum).SchedNum == 0) { // not a schedule so get from output processor

                    state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value = RuntimeLanguageProcessor::SetErlValueNumber(
                        GetInternalVariableValue(
                            state, state.dataRuntimeLang->Sensor(SensorNum).VariableType, state.dataRuntimeLang->Sensor(SensorNum).Index),
                        state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value);
                } else { // schedule so use schedule service

                    state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value = RuntimeLanguageProcessor::SetErlValueNumber(
                        ScheduleManager::GetCurrentScheduleValue(state, state.dataRuntimeLang->Sensor(SensorNum).SchedNum),
                        state.dataRuntimeLang->ErlVariable(ErlVariableNum).Value);
                }
            }
        }
    }

    void ReportEMS(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   June 2006
        // PURPOSE OF THIS SUBROUTINE:
        // Calculates report variables.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        RuntimeLanguageProcessor::ReportRuntimeLanguage(state);
    }

    void GetEMSInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   June 2006
        //       MODIFIED       BG April 2009, finishing, renaming, etc.

        // PURPOSE OF THIS SUBROUTINE:
        // Gets the EMS input from the input file.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int IOStat;    // IO Status when calling get input subroutine
        bool ErrorsFound(false);
        Array1D_string cAlphaFieldNames;
        Array1D_string cNumericFieldNames;
        Array1D_bool lNumericFieldBlanks;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        std::string cCurrentModuleObject;
        OutputProcessor::VariableType VarType;
        int TotalArgs(0); // argument for call to GetObjectDefMaxArgs
        bool errFlag;

        cCurrentModuleObject = "EnergyManagementSystem:Sensor";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        int MaxNumNumbers = NumNums;
        int MaxNumAlphas = NumAlphas;
        cCurrentModuleObject = "EnergyManagementSystem:Actuator";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:ProgramCallingManager";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:Program";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:Subroutine";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "EnergyManagementSystem:OutputVariable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:Variable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:Actuator";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Variable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Actuator";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Variable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Actuator";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);
        //  cCurrentModuleObject = 'EnergyManagementSystem:Sensor'
        //  CALL state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(cCurrentModuleObject,TotalArgs,NumAlphas,NumNums)
        //  MaxNumNumbers=MAX(MaxNumNumbers,NumNums)
        //  MaxNumAlphas=MAX(MaxNumAlphas,NumAlphas)
        cCurrentModuleObject = "EnergyManagementSystem:GlobalVariable";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);
        MaxNumNumbers = max(MaxNumNumbers, NumNums);
        MaxNumAlphas = max(MaxNumAlphas, NumAlphas);

        cAlphaFieldNames.allocate(MaxNumAlphas);
        cAlphaArgs.allocate(MaxNumAlphas);
        lAlphaFieldBlanks.dimension(MaxNumAlphas, false);
        cNumericFieldNames.allocate(MaxNumNumbers);
        rNumericArgs.dimension(MaxNumNumbers, 0.0);
        lNumericFieldBlanks.dimension(MaxNumNumbers, false);

        cCurrentModuleObject = "EnergyManagementSystem:Sensor";
        if (state.dataRuntimeLang->NumSensors > 0) {
            state.dataRuntimeLang->Sensor.allocate(state.dataRuntimeLang->NumSensors);

            for (int SensorNum = 1; SensorNum <= state.dataRuntimeLang->NumSensors; ++SensorNum) {
                auto &thisSensor = state.dataRuntimeLang->Sensor(SensorNum);
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         SensorNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);
                DataRuntimeLanguage::ValidateEMSVariableName(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(1), errFlag, ErrorsFound);
                if (!errFlag) {
                    thisSensor.Name = cAlphaArgs(1);

                    // really needs to check for conflicts with program and function names too...done later
                    int VariableNum = RuntimeLanguageProcessor::FindEMSVariable(state, cAlphaArgs(1), 0);

                    if (VariableNum > 0) {
                        ShowSevereError(state, format("Invalid {}={}", cAlphaFieldNames(1), cAlphaArgs(1)));
                        ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, "Object name conflicts with a global variable name in EMS");
                        ErrorsFound = true;
                    } else {
                        VariableNum = RuntimeLanguageProcessor::NewEMSVariable(state, cAlphaArgs(1), 0);
                        thisSensor.VariableNum = VariableNum;
                        state.dataRuntimeLang->ErlVariable(VariableNum).Value.initialized = true;
                    }
                }

                if (cAlphaArgs(2) == "*") cAlphaArgs(2).clear();
                thisSensor.UniqueKeyName = cAlphaArgs(2);
                thisSensor.OutputVarName = cAlphaArgs(3);

                int VarIndex = GetMeterIndex(state, cAlphaArgs(3));
                if (VarIndex > 0) {
                    if (!lAlphaFieldBlanks(2)) {
                        ShowWarningError(state, format("Unused{}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, "Meter Name found; Key Name will be ignored"); // why meters have no keys..
                    } else {
                        thisSensor.VariableType = OutputProcessor::VariableType::Meter;
                        thisSensor.Index = VarIndex;
                        thisSensor.CheckedOkay = true;
                    }
                } else {
                    // Search for variable names
                    GetVariableTypeAndIndex(state, cAlphaArgs(3), cAlphaArgs(2), VarType, VarIndex);
                    if (VarType != OutputProcessor::VariableType::NotFound) {
                        thisSensor.VariableType = VarType;
                        if (VarIndex != 0) {
                            thisSensor.Index = VarIndex;
                            thisSensor.CheckedOkay = true;
                        }
                    }
                }

            } // SensorNum
        }

        cCurrentModuleObject = "EnergyManagementSystem:Actuator";

        if (state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed +
                state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed >
            0) {
            state.dataRuntimeLang->EMSActuatorUsed.allocate(state.dataRuntimeLang->numActuatorsUsed +
                                                            state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                                                            state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed +
                                                            state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed);
            for (int ActuatorNum = 1;
                 ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                                    state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed +
                                    state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed;
                 ++ActuatorNum) {
                auto &thisEMSactuator = state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum);
                // If we process the ExternalInterface actuators, all we need to do is to change the
                // name of the module object, and shift the ActuatorNum in GetObjectItem
                if (ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed) {
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             cCurrentModuleObject,
                                                                             ActuatorNum,
                                                                             cAlphaArgs,
                                                                             NumAlphas,
                                                                             rNumericArgs,
                                                                             NumNums,
                                                                             IOStat,
                                                                             lNumericFieldBlanks,
                                                                             lAlphaFieldBlanks,
                                                                             cAlphaFieldNames,
                                                                             cNumericFieldNames);
                } else if (ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed) {
                    cCurrentModuleObject = "ExternalInterface:Actuator";
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             cCurrentModuleObject,
                                                                             ActuatorNum - state.dataRuntimeLang->numActuatorsUsed,
                                                                             cAlphaArgs,
                                                                             NumAlphas,
                                                                             rNumericArgs,
                                                                             NumNums,
                                                                             IOStat,
                                                                             lNumericFieldBlanks,
                                                                             lAlphaFieldBlanks,
                                                                             cAlphaFieldNames,
                                                                             cNumericFieldNames);
                } else if (ActuatorNum <= (state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                                           state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed)) {
                    cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Actuator";
                    state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                             cCurrentModuleObject,
                                                                             ActuatorNum - state.dataRuntimeLang->numActuatorsUsed -
                                                                                 state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed,
                                                                             cAlphaArgs,
                                                                             NumAlphas,
                                                                             rNumericArgs,
                                                                             NumNums,
                                                                             IOStat,
                                                                             lNumericFieldBlanks,
                                                                             lAlphaFieldBlanks,
                                                                             cAlphaFieldNames,
                                                                             cNumericFieldNames);
                } else if (ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                                              state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed +
                                              state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed) {
                    cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Actuator";
                    state.dataInputProcessing->inputProcessor->getObjectItem(
                        state,
                        cCurrentModuleObject,
                        ActuatorNum - state.dataRuntimeLang->numActuatorsUsed - state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed -
                            state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed,
                        cAlphaArgs,
                        NumAlphas,
                        rNumericArgs,
                        NumNums,
                        IOStat,
                        lNumericFieldBlanks,
                        lAlphaFieldBlanks,
                        cAlphaFieldNames,
                        cNumericFieldNames);
                }

                DataRuntimeLanguage::ValidateEMSVariableName(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(1), errFlag, ErrorsFound);
                if (!errFlag) {
                    thisEMSactuator.Name = cAlphaArgs(1);

                    // really needs to check for conflicts with program and function names too...
                    int VariableNum = RuntimeLanguageProcessor::FindEMSVariable(state, cAlphaArgs(1), 0);

                    if (VariableNum > 0) {
                        ShowSevereError(state, format("Invalid {}={}", cAlphaFieldNames(1), cAlphaArgs(1)));
                        ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, "Object name conflicts with a global variable name in EMS");
                        ErrorsFound = true;
                    } else {
                        VariableNum = RuntimeLanguageProcessor::NewEMSVariable(state, cAlphaArgs(1), 0);
                        thisEMSactuator.ErlVariableNum = VariableNum;
                        // initialize Erl variable for actuator to null
                        state.dataRuntimeLang->ErlVariable(VariableNum).Value = state.dataRuntimeLang->Null;
                        if (ActuatorNum > state.dataRuntimeLang->numActuatorsUsed) {
                            // Initialize variables for the ExternalInterface variables
                            RuntimeLanguageProcessor::ExternalInterfaceInitializeErlVariable(
                                state, VariableNum, RuntimeLanguageProcessor::SetErlValueNumber(rNumericArgs(1)), lNumericFieldBlanks(1));
                        }
                    }
                }

                // need to store characters to finish processing later (once available Actuators have all been setup)
                thisEMSactuator.ComponentTypeName = cAlphaArgs(3);
                thisEMSactuator.UniqueIDName = cAlphaArgs(2);
                thisEMSactuator.ControlTypeName = cAlphaArgs(4);

                int ActuatorVariableNum;
                bool FoundActuatorName = false;
                for (ActuatorVariableNum = 1; ActuatorVariableNum <= state.dataRuntimeLang->numEMSActuatorsAvailable; ++ActuatorVariableNum) {
                    if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).ComponentTypeName,
                                                    cAlphaArgs(3))) {
                        if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).UniqueIDName,
                                                        cAlphaArgs(2))) {
                            if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).ControlTypeName,
                                                            cAlphaArgs(4))) {
                                FoundActuatorName = true;
                                break;
                            }
                        }
                    }
                }

                if (FoundActuatorName) {
                    // SetupNodeSetPointAsActuators has NOT been called yet at this point
                    thisEMSactuator.ActuatorVariableNum = ActuatorVariableNum;
                    thisEMSactuator.CheckedOkay = true;

                    int nHandle = state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).handleCount;
                    if (nHandle > 0) {
                        EnergyPlus::ShowWarningError(state,
                                                     format("Seems like you already tried to get a Handle on this Actuator {}times.", nHandle));
                        EnergyPlus::ShowContinueError(state,
                                                      format("Occurred for componentType='{}', controlType='{}', uniqueKey='{}'.",
                                                             thisEMSactuator.ComponentTypeName,
                                                             thisEMSactuator.ControlTypeName,
                                                             thisEMSactuator.UniqueIDName));
                        EnergyPlus::ShowContinueError(state, "You should take note that there is a risk of overwritting.");
                    }
                    ++state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).handleCount;
                }
            } // ActuatorNum
        }

        cCurrentModuleObject = "EnergyManagementSystem:InternalVariable";
        state.dataRuntimeLang->NumInternalVariablesUsed = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataRuntimeLang->NumInternalVariablesUsed > 0) {
            state.dataRuntimeLang->EMSInternalVarsUsed.allocate(state.dataRuntimeLang->NumInternalVariablesUsed);

            for (int InternVarNum = 1; InternVarNum <= state.dataRuntimeLang->NumInternalVariablesUsed; ++InternVarNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         InternVarNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                DataRuntimeLanguage::ValidateEMSVariableName(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(1), errFlag, ErrorsFound);
                if (!errFlag) {
                    state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).Name = cAlphaArgs(1);
                    int VariableNum = RuntimeLanguageProcessor::FindEMSVariable(state, cAlphaArgs(1), 0);
                    if (VariableNum > 0) {
                        ShowSevereError(state, format("Invalid {}={}", cAlphaFieldNames(1), cAlphaArgs(1)));
                        ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, "Object name conflicts with a global variable name in EMS");
                        ErrorsFound = true;
                    } else {
                        VariableNum = RuntimeLanguageProcessor::NewEMSVariable(state, cAlphaArgs(1), 0);
                        state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).ErlVariableNum = VariableNum;
                    }

                    state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).UniqueIDName = cAlphaArgs(2);
                    state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).InternalDataTypeName = cAlphaArgs(3);

                    bool FoundObjectName = false;
                    int InternalVarAvailNum; // do loop counter for internal variables available (inner)
                    for (InternalVarAvailNum = 1; InternalVarAvailNum <= state.dataRuntimeLang->numEMSInternalVarsAvailable; ++InternalVarAvailNum) {
                        if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).DataTypeName,
                                                        cAlphaArgs(3))) {
                            if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).UniqueIDName,
                                                            cAlphaArgs(2))) {
                                FoundObjectName = true;
                                break; // InternalVarAvailNum now holds needed index pointer
                            }
                        }
                    }

                    if (FoundObjectName) {
                        state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).InternVarNum = InternalVarAvailNum;
                        state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).CheckedOkay = true;
                    }
                }
            }
        }

        RuntimeLanguageProcessor::InitializeRuntimeLanguage(
            state); // Loads built-in globals and functions, then performs GetInput for runtime language objects

        if (state.dataRuntimeLang->NumProgramCallManagers > 0) {
            cCurrentModuleObject = "EnergyManagementSystem:ProgramCallingManager";
            state.dataRuntimeLang->EMSProgramCallManager.allocate(state.dataRuntimeLang->NumProgramCallManagers);

            for (int CallManagerNum = 1; CallManagerNum <= state.dataRuntimeLang->NumProgramCallManagers; ++CallManagerNum) {

                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         CallManagerNum,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         lNumericFieldBlanks,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         cNumericFieldNames);

                state.dataRuntimeLang->EMSProgramCallManager(CallManagerNum).Name = cAlphaArgs(1);

                state.dataRuntimeLang->EMSProgramCallManager(CallManagerNum).CallingPoint =
                    static_cast<EMSCallFrom>(getEnumerationValue(EMSCallFromNamesUC, UtilityRoutines::MakeUPPERCase(cAlphaArgs(2))));
                ErrorsFound = ErrorsFound || (state.dataRuntimeLang->EMSProgramCallManager(CallManagerNum).CallingPoint == EMSCallFrom::Invalid);

                int NumErlProgramsThisManager = NumAlphas - 2; // temporary size of Erl programs in EMSProgramCallManager
                state.dataRuntimeLang->EMSProgramCallManager(CallManagerNum).NumErlPrograms = NumErlProgramsThisManager;
                state.dataRuntimeLang->EMSProgramCallManager(CallManagerNum).ErlProgramARR.allocate(NumErlProgramsThisManager);
                int ManagerProgramNum = 0; // index counter for Erl programs inside EMSProgramCallManager
                for (int AlphaNum = 3; AlphaNum <= NumAlphas; ++AlphaNum) {
                    // find program name in Stack structure
                    if (lAlphaFieldBlanks(AlphaNum)) { // throw error
                        ShowSevereError(state, format("Invalid {}={}", cAlphaFieldNames(AlphaNum), cAlphaArgs(AlphaNum)));
                        ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, "Program names cannot be blank");
                        ErrorsFound = true;
                    }

                    int StackNum = UtilityRoutines::FindItemInList(cAlphaArgs(AlphaNum), state.dataRuntimeLang->ErlStack);

                    if (StackNum > 0) { // found it
                        // check for duplicate and warn.
                        for (int Loop = 1; Loop <= ManagerProgramNum; ++Loop) {
                            if (state.dataRuntimeLang->EMSProgramCallManager(CallManagerNum).ErlProgramARR(Loop) == StackNum) {
                                ShowWarningError(state, format("Duplicate {}={}", cAlphaFieldNames(AlphaNum), cAlphaArgs(AlphaNum)));
                                ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(state, "Erl program appears more than once, and the simulation continues.");
                            }
                        }

                        ++ManagerProgramNum;

                        state.dataRuntimeLang->EMSProgramCallManager(CallManagerNum).ErlProgramARR(ManagerProgramNum) = StackNum;

                    } else {
                        ShowSevereError(state, format("Invalid {}={}", cAlphaFieldNames(AlphaNum), cAlphaArgs(AlphaNum)));
                        ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                        ShowContinueError(state, "Program Name not found.");
                        ErrorsFound = true;
                    }
                } // AlphaNum
            }

        } else { // no program calling manager in input
            if (state.dataRuntimeLang->NumErlPrograms > 0) {
                cCurrentModuleObject = "EnergyManagementSystem:ProgramCallingManager";
                ShowWarningError(state, format("Energy Management System is missing input object {}", cCurrentModuleObject));
                ShowContinueError(state, "EnergyPlus Runtime Language programs need a calling manager to control when they get executed");
            }
        }

        cAlphaFieldNames.deallocate();
        cAlphaArgs.deallocate();
        lAlphaFieldBlanks.deallocate();
        cNumericFieldNames.deallocate();
        rNumericArgs.deallocate();
        lNumericFieldBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting Energy Management System input. Preceding condition causes termination.");
        }
    }

    void ProcessEMSInput(EnergyPlusData &state, bool const reportErrors) // .  If true, then report out errors ,otherwise setup what we can
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // contains Some input checks that need to be deferred until later in the simulation

        // METHODOLOGY EMPLOYED:
        // Loop over objects doing input checks.
        // Had to break up get user input into two phases because
        // the actuators can't be set up until all the HVAC systems are read in, sized, etc.
        // but we also want to allow customizing sizing calcs which occur much earlier in the simulation.
        //  so here we do a final pass and throw the errors that would usually occur during get input.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        OutputProcessor::VariableType VarType;
        bool ErrorsFound(false);
        bool FoundObjectType;
        bool FoundObjectName;
        bool FoundActuatorName;
        int ActuatorVariableNum;
        int InternVarNum;        // local do loop index
        int InternalVarAvailNum; // local do loop index
        std::string cCurrentModuleObject;

        cCurrentModuleObject = "EnergyManagementSystem:Sensor";
        for (int SensorNum = 1; SensorNum <= state.dataRuntimeLang->NumSensors; ++SensorNum) {
            if (state.dataRuntimeLang->Sensor(SensorNum).CheckedOkay) continue;

            // try again to process sensor.
            int VarIndex = GetMeterIndex(state, state.dataRuntimeLang->Sensor(SensorNum).OutputVarName);
            if (VarIndex > 0) {

                state.dataRuntimeLang->Sensor(SensorNum).VariableType = OutputProcessor::VariableType::Meter;
                state.dataRuntimeLang->Sensor(SensorNum).Index = VarIndex;

            } else {
                // Search for variable names
                GetVariableTypeAndIndex(state,
                                        state.dataRuntimeLang->Sensor(SensorNum).OutputVarName,
                                        state.dataRuntimeLang->Sensor(SensorNum).UniqueKeyName,
                                        VarType,
                                        VarIndex);
                if (VarType == OutputProcessor::VariableType::NotFound) {
                    if (reportErrors) {
                        ShowSevereError(
                            state,
                            format("Invalid Output:Variable or Output:Meter Name ={}", state.dataRuntimeLang->Sensor(SensorNum).OutputVarName));
                        ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataRuntimeLang->Sensor(SensorNum).Name));
                        ShowContinueError(state, "Output:Variable Name not found");
                        ErrorsFound = true;
                    }
                } else if (VarIndex == 0) {
                    if (reportErrors) {
                        ShowSevereError(state,
                                        format("Invalid Output:Variable or Output:Meter Index Key Name ={}",
                                               state.dataRuntimeLang->Sensor(SensorNum).UniqueKeyName));
                        ShowContinueError(state,
                                          format("For Output:Variable or Output:Meter = {}", state.dataRuntimeLang->Sensor(SensorNum).OutputVarName));
                        ShowContinueError(state, format("Entered in {}={}", cCurrentModuleObject, state.dataRuntimeLang->Sensor(SensorNum).Name));
                        ShowContinueError(state, "Unique Key Name not found.");
                        ErrorsFound = true;
                    }
                } else {
                    state.dataRuntimeLang->Sensor(SensorNum).VariableType = VarType;
                    state.dataRuntimeLang->Sensor(SensorNum).Index = VarIndex;
                    state.dataRuntimeLang->Sensor(SensorNum).CheckedOkay = true;
                    // If variable is Schedule Value, then get the schedule id to register it as being used
                    if (UtilityRoutines::SameString(state.dataRuntimeLang->Sensor(SensorNum).OutputVarName, "Schedule Value")) {
                        state.dataRuntimeLang->Sensor(SensorNum).SchedNum =
                            ScheduleManager::GetScheduleIndex(state, state.dataRuntimeLang->Sensor(SensorNum).UniqueKeyName);
                        if (state.dataRuntimeLang->Sensor(SensorNum).SchedNum == 0) {
                            state.dataRuntimeLang->Sensor(SensorNum).CheckedOkay = false;
                            if (reportErrors) {
                                ShowSevereError(state,
                                                format("Invalid Output:Variable or Output:Meter Index Key Name ={}",
                                                       state.dataRuntimeLang->Sensor(SensorNum).UniqueKeyName));
                                ShowContinueError(
                                    state,
                                    format("For Output:Variable or Output:Meter = {}", state.dataRuntimeLang->Sensor(SensorNum).OutputVarName));
                                ShowContinueError(state,
                                                  format("Entered in {}={}", cCurrentModuleObject, state.dataRuntimeLang->Sensor(SensorNum).Name));
                                ShowContinueError(state, "Schedule Name not found.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            }

        } // SensorNum

        // added for FMU
        for (int ActuatorNum = 1; ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                                                     state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed +
                                                     state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed;
             ++ActuatorNum) {
            // If we process the ExternalInterface actuators, all we need to do is to change the

            if (ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed) {
                cCurrentModuleObject = "EnergyManagementSystem:Actuator";
            } else if (ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed) {
                cCurrentModuleObject = "ExternalInterface:Actuator";
            } else if (ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                                          state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed) {
                cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitImport:To:Actuator";
            } else if (ActuatorNum <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed +
                                          state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitImportActuatorsUsed +
                                          state.dataRuntimeLang->NumExternalInterfaceFunctionalMockupUnitExportActuatorsUsed) {
                cCurrentModuleObject = "ExternalInterface:FunctionalMockupUnitExport:To:Actuator";
            }

            if (state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).CheckedOkay) continue;
            FoundObjectType = false;
            FoundObjectName = false;
            FoundActuatorName = false;
            for (ActuatorVariableNum = 1; ActuatorVariableNum <= state.dataRuntimeLang->numEMSActuatorsAvailable; ++ActuatorVariableNum) {
                if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).ComponentTypeName,
                                                state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).ComponentTypeName)) {
                    FoundObjectType = true;
                    if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).UniqueIDName,
                                                    state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).UniqueIDName)) {
                        FoundObjectName = true;
                        if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).ControlTypeName,
                                                        state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).ControlTypeName)) {
                            FoundActuatorName = true;
                            break;
                        }
                    }
                }
            }

            if (!FoundObjectType) {
                if (reportErrors) {
                    ShowSevereError(
                        state, format("Invalid Actuated Component Type ={}", state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).ComponentTypeName));
                    ShowContinueError(state,
                                      format("Entered in {}={}", cCurrentModuleObject, state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).Name));
                    ShowContinueError(state, "Component Type not found");
                    if (state.dataRuntimeLang->OutputEDDFile) {
                        ShowContinueError(state, "Review .edd file for valid component types.");
                    } else {
                        ShowContinueError(state, "Use Output:EnergyManagementSystem object to create .edd file for valid component types.");
                    }
                    ErrorsFound = true;
                }
            }

            if (!FoundObjectName) {
                if (reportErrors) {
                    ShowSevereError(
                        state,
                        format("Invalid Actuated Component Unique Name ={}", state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).UniqueIDName));
                    ShowContinueError(state,
                                      format("Entered in {}={}", cCurrentModuleObject, state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).Name));
                    ShowContinueError(state, "Component Unique key name not found ");
                    if (state.dataRuntimeLang->OutputEDDFile) {
                        ShowContinueError(state, "Review edd file for valid component names.");
                    } else {
                        ShowContinueError(state, "Use Output:EnergyManagementSystem object to create .edd file for valid component names.");
                    }
                    ErrorsFound = true;
                }
            }

            if (!FoundActuatorName) {
                if (reportErrors) {
                    ShowSevereError(
                        state,
                        format("Invalid Actuated Component Control Type ={}", state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).ControlTypeName));
                    ShowContinueError(state,
                                      format("Entered in {}={}", cCurrentModuleObject, state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).Name));
                    ShowContinueError(state, "Control Type not found");
                    if (state.dataRuntimeLang->OutputEDDFile) {
                        ShowContinueError(state, "Review edd file for valid component control types.");
                    } else {
                        ShowContinueError(state, "Use Output:EnergyManagementSystem object to create .edd file for valid component control types.");
                    }
                    ErrorsFound = true;
                }
            } else {
                state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).ActuatorVariableNum = ActuatorVariableNum;
                state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).CheckedOkay = true;
                int nHandle = state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).handleCount;
                if (nHandle > 0) {
                    EnergyPlus::ShowWarningError(state, format("Seems like you already tried to get a Handle on this Actuator {}times.", nHandle));
                    EnergyPlus::ShowContinueError(state,
                                                  format("Occurred for componentType='{}', controlType='{}', uniqueKey='{}'.",
                                                         state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).ComponentTypeName,
                                                         state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).ControlTypeName,
                                                         state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).UniqueIDName));
                    EnergyPlus::ShowContinueError(state, "You should take note that there is a risk of overwritting.");
                }
                ++state.dataRuntimeLang->EMSActuatorAvailable(ActuatorVariableNum).handleCount;

                // Warn if actuator applied to an air boundary surface
                if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).ComponentTypeName,
                                                "AIRFLOW NETWORK WINDOW/DOOR OPENING")) {
                    int actuatedSurfNum =
                        UtilityRoutines::FindItemInList(state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).UniqueIDName, state.dataSurface->Surface);
                    if (actuatedSurfNum > 0) {
                        if (state.dataSurface->Surface(actuatedSurfNum).IsAirBoundarySurf) {
                            ShowWarningError(
                                state,
                                format("GetEMSInput: EnergyManagementSystem:Actuator={} actuates an opening attached to an air boundary surface.",
                                       state.dataRuntimeLang->EMSActuatorUsed(ActuatorNum).Name));
                        }
                    }
                }
            }
        } // ActuatorNum

        cCurrentModuleObject = "EnergyManagementSystem:InternalVariable";
        for (InternVarNum = 1; InternVarNum <= state.dataRuntimeLang->NumInternalVariablesUsed; ++InternVarNum) {
            if (state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).CheckedOkay) continue;
            FoundObjectType = false;
            FoundObjectName = false;
            for (InternalVarAvailNum = 1; InternalVarAvailNum <= state.dataRuntimeLang->numEMSInternalVarsAvailable; ++InternalVarAvailNum) {
                if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).DataTypeName,
                                                state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).InternalDataTypeName)) {
                    FoundObjectType = true;
                    if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).UniqueIDName,
                                                    state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).UniqueIDName)) {
                        FoundObjectName = true;
                        break; // InternalVarAvailNum now holds needed index pointer
                    }
                }
            }

            if (!FoundObjectType) {
                if (reportErrors) {
                    ShowSevereError(
                        state,
                        format("Invalid Internal Data Type ={}", state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).InternalDataTypeName));
                    ShowContinueError(
                        state, format("Entered in {}={}", cCurrentModuleObject, state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).Name));
                    ShowContinueError(state, "Internal data type name not found");
                    ErrorsFound = true;
                }
            }

            if (!FoundObjectName) {
                if (reportErrors) {
                    ShowSevereError(
                        state,
                        format("Invalid Internal Data Index Key Name ={}", state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).UniqueIDName));
                    ShowContinueError(
                        state, format("Entered in {}={}", cCurrentModuleObject, state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).Name));
                    ShowContinueError(state, "Internal data unique identifier not found");
                    ErrorsFound = true;
                }
            } else {
                state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).InternVarNum = InternalVarAvailNum;
                state.dataRuntimeLang->EMSInternalVarsUsed(InternVarNum).CheckedOkay = true;
            }
        }
        if (reportErrors) {
            EchoOutActuatorKeyChoices(state);
            EchoOutInternalVariableChoices(state);
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in processing Energy Management System input. Preceding condition causes termination.");
        }

        if (reportErrors) {
            RuntimeLanguageProcessor::BeginEnvrnInitializeRuntimeLanguage(state);
        }
    }

    void GetVariableTypeAndIndex(
        EnergyPlusData &state, std::string const &VarName, std::string const &VarKeyName, OutputProcessor::VariableType &VarType, int &VarIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   June 2006

        // PURPOSE OF THIS SUBROUTINE:
        // local helper routine intended to lookup report variables only.
        //    Use GetMeterIndex for meters.

        // METHODOLOGY EMPLOYED:
        // make calls to OutputProcessor methods GetVariableKeyCountandType and GetVariableKeys

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumKeys;
        OutputProcessor::StoreType AvgOrSum;
        OutputProcessor::TimeStepType StepType;
        OutputProcessor::Unit Units(OutputProcessor::Unit::None);
        Array1D_string KeyName;
        Array1D_int KeyIndex;

        VarType = OutputProcessor::VariableType::NotFound;
        VarIndex = 0;
        GetVariableKeyCountandType(state, VarName, NumKeys, VarType, AvgOrSum, StepType, Units);

        // note that schedules are not getting VarType set right...

        if (NumKeys > 0) {
            KeyName.allocate(NumKeys);
            KeyIndex.allocate(NumKeys);
            GetVariableKeys(state, VarName, VarType, KeyName, KeyIndex);

            if (KeyName(1) == "ENVIRONMENT") {
                VarIndex = KeyIndex(1);
            } else {
                int KeyNum;
                bool Found = false;
                for (KeyNum = 1; KeyNum <= NumKeys; ++KeyNum) {
                    if (KeyName(KeyNum) == VarKeyName) {
                        Found = true;
                        break;
                    }
                }
                if (Found) VarIndex = KeyIndex(KeyNum);
            }

            KeyName.deallocate();
            KeyIndex.deallocate();
        }
    }

    void EchoOutActuatorKeyChoices(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   April 2009

        // PURPOSE OF THIS SUBROUTINE:
        // echo out actuators registered with SetupEMSActuator for user access

        // METHODOLOGY EMPLOYED:
        // mine structure and write to edd file
        // note this executes after final processing and sizing-related calling points may already execute Erl programs

        if (state.dataRuntimeLang->OutputEMSActuatorAvailFull) {

            print(state.files.edd, "! <EnergyManagementSystem:Actuator Available>, Component Unique Name, Component Type,  Control Type, Units\n");
            for (int ActuatorLoop = 1; ActuatorLoop <= state.dataRuntimeLang->numEMSActuatorsAvailable; ++ActuatorLoop) {
                print(state.files.edd,
                      "EnergyManagementSystem:Actuator Available,{},{},{},{}\n",
                      state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).UniqueIDName,
                      state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).ComponentTypeName,
                      state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).ControlTypeName,
                      state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).Units);
            }
        } else if (state.dataRuntimeLang->OutputEMSActuatorAvailSmall) {
            print(state.files.edd, "! <EnergyManagementSystem:Actuator Available>, *, Component Type, Control Type, Units\n");
            int FoundTypeName;
            int FoundControlType;
            for (int ActuatorLoop = 1; ActuatorLoop <= state.dataRuntimeLang->numEMSActuatorsAvailable; ++ActuatorLoop) {
                if (ActuatorLoop + 1 <= state.dataRuntimeLang->numEMSActuatorsAvailable) {
                    FoundTypeName = UtilityRoutines::FindItemInList(
                        state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).ComponentTypeName,
                        state.dataRuntimeLang->EMSActuatorAvailable({ActuatorLoop + 1, state.dataRuntimeLang->numEMSActuatorsAvailable}),
                        &DataRuntimeLanguage::EMSActuatorAvailableType::ComponentTypeName,
                        state.dataRuntimeLang->numEMSActuatorsAvailable - (ActuatorLoop + 1));
                    FoundControlType = UtilityRoutines::FindItemInList(
                        state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).ControlTypeName,
                        state.dataRuntimeLang->EMSActuatorAvailable({ActuatorLoop + 1, state.dataRuntimeLang->numEMSActuatorsAvailable}),
                        &DataRuntimeLanguage::EMSActuatorAvailableType::ControlTypeName,
                        state.dataRuntimeLang->numEMSActuatorsAvailable - (ActuatorLoop + 1));
                } else {
                    FoundTypeName = 1;
                    FoundControlType = 1;
                }
                if ((FoundTypeName == 0) || (FoundControlType == 0)) {
                    print(state.files.edd,
                          "EnergyManagementSystem:Actuator Available, *,{},{},{}\n",
                          state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).ComponentTypeName,
                          state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).ControlTypeName,
                          state.dataRuntimeLang->EMSActuatorAvailable(ActuatorLoop).Units);
                }
            }
        }
    }

    void EchoOutInternalVariableChoices(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   April 2009

        // PURPOSE OF THIS SUBROUTINE:
        // echo out actuators registered with SetupEMSActuator for user access

        // METHODOLOGY EMPLOYED:
        // mine structure and write to eio file

        if (state.dataRuntimeLang->OutputEMSInternalVarsFull) {

            print(state.files.edd, "! <EnergyManagementSystem:InternalVariable Available>, Unique Name, Internal Data Type, Units \n");
            for (int InternalDataLoop = 1; InternalDataLoop <= state.dataRuntimeLang->numEMSInternalVarsAvailable; ++InternalDataLoop) {
                print(state.files.edd,
                      "EnergyManagementSystem:InternalVariable Available,{},{},{}\n",
                      state.dataRuntimeLang->EMSInternalVarsAvailable(InternalDataLoop).UniqueIDName,
                      state.dataRuntimeLang->EMSInternalVarsAvailable(InternalDataLoop).DataTypeName,
                      state.dataRuntimeLang->EMSInternalVarsAvailable(InternalDataLoop).Units);
            }

        } else if (state.dataRuntimeLang->OutputEMSInternalVarsSmall) {
            print(state.files.edd, "! <EnergyManagementSystem:InternalVariable Available>, *, Internal Data Type\n");
            for (int InternalDataLoop = 1; InternalDataLoop <= state.dataRuntimeLang->numEMSInternalVarsAvailable; ++InternalDataLoop) {
                int Found(0);
                if (InternalDataLoop + 1 <= state.dataRuntimeLang->numEMSInternalVarsAvailable) {
                    Found = UtilityRoutines::FindItemInList(
                        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalDataLoop).DataTypeName,
                        state.dataRuntimeLang->EMSInternalVarsAvailable({InternalDataLoop + 1, state.dataRuntimeLang->numEMSInternalVarsAvailable}),
                        &DataRuntimeLanguage::InternalVarsAvailableType::DataTypeName,
                        state.dataRuntimeLang->numEMSInternalVarsAvailable - (InternalDataLoop + 1));
                }
                if (Found == 0) {
                    print(state.files.edd,
                          "EnergyManagementSystem:InternalVariable Available, *,{},{}\n",
                          state.dataRuntimeLang->EMSInternalVarsAvailable(InternalDataLoop).DataTypeName,
                          state.dataRuntimeLang->EMSInternalVarsAvailable(InternalDataLoop).Units);
                }
            }
        }
    }

    void SetupNodeSetPointsAsActuators(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // make system nodes in model available for EMS control

        // METHODOLOGY EMPLOYED:
        // Loop over node structures and make calls to SetupEMSActuator
        // the pattern for the basic node setpoints is a little different in that the actuators directly
        // affect the node variables, rather than using separate logical override flag and ems values

        state.dataEMSMgr->lDummy = false;

        if (state.dataLoopNodes->NumOfNodes > 0) {

            for (int LoopNode = 1; LoopNode <= state.dataLoopNodes->NumOfNodes; ++LoopNode) {
                // setup the setpoint for each type of variable that can be controlled
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Temperature Setpoint",
                                 "[C]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).TempSetPoint);
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Temperature Minimum Setpoint",
                                 "[C]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).TempSetPointLo);
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Temperature Maximum Setpoint",
                                 "[C]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).TempSetPointHi);
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Humidity Ratio Setpoint",
                                 "[kgWater/kgDryAir]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).HumRatSetPoint);
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Humidity Ratio Maximum Setpoint",
                                 "[kgWater/kgDryAir]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).HumRatMax);
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Humidity Ratio Minimum Setpoint",
                                 "[kgWater/kgDryAir]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).HumRatMin);
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Mass Flow Rate Setpoint",
                                 "[kg/s]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).MassFlowRateSetPoint);
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Mass Flow Rate Maximum Available Setpoint",
                                 "[kg/s]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).MassFlowRateMaxAvail);
                SetupEMSActuator(state,
                                 "System Node Setpoint",
                                 state.dataLoopNodes->NodeID(LoopNode),
                                 "Mass Flow Rate Minimum Available Setpoint",
                                 "[kg/s]",
                                 state.dataEMSMgr->lDummy,
                                 state.dataLoopNodes->Node(LoopNode).MassFlowRateMinAvail);
            }

        } // NumOfNodes > 0

        if (state.dataOutAirNodeMgr->NumOutsideAirNodes > 0) {
            for (int OutsideAirNodeNum = 1; OutsideAirNodeNum <= state.dataOutAirNodeMgr->NumOutsideAirNodes; ++OutsideAirNodeNum) {
                int NodeNum = state.dataOutAirNodeMgr->OutsideAirNodeList(OutsideAirNodeNum);
                SetupEMSActuator(state,
                                 "Outdoor Air System Node",
                                 state.dataLoopNodes->NodeID(NodeNum),
                                 "Drybulb Temperature",
                                 "[C]",
                                 state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirDryBulb,
                                 state.dataLoopNodes->Node(NodeNum).EMSValueForOutAirDryBulb);
                SetupEMSActuator(state,
                                 "Outdoor Air System Node",
                                 state.dataLoopNodes->NodeID(NodeNum),
                                 "Wetbulb Temperature",
                                 "[C]",
                                 state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirWetBulb,
                                 state.dataLoopNodes->Node(NodeNum).EMSValueForOutAirWetBulb);
                SetupEMSActuator(state,
                                 "Outdoor Air System Node",
                                 state.dataLoopNodes->NodeID(NodeNum),
                                 "Wind Speed",
                                 "[m/s]",
                                 state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirWindSpeed,
                                 state.dataLoopNodes->Node(NodeNum).EMSValueForOutAirWindSpeed);
                SetupEMSActuator(state,
                                 "Outdoor Air System Node",
                                 state.dataLoopNodes->NodeID(NodeNum),
                                 "Wind Direction",
                                 "[degree]",
                                 state.dataLoopNodes->Node(NodeNum).EMSOverrideOutAirWindDir,
                                 state.dataLoopNodes->Node(NodeNum).EMSValueForOutAirWindDir);
                for (int ActuatorUsedLoop = 1; ActuatorUsedLoop <= state.dataRuntimeLang->numActuatorsUsed; ActuatorUsedLoop++) {
                    if (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorUsed(ActuatorUsedLoop).ComponentTypeName,
                                                    "Outdoor Air System Node") &&
                        UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorUsed(ActuatorUsedLoop).UniqueIDName,
                                                    state.dataLoopNodes->NodeID(NodeNum))) {
                        state.dataLoopNodes->Node(NodeNum).IsLocalNode = true;
                        break;
                    }
                }
            }
        }
    }

    void UpdateEMSTrendVariables(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // Logged trend data

        // METHODOLOGY EMPLOYED:
        // Store current value of Erl Variable in Trend stack
        // Trend arrays are pushed so that the latest value is
        //  always at index 1.  old values get lost.

        // checks with quick return if no updates needed.
        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) return;
        if (state.dataRuntimeLang->NumErlTrendVariables == 0) return;

        for (int TrendNum = 1; TrendNum <= state.dataRuntimeLang->NumErlTrendVariables; ++TrendNum) {
            int ErlVarNum = state.dataRuntimeLang->TrendVariable(TrendNum).ErlVariablePointer;
            int TrendDepth = state.dataRuntimeLang->TrendVariable(TrendNum).LogDepth;
            if ((ErlVarNum > 0) && (TrendDepth > 0)) {
                Real64 currentVal = state.dataRuntimeLang->ErlVariable(ErlVarNum).Value.Number;
                // push into trend
                state.dataRuntimeLang->TrendVariable(TrendNum).tempTrendARR = state.dataRuntimeLang->TrendVariable(TrendNum).TrendValARR;
                state.dataRuntimeLang->TrendVariable(TrendNum).TrendValARR(1) = currentVal;
                state.dataRuntimeLang->TrendVariable(TrendNum).TrendValARR({2, TrendDepth}) =
                    state.dataRuntimeLang->TrendVariable(TrendNum).tempTrendARR({1, TrendDepth - 1});
            }
        }
    }

    bool CheckIfNodeSetPointManaged(EnergyPlusData &state, int const NodeNum, SPControlType const SetPointType, bool byHandle)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009
        //       MODIFIED       July 2020, Julien Marrec of EffiBEM: added option to check by handle (for API)

        // PURPOSE OF THIS SUBROUTINE:
        // Provide method to verify that a specific node is (probably) managed by EMS

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool FoundControl(false);

        std::string cNodeName = state.dataLoopNodes->NodeID(NodeNum);
        std::string cComponentTypeName = "System Node Setpoint";
        std::string_view cControlTypeName = controlTypeName[static_cast<int>(SetPointType)];

        if (byHandle) {
            for (int Loop = 1; Loop <= state.dataRuntimeLang->numEMSActuatorsAvailable; ++Loop) {
                if ((state.dataRuntimeLang->EMSActuatorAvailable(Loop).handleCount > 0) &&
                    (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(Loop).ComponentTypeName, cComponentTypeName)) &&
                    (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(Loop).UniqueIDName, cNodeName)) &&
                    (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorAvailable(Loop).ControlTypeName, cControlTypeName))) {
                    FoundControl = true;
                    break;
                }
            }
            if (!FoundControl) {
                ShowWarningError(state,
                                 format("Missing '{}' for node named named '{}'.",
                                        format(controlTypeName[static_cast<int>(SetPointType)]),
                                        state.dataLoopNodes->NodeID(NodeNum)));
            }
        } else {
            for (int Loop = 1; Loop <= state.dataRuntimeLang->numActuatorsUsed + state.dataRuntimeLang->NumExternalInterfaceActuatorsUsed; ++Loop) {
                if ((UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorUsed(Loop).ComponentTypeName, cComponentTypeName)) &&
                    (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorUsed(Loop).UniqueIDName, cNodeName)) &&
                    (UtilityRoutines::SameString(state.dataRuntimeLang->EMSActuatorUsed(Loop).ControlTypeName, cControlTypeName))) {
                    FoundControl = true;
                    break;
                }
            }
        }

        return FoundControl;
    }

    bool CheckIfNodeSetPointManagedByEMS(EnergyPlusData &state,
                                         int const NodeNum, // index of node being checked.
                                         SPControlType const SetPointType,
                                         bool &ErrorFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // Provide method to verify that a specific node is (probably) managed by EMS

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool FoundControl = CheckIfNodeSetPointManaged(state, NodeNum, SetPointType, false);

        if ((!ErrorFlag) && (!FoundControl)) {
            int numPythonPlugins = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "PythonPlugin:Instance");
            int numActiveCallbacks = PluginManagement::PluginManager::numActiveCallbacks(state); // errorCallback;
            if ((numPythonPlugins + numActiveCallbacks) == 0) {
                ErrorFlag = true;
            } else {
                // We'll defer to checking at the end whether a Plugin / API called getActuatorHandle on it
                auto &nodeSetpointCheck = state.dataLoopNodes->NodeSetpointCheck(NodeNum);
                nodeSetpointCheck.needsSetpointChecking = true;

                switch (SetPointType) {
                case SPControlType::TemperatureSetPoint: {
                    nodeSetpointCheck.checkTemperatureSetPoint = true;
                } break;
                case SPControlType::TemperatureMinSetPoint: {
                    nodeSetpointCheck.checkTemperatureMinSetPoint = true;
                } break;
                case SPControlType::TemperatureMaxSetPoint: {
                    nodeSetpointCheck.checkTemperatureMaxSetPoint = true;
                } break;
                case SPControlType::HumidityRatioSetPoint: {
                    nodeSetpointCheck.checkHumidityRatioSetPoint = true;
                } break;
                case SPControlType::HumidityRatioMinSetPoint: {
                    nodeSetpointCheck.checkHumidityRatioMinSetPoint = true;
                } break;
                case SPControlType::HumidityRatioMaxSetPoint: {
                    nodeSetpointCheck.checkHumidityRatioMaxSetPoint = true;
                } break;
                case SPControlType::MassFlowRateSetPoint: {
                    nodeSetpointCheck.checkMassFlowRateSetPoint = true;
                } break;
                case SPControlType::MassFlowRateMinSetPoint: {
                    nodeSetpointCheck.checkMassFlowRateMinSetPoint = true;
                } break;
                case SPControlType::MassFlowRateMaxSetPoint: {
                    nodeSetpointCheck.checkMassFlowRateMaxSetPoint = true;
                } break;
                default:
                    break;
                }
            }
        }

        return FoundControl;
    }

    void checkSetpointNodesAtEnd(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Julien Marrec of EffiBEM
        //       DATE WRITTEN   July 2020

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine checks any nodes where we couldn't find a Setpoint in EMS, after the PythonPlugin / API have been called
        // so we can check if getActuatorHandle was ever actually called for that node.

        bool FatalErrorFlag = false;

        for (int NodeNum = 1; NodeNum <= state.dataLoopNodes->NumOfNodes; ++NodeNum) {
            auto &nodeSetpointCheck = state.dataLoopNodes->NodeSetpointCheck(NodeNum);

            if (nodeSetpointCheck.needsSetpointChecking) {
                // Start by setting it to false (assume matched)
                nodeSetpointCheck.needsSetpointChecking = false;

                if (nodeSetpointCheck.checkTemperatureSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |= !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::TemperatureSetPoint, true);
                }
                if (nodeSetpointCheck.checkTemperatureMinSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |=
                        !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::TemperatureMinSetPoint, true);
                }
                if (nodeSetpointCheck.checkTemperatureMaxSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |=
                        !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::TemperatureMaxSetPoint, true);
                }
                if (nodeSetpointCheck.checkHumidityRatioSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |=
                        !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::HumidityRatioSetPoint, true);
                }
                if (nodeSetpointCheck.checkHumidityRatioMinSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |=
                        !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::HumidityRatioMinSetPoint, true);
                }
                if (nodeSetpointCheck.checkHumidityRatioMaxSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |=
                        !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::HumidityRatioMaxSetPoint, true);
                }
                if (nodeSetpointCheck.checkMassFlowRateSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |= !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::MassFlowRateSetPoint, true);
                }
                if (nodeSetpointCheck.checkMassFlowRateMinSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |=
                        !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::MassFlowRateMinSetPoint, true);
                }
                if (nodeSetpointCheck.checkMassFlowRateMaxSetPoint) {
                    nodeSetpointCheck.needsSetpointChecking |=
                        !CheckIfNodeSetPointManaged(state, NodeNum, SPControlType::MassFlowRateMaxSetPoint, true);
                }

                if (nodeSetpointCheck.needsSetpointChecking) {
                    FatalErrorFlag = true;
                }
            }
        }

        if (FatalErrorFlag) {
            ShowFatalError(state,
                           "checkSetpointNodesAtEnd: At least one node does not have a setpoint attached, "
                           "neither via a SetpointManager, EMS:Actuator, or API");
        }
    }

    bool CheckIfNodeMoreInfoSensedByEMS(EnergyPlusData &state,
                                        int const nodeNum, // index of node being checked.
                                        std::string const &varName)
    {
        bool returnValue = false;
        for (int loop = 1; loop <= state.dataRuntimeLang->NumSensors; ++loop) {
            if (state.dataRuntimeLang->Sensor(loop).UniqueKeyName == state.dataLoopNodes->NodeID(nodeNum) &&
                UtilityRoutines::SameString(state.dataRuntimeLang->Sensor(loop).OutputVarName, varName)) {
                returnValue = true;
            }
        }

        return returnValue;
    }

    void SetupPrimaryAirSystemAvailMgrAsActuators(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // make air system status available as EMS actuator

        state.dataEMSMgr->lDummy2 = false;

        if (allocated(state.dataAirLoop->PriAirSysAvailMgr)) {
            int numAirLoops = isize(state.dataAirLoop->PriAirSysAvailMgr);
            for (int Loop = 1; Loop <= numAirLoops; ++Loop) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC",
                                 state.dataAirSystemsData->PrimaryAirSystems(Loop).Name,
                                 "Availability Status",
                                 "[ ]",
                                 state.dataEMSMgr->lDummy2,
                                 state.dataAirLoop->PriAirSysAvailMgr(Loop).AvailStatus);
            }
        }
    }

    void SetupWindowShadingControlActuators(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // make calls to SetupEMSactuator for public data for Window Shades

        // METHODOLOGY EMPLOYED:
        // Loop thru SurfaceWindow and register any shading controls

        for (int loopSurfNum = 1; loopSurfNum <= state.dataSurface->TotSurfaces; ++loopSurfNum) {

            if (state.dataSurface->Surface(loopSurfNum).Class != DataSurfaces::SurfaceClass::Window) continue;
            if (state.dataSurface->Surface(loopSurfNum).ExtBoundCond != DataSurfaces::ExternalEnvironment) continue;
            if (!state.dataSurface->Surface(loopSurfNum).HasShadeControl) continue;

            if (state.dataSurface->SurfWinHasShadeOrBlindLayer(loopSurfNum)) {
                SetupEMSActuator(state,
                                 "Window Shading Control",
                                 state.dataSurface->Surface(loopSurfNum).Name,
                                 "Control Status",
                                 "[ShadeStatus]",
                                 state.dataSurface->SurfWinShadingFlagEMSOn(loopSurfNum),
                                 state.dataSurface->SurfWinShadingFlagEMSValue(loopSurfNum));
                if (state.dataSurface->SurfWinMovableSlats(loopSurfNum)) {
                    SetupEMSActuator(state,
                                     "Window Shading Control",
                                     state.dataSurface->Surface(loopSurfNum).Name,
                                     "Slat Angle",
                                     "[degrees]",
                                     state.dataSurface->SurfWinSlatAngThisTSDegEMSon(loopSurfNum),
                                     state.dataSurface->SurfWinSlatAngThisTSDegEMSValue(loopSurfNum));
                }
            } else if (state.dataSurface->WindowShadingControl(state.dataSurface->Surface(loopSurfNum).activeWindowShadingControl).ShadingType ==
                       DataSurfaces::WinShadingType::ExtScreen) {
                SetupEMSActuator(state,
                                 "Window Shading Control",
                                 state.dataSurface->Surface(loopSurfNum).Name,
                                 "Control Status",
                                 "[ShadeStatus]",
                                 state.dataSurface->SurfWinShadingFlagEMSOn(loopSurfNum),
                                 state.dataSurface->SurfWinShadingFlagEMSValue(loopSurfNum));
            } else {
                if (state.dataSurface->WindowShadingControl(state.dataSurface->Surface(loopSurfNum).activeWindowShadingControl).ShadingType !=
                    DataSurfaces::WinShadingType::SwitchableGlazing) {
                    ShowSevereError(state,
                                    format("Missing shade or blind layer in window construction name = '{}', surface name = '{}'.",
                                           state.dataConstruction->Construct(state.dataSurface->Surface(loopSurfNum).activeShadedConstruction).Name,
                                           state.dataSurface->Surface(loopSurfNum).Name));
                    ShowContinueError(state,
                                      "...'Control Status' or 'Slat Angle' EMS Actuator cannot be set for a construction that does not have a shade "
                                      "or a blind layer.");
                    ShowContinueError(state, "...Add shade or blind layer to this construction in order to be able to apply EMS Actuator.");
                }
            }
        }
    }

    void SetupThermostatActuators(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // Make zone thermostats, humidistats, and comfort controls available to EMS

        // METHODOLOGY EMPLOYED:
        // Loop over structures and call SetupEMSactuator for public data in DataZoneControls.

        for (int Loop = 1; Loop <= state.dataZoneCtrls->NumTempControlledZones; ++Loop) {
            SetupEMSActuator(state,
                             "Zone Temperature Control",
                             state.dataZoneCtrls->TempControlledZone(Loop).ZoneName,
                             "Heating Setpoint",
                             "[C]",
                             state.dataZoneCtrls->TempControlledZone(Loop).EMSOverrideHeatingSetPointOn,
                             state.dataZoneCtrls->TempControlledZone(Loop).EMSOverrideHeatingSetPointValue);
            SetupEMSActuator(state,
                             "Zone Temperature Control",
                             state.dataZoneCtrls->TempControlledZone(Loop).ZoneName,
                             "Cooling Setpoint",
                             "[C]",
                             state.dataZoneCtrls->TempControlledZone(Loop).EMSOverrideCoolingSetPointOn,
                             state.dataZoneCtrls->TempControlledZone(Loop).EMSOverrideCoolingSetPointValue);
        }

        for (int Loop = 1; Loop <= state.dataZoneCtrls->NumHumidityControlZones; ++Loop) {
            SetupEMSActuator(state,
                             "Zone Humidity Control",
                             state.dataZoneCtrls->HumidityControlZone(Loop).ZoneName,
                             "Relative Humidity Humidifying Setpoint",
                             "[%]",
                             state.dataZoneCtrls->HumidityControlZone(Loop).EMSOverrideHumidifySetPointOn,
                             state.dataZoneCtrls->HumidityControlZone(Loop).EMSOverrideHumidifySetPointValue);
            SetupEMSActuator(state,
                             "Zone Humidity Control",
                             state.dataZoneCtrls->HumidityControlZone(Loop).ZoneName,
                             "Relative Humidity Dehumidifying Setpoint",
                             "[%]",
                             state.dataZoneCtrls->HumidityControlZone(Loop).EMSOverrideDehumidifySetPointOn,
                             state.dataZoneCtrls->HumidityControlZone(Loop).EMSOverrideDehumidifySetPointValue);
        }

        for (int Loop = 1; Loop <= state.dataZoneCtrls->NumComfortControlledZones; ++Loop) {
            SetupEMSActuator(state,
                             "Zone Comfort Control",
                             state.dataZoneCtrls->ComfortControlledZone(Loop).ZoneName,
                             "Heating Setpoint",
                             "[]",
                             state.dataZoneCtrls->ComfortControlledZone(Loop).EMSOverrideHeatingSetPointOn,
                             state.dataZoneCtrls->ComfortControlledZone(Loop).EMSOverrideHeatingSetPointValue);
            SetupEMSActuator(state,
                             "Zone Comfort Control",
                             state.dataZoneCtrls->ComfortControlledZone(Loop).ZoneName,
                             "Cooling Setpoint",
                             "[]",
                             state.dataZoneCtrls->ComfortControlledZone(Loop).EMSOverrideCoolingSetPointOn,
                             state.dataZoneCtrls->ComfortControlledZone(Loop).EMSOverrideCoolingSetPointValue);
        }
    }

    void SetupSurfaceConvectionActuators(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // Setup EMS actuators available for surface convection coefficients

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            SetupEMSActuator(state,
                             "Surface",
                             state.dataSurface->Surface(SurfNum).Name,
                             "Interior Surface Convection Heat Transfer Coefficient",
                             "[W/m2-K]",
                             state.dataSurface->SurfEMSOverrideIntConvCoef(SurfNum),
                             state.dataSurface->SurfEMSValueForIntConvCoef(SurfNum));
            SetupEMSActuator(state,
                             "Surface",
                             state.dataSurface->Surface(SurfNum).Name,
                             "Exterior Surface Convection Heat Transfer Coefficient",
                             "[W/m2-K]",
                             state.dataSurface->SurfEMSOverrideExtConvCoef(SurfNum),
                             state.dataSurface->SurfEMSValueForExtConvCoef(SurfNum));
        }
    }

    void SetupSurfaceConstructionActuators(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Jan 2012

        // PURPOSE OF THIS SUBROUTINE:
        // setup EMS actuators available for surface construction

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {

            if (!state.dataSurface->Surface(SurfNum).HeatTransSurf) continue;

            SetupEMSActuator(state,
                             "Surface",
                             state.dataSurface->Surface(SurfNum).Name,
                             "Construction State",
                             "[ ]",
                             state.dataSurface->SurfEMSConstructionOverrideON(SurfNum),
                             state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum));
        }

        // Setup error checking storage

        if (!allocated(state.dataRuntimeLang->EMSConstructActuatorChecked))
            state.dataRuntimeLang->EMSConstructActuatorChecked.allocate(state.dataHeatBal->TotConstructs, state.dataSurface->TotSurfaces);
        state.dataRuntimeLang->EMSConstructActuatorChecked = false;

        if (!allocated(state.dataRuntimeLang->EMSConstructActuatorIsOkay))
            state.dataRuntimeLang->EMSConstructActuatorIsOkay.allocate(state.dataHeatBal->TotConstructs, state.dataSurface->TotSurfaces);
        state.dataRuntimeLang->EMSConstructActuatorIsOkay = false;
    }

    void SetupSurfaceOutdoorBoundaryConditionActuators(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   May 2013

        // PURPOSE OF THIS SUBROUTINE:
        // setup EMS actuators for outside boundary conditions by surface

        // METHODOLOGY EMPLOYED:
        // loop through all surfaces, cycle if not heat transfer or outdoors BC

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {

            if (!state.dataSurface->Surface(SurfNum).HeatTransSurf) continue;
            if (state.dataSurface->Surface(SurfNum).ExtBoundCond != DataSurfaces::ExternalEnvironment) continue;

            SetupEMSActuator(state,
                             "Surface",
                             state.dataSurface->Surface(SurfNum).Name,
                             "View Factor To Ground",
                             "[ ]",
                             state.dataSurface->SurfViewFactorGroundEMSOverrideOn(SurfNum),
                             state.dataSurface->SurfViewFactorGroundEMSOverrideValue(SurfNum));

            SetupEMSActuator(state,
                             "Surface",
                             state.dataSurface->Surface(SurfNum).Name,
                             "Outdoor Air Drybulb Temperature",
                             "[C]",
                             state.dataSurface->SurfOutDryBulbTempEMSOverrideOn(SurfNum),
                             state.dataSurface->SurfOutDryBulbTempEMSOverrideValue(SurfNum));

            SetupEMSActuator(state,
                             "Surface",
                             state.dataSurface->Surface(SurfNum).Name,
                             "Outdoor Air Wetbulb Temperature",
                             "[C]",
                             state.dataSurface->SurfOutWetBulbTempEMSOverrideOn(SurfNum),
                             state.dataSurface->SurfOutWetBulbTempEMSOverrideValue(SurfNum));
            if (state.dataSurface->Surface(SurfNum).ExtWind) {
                SetupEMSActuator(state,
                                 "Surface",
                                 state.dataSurface->Surface(SurfNum).Name,
                                 "Outdoor Air Wind Speed",
                                 "[m/s]",
                                 state.dataSurface->SurfWindSpeedEMSOverrideOn(SurfNum),
                                 state.dataSurface->SurfWindSpeedEMSOverrideValue(SurfNum));
                SetupEMSActuator(state,
                                 "Surface",
                                 state.dataSurface->Surface(SurfNum).Name,
                                 "Outdoor Air Wind Direction",
                                 "[degree]",
                                 state.dataSurface->SurfWindDirEMSOverrideOn(SurfNum),
                                 state.dataSurface->SurfWindDirEMSOverrideValue(SurfNum));
            }
        }
    }

    void SetupZoneInfoAsInternalDataAvail(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   May 2009

        // PURPOSE OF THIS SUBROUTINE:
        // set up zone-related info as internal data

        if (!state.dataHeatBal->Zone.empty()) {
            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                auto &zone = state.dataHeatBal->Zone(ZoneNum);

                SetupEMSInternalVariable(state, "Zone Floor Area", zone.Name, "[m2]", zone.FloorArea);
                SetupEMSInternalVariable(state, "Zone Air Volume", zone.Name, "[m3]", zone.Volume);
                SetupEMSInternalVariable(state, "Zone Multiplier", zone.Name, "[ ]", zone.Multiplier);
                SetupEMSInternalVariable(state, "Zone List Multiplier", zone.Name, "[ ]", zone.ListMultiplier);
            }
        }
    }

    void SetupZoneOutdoorBoundaryConditionActuators(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         X Luo
        //       DATE WRITTEN   July 2017

        // PURPOSE OF THIS SUBROUTINE:
        // setup EMS actuators for outside boundary conditions by surface

        // METHODOLOGY EMPLOYED:
        // loop through all surfaces, cycle if not heat transfer or outdoors BC

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            auto &zone = state.dataHeatBal->Zone(ZoneNum);

            SetupEMSActuator(state,
                             "Zone",
                             zone.Name,
                             "Outdoor Air Drybulb Temperature",
                             "[C]",
                             zone.OutDryBulbTempEMSOverrideOn,
                             zone.OutDryBulbTempEMSOverrideValue);
            SetupEMSActuator(state,
                             "Zone",
                             zone.Name,
                             "Outdoor Air Wetbulb Temperature",
                             "[C]",
                             zone.OutWetBulbTempEMSOverrideOn,
                             zone.OutWetBulbTempEMSOverrideValue);
            SetupEMSActuator(
                state, "Zone", zone.Name, "Outdoor Air Wind Speed", "[m/s]", zone.WindSpeedEMSOverrideOn, zone.WindSpeedEMSOverrideValue);
            SetupEMSActuator(
                state, "Zone", zone.Name, "Outdoor Air Wind Direction", "[degree]", zone.WindDirEMSOverrideOn, zone.WindDirEMSOverrideValue);
        }
    }

    void checkForUnusedActuatorsAtEnd(EnergyPlusData &state)
    {
        // call at end of simulation to check if any of the user's actuators were never initialized.
        // Could be a mistake we want to help users catch // Issue #4404.
        for (int actuatorUsedLoop = 1; actuatorUsedLoop <= state.dataRuntimeLang->numActuatorsUsed; ++actuatorUsedLoop) {
            if (!state.dataRuntimeLang->ErlVariable(state.dataRuntimeLang->EMSActuatorUsed(actuatorUsedLoop).ErlVariableNum).Value.initialized) {
                ShowWarningError(state,
                                 "checkForUnusedActuatorsAtEnd: Unused EMS Actuator detected, suggesting possible unintended programming error or "
                                 "spelling mistake.");
                ShowContinueError(state,
                                  format("Check Erl programs related to EMS actuator variable name = {}",
                                         state.dataRuntimeLang->EMSActuatorUsed(actuatorUsedLoop).Name));
                ShowContinueError(state,
                                  format("EMS Actuator type name = {}", state.dataRuntimeLang->EMSActuatorUsed(actuatorUsedLoop).ComponentTypeName));
                ShowContinueError(
                    state, format("EMS Actuator unique component name = {}", state.dataRuntimeLang->EMSActuatorUsed(actuatorUsedLoop).UniqueIDName));
                ShowContinueError(state,
                                  format("EMS Actuator control type = {}", state.dataRuntimeLang->EMSActuatorUsed(actuatorUsedLoop).ControlTypeName));
            }
        }
    }

} // namespace EMSManager

// Moved these setup EMS actuator routines out of module to solve circular use problems between
//  ScheduleManager and OutputProcessor. Followed pattern used for SetupOutputVariable

void SetupEMSActuator(EnergyPlusData &state,
                      std::string_view cComponentTypeName,
                      std::string_view cUniqueIDName,
                      std::string_view cControlTypeName,
                      std::string_view cUnits,
                      bool &lEMSActuated,
                      Real64 &rValue)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   June 2006
    //       MODIFIED       Brent Griffith April 2009,

    // PURPOSE OF THIS SUBROUTINE:
    // register a new actuator for EMS
    //   set  up pointer to logical and real value

    // METHODOLOGY EMPLOYED:
    // push size of ActuatorVariable and add a new one.
    //  check for duplicates.

    std::string const UpperCaseObjectType(UtilityRoutines::MakeUPPERCase(cComponentTypeName));
    std::string const UpperCaseObjectName(UtilityRoutines::MakeUPPERCase(cUniqueIDName));
    std::string const UpperCaseActuatorName(UtilityRoutines::MakeUPPERCase(cControlTypeName));

    DataRuntimeLanguage::EMSActuatorKey const key(UpperCaseObjectType, UpperCaseObjectName, UpperCaseActuatorName);

    if (state.dataRuntimeLang->EMSActuator_lookup.find(key) == state.dataRuntimeLang->EMSActuator_lookup.end()) {
        if (state.dataRuntimeLang->numEMSActuatorsAvailable == 0) {
            state.dataRuntimeLang->EMSActuatorAvailable.allocate(state.dataRuntimeLang->varsAvailableAllocInc);
            state.dataRuntimeLang->numEMSActuatorsAvailable = 1;
            state.dataRuntimeLang->maxEMSActuatorsAvailable = state.dataRuntimeLang->varsAvailableAllocInc;
        } else {
            if (state.dataRuntimeLang->numEMSActuatorsAvailable + 1 > state.dataRuntimeLang->maxEMSActuatorsAvailable) {
                state.dataRuntimeLang->EMSActuatorAvailable.redimension(state.dataRuntimeLang->maxEMSActuatorsAvailable *= 2);
            }
            ++state.dataRuntimeLang->numEMSActuatorsAvailable;
        }

        auto &actuator(state.dataRuntimeLang->EMSActuatorAvailable(state.dataRuntimeLang->numEMSActuatorsAvailable));
        actuator.ComponentTypeName = cComponentTypeName;
        actuator.UniqueIDName = cUniqueIDName;
        actuator.ControlTypeName = cControlTypeName;
        actuator.Units = cUnits;
        actuator.Actuated = &lEMSActuated; // Pointer assigment
        actuator.RealValue = &rValue;      // Pointer assigment
        actuator.PntrVarTypeUsed = DataRuntimeLanguage::PtrDataType::Real;
        state.dataRuntimeLang->EMSActuator_lookup.insert(key);
    }
}

void SetupEMSActuator(EnergyPlusData &state,
                      std::string_view cComponentTypeName,
                      std::string_view cUniqueIDName,
                      std::string_view cControlTypeName,
                      std::string_view cUnits,
                      bool &lEMSActuated,
                      int &iValue)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   May 2009

    // PURPOSE OF THIS SUBROUTINE:
    // register a new actuator for EMS
    //   set  up pointer to logical and integer value

    // METHODOLOGY EMPLOYED:
    // push size of ActuatorVariable and add a new one.
    //  check for duplicates.

    std::string const UpperCaseObjectType(UtilityRoutines::MakeUPPERCase(cComponentTypeName));
    std::string const UpperCaseObjectName(UtilityRoutines::MakeUPPERCase(cUniqueIDName));
    std::string const UpperCaseActuatorName(UtilityRoutines::MakeUPPERCase(cControlTypeName));

    DataRuntimeLanguage::EMSActuatorKey const key(UpperCaseObjectType, UpperCaseObjectName, UpperCaseActuatorName);

    if (state.dataRuntimeLang->EMSActuator_lookup.find(key) == state.dataRuntimeLang->EMSActuator_lookup.end()) {
        if (state.dataRuntimeLang->numEMSActuatorsAvailable == 0) {
            state.dataRuntimeLang->EMSActuatorAvailable.allocate(state.dataRuntimeLang->varsAvailableAllocInc);
            state.dataRuntimeLang->numEMSActuatorsAvailable = 1;
            state.dataRuntimeLang->maxEMSActuatorsAvailable = state.dataRuntimeLang->varsAvailableAllocInc;
        } else {
            if (state.dataRuntimeLang->numEMSActuatorsAvailable + 1 > state.dataRuntimeLang->maxEMSActuatorsAvailable) {
                state.dataRuntimeLang->EMSActuatorAvailable.redimension(state.dataRuntimeLang->maxEMSActuatorsAvailable *= 2);
            }
            ++state.dataRuntimeLang->numEMSActuatorsAvailable;
        }

        auto &actuator(state.dataRuntimeLang->EMSActuatorAvailable(state.dataRuntimeLang->numEMSActuatorsAvailable));
        actuator.ComponentTypeName = cComponentTypeName;
        actuator.UniqueIDName = cUniqueIDName;
        actuator.ControlTypeName = cControlTypeName;
        actuator.Units = cUnits;
        actuator.Actuated = &lEMSActuated; // Pointer assigment
        actuator.IntValue = &iValue;       // Pointer assigment
        actuator.PntrVarTypeUsed = DataRuntimeLanguage::PtrDataType::Integer;
        state.dataRuntimeLang->EMSActuator_lookup.insert(key);
    }
}

void SetupEMSActuator(EnergyPlusData &state,
                      std::string_view cComponentTypeName,
                      std::string_view cUniqueIDName,
                      std::string_view cControlTypeName,
                      std::string_view cUnits,
                      bool &lEMSActuated,
                      bool &lValue)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   August 2009

    // PURPOSE OF THIS SUBROUTINE:
    // register a new actuator for EMS
    //   set  up pointer to logical and logical value

    // METHODOLOGY EMPLOYED:
    // push size of ActuatorVariable and add a new one.
    //  check for duplicates.

    std::string const UpperCaseObjectType(UtilityRoutines::MakeUPPERCase(cComponentTypeName));
    std::string const UpperCaseObjectName(UtilityRoutines::MakeUPPERCase(cUniqueIDName));
    std::string const UpperCaseActuatorName(UtilityRoutines::MakeUPPERCase(cControlTypeName));

    DataRuntimeLanguage::EMSActuatorKey const key(UpperCaseObjectType, UpperCaseObjectName, UpperCaseActuatorName);

    if (state.dataRuntimeLang->EMSActuator_lookup.find(key) == state.dataRuntimeLang->EMSActuator_lookup.end()) {
        if (state.dataRuntimeLang->numEMSActuatorsAvailable == 0) {
            state.dataRuntimeLang->EMSActuatorAvailable.allocate(state.dataRuntimeLang->varsAvailableAllocInc);
            state.dataRuntimeLang->numEMSActuatorsAvailable = 1;
            state.dataRuntimeLang->maxEMSActuatorsAvailable = state.dataRuntimeLang->varsAvailableAllocInc;
        } else {
            if (state.dataRuntimeLang->numEMSActuatorsAvailable + 1 > state.dataRuntimeLang->maxEMSActuatorsAvailable) {
                state.dataRuntimeLang->EMSActuatorAvailable.redimension(state.dataRuntimeLang->maxEMSActuatorsAvailable *= 2);
            }
            ++state.dataRuntimeLang->numEMSActuatorsAvailable;
        }

        auto &actuator(state.dataRuntimeLang->EMSActuatorAvailable(state.dataRuntimeLang->numEMSActuatorsAvailable));
        actuator.ComponentTypeName = cComponentTypeName;
        actuator.UniqueIDName = cUniqueIDName;
        actuator.ControlTypeName = cControlTypeName;
        actuator.Units = cUnits;
        actuator.Actuated = &lEMSActuated; // Pointer assigment
        actuator.LogValue = &lValue;       // Pointer assigment
        actuator.PntrVarTypeUsed = DataRuntimeLanguage::PtrDataType::Logical;
        state.dataRuntimeLang->EMSActuator_lookup.insert(key);
    }
}

void SetupEMSInternalVariable(
    EnergyPlusData &state, std::string_view cDataTypeName, std::string_view cUniqueIDName, std::string_view cUnits, Real64 &rValue)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   May 2009

    // PURPOSE OF THIS SUBROUTINE:
    // Setup internal data source and make available to EMS

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool FoundDuplicate = false;

    for (int InternalVarAvailNum = 1; InternalVarAvailNum <= state.dataRuntimeLang->numEMSInternalVarsAvailable; ++InternalVarAvailNum) {
        if ((UtilityRoutines::SameString(cDataTypeName, state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).DataTypeName)) &&
            (UtilityRoutines::SameString(cUniqueIDName, state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).UniqueIDName))) {
            FoundDuplicate = true;
            break;
        }
    }

    if (FoundDuplicate) {
        ShowSevereError(state, "Duplicate internal variable was sent to SetupEMSInternalVariable.");
        ShowContinueError(state, format("Internal variable type = {} ; name = {}", cDataTypeName, cUniqueIDName));
        ShowContinueError(state, "Called from SetupEMSInternalVariable.");
    } else {
        // add new internal data variable
        if (state.dataRuntimeLang->numEMSInternalVarsAvailable == 0) {
            state.dataRuntimeLang->EMSInternalVarsAvailable.allocate(state.dataRuntimeLang->varsAvailableAllocInc);
            state.dataRuntimeLang->numEMSInternalVarsAvailable = 1;
            state.dataRuntimeLang->maxEMSInternalVarsAvailable = state.dataRuntimeLang->varsAvailableAllocInc;
        } else {
            if (state.dataRuntimeLang->numEMSInternalVarsAvailable + 1 > state.dataRuntimeLang->maxEMSInternalVarsAvailable) {
                state.dataRuntimeLang->EMSInternalVarsAvailable.redimension(state.dataRuntimeLang->maxEMSInternalVarsAvailable +=
                                                                            state.dataRuntimeLang->varsAvailableAllocInc);
            }
            ++state.dataRuntimeLang->numEMSInternalVarsAvailable;
        }

        int InternalVarAvailNum = state.dataRuntimeLang->numEMSInternalVarsAvailable;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).DataTypeName = cDataTypeName;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).UniqueIDName = cUniqueIDName;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).Units = cUnits;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).RealValue = &rValue;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).PntrVarTypeUsed = DataRuntimeLanguage::PtrDataType::Real;
    }
}

void SetupEMSInternalVariable(
    EnergyPlusData &state, std::string_view cDataTypeName, std::string_view cUniqueIDName, std::string_view cUnits, int &iValue)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   May 2009

    // PURPOSE OF THIS SUBROUTINE:
    // Setup internal data source and make available to EMS

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool FoundDuplicate = false;

    for (int InternalVarAvailNum = 1; InternalVarAvailNum <= state.dataRuntimeLang->numEMSInternalVarsAvailable; ++InternalVarAvailNum) {
        if ((UtilityRoutines::SameString(cDataTypeName, state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).DataTypeName)) &&
            (UtilityRoutines::SameString(cUniqueIDName, state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).UniqueIDName))) {
            FoundDuplicate = true;
            break;
        }
    }

    if (FoundDuplicate) {
        ShowSevereError(state, "Duplicate internal variable was sent to SetupEMSInternalVariable.");
        ShowContinueError(state, format("Internal variable type = {} ; name = {}", cDataTypeName, cUniqueIDName));
        ShowContinueError(state, "called from SetupEMSInternalVariable");
    } else {
        // add new internal data variable
        if (state.dataRuntimeLang->numEMSInternalVarsAvailable == 0) {
            state.dataRuntimeLang->EMSInternalVarsAvailable.allocate(state.dataRuntimeLang->varsAvailableAllocInc);
            state.dataRuntimeLang->numEMSInternalVarsAvailable = 1;
            state.dataRuntimeLang->maxEMSInternalVarsAvailable = state.dataRuntimeLang->varsAvailableAllocInc;
        } else {
            if (state.dataRuntimeLang->numEMSInternalVarsAvailable + 1 > state.dataRuntimeLang->maxEMSInternalVarsAvailable) {
                state.dataRuntimeLang->EMSInternalVarsAvailable.redimension(state.dataRuntimeLang->maxEMSInternalVarsAvailable +=
                                                                            state.dataRuntimeLang->varsAvailableAllocInc);
            }
            ++state.dataRuntimeLang->numEMSInternalVarsAvailable;
        }

        int InternalVarAvailNum = state.dataRuntimeLang->numEMSInternalVarsAvailable;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).DataTypeName = cDataTypeName;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).UniqueIDName = cUniqueIDName;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).Units = cUnits;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).IntValue = &iValue;
        state.dataRuntimeLang->EMSInternalVarsAvailable(InternalVarAvailNum).PntrVarTypeUsed = DataRuntimeLanguage::PtrDataType::Integer;
    }
}

} // namespace EnergyPlus
