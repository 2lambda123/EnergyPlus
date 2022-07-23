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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACManager.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SystemAvailabilityManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::HeatBalanceAirManager {
// Module containing the air heat balance simulation routines
// calculation (initialization) routines

// MODULE INFORMATION:
//       AUTHOR         Richard J. Liesen
//       DATE WRITTEN   February 1998
//       MODIFIED       May-July 2000 Joe Huang for Comis Link
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// manage the air simluation heat balance on the building.

// METHODOLOGY EMPLOYED:

// REFERENCES:
// The heat balance method is outlined in the "Tarp Alogorithms Manual"
// The methods are also summarized in many BSO Theses and papers.

// OTHER NOTES:
// This module was created from IBLAST subroutines

// USE STATEMENTS:
using namespace DataEnvironment;
using namespace DataHeatBalFanSys;
using namespace DataHeatBalance;
using namespace DataSurfaces;

// Use statements for access to subroutines in other modules
using Psychrometrics::PsyCpAirFnW;
using Psychrometrics::PsyHFnTdbW;
using Psychrometrics::PsyRhoAirFnPbTdbW;
using Psychrometrics::PsyTdbFnHW;

enum class AirflowSpec
{
    Invalid = -1,
    Flow,
    FlowPerZone,
    FlowPerArea,
    FlowPerPerson,
    AirChanges,
    Num
};
constexpr std::array<std::string_view, static_cast<int>(AirflowSpec::Num)> airflowNamesUC = {
    "FLOW", "FLOW/ZONE", "FLOW/AREA", "FLOW/PERSON", "AIRCHANGES/HOUR"};

enum class AirflowSpecAlt
{
    Invalid = -1,
    Flow,
    FlowPerZone,
    FlowPerArea,
    FlowPerExteriorArea,
    FlowPerExteriorWallArea,
    AirChanges,
    Num
};
constexpr std::array<std::string_view, static_cast<int>(AirflowSpecAlt::Num)> airflowAltNamesUC = {
    "FLOW", "FLOW/ZONE", "FLOW/AREA", "FLOW/EXTERIORAREA", "FLOW/EXTERIORWALLAREA", "AIRCHANGES/HOUR"};

constexpr std::array<std::string_view, static_cast<int>(VentilationType::Num)> ventilationTypeNamesUC = {"NATURAL", "INTAKE", "EXHAUST", "BALANCED"};

constexpr std::array<std::string_view, static_cast<int>(DataRoomAirModel::RoomAirModel::Num)> roomAirModelNamesUC = {
    "USERDEFINED",
    "MIXING",
    "ONENODEDISPLACEMENTVENTILATION",
    "THREENODEDISPLACEMENTVENTILATION",
    "CROSSVENTILATION",
    "UNDERFLOORAIRDISTRIBUTIONINTERIOR",
    "UNDERFLOORAIRDISTRIBUTIONEXTERIOR",
    "AIRFLOWNETWORK"};

constexpr std::array<std::string_view, static_cast<int>(DataRoomAirModel::CouplingScheme::Num)> couplingSchemeNamesUC = {"DIRECT", "INDIRECT"};

void ManageAirHeatBalance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages the heat air balance method of calculating
    // building thermal loads.  It is called from the HeatBalanceManager
    // at the time step level.  This driver manages the calls to all of
    // the other drivers and simulation algorithms.

    // Obtains and Allocates heat balance related parameters from input file
    if (state.dataHeatBalAirMgr->ManageAirHeatBalanceGetInputFlag) {
        GetAirHeatBalanceInput(state);
        state.dataHeatBalAirMgr->ManageAirHeatBalanceGetInputFlag = false;
    }

    InitAirHeatBalance(state); // Initialize all heat balance related parameters

    // Solve the zone heat balance 'Detailed' solution
    // Call the air surface heat balances
    CalcHeatBalanceAir(state);

    ReportZoneMeanAirTemp(state);
}

void GetAirHeatBalanceInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is the main routine to call other input routines

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger events.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    bool ErrorsFound(false);

    GetAirFlowFlag(state, ErrorsFound);

    SetZoneMassConservationFlag(state);

    // get input parameters for modeling of room air flow
    GetRoomAirModelParameters(state, ErrorsFound);

    if (ErrorsFound) {
        ShowFatalError(state, "GetAirHeatBalanceInput: Errors found in getting Air inputs");
    }
}

void GetAirFlowFlag(EnergyPlusData &state, bool &ErrorsFound) // Set to true if errors found
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Garrett Westmacott
    //       DATE WRITTEN   February 2000
    //       MODIFIED       Oct 2003, FCW: Change "Infiltration-Air Change Rate" from Sum to State
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calls the routine to get simple air flow input data.

    // METHODOLOGY EMPLOYED:
    // Modelled after 'Modual Example' in Guide for Module Developers

    // Using/Aliasing
    using ScheduleManager::GetScheduleIndex;

    state.dataHeatBal->AirFlowFlag = true; // UseSimpleAirFlow;

    GetSimpleAirModelInputs(state, ErrorsFound);
    if (state.dataHeatBal->TotInfiltration + state.dataHeatBal->TotVentilation + state.dataHeatBal->TotMixing + state.dataHeatBal->TotCrossMixing +
            state.dataHeatBal->TotRefDoorMixing >
        0) {
        static constexpr std::string_view Format_720("! <AirFlow Model>, Simple\n AirFlow Model, {}\n");
        print(state.files.eio, Format_720, "Simple");
    }
}

void SetZoneMassConservationFlag(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION :
    // AUTHOR         Bereket Nigusse, FSEC
    // DATE WRITTEN   February 2014
    // MODIFIED

    // PURPOSE OF THIS SUBROUTINE :
    // This subroutine sets the zone mass conservation flag to true.

    if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance &&
        state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing) {
        for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
            state.dataHeatBalFanSys->ZoneMassBalanceFlag(state.dataHeatBal->Mixing(Loop).ZonePtr) = true;
            state.dataHeatBalFanSys->ZoneMassBalanceFlag(state.dataHeatBal->Mixing(Loop).FromZone) = true;
        }
    }
}

void GetSimpleAirModelInputs(EnergyPlusData &state, bool &ErrorsFound) // IF errors found in input
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2000
    //       MODIFIED       Oct 2003,FCW: change "Infiltration-Air Change Rate" from Sum to State
    //       MODIFIED       Jan 2008,LG: Allow multiple infiltration and ventilation objects per zone
    //                      May 2009, BG: added calls to setup for possible EMS override
    //                      August 2011, TKS: added refrigeration door mixing

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the input for the "simple" air flow model.

    // Using/Aliasing
    using General::CheckCreatedZoneItemName;
    using ScheduleManager::CheckScheduleValueMinMax;
    using ScheduleManager::GetScheduleIndex;
    using ScheduleManager::GetScheduleMinValue;
    using ScheduleManager::GetScheduleName;
    using ScheduleManager::GetScheduleValuesForDay;
    using SystemAvailabilityManager::GetHybridVentilationControlStatus;

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr VentilTempLimit(100.0);                                    // degrees Celsius
    Real64 constexpr MixingTempLimit(100.0);                                    // degrees Celsius
    Real64 constexpr VentilWSLimit(40.0);                                       // m/s
    static constexpr std::string_view RoutineName("GetSimpleAirModelInputs: "); // include trailing blank space
    // Refrigeration Door Mixing Protection types, factors used to moderate mixing flow.
    Real64 constexpr RefDoorNone(0.0);
    Real64 constexpr RefDoorAirCurtain(0.5);
    Real64 constexpr RefDoorStripCurtain(0.9);

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array2D<Real64> SVals1;
    Array2D<Real64> SVals2;
    int NumAlpha;  // Number of Alphas for each GetobjectItem call
    int NumNumber; // Number of Numbers for each GetobjectItem call
    int maxAlpha;  // max of Alphas for allocation
    int maxNumber; // max of Numbers for allocation
    int NumArgs;
    int IOStat;
    Array1D_string cAlphaFieldNames;
    Array1D_string cNumericFieldNames;
    Array1D_bool lNumericFieldBlanks;
    Array1D_bool lAlphaFieldBlanks;
    Array1D_string cAlphaArgs;
    Array1D<Real64> rNumericArgs;
    std::string cCurrentModuleObject;

    Array1D_bool RepVarSet;
    bool IsNotOK;

    std::string StringOut;
    std::string NameThisObject;
    int VentiCount;
    bool ControlFlag;
    int Item1;
    bool errFlag;
    int ZLItem;
    Array1D<Real64> TotInfilVentFlow;
    Array1D<Real64> TotMixingFlow;
    Array1D<Real64> ZoneMixingNum;
    int ConnectTest;
    int ConnectionNumber;
    int NumbNum;
    int AlphaNum;
    int Zone1Num;
    int Zone2Num;
    int ZoneNumA;
    int ZoneNumB;
    int SourceCount;
    int ReceivingCount;
    int IsSourceZone;

    // Formats
    static constexpr std::string_view Format_720(" {} Airflow Stats Nominal, {},{},{},{:.2R},{:.1R},");
    static constexpr std::string_view Format_721(
        "! <{} Airflow Stats Nominal>,Name,Schedule Name,Zone Name, Zone Floor Area {{m2}}, # Zone Occupants,{}\n");
    static constexpr std::string_view Format_722(" {}, {}\n");

    RepVarSet.dimension(state.dataGlobal->NumOfZones, true);

    // Following used for reporting
    state.dataHeatBal->ZnAirRpt.allocate(state.dataGlobal->NumOfZones);

    for (int Loop = 1; Loop <= state.dataGlobal->NumOfZones; ++Loop) {
        // CurrentModuleObject='Zone'
        SetupOutputVariable(state,
                            "Zone Mean Air Temperature",
                            OutputProcessor::Unit::C,
                            state.dataHeatBal->ZnAirRpt(Loop).MeanAirTemp,
                            OutputProcessor::SOVTimeStepType::Zone,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Operative Temperature",
                            OutputProcessor::Unit::C,
                            state.dataHeatBal->ZnAirRpt(Loop).OperativeTemp,
                            OutputProcessor::SOVTimeStepType::Zone,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Mean Air Dewpoint Temperature",
                            OutputProcessor::Unit::C,
                            state.dataHeatBal->ZnAirRpt(Loop).MeanAirDewPointTemp,
                            OutputProcessor::SOVTimeStepType::Zone,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Mean Air Humidity Ratio",
                            OutputProcessor::Unit::kgWater_kgDryAir,
                            state.dataHeatBal->ZnAirRpt(Loop).MeanAirHumRat,
                            OutputProcessor::SOVTimeStepType::Zone,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Air Heat Balance Internal Convective Heat Gain Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).SumIntGains,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Air Heat Balance Surface Convection Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).SumHADTsurfs,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Air Heat Balance Interzone Air Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).SumMCpDTzones,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Air Heat Balance Outdoor Air Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).SumMCpDtInfil,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Air Heat Balance System Air Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).SumMCpDTsystem,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Air Heat Balance System Convective Heat Gain Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).SumNonAirSystem,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Air Heat Balance Air Energy Storage Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).CzdTdt,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        if (state.dataGlobal->DisplayAdvancedReportVariables) {
            SetupOutputVariable(state,
                                "Zone Phase Change Material Melting Enthalpy",
                                OutputProcessor::Unit::J_kg,
                                state.dataHeatBal->ZnAirRpt(Loop).SumEnthalpyM,
                                OutputProcessor::SOVTimeStepType::Zone,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Zone(Loop).Name);
            SetupOutputVariable(state,
                                "Zone Phase Change Material Freezing Enthalpy",
                                OutputProcessor::Unit::J_kg,
                                state.dataHeatBal->ZnAirRpt(Loop).SumEnthalpyH,
                                OutputProcessor::SOVTimeStepType::Zone,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Zone(Loop).Name);
            SetupOutputVariable(state,
                                "Zone Air Heat Balance Deviation Rate",
                                OutputProcessor::Unit::W,
                                state.dataHeatBal->ZnAirRpt(Loop).imBalance,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Zone(Loop).Name);
        }

        SetupOutputVariable(state,
                            "Zone Exfiltration Heat Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).ExfilTotalLoss,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Exfiltration Sensible Heat Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).ExfilSensiLoss,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Exfiltration Latent Heat Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).ExfilLatentLoss,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Exhaust Air Heat Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).ExhTotalLoss,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Exhaust Air Sensible Heat Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).ExhSensiLoss,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
        SetupOutputVariable(state,
                            "Zone Exhaust Air Latent Heat Transfer Rate",
                            OutputProcessor::Unit::W,
                            state.dataHeatBal->ZnAirRpt(Loop).ExhLatentLoss,
                            OutputProcessor::SOVTimeStepType::System,
                            OutputProcessor::SOVStoreType::Average,
                            state.dataHeatBal->Zone(Loop).Name);
    }

    SetupOutputVariable(state,
                        "Site Total Zone Exfiltration Heat Loss",
                        OutputProcessor::Unit::J,
                        state.dataHeatBal->ZoneTotalExfiltrationHeatLoss,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Summed,
                        "Environment");
    SetupOutputVariable(state,
                        "Site Total Zone Exhaust Air Heat Loss",
                        OutputProcessor::Unit::J,
                        state.dataHeatBal->ZoneTotalExhaustHeatLoss,
                        OutputProcessor::SOVTimeStepType::System,
                        OutputProcessor::SOVStoreType::Summed,
                        "Environment");

    cCurrentModuleObject = "ZoneAirBalance:OutdoorAir";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = NumAlpha;
    maxNumber = NumNumber;
    cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneMixing";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneCrossMixing";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);
    cCurrentModuleObject = "ZoneRefrigerationDoorMixing";
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, NumArgs, NumAlpha, NumNumber);
    maxAlpha = max(NumAlpha, maxAlpha);
    maxNumber = max(NumNumber, maxNumber);

    cAlphaArgs.allocate(maxAlpha);
    cAlphaFieldNames.allocate(maxAlpha);
    cNumericFieldNames.allocate(maxNumber);
    rNumericArgs.dimension(maxNumber, 0.0);
    lAlphaFieldBlanks.dimension(maxAlpha, true);
    lNumericFieldBlanks.dimension(maxNumber, true);

    cCurrentModuleObject = "ZoneAirBalance:OutdoorAir";
    state.dataHeatBal->TotZoneAirBalance = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

    state.dataHeatBal->ZoneAirBalance.allocate(state.dataHeatBal->TotZoneAirBalance);

    for (int Loop = 1; Loop <= state.dataHeatBal->TotZoneAirBalance; ++Loop) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 Loop,
                                                                 cAlphaArgs,
                                                                 NumAlpha,
                                                                 rNumericArgs,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);
        IsNotOK = false;
        UtilityRoutines::IsNameEmpty(state, cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);
        state.dataHeatBal->ZoneAirBalance(Loop).Name = cAlphaArgs(1);
        state.dataHeatBal->ZoneAirBalance(Loop).ZoneName = cAlphaArgs(2);
        state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), state.dataHeatBal->Zone);
        if (state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr == 0) {
            ShowSevereError(state,
                            format(R"({}{}="{}", invalid (not found) {}="{}".)",
                                   RoutineName,
                                   cCurrentModuleObject,
                                   cAlphaArgs(1),
                                   cAlphaFieldNames(2),
                                   cAlphaArgs(2)));
            ErrorsFound = true;
        }
        GlobalNames::IntraObjUniquenessCheck(
            state, cAlphaArgs(2), cCurrentModuleObject, cAlphaFieldNames(2), state.dataHeatBalAirMgr->UniqueZoneNames, IsNotOK);
        if (IsNotOK) {
            ShowSevereError(state,
                            format(R"({}{}="{}", a duplicated object {}="{}" is found.)",
                                   RoutineName,
                                   cCurrentModuleObject,
                                   cAlphaArgs(1),
                                   cAlphaFieldNames(2),
                                   cAlphaArgs(2)));
            ShowContinueError(state, "A zone can only have one " + cCurrentModuleObject + " object.");
            ErrorsFound = true;
        }

        {
            state.dataHeatBal->ZoneAirBalance(Loop).BalanceMethod = static_cast<AirBalance>(
                getEnumerationValue(DataHeatBalance::AirBalanceTypeNamesUC,
                                    UtilityRoutines::MakeUPPERCase(cAlphaArgs(3)))); // Air balance method type character input-->convert to enum
            if (state.dataHeatBal->ZoneAirBalance(Loop).BalanceMethod == AirBalance::Invalid) {
                state.dataHeatBal->ZoneAirBalance(Loop).BalanceMethod = AirBalance::None;
                ShowWarningError(state,
                                 format("{}{} = {} not valid choice for {}={}",
                                        RoutineName,
                                        cAlphaFieldNames(3),
                                        cAlphaArgs(3),
                                        cCurrentModuleObject,
                                        cAlphaArgs(1)));
                ShowContinueError(state, "The default choice \"NONE\" is assigned");
            }
        }

        state.dataHeatBal->ZoneAirBalance(Loop).InducedAirRate = rNumericArgs(1);
        if (rNumericArgs(1) < 0.0) {
            ShowSevereError(state,
                            format("{}{}=\"{}\", invalid Induced Outdoor Air Due to Duct Leakage Unbalance specification [<0.0]={:.3R}",
                                   RoutineName,
                                   cCurrentModuleObject,
                                   cAlphaArgs(1),
                                   rNumericArgs(1)));
            ErrorsFound = true;
        }

        state.dataHeatBal->ZoneAirBalance(Loop).InducedAirSchedPtr = GetScheduleIndex(state, cAlphaArgs(4));
        if (state.dataHeatBal->ZoneAirBalance(Loop).InducedAirSchedPtr == 0) {
            if (lAlphaFieldBlanks(4)) {
                ShowSevereError(
                    state,
                    format("{}{}=\"{}\",{} is required but field is blank.", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(4)));
            } else {
                ShowSevereError(state,
                                format(R"({}{}="{}", invalid (not found) {}="{}".)",
                                       RoutineName,
                                       cCurrentModuleObject,
                                       cAlphaArgs(1),
                                       cAlphaFieldNames(4),
                                       cAlphaArgs(4)));
            }
            ErrorsFound = true;
        }
        if (!CheckScheduleValueMinMax(state, state.dataHeatBal->ZoneAirBalance(Loop).InducedAirSchedPtr, ">=", 0.0, "<=", 1.0)) {
            ShowSevereError(state,
                            format("{} = {}:  Error found in {} = {}",
                                   cCurrentModuleObject,
                                   state.dataHeatBal->ZoneAirBalance(Loop).Name,
                                   cAlphaFieldNames(4),
                                   cAlphaArgs(4)));
            ShowContinueError(state, "Schedule values must be (>=0., <=1.)");
            ErrorsFound = true;
        }

        // Check whether this zone is also controleld by hybrid ventilation object with ventilation control option or not
        ControlFlag = GetHybridVentilationControlStatus(state, state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr);
        if (ControlFlag && state.dataHeatBal->ZoneAirBalance(Loop).BalanceMethod == AirBalance::Quadrature) {
            state.dataHeatBal->ZoneAirBalance(Loop).BalanceMethod = AirBalance::None;
            ShowWarningError(
                state,
                format("{} = {}: This Zone ({}) is controlled by AvailabilityManager:HybridVentilation with Simple Airflow Control Type option.",
                       cCurrentModuleObject,
                       state.dataHeatBal->ZoneAirBalance(Loop).Name,
                       cAlphaArgs(2)));
            ShowContinueError(state,
                              "Air balance method type QUADRATURE and Simple Airflow Control Type cannot co-exist. The NONE method is assigned");
        }

        if (state.dataHeatBal->ZoneAirBalance(Loop).BalanceMethod == AirBalance::Quadrature) {
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceHeatLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceHeatGain,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Latent Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceLatentLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Latent Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceLatentGain,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Total Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceTotalLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Total Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceTotalGain,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Current Density Volume Flow Rate",
                                OutputProcessor::Unit::m3_s,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceVdotCurDensity,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Standard Density Volume Flow Rate",
                                OutputProcessor::Unit::m3_s,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceVdotStdDensity,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Current Density Volume",
                                OutputProcessor::Unit::m3,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceVolumeCurDensity,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Standard Density Volume",
                                OutputProcessor::Unit::m3,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceVolumeStdDensity,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Mass",
                                OutputProcessor::Unit::kg,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceMass,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Mass Flow Rate",
                                OutputProcessor::Unit::kg_s,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceMdot,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Changes per Hour",
                                OutputProcessor::Unit::ach,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceAirChangeRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
            SetupOutputVariable(state,
                                "Zone Combined Outdoor Air Fan Electricity Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->ZnAirRpt(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).OABalanceFanElec,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name,
                                _,
                                "Electricity",
                                "Fans",
                                "Ventilation (simple)",
                                "Building",
                                state.dataHeatBal->Zone(state.dataHeatBal->ZoneAirBalance(Loop).ZonePtr).Name);
        }
    }

    // Set up and process ZoneInfiltration:* inputs

    cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
    int numDesignFlowInfiltrationObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    int totDesignFlowInfiltration = 0; // Total ZoneInfiltration:DesignFlowRate instances after expansion to spaces
    EPVector<DataHeatBalance::GlobalInternalGainMiscObject> infiltrationDesignFlowRateObjects;
    infiltrationDesignFlowRateObjects.allocate(numDesignFlowInfiltrationObjects);
    InternalHeatGains::setupIHGZonesAndSpaces(
        state, cCurrentModuleObject, infiltrationDesignFlowRateObjects, numDesignFlowInfiltrationObjects, totDesignFlowInfiltration, ErrorsFound);

    cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
    int numLeakageAreaInfiltrationObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    int totLeakageAreaInfiltration = 0; // Total ZoneInfiltration:EffectiveLeakageArea instances after expansion to spaces
    EPVector<DataHeatBalance::GlobalInternalGainMiscObject> infiltrationLeakageAreaObjects;
    infiltrationDesignFlowRateObjects.allocate(numDesignFlowInfiltrationObjects);
    bool const zoneListNotAllowed = true;
    InternalHeatGains::setupIHGZonesAndSpaces(state,
                                              cCurrentModuleObject,
                                              infiltrationLeakageAreaObjects,
                                              numLeakageAreaInfiltrationObjects,
                                              totLeakageAreaInfiltration,
                                              ErrorsFound,
                                              zoneListNotAllowed);

    cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
    int numFlowCoefficientInfiltrationObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    int totFlowCoefficientInfiltration = 0; // Total ZoneInfiltration:FlowCoefficient instances after expansion to spaces
    EPVector<DataHeatBalance::GlobalInternalGainMiscObject> infiltrationFlowCoefficientObjects;
    infiltrationDesignFlowRateObjects.allocate(numFlowCoefficientInfiltrationObjects);
    InternalHeatGains::setupIHGZonesAndSpaces(state,
                                              cCurrentModuleObject,
                                              infiltrationFlowCoefficientObjects,
                                              numFlowCoefficientInfiltrationObjects,
                                              totFlowCoefficientInfiltration,
                                              ErrorsFound,
                                              zoneListNotAllowed);

    state.dataHeatBal->TotInfiltration = totDesignFlowInfiltration + totLeakageAreaInfiltration + totFlowCoefficientInfiltration;
    state.dataHeatBal->Infiltration.allocate(state.dataHeatBal->TotInfiltration);
    state.dataHeatBalAirMgr->UniqueInfiltrationNames.reserve(static_cast<unsigned>(state.dataHeatBal->TotInfiltration));

    int infiltrationNum = 0;
    if (totDesignFlowInfiltration > 0) {
        cCurrentModuleObject = "ZoneInfiltration:DesignFlowRate";
        for (int infilInputNum = 1; infilInputNum <= numDesignFlowInfiltrationObjects; ++infilInputNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     infilInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            // Create one Infiltration instance for every space associated with this input object
            auto &thisInfiltrationInput = infiltrationDesignFlowRateObjects(infilInputNum);
            for (int Item1 = 1; Item1 <= thisInfiltrationInput.numOfSpaces; ++Item1) {
                ++infiltrationNum;
                auto &thisInfiltration = state.dataHeatBal->Infiltration(infiltrationNum);
                thisInfiltration.Name = thisInfiltrationInput.names(Item1);
                int const spaceNum = thisInfiltrationInput.spaceNums(Item1);
                int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                thisInfiltration.spaceIndex = spaceNum;
                thisInfiltration.ZonePtr = zoneNum;

                thisInfiltration.ModelType = DataHeatBalance::InfiltrationModelType::DesignFlowRate;
                thisInfiltration.SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (thisInfiltration.SchedPtr == 0) {
                    if (Item1 == 1) {
                        if (lAlphaFieldBlanks(3)) {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\",{} is required but field is blank.",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   cAlphaArgs(1),
                                                   cAlphaFieldNames(3)));
                        } else {
                            ShowSevereError(state,
                                            std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                                cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                        }
                        ErrorsFound = true;
                    }
                }

                // setup a flag if the outdoor air balance method is applied
                if (thisInfiltration.ZonePtr > 0 && state.dataHeatBal->TotZoneAirBalance > 0) {
                    for (int i = 1; i <= state.dataHeatBal->TotZoneAirBalance; ++i) {
                        if (thisInfiltration.ZonePtr == state.dataHeatBal->ZoneAirBalance(i).ZonePtr) {
                            if (state.dataHeatBal->ZoneAirBalance(i).BalanceMethod == AirBalance::Quadrature) {
                                thisInfiltration.QuadratureSum = true;
                                thisInfiltration.OABalancePtr = i;
                                break;
                            }
                        }
                    }
                }

                // Set space flow fractions
                // Infiltration equipment design level calculation method.
                AirflowSpecAlt flow =
                    static_cast<AirflowSpecAlt>(getEnumerationValue(airflowAltNamesUC, cAlphaArgs(4))); // NOLINT(modernize-use-auto)
                switch (flow) {
                case AirflowSpecAlt::Flow:
                case AirflowSpecAlt::FlowPerZone:
                    thisInfiltration.DesignLevel = rNumericArgs(1);
                    if (lNumericFieldBlanks(1)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(1)));
                    }
                    break;

                case AirflowSpecAlt::FlowPerArea:
                    if (thisInfiltration.ZonePtr != 0) {
                        if (rNumericArgs(2) >= 0.0) {
                            thisInfiltration.DesignLevel = rNumericArgs(2) * state.dataHeatBal->Zone(thisInfiltration.ZonePtr).FloorArea;
                            if (thisInfiltration.ZonePtr > 0) {
                                if (state.dataHeatBal->Zone(thisInfiltration.ZonePtr).FloorArea <= 0.0) {
                                    ShowWarningError(state,
                                                     format("{}{}=\"{}\", {} specifies {}, but Zone Floor Area = 0.  0 Infiltration will result.",
                                                            RoutineName,
                                                            cCurrentModuleObject,
                                                            thisInfiltration.Name,
                                                            cAlphaFieldNames(4),
                                                            cNumericFieldNames(2)));
                                }
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/area specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisInfiltration.Name,
                                                   rNumericArgs(2)));
                            ErrorsFound = true;
                        }
                    }
                    if (lAlphaFieldBlanks(2)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(2)));
                    }
                    break;

                case AirflowSpecAlt::FlowPerExteriorArea:
                    if (thisInfiltration.ZonePtr != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            thisInfiltration.DesignLevel = rNumericArgs(3) * state.dataHeatBal->Zone(thisInfiltration.ZonePtr).ExteriorTotalSurfArea;
                            if (state.dataHeatBal->Zone(thisInfiltration.ZonePtr).ExteriorTotalSurfArea <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Exterior Surface Area = 0.  0 Infiltration will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisInfiltration.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(3)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{} = \"{}\", invalid flow/exteriorarea specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisInfiltration.Name,
                                                   rNumericArgs(3)));
                            ErrorsFound = true;
                        }
                    }
                    if (lAlphaFieldBlanks(3)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(3)));
                    }
                    break;

                case AirflowSpecAlt::FlowPerExteriorWallArea:
                    if (thisInfiltration.ZonePtr != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            thisInfiltration.DesignLevel = rNumericArgs(3) * state.dataHeatBal->Zone(thisInfiltration.ZonePtr).ExtGrossWallArea;
                            if (state.dataHeatBal->Zone(thisInfiltration.ZonePtr).ExtGrossWallArea <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Exterior Wall Area = 0.  0 Infiltration will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisInfiltration.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(3)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{} = \"{}\", invalid flow/exteriorwallarea specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisInfiltration.Name,
                                                   rNumericArgs(3)));
                            ErrorsFound = true;
                        }
                    }
                    if (lAlphaFieldBlanks(3)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(3)));
                    }
                    break;

                case AirflowSpecAlt::AirChanges:
                    if (thisInfiltration.ZonePtr != 0) {
                        if (rNumericArgs(4) >= 0.0) {
                            thisInfiltration.DesignLevel =
                                rNumericArgs(4) * state.dataHeatBal->Zone(thisInfiltration.ZonePtr).Volume / DataGlobalConstants::SecInHour;
                            if (state.dataHeatBal->Zone(thisInfiltration.ZonePtr).Volume <= 0.0) {
                                ShowWarningError(state,
                                                 format("{}{}=\"{}\", {} specifies {}, but Zone Volume = 0.  0 Infiltration will result.",
                                                        RoutineName,
                                                        cCurrentModuleObject,
                                                        thisInfiltration.Name,
                                                        cAlphaFieldNames(4),
                                                        cNumericFieldNames(4)));
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}In {} = \"{}\", invalid ACH (air changes per hour) specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisInfiltration.Name,
                                                   rNumericArgs(4)));
                            ErrorsFound = true;
                        }
                    }
                    if (lAlphaFieldBlanks(4)) {
                        ShowWarningError(state,
                                         format("{}{}=\"{}\", {} specifies {}, but that field is blank.  0 Infiltration will result.",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                thisInfiltration.Name,
                                                cAlphaFieldNames(4),
                                                cNumericFieldNames(4)));
                    }
                    break;

                default:
                    if (Item1 == 1) {
                        ShowSevereError(
                            state,
                            format("{}{}=\"{}\", invalid calculation method={}", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(4)));
                        ErrorsFound = true;
                    }
                }

                thisInfiltration.ConstantTermCoef = !lNumericFieldBlanks(5) ? rNumericArgs(5) : 1.0;
                thisInfiltration.TemperatureTermCoef = !lNumericFieldBlanks(6) ? rNumericArgs(6) : 0.0;
                thisInfiltration.VelocityTermCoef = !lNumericFieldBlanks(7) ? rNumericArgs(7) : 0.0;
                thisInfiltration.VelocitySQTermCoef = !lNumericFieldBlanks(8) ? rNumericArgs(8) : 0.0;

                if (thisInfiltration.ConstantTermCoef == 0.0 && thisInfiltration.TemperatureTermCoef == 0.0 &&
                    thisInfiltration.VelocityTermCoef == 0.0 && thisInfiltration.VelocitySQTermCoef == 0.0) {
                    if (Item1 == 1) {
                        ShowWarningError(
                            state,
                            format(
                                R"({}{}="{}", in {}="{}".)", RoutineName, cCurrentModuleObject, cAlphaArgs(1), cAlphaFieldNames(2), cAlphaArgs(2)));
                        ShowContinueError(state, "Infiltration Coefficients are all zero.  No Infiltration will be reported.");
                    }
                }
            }
        }
    }

    if (totLeakageAreaInfiltration > 0) {
        cCurrentModuleObject = "ZoneInfiltration:EffectiveLeakageArea";
        for (int infilInputNum = 1; infilInputNum <= numDesignFlowInfiltrationObjects; ++infilInputNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     infilInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);
            // Create one Infiltration instance for every space associated with this input object
            auto &thisInfiltrationInput = infiltrationLeakageAreaObjects(infilInputNum);
            for (int Item1 = 1; Item1 <= thisInfiltrationInput.numOfSpaces; ++Item1) {
                ++infiltrationNum;
                auto &thisInfiltration = state.dataHeatBal->Infiltration(infiltrationNum);
                thisInfiltration.Name = thisInfiltrationInput.names(Item1);
                int const spaceNum = thisInfiltrationInput.spaceNums(Item1);
                int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                thisInfiltration.spaceIndex = spaceNum;
                thisInfiltration.ZonePtr = zoneNum;

                thisInfiltration.ModelType = DataHeatBalance::InfiltrationModelType::ShermanGrimsrud;

                // setup a flag if the outdoor air balance method is applied
                if (thisInfiltration.ZonePtr > 0 && state.dataHeatBal->TotZoneAirBalance > 0) {
                    for (int i = 1; i <= state.dataHeatBal->TotZoneAirBalance; ++i) {
                        if (thisInfiltration.ZonePtr == state.dataHeatBal->ZoneAirBalance(i).ZonePtr) {
                            if (state.dataHeatBal->ZoneAirBalance(i).BalanceMethod == AirBalance::Quadrature) {
                                thisInfiltration.QuadratureSum = true;
                                thisInfiltration.OABalancePtr = i;
                                break;
                            }
                        }
                    }
                }

                thisInfiltration.SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (thisInfiltration.SchedPtr == 0) {
                    if (lAlphaFieldBlanks(3)) {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\",{} is required but field is blank.",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1),
                                               cAlphaFieldNames(3)));
                    } else {
                        ShowSevereError(state,
                                        format(R"({}{}="{}", invalid (not found) {}="{}".)",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1),
                                               cAlphaFieldNames(3),
                                               cAlphaArgs(3)));
                    }
                    ErrorsFound = true;
                }
                thisInfiltration.LeakageArea = rNumericArgs(1);
                thisInfiltration.BasicStackCoefficient = rNumericArgs(2);
                thisInfiltration.BasicWindCoefficient = rNumericArgs(3);

                // check if zone has exterior surfaces
                if (thisInfiltration.ZonePtr > 0) {
                    if (state.dataHeatBal->Zone(thisInfiltration.ZonePtr).ExteriorTotalSurfArea <= 0.0) {
                        ShowWarningError(state,
                                         format(R"({}{}="{}", {}="{}" does not have surfaces exposed to outdoors.)",
                                                RoutineName,
                                                cCurrentModuleObject,
                                                cAlphaArgs(1),
                                                cAlphaFieldNames(2),
                                                cAlphaArgs(2)));
                        ShowContinueError(state, "Infiltration model is appropriate for exterior zones not interior zones, simulation continues.");
                    }
                }
            }
        }
    }

    if (totFlowCoefficientInfiltration > 0) {
        cCurrentModuleObject = "ZoneInfiltration:FlowCoefficient";
        for (int infilInputNum = 1; infilInputNum <= numDesignFlowInfiltrationObjects; ++infilInputNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     infilInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);
            // Create one Infiltration instance for every space associated with this input object
            auto &thisInfiltrationInput = infiltrationDesignFlowRateObjects(infilInputNum);
            for (int Item1 = 1; Item1 <= thisInfiltrationInput.numOfSpaces; ++Item1) {
                ++infiltrationNum;
                auto &thisInfiltration = state.dataHeatBal->Infiltration(infiltrationNum);
                thisInfiltration.Name = thisInfiltrationInput.names(Item1);
                int const spaceNum = thisInfiltrationInput.spaceNums(Item1);
                int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                thisInfiltration.spaceIndex = spaceNum;
                thisInfiltration.ZonePtr = zoneNum;

                thisInfiltration.ModelType = DataHeatBalance::InfiltrationModelType::AIM2;

                // setup a flag if the outdoor air balance method is applied
                if (thisInfiltration.ZonePtr > 0 && state.dataHeatBal->TotZoneAirBalance > 0) {
                    for (int i = 1; i <= state.dataHeatBal->TotZoneAirBalance; ++i) {
                        if (thisInfiltration.ZonePtr == state.dataHeatBal->ZoneAirBalance(i).ZonePtr) {
                            if (state.dataHeatBal->ZoneAirBalance(i).BalanceMethod == AirBalance::Quadrature) {
                                thisInfiltration.QuadratureSum = true;
                                thisInfiltration.OABalancePtr = i;
                                break;
                            }
                        }
                    }
                }

                thisInfiltration.SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (thisInfiltration.SchedPtr == 0) {
                    if (lAlphaFieldBlanks(3)) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                            " is required but field is blank.");
                    } else {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                            cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                    }
                    ErrorsFound = true;
                }
                thisInfiltration.FlowCoefficient = rNumericArgs(1);
                thisInfiltration.AIM2StackCoefficient = rNumericArgs(2);
                thisInfiltration.PressureExponent = rNumericArgs(3);
                thisInfiltration.AIM2WindCoefficient = rNumericArgs(4);
                thisInfiltration.ShelterFactor = rNumericArgs(5);

                // check if zone has exterior surfaces
                if (thisInfiltration.ZonePtr > 0) {
                    if (state.dataHeatBal->Zone(thisInfiltration.ZonePtr).ExteriorTotalSurfArea <= 0.0) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(2) +
                                             "=\"" + cAlphaArgs(2) + "\" does not have surfaces exposed to outdoors.");
                        ShowContinueError(state, "Infiltration model is appropriate for exterior zones not interior zones, simulation continues.");
                    }
                }
            }
        }
    }

    // setup zone-level infiltration reports
    for (int Loop = 1; Loop <= state.dataHeatBal->TotInfiltration; ++Loop) {
        if (state.dataHeatBal->Infiltration(Loop).ZonePtr > 0 && !state.dataHeatBal->Infiltration(Loop).QuadratureSum) {
            // Object report variables
            SetupOutputVariable(state,
                                "Infiltration Sensible Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->Infiltration(Loop).InfilHeatLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Sensible Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->Infiltration(Loop).InfilHeatGain,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Latent Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->Infiltration(Loop).InfilLatentLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Latent Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->Infiltration(Loop).InfilLatentGain,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Total Heat Loss Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->Infiltration(Loop).InfilTotalLoss,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Total Heat Gain Energy",
                                OutputProcessor::Unit::J,
                                state.dataHeatBal->Infiltration(Loop).InfilTotalGain,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Current Density Volume Flow Rate",
                                OutputProcessor::Unit::m3_s,
                                state.dataHeatBal->Infiltration(Loop).InfilVdotCurDensity,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Standard Density Volume Flow Rate",
                                OutputProcessor::Unit::m3_s,
                                state.dataHeatBal->Infiltration(Loop).InfilVdotStdDensity,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Current Density Volume",
                                OutputProcessor::Unit::m3,
                                state.dataHeatBal->Infiltration(Loop).InfilVolumeCurDensity,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Standard Density Volume",
                                OutputProcessor::Unit::m3,
                                state.dataHeatBal->Infiltration(Loop).InfilVolumeStdDensity,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Mass",
                                OutputProcessor::Unit::kg,
                                state.dataHeatBal->Infiltration(Loop).InfilMass,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Summed,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Mass Flow Rate",
                                OutputProcessor::Unit::kg_s,
                                state.dataHeatBal->Infiltration(Loop).InfilMdot,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);
            SetupOutputVariable(state,
                                "Infiltration Air Change Rate",
                                OutputProcessor::Unit::ach,
                                state.dataHeatBal->Infiltration(Loop).InfilAirChangeRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Infiltration(Loop).Name);

            if (RepVarSet(state.dataHeatBal->Infiltration(Loop).ZonePtr)) {
                RepVarSet(state.dataHeatBal->Infiltration(Loop).ZonePtr) = false;
                SetupOutputVariable(state,
                                    "Zone Infiltration Sensible Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilHeatLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Sensible Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilHeatGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Latent Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilLatentLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Latent Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilLatentGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Total Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilTotalLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Total Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilTotalGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Current Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVdotCurDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Standard Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVdotStdDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Current Density Volume",
                                    OutputProcessor::Unit::m3,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVolumeCurDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Standard Density Volume",
                                    OutputProcessor::Unit::m3,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilVolumeStdDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Mass",
                                    OutputProcessor::Unit::kg,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilMass,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilMdot,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Infiltration Air Change Rate",
                                    OutputProcessor::Unit::ach,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Infiltration(Loop).ZonePtr).InfilAirChangeRate,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Infiltration(Loop).ZonePtr).Name);
            }
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSActuator(state,
                             "Zone Infiltration",
                             state.dataHeatBal->Infiltration(Loop).Name,
                             "Air Exchange Flow Rate",
                             "[m3/s]",
                             state.dataHeatBal->Infiltration(Loop).EMSOverrideOn,
                             state.dataHeatBal->Infiltration(Loop).EMSAirFlowRateValue);
        }
    }

    // Set up and process ZoneVentilation:* inputs

    RepVarSet = true;

    cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
    int numDesignFlowVentilationObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    int totDesignFlowVentilation = 0; // Total ZoneVentilation:DesignFlowRate instances after expansion to spaces
    EPVector<DataHeatBalance::GlobalInternalGainMiscObject> ventilationDesignFlowRateObjects;
    ventilationDesignFlowRateObjects.allocate(numDesignFlowVentilationObjects);
    InternalHeatGains::setupIHGZonesAndSpaces(
        state, cCurrentModuleObject, ventilationDesignFlowRateObjects, numDesignFlowVentilationObjects, totDesignFlowVentilation, ErrorsFound);

    cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
    int numWindStackVentilationObjects = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    int totWindStackVentilation = 0; // Total ZoneVentilation:WindandStackOpenArea instances after expansion to spaces
    EPVector<DataHeatBalance::GlobalInternalGainMiscObject> ventilationWindStackObjects;
    ventilationWindStackObjects.allocate(numWindStackVentilationObjects);
    InternalHeatGains::setupIHGZonesAndSpaces(state,
                                              cCurrentModuleObject,
                                              ventilationWindStackObjects,
                                              numWindStackVentilationObjects,
                                              totWindStackVentilation,
                                              ErrorsFound,
                                              zoneListNotAllowed);

    state.dataHeatBal->TotVentilation = numDesignFlowVentilationObjects + numWindStackVentilationObjects;
    state.dataHeatBal->Ventilation.allocate(state.dataHeatBal->TotVentilation);

    int ventilationNum = 0;
    if (numDesignFlowVentilationObjects > 0) {
        cCurrentModuleObject = "ZoneVentilation:DesignFlowRate";
        for (int ventInputNum = 1; ventInputNum <= numDesignFlowVentilationObjects; ++ventInputNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     ventInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            auto &thisVentilationInput = ventilationDesignFlowRateObjects(ventInputNum);
            for (int Item1 = 1; Item1 <= thisVentilationInput.numOfSpaces; ++Item1) {
                ++ventilationNum;
                auto &thisVentilation = state.dataHeatBal->Ventilation(ventilationNum);
                thisVentilation.Name = thisVentilationInput.names(Item1);
                int const spaceNum = thisVentilationInput.spaceNums(Item1);
                int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                thisVentilation.spaceIndex = spaceNum;
                thisVentilation.ZonePtr = zoneNum;

                // setup a flag if the outdoor air balance method is applied
                if (thisVentilation.ZonePtr > 0 && state.dataHeatBal->TotZoneAirBalance > 0) {
                    for (int i = 1; i <= state.dataHeatBal->TotZoneAirBalance; ++i) {
                        if (thisVentilation.ZonePtr == state.dataHeatBal->ZoneAirBalance(i).ZonePtr) {
                            if (state.dataHeatBal->ZoneAirBalance(i).BalanceMethod == AirBalance::Quadrature) {
                                thisVentilation.QuadratureSum = true;
                                thisVentilation.OABalancePtr = i;
                                break;
                            }
                        }
                    }
                }

                thisVentilation.ModelType = DataHeatBalance::VentilationModelType::DesignFlowRate;
                thisVentilation.SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (thisVentilation.SchedPtr == 0) {
                    if (Item1 == 1) {
                        if (lAlphaFieldBlanks(3)) {
                            ShowSevereError(state,
                                            std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                                " is required but field is blank.");
                        } else {
                            ShowSevereError(state,
                                            std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                                cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                        }
                    }
                    ErrorsFound = true;
                }

                // Ventilation equipment design level calculation method
                AirflowSpec flow = static_cast<AirflowSpec>(getEnumerationValue(airflowNamesUC, cAlphaArgs(4))); // NOLINT(modernize-use-auto)
                switch (flow) {
                case AirflowSpec::Flow:
                case AirflowSpec::FlowPerZone:
                    thisVentilation.DesignLevel = rNumericArgs(1);
                    if (lAlphaFieldBlanks(1)) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\", " +
                                             cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(1) +
                                             ", but that field is blank.  0 Ventilation will result.");
                    }
                    break;

                case AirflowSpec::FlowPerArea:
                    if (thisVentilation.ZonePtr != 0) {
                        if (rNumericArgs(2) >= 0.0) {
                            thisVentilation.DesignLevel = rNumericArgs(2) * state.dataHeatBal->Zone(thisVentilation.ZonePtr).FloorArea;
                            if (state.dataHeatBal->Zone(thisVentilation.ZonePtr).FloorArea <= 0.0) {
                                ShowWarningError(state,
                                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\", " +
                                                     cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(2) +
                                                     ", but Zone Floor Area = 0.  0 Ventilation will result.");
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/area specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisVentilation.Name,
                                                   rNumericArgs(2)));
                            ErrorsFound = true;
                        }
                    }
                    if (lAlphaFieldBlanks(2)) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\", " +
                                             cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(2) +
                                             ", but that field is blank.  0 Ventilation will result.");
                    }
                    break;

                case AirflowSpec::FlowPerPerson:
                    if (thisVentilation.ZonePtr != 0) {
                        if (rNumericArgs(3) >= 0.0) {
                            thisVentilation.DesignLevel = rNumericArgs(3) * state.dataHeatBal->Zone(thisVentilation.ZonePtr).TotOccupants;
                            if (state.dataHeatBal->Zone(thisVentilation.ZonePtr).TotOccupants <= 0.0) {
                                ShowWarningError(state,
                                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\", " +
                                                     cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(3) +
                                                     ", but Zone Total Occupants = 0.  0 Ventilation will result.");
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisVentilation.Name,
                                                   rNumericArgs(3)));
                            ErrorsFound = true;
                        }
                    }
                    if (lAlphaFieldBlanks(3)) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\", " +
                                             cAlphaFieldNames(4) + "specifies " + cNumericFieldNames(3) +
                                             ", but that field is blank.  0 Ventilation will result.");
                    }
                    break;

                case AirflowSpec::AirChanges:
                    if (thisVentilation.ZonePtr != 0) {
                        if (rNumericArgs(4) >= 0.0) {
                            thisVentilation.DesignLevel =
                                rNumericArgs(4) * state.dataHeatBal->Zone(thisVentilation.ZonePtr).Volume / DataGlobalConstants::SecInHour;
                            if (state.dataHeatBal->Zone(thisVentilation.ZonePtr).Volume <= 0.0) {
                                ShowWarningError(state,
                                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\", " +
                                                     cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(4) +
                                                     ", but Zone Volume = 0.  0 Ventilation will result.");
                            }
                        } else {
                            ShowSevereError(state,
                                            format("{}{}=\"{}\", invalid ACH (air changes per hour) specification [<0.0]={:.3R}",
                                                   RoutineName,
                                                   cCurrentModuleObject,
                                                   thisVentilation.Name,
                                                   rNumericArgs(5)));
                            ErrorsFound = true;
                        }
                    }
                    if (lAlphaFieldBlanks(4)) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\", " +
                                             cAlphaFieldNames(4) + " specifies " + cNumericFieldNames(4) +
                                             ", but that field is blank.  0 Ventilation will result.");
                    }
                    break;

                default:
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                            "\", invalid calculation method=" + cAlphaArgs(4));
                        ErrorsFound = true;
                    }
                }

                if (cAlphaArgs(5).empty()) {
                    thisVentilation.FanType = DataHeatBalance::VentilationType::Natural;
                } else {
                    thisVentilation.FanType = static_cast<VentilationType>(getEnumerationValue(ventilationTypeNamesUC, cAlphaArgs(5)));
                    if (thisVentilation.FanType == VentilationType::Invalid) {
                        ShowSevereError(state,
                                        format(R"({}{}="{}". invalid {}="{}".)",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               thisVentilation.Name,
                                               cAlphaFieldNames(5),
                                               cAlphaArgs(5)));
                        ErrorsFound = true;
                    }
                }

                thisVentilation.FanPressure = rNumericArgs(5);
                if (thisVentilation.FanPressure < 0.0) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\", " +
                                            cNumericFieldNames(5) + " must be >=0");
                        ErrorsFound = true;
                    }
                }

                thisVentilation.FanEfficiency = rNumericArgs(6);
                if ((thisVentilation.FanEfficiency <= 0.0) || (thisVentilation.FanEfficiency > 1.0)) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + thisVentilation.Name + "\"," +
                                            cNumericFieldNames(6) + " must be in range >0 and <= 1");
                        ErrorsFound = true;
                    }
                }

                // Override any user input for cases where natural ventilation is being used
                if (thisVentilation.FanType == DataHeatBalance::VentilationType::Natural) {
                    thisVentilation.FanPressure = 0.0;
                    thisVentilation.FanEfficiency = 1.0;
                }

                thisVentilation.ConstantTermCoef = !lNumericFieldBlanks(7) ? rNumericArgs(7) : 1.0;
                thisVentilation.TemperatureTermCoef = !lNumericFieldBlanks(8) ? rNumericArgs(8) : 0.0;
                thisVentilation.VelocityTermCoef = !lNumericFieldBlanks(9) ? rNumericArgs(9) : 0.0;
                thisVentilation.VelocitySQTermCoef = !lNumericFieldBlanks(10) ? rNumericArgs(10) : 0.0;

                if (thisVentilation.ConstantTermCoef == 0.0 && thisVentilation.TemperatureTermCoef == 0.0 &&
                    thisVentilation.VelocityTermCoef == 0.0 && thisVentilation.VelocitySQTermCoef == 0.0) {
                    if (Item1 == 1) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", in " + cAlphaFieldNames(2) +
                                             "=\"" + cAlphaArgs(2) + "\".");
                        ShowContinueError(state, "Ventilation Coefficients are all zero.  No Ventilation will be reported.");
                    }
                }

                if (!lNumericFieldBlanks(11)) {
                    thisVentilation.MinIndoorTemperature = rNumericArgs(11);
                } else {
                    thisVentilation.MinIndoorTemperature = -VentilTempLimit;
                }
                //    Ventilation(Loop)%MinIndoorTemperature = rNumericArgs(11)
                if ((thisVentilation.MinIndoorTemperature < -VentilTempLimit) || (thisVentilation.MinIndoorTemperature > VentilTempLimit)) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\" must have " +
                                            cNumericFieldNames(11) + " between -100C and 100C.");
                        ShowContinueError(state, format("...value entered=[{:.2R}].", rNumericArgs(11)));
                        ErrorsFound = true;
                    }
                }

                thisVentilation.MinIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
                if (thisVentilation.MinIndoorTempSchedPtr > 0) {
                    if (Item1 == 1) {
                        if (!lNumericFieldBlanks(11))
                            ShowWarningError(
                                state,
                                std::string{RoutineName} +
                                    "The Minimum Indoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                    cCurrentModuleObject + " object = " + cAlphaArgs(1));
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(state, thisVentilation.MinIndoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                            ShowSevereError(
                                state,
                                std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a minimum indoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(6));
                            ErrorsFound = true;
                        }
                    }
                }
                if (thisVentilation.MinIndoorTempSchedPtr == 0 && lNumericFieldBlanks(11) && (!lAlphaFieldBlanks(6))) {
                    if (Item1 == 1) {
                        ShowWarningError(
                            state,
                            format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                   RoutineName,
                                   cNumericFieldNames(11),
                                   -VentilTempLimit));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                }
                // Check Minimum indoor temperature value and schedule fields
                if (!lNumericFieldBlanks(11) && (!cAlphaArgs(6).empty() && thisVentilation.MinIndoorTempSchedPtr == 0)) {
                    if (Item1 == 1) {
                        ShowWarningError(state,
                                         format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                                RoutineName,
                                                cAlphaFieldNames(6),
                                                cAlphaArgs(6),
                                                rNumericArgs(11)));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                }

                thisVentilation.MaxIndoorTemperature = !lNumericFieldBlanks(12) ? rNumericArgs(12) : VentilTempLimit;
                if ((thisVentilation.MaxIndoorTemperature < -VentilTempLimit) || (thisVentilation.MaxIndoorTemperature > VentilTempLimit)) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                            " must have a maximum indoor temperature between -100C and 100C");
                        ErrorsFound = true;
                    }
                }

                thisVentilation.MaxIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(7));
                if (thisVentilation.MaxIndoorTempSchedPtr > 0) {
                    if (Item1 == 1) {
                        if (!lNumericFieldBlanks(12))
                            ShowWarningError(
                                state,
                                std::string{RoutineName} +
                                    "The Maximum Indoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                    cCurrentModuleObject + " object = " + cAlphaArgs(1));
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(state, thisVentilation.MaxIndoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                            ShowSevereError(
                                state,
                                cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                    " must have a maximum indoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(7));
                            ErrorsFound = true;
                        }
                    }
                }
                if (thisVentilation.MaxIndoorTempSchedPtr == 0 && lNumericFieldBlanks(12) && (!lAlphaFieldBlanks(7))) {
                    if (Item1 == 1) {
                        ShowWarningError(
                            state,
                            format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                   RoutineName,
                                   cNumericFieldNames(12),
                                   VentilTempLimit));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                }
                // Check Maximum indoor temperature value and schedule fields
                if (!lNumericFieldBlanks(12) && ((!lAlphaFieldBlanks(7)) && thisVentilation.MaxIndoorTempSchedPtr == 0)) {
                    if (Item1 == 1) {
                        ShowWarningError(state,
                                         format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                                RoutineName,
                                                cAlphaFieldNames(7),
                                                cAlphaArgs(7),
                                                rNumericArgs(12)));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                }

                thisVentilation.DelTemperature = !lNumericFieldBlanks(13) ? rNumericArgs(13) : -VentilTempLimit;
                //    Ventilation(Loop)%DelTemperature = rNumericArgs(13)  !  3/12/03  Negative del temp now allowed COP

                thisVentilation.DeltaTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
                if (thisVentilation.DeltaTempSchedPtr > 0) {
                    if (Item1 == 1) {
                        if (!lNumericFieldBlanks(13))
                            ShowWarningError(
                                state,
                                std::string{RoutineName} +
                                    "The Delta Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                    cCurrentModuleObject + " object = " + cAlphaArgs(1));
                        // Check min value in the schedule to ensure both values are within the range
                        if (GetScheduleMinValue(state, thisVentilation.DeltaTempSchedPtr) < -VentilTempLimit) {
                            ShowSevereError(state,
                                            std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                                " must have a delta temperature equal to or above -100C defined in the schedule = " + cAlphaArgs(8));
                            ErrorsFound = true;
                        }
                    }
                }
                if (thisVentilation.DeltaTempSchedPtr == 0 && lNumericFieldBlanks(13) && (!lAlphaFieldBlanks(8))) {
                    if (Item1 == 1) {
                        ShowWarningError(
                            state,
                            format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                   RoutineName,
                                   cNumericFieldNames(13),
                                   VentilTempLimit));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                }

                if (!lNumericFieldBlanks(13) && ((!lAlphaFieldBlanks(8)) && thisVentilation.DeltaTempSchedPtr == 0)) {
                    if (Item1 == 1) {
                        ShowWarningError(state,
                                         format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                                RoutineName,
                                                cAlphaFieldNames(8),
                                                cAlphaArgs(8),
                                                rNumericArgs(13)));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                }

                thisVentilation.MinOutdoorTemperature = !lNumericFieldBlanks(14) ? rNumericArgs(14) : -VentilTempLimit;
                if ((thisVentilation.MinOutdoorTemperature < -VentilTempLimit) || (thisVentilation.MinOutdoorTemperature > VentilTempLimit)) {
                    if (Item1 == 1) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have " +
                                            cNumericFieldNames(14) + " between -100C and 100C");
                        ErrorsFound = true;
                    }
                }

                thisVentilation.MinOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(9));
                if (Item1 == 1) {
                    if (thisVentilation.MinOutdoorTempSchedPtr > 0) {
                        if (!lNumericFieldBlanks(14))
                            ShowWarningError(state,
                                             std::string{RoutineName} +
                                                 "The Minimum Outdoor Temperature value and schedule are provided. The scheduled temperature will be "
                                                 "used in the " +
                                                 cCurrentModuleObject + " object = " + cAlphaArgs(1));
                        // Check min and max values in the schedule to ensure both values are within the range
                        if (!CheckScheduleValueMinMax(state, thisVentilation.MinOutdoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                            ShowSevereError(
                                state,
                                std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a minimum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(9));
                            ErrorsFound = true;
                        }
                    }
                    if (thisVentilation.MinOutdoorTempSchedPtr == 0 && lNumericFieldBlanks(14) && (!lAlphaFieldBlanks(9))) {
                        ShowWarningError(state,
                                         format("{}Minimum Outdoor Temperature: the value field is blank and schedule field is invalid. The "
                                                "default value will be used ({:.1R}) ",
                                                RoutineName,
                                                -VentilTempLimit));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                    // Check Minimum outdoor temperature value and schedule fields
                    if (!lNumericFieldBlanks(14) && ((!lAlphaFieldBlanks(9)) && thisVentilation.MinOutdoorTempSchedPtr == 0)) {
                        ShowWarningError(state,
                                         format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                                RoutineName,
                                                cAlphaFieldNames(9),
                                                cAlphaArgs(9),
                                                rNumericArgs(14)));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                }

                thisVentilation.MaxOutdoorTemperature = !lNumericFieldBlanks(15) ? rNumericArgs(15) : VentilTempLimit;
                if (Item1 == 1) {
                    if ((thisVentilation.MaxOutdoorTemperature < -VentilTempLimit) || (thisVentilation.MaxOutdoorTemperature > VentilTempLimit)) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have a " +
                                            cNumericFieldNames(15) + " between -100C and 100C");
                        ErrorsFound = true;
                    }
                }

                thisVentilation.MaxOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(10));
                if (Item1 == 1) {
                    if (thisVentilation.MaxOutdoorTempSchedPtr > 0) {
                        if (!lNumericFieldBlanks(15))
                            ShowWarningError(state,
                                             std::string{RoutineName} +
                                                 "The Maximum Outdoor Temperature value and schedule are provided. The scheduled temperature will be "
                                                 "used in the " +
                                                 cCurrentModuleObject + " object = " + cAlphaArgs(1));
                        if (!CheckScheduleValueMinMax(state, thisVentilation.MaxOutdoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                            ShowSevereError(
                                state,
                                std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                    " must have a maximum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(10));
                            ErrorsFound = true;
                        }
                    }
                    if (thisVentilation.MaxOutdoorTempSchedPtr == 0 && lNumericFieldBlanks(15) && (!lAlphaFieldBlanks(10))) {
                        ShowWarningError(
                            state,
                            format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                   RoutineName,
                                   cNumericFieldNames(15),
                                   VentilTempLimit));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                    // Check Maximum outdoor temperature value and schedule fields
                    if (!lNumericFieldBlanks(15) && ((!lAlphaFieldBlanks(10)) && thisVentilation.MaxOutdoorTempSchedPtr == 0)) {
                        ShowWarningError(state,
                                         format("{}{} = {}is invalid. The constant value will be used at {:.1R} degrees C ",
                                                RoutineName,
                                                cAlphaFieldNames(10),
                                                cAlphaArgs(10),
                                                rNumericArgs(15)));
                        ShowContinueError(state,
                                          "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                    }
                }

                thisVentilation.MaxWindSpeed = !lNumericFieldBlanks(16) ? rNumericArgs(16) : VentilWSLimit;
                if (Item1 == 1) {
                    if ((thisVentilation.MaxWindSpeed < -VentilWSLimit) || (thisVentilation.MaxWindSpeed > VentilWSLimit)) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                            " must have a maximum wind speed between -40 m/s and 40 m/s");
                        ErrorsFound = true;
                    }
                }

                // Report variables should be added for individual VENTILATION objects, in addition to zone totals below

                if (thisVentilation.ZonePtr > 0) {
                    if (RepVarSet(thisVentilation.ZonePtr) && !thisVentilation.QuadratureSum) {
                        RepVarSet(thisVentilation.ZonePtr) = false;
                        SetupOutputVariable(state,
                                            "Zone Ventilation Sensible Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilHeatLoss,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Sensible Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilHeatGain,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Latent Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilLatentLoss,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Latent Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilLatentGain,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Total Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilTotalLoss,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Total Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilTotalGain,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Volume Flow Rate",
                                            OutputProcessor::Unit::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotCurDensity,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Volume Flow Rate",
                                            OutputProcessor::Unit::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotStdDensity,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Volume",
                                            OutputProcessor::Unit::m3,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVolumeCurDensity,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Volume",
                                            OutputProcessor::Unit::m3,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVolumeStdDensity,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Mass",
                                            OutputProcessor::Unit::kg,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilMass,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Mass Flow Rate",
                                            OutputProcessor::Unit::kg_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilMdot,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Air Change Rate",
                                            OutputProcessor::Unit::ach,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirChangeRate,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Fan Electricity Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilFanElec,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name,
                                            _,
                                            "Electricity",
                                            "Fans",
                                            "Ventilation (simple)",
                                            "Building",
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Air Inlet Temperature",
                                            OutputProcessor::Unit::C,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirTemp,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                    }
                }

                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(state,
                                     "Zone Ventilation",
                                     thisVentilation.Name,
                                     "Air Exchange Flow Rate",
                                     "[m3/s]",
                                     thisVentilation.EMSSimpleVentOn,
                                     thisVentilation.EMSimpleVentFlowRate);
                }
            }
        }
    }

    if (numWindStackVentilationObjects > 0) {
        cCurrentModuleObject = "ZoneVentilation:WindandStackOpenArea";
        for (int ventInputNum = 1; ventInputNum <= numWindStackVentilationObjects; ++ventInputNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     ventInputNum,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);

            auto &thisVentilationInput = ventilationWindStackObjects(ventInputNum);
            for (int Item1 = 1; Item1 <= thisVentilationInput.numOfSpaces; ++Item1) {
                ++ventilationNum;
                auto &thisVentilation = state.dataHeatBal->Ventilation(ventilationNum);
                thisVentilation.Name = thisVentilationInput.names(Item1);
                int const spaceNum = thisVentilationInput.spaceNums(Item1);
                int const zoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
                thisVentilation.spaceIndex = spaceNum;
                thisVentilation.ZonePtr = zoneNum;

                thisVentilation.ModelType = DataHeatBalance::VentilationModelType::WindAndStack;

                // setup a flag if the outdoor air balance method is applied
                if (thisVentilation.ZonePtr > 0 && state.dataHeatBal->TotZoneAirBalance > 0) {
                    for (int i = 1; i <= state.dataHeatBal->TotZoneAirBalance; ++i) {
                        if (thisVentilation.ZonePtr == state.dataHeatBal->ZoneAirBalance(i).ZonePtr) {
                            if (state.dataHeatBal->ZoneAirBalance(i).BalanceMethod == AirBalance::Quadrature) {
                                thisVentilation.QuadratureSum = true;
                                thisVentilation.OABalancePtr = i;
                                break;
                            }
                        }
                    }
                }

                thisVentilation.OpenArea = rNumericArgs(1);
                if (thisVentilation.OpenArea < 0.0) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(1) +
                                        " must be positive.");
                    ErrorsFound = true;
                }

                thisVentilation.OpenAreaSchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
                if (thisVentilation.OpenAreaSchedPtr == 0) {
                    if (lAlphaFieldBlanks(3)) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(3) +
                                            " is required but field is blank.");
                    } else {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                            cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                    }
                    ErrorsFound = true;
                }

                thisVentilation.OpenEff = rNumericArgs(2);
                if (thisVentilation.OpenEff != DataGlobalConstants::AutoCalculate &&
                    (thisVentilation.OpenEff < 0.0 || thisVentilation.OpenEff > 1.0)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(2) +
                                        " must be between 0 and 1.");
                    ErrorsFound = true;
                }

                thisVentilation.EffAngle = rNumericArgs(3);
                if (thisVentilation.EffAngle < 0.0 || thisVentilation.EffAngle >= 360.0) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(3) +
                                        " must be between 0 and 360.");
                    ErrorsFound = true;
                }

                thisVentilation.DH = rNumericArgs(4);
                if (thisVentilation.DH < 0.0) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(4) +
                                        " must be positive.");
                    ErrorsFound = true;
                }

                thisVentilation.DiscCoef = rNumericArgs(5);
                if (thisVentilation.DiscCoef != DataGlobalConstants::AutoCalculate &&
                    (thisVentilation.DiscCoef < 0.0 || thisVentilation.DiscCoef > 1.0)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cNumericFieldNames(5) +
                                        " must be between 0 and 1.");
                    ErrorsFound = true;
                }

                if (!lNumericFieldBlanks(6)) {
                    thisVentilation.MinIndoorTemperature = rNumericArgs(6);
                } else {
                    thisVentilation.MinIndoorTemperature = -VentilTempLimit;
                }
                if ((thisVentilation.MinIndoorTemperature < -VentilTempLimit) || (thisVentilation.MinIndoorTemperature > VentilTempLimit)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have " +
                                        cNumericFieldNames(6) + " between -100C and 100C");
                    ErrorsFound = true;
                }

                thisVentilation.MinIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(4));
                if (thisVentilation.MinIndoorTempSchedPtr > 0) {
                    if (!lNumericFieldBlanks(6))
                        ShowWarningError(
                            state,
                            std::string{RoutineName} +
                                "The Minimum Indoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                cCurrentModuleObject + " object = " + cAlphaArgs(1));
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(state, thisVentilation.MinIndoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                " must have a minimum indoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(4));
                        ErrorsFound = true;
                    }
                }
                if (thisVentilation.MinIndoorTempSchedPtr == 0 && lNumericFieldBlanks(6) && (!lAlphaFieldBlanks(4))) {
                    ShowWarningError(state,
                                     format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                            RoutineName,
                                            cNumericFieldNames(6),
                                            -VentilTempLimit));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }
                // Check Minimum indoor temperature value and schedule fields
                if (!lNumericFieldBlanks(6) && (!cAlphaArgs(4).empty() && thisVentilation.MinIndoorTempSchedPtr == 0)) {
                    ShowWarningError(state,
                                     format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                            RoutineName,
                                            cAlphaFieldNames(4),
                                            cAlphaArgs(4),
                                            rNumericArgs(11)));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }

                if (!lNumericFieldBlanks(7)) {
                    thisVentilation.MaxIndoorTemperature = rNumericArgs(7);
                } else {
                    thisVentilation.MaxIndoorTemperature = VentilTempLimit;
                }
                if ((thisVentilation.MaxIndoorTemperature < -VentilTempLimit) || (thisVentilation.MaxIndoorTemperature > VentilTempLimit)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                        "\" must have a maximum indoor temperature between -100C and 100C");
                    ErrorsFound = true;
                }

                thisVentilation.MaxIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(5));
                if (thisVentilation.MaxIndoorTempSchedPtr > 0) {
                    if (!lNumericFieldBlanks(7))
                        ShowWarningError(
                            state,
                            std::string{RoutineName} +
                                "The Maximum Indoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                cCurrentModuleObject + " object = " + cAlphaArgs(1));
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(state, thisVentilation.MaxIndoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                        ShowSevereError(
                            state,
                            cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a maximum indoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(5));
                        ErrorsFound = true;
                    }
                }
                if (thisVentilation.MaxIndoorTempSchedPtr == 0 && lNumericFieldBlanks(7) && (!lAlphaFieldBlanks(5))) {
                    ShowWarningError(state,
                                     format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                            RoutineName,
                                            cNumericFieldNames(7),
                                            VentilTempLimit));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }
                // Check Maximum indoor temperature value and schedule fields
                if (!lNumericFieldBlanks(7) && ((!lAlphaFieldBlanks(5)) && thisVentilation.MaxIndoorTempSchedPtr == 0)) {
                    ShowWarningError(state,
                                     format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                            RoutineName,
                                            cAlphaFieldNames(7),
                                            cAlphaArgs(5),
                                            rNumericArgs(7)));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }

                if (!lNumericFieldBlanks(8)) {
                    thisVentilation.DelTemperature = rNumericArgs(8);
                } else {
                    thisVentilation.DelTemperature = -VentilTempLimit;
                }

                thisVentilation.DeltaTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
                if (thisVentilation.DeltaTempSchedPtr > 0) {
                    if (!lNumericFieldBlanks(8))
                        ShowWarningError(state,
                                         std::string{RoutineName} +
                                             "The Delta Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                             cCurrentModuleObject + " object = " + cAlphaArgs(1));
                    // Check min value in the schedule to ensure both values are within the range
                    if (GetScheduleMinValue(state, thisVentilation.DeltaTempSchedPtr) < -VentilTempLimit) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                            " must have a delta temperature equal to or above -100C defined in the schedule = " + cAlphaArgs(8));
                        ErrorsFound = true;
                    }
                }
                if (thisVentilation.DeltaTempSchedPtr == 0 && lNumericFieldBlanks(8) && (!lAlphaFieldBlanks(6))) {
                    ShowWarningError(state,
                                     format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                            RoutineName,
                                            cNumericFieldNames(8),
                                            VentilTempLimit));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }
                if (!lNumericFieldBlanks(8) && ((!lAlphaFieldBlanks(6)) && thisVentilation.DeltaTempSchedPtr == 0)) {
                    ShowWarningError(state,
                                     format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                            RoutineName,
                                            cAlphaFieldNames(6),
                                            cAlphaArgs(6),
                                            rNumericArgs(8)));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }

                thisVentilation.MinOutdoorTemperature = !lNumericFieldBlanks(9) ? rNumericArgs(9) : -VentilTempLimit;
                if ((thisVentilation.MinOutdoorTemperature < -VentilTempLimit) || (thisVentilation.MinOutdoorTemperature > VentilTempLimit)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have " +
                                        cNumericFieldNames(9) + " between -100C and 100C");
                    ErrorsFound = true;
                }

                thisVentilation.MinOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(7));
                if (thisVentilation.MinOutdoorTempSchedPtr > 0) {
                    if (!lNumericFieldBlanks(9))
                        ShowWarningError(
                            state,
                            std::string{RoutineName} +
                                "The Minimum Outdoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                cCurrentModuleObject + " object = " + cAlphaArgs(1));
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(state, thisVentilation.MinOutdoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                " must have a minimum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(7));
                        ErrorsFound = true;
                    }
                }
                if (thisVentilation.MinOutdoorTempSchedPtr == 0 && lNumericFieldBlanks(9) && (!lAlphaFieldBlanks(7))) {
                    ShowWarningError(
                        state,
                        format("{}Minimum Outdoor Temperature: the value field is blank and schedule field is invalid. The default value "
                               "will be used ({:.1R}) ",
                               RoutineName,
                               -VentilTempLimit));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }
                // Check Minimum outdoor temperature value and schedule fields
                if (!lNumericFieldBlanks(9) && ((!lAlphaFieldBlanks(7)) && thisVentilation.MinOutdoorTempSchedPtr == 0)) {
                    ShowWarningError(state,
                                     format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                            RoutineName,
                                            cAlphaFieldNames(7),
                                            cAlphaArgs(7),
                                            rNumericArgs(14)));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }

                thisVentilation.MaxOutdoorTemperature = !lNumericFieldBlanks(10) ? rNumericArgs(10) : VentilTempLimit;
                if ((thisVentilation.MaxOutdoorTemperature < -VentilTempLimit) || (thisVentilation.MaxOutdoorTemperature > VentilTempLimit)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) + " must have a " +
                                        cNumericFieldNames(10) + " between -100C and 100C");
                    ErrorsFound = true;
                }

                thisVentilation.MaxOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
                if (thisVentilation.MaxOutdoorTempSchedPtr > 0) {
                    if (!lNumericFieldBlanks(10))
                        ShowWarningError(
                            state,
                            std::string{RoutineName} +
                                "The Maximum Outdoor Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                cCurrentModuleObject + " object = " + cAlphaArgs(1));
                    if (!CheckScheduleValueMinMax(state, thisVentilation.MaxOutdoorTempSchedPtr, ">=", -VentilTempLimit, "<=", VentilTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                " must have a maximum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(8));
                        ErrorsFound = true;
                    }
                }
                if (thisVentilation.MaxOutdoorTempSchedPtr == 0 && lNumericFieldBlanks(10) && (!lAlphaFieldBlanks(8))) {
                    ShowWarningError(state,
                                     format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                            RoutineName,
                                            cNumericFieldNames(10),
                                            VentilTempLimit));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }
                // Check Maximum outdoor temperature value and schedule fields
                if (!lNumericFieldBlanks(10) && ((!lAlphaFieldBlanks(8)) && thisVentilation.MaxOutdoorTempSchedPtr == 0)) {
                    ShowWarningError(state,
                                     format("{}{} = {}is invalid. The constant value will be used at {:.1R} degrees C ",
                                            RoutineName,
                                            cAlphaFieldNames(8),
                                            cAlphaArgs(8),
                                            rNumericArgs(10)));
                    ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
                }

                thisVentilation.MaxWindSpeed = !lNumericFieldBlanks(11) ? rNumericArgs(11) : VentilWSLimit;
                if ((thisVentilation.MaxWindSpeed < -VentilWSLimit) || (thisVentilation.MaxWindSpeed > VentilWSLimit)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                        " must have a maximum wind speed between 0 m/s and 40 m/s");
                    ErrorsFound = true;
                }

                // Report variables should be added for individual VENTILATION objects, in addition to zone totals below

                if (thisVentilation.ZonePtr > 0) {
                    if (RepVarSet(thisVentilation.ZonePtr) && !thisVentilation.QuadratureSum) {
                        RepVarSet(thisVentilation.ZonePtr) = false;
                        SetupOutputVariable(state,
                                            "Zone Ventilation Sensible Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilHeatLoss,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Sensible Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilHeatGain,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Latent Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilLatentLoss,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Latent Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilLatentGain,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Total Heat Loss Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilTotalLoss,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Total Heat Gain Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilTotalGain,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Volume Flow Rate",
                                            OutputProcessor::Unit::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotCurDensity,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Volume Flow Rate",
                                            OutputProcessor::Unit::m3_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVdotStdDensity,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Current Density Volume",
                                            OutputProcessor::Unit::m3,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVolumeCurDensity,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Standard Density Volume",
                                            OutputProcessor::Unit::m3,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilVolumeStdDensity,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Mass",
                                            OutputProcessor::Unit::kg,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilMass,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Mass Flow Rate",
                                            OutputProcessor::Unit::kg_s,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilMdot,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Air Change Rate",
                                            OutputProcessor::Unit::ach,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirChangeRate,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Fan Electricity Energy",
                                            OutputProcessor::Unit::J,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilFanElec,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Summed,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name,
                                            _,
                                            "Electricity",
                                            "Fans",
                                            "Ventilation (simple)",
                                            "Building",
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                        SetupOutputVariable(state,
                                            "Zone Ventilation Air Inlet Temperature",
                                            OutputProcessor::Unit::C,
                                            state.dataHeatBal->ZnAirRpt(thisVentilation.ZonePtr).VentilAirTemp,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(thisVentilation.ZonePtr).Name);
                    }
                }

                if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                    SetupEMSActuator(state,
                                     "Zone Ventilation",
                                     thisVentilation.Name,
                                     "Air Exchange Flow Rate",
                                     "[m3/s]",
                                     thisVentilation.EMSSimpleVentOn,
                                     thisVentilation.EMSimpleVentFlowRate);
                }
            }
        }
    }

    RepVarSet = true;

    cCurrentModuleObject = "ZoneMixing";
    state.dataHeatBal->TotMixing = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    state.dataHeatBal->Mixing.allocate(state.dataHeatBal->TotMixing);

    for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 Loop,
                                                                 cAlphaArgs,
                                                                 NumAlpha,
                                                                 rNumericArgs,
                                                                 NumNumber,
                                                                 IOStat,
                                                                 lNumericFieldBlanks,
                                                                 lAlphaFieldBlanks,
                                                                 cAlphaFieldNames,
                                                                 cNumericFieldNames);
        UtilityRoutines::IsNameEmpty(state, cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

        state.dataHeatBal->Mixing(Loop).Name = cAlphaArgs(1);

        state.dataHeatBal->Mixing(Loop).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), state.dataHeatBal->Zone);
        if (state.dataHeatBal->Mixing(Loop).ZonePtr == 0) {
            ShowSevereError(state,
                            std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
            ErrorsFound = true;
        }

        state.dataHeatBal->Mixing(Loop).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));

        if (state.dataHeatBal->Mixing(Loop).SchedPtr == 0) {
            if (lAlphaFieldBlanks(3)) {
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                    " is required but field is blank.");
            } else {
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                    cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
            }
            ErrorsFound = true;
        }

        // Mixing equipment design level calculation method
        AirflowSpec flow = static_cast<AirflowSpec>(getEnumerationValue(airflowNamesUC, cAlphaArgs(4))); // NOLINT(modernize-use-auto)
        switch (flow) {
        case AirflowSpec::Flow:
        case AirflowSpec::FlowPerZone:
            state.dataHeatBal->Mixing(Loop).DesignLevel = rNumericArgs(1);
            if (lAlphaFieldBlanks(1)) {
                ShowWarningError(state,
                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                     " specifies " + cNumericFieldNames(1) + ", but that field is blank.  0 Mixing will result.");
            }
            break;

        case AirflowSpec::FlowPerArea:
            if (state.dataHeatBal->Mixing(Loop).ZonePtr != 0) {
                if (rNumericArgs(2) >= 0.0) {
                    state.dataHeatBal->Mixing(Loop).DesignLevel =
                        rNumericArgs(2) * state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).FloorArea;
                    if (state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).FloorArea <= 0.0) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                             " specifies " + cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Mixing will result.");
                    }
                } else {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1),
                                           rNumericArgs(2)));
                    ErrorsFound = true;
                }
            }
            if (lNumericFieldBlanks(2)) {
                ShowWarningError(state,
                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                     " specifies " + cNumericFieldNames(2) + ", but that field is blank.  0 Mixing will result.");
            }
            break;

        case AirflowSpec::FlowPerPerson:
            if (state.dataHeatBal->Mixing(Loop).ZonePtr != 0) {
                if (rNumericArgs(3) >= 0.0) {
                    state.dataHeatBal->Mixing(Loop).DesignLevel =
                        rNumericArgs(3) * state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).TotOccupants;
                    if (state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).TotOccupants <= 0.0) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                             " specifies " + cNumericFieldNames(3) + ", but Zone Total Occupants = 0.  0 Mixing will result.");
                    }
                } else {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1),
                                           rNumericArgs(3)));
                    ErrorsFound = true;
                }
            }
            if (lNumericFieldBlanks(3)) {
                ShowWarningError(state,
                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                     " specifies " + cNumericFieldNames(3) + ", but that field is blank.  0 Mixing will result.");
            }
            break;

        case AirflowSpec::AirChanges:
            if (state.dataHeatBal->Mixing(Loop).ZonePtr != 0) {
                if (rNumericArgs(4) >= 0.0) {
                    state.dataHeatBal->Mixing(Loop).DesignLevel =
                        rNumericArgs(4) * state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Volume / DataGlobalConstants::SecInHour;
                    if (state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Volume <= 0.0) {
                        ShowWarningError(state,
                                         std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                             " specifies " + cNumericFieldNames(4) + ", but Zone Volume = 0.  0 Mixing will result.");
                    }
                } else {
                    ShowSevereError(state,
                                    format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                           RoutineName,
                                           cCurrentModuleObject,
                                           cAlphaArgs(1),
                                           rNumericArgs(4)));
                    ErrorsFound = true;
                }
            }
            if (lAlphaFieldBlanks(4)) {
                ShowWarningError(state,
                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                     " specifies " + cNumericFieldNames(4) + ", but that field is blank.  0 Mixing will result.");
            }
            break;

        default:
            ShowSevereError(
                state, std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid calculation method=" + cAlphaArgs(4));
            ErrorsFound = true;
        }

        state.dataHeatBal->Mixing(Loop).FromZone = UtilityRoutines::FindItemInList(cAlphaArgs(5), state.dataHeatBal->Zone);
        if (state.dataHeatBal->Mixing(Loop).FromZone == 0) {
            ShowSevereError(state,
                            std::string{RoutineName} + cAlphaFieldNames(5) + " not found=" + cAlphaArgs(5) + " for " + cCurrentModuleObject + '=' +
                                cAlphaArgs(1));
            ErrorsFound = true;
        }
        state.dataHeatBal->Mixing(Loop).DeltaTemperature = rNumericArgs(5);

        if (NumAlpha > 5) {
            state.dataHeatBal->Mixing(Loop).DeltaTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
            if (state.dataHeatBal->Mixing(Loop).DeltaTempSchedPtr > 0) {
                if (!lNumericFieldBlanks(5))
                    ShowWarningError(state,
                                     std::string{RoutineName} +
                                         "The Delta Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                         cCurrentModuleObject + " object = " + cAlphaArgs(1));
                if (GetScheduleMinValue(state, state.dataHeatBal->Mixing(Loop).DeltaTempSchedPtr) < -MixingTempLimit) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                        " must have a delta temperature equal to or above -100C defined in the schedule = " + cAlphaArgs(6));
                    ErrorsFound = true;
                }
            }
        }
        if (state.dataHeatBal->Mixing(Loop).DeltaTempSchedPtr == 0 && lNumericFieldBlanks(5) && (!lAlphaFieldBlanks(6))) {
            ShowWarningError(state,
                             format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                    RoutineName,
                                    cNumericFieldNames(5),
                                    rNumericArgs(5)));
            ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
        }
        if (!lNumericFieldBlanks(5) && ((!lAlphaFieldBlanks(6)) && state.dataHeatBal->Mixing(Loop).DeltaTempSchedPtr == 0)) {
            ShowWarningError(state,
                             format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                    RoutineName,
                                    cAlphaFieldNames(6),
                                    cAlphaArgs(6),
                                    rNumericArgs(5)));
            ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
        }

        if (NumAlpha > 6) {
            state.dataHeatBal->Mixing(Loop).MinIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(7));
            if (state.dataHeatBal->Mixing(Loop).MinIndoorTempSchedPtr == 0) {
                if ((!lAlphaFieldBlanks(7))) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cAlphaFieldNames(7) + " not found=" + cAlphaArgs(7) + " for " + cCurrentModuleObject +
                                        '=' + cAlphaArgs(1));
                    ErrorsFound = true;
                }
            }
            if (state.dataHeatBal->Mixing(Loop).MinIndoorTempSchedPtr > 0) {
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(
                        state, state.dataHeatBal->Mixing(Loop).MinIndoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + " statement = " + cAlphaArgs(1) +
                                        " must have a minimum zone temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(7));
                    ErrorsFound = true;
                }
            }
        }

        if (NumAlpha > 7) {
            state.dataHeatBal->Mixing(Loop).MaxIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
            if (state.dataHeatBal->Mixing(Loop).MaxIndoorTempSchedPtr == 0) {
                if ((!lAlphaFieldBlanks(8))) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(8) +
                                        " not found=\"" + cAlphaArgs(8) + "\".");
                    ErrorsFound = true;
                }
            }
            if (state.dataHeatBal->Mixing(Loop).MaxIndoorTempSchedPtr > 0) {
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(
                        state, state.dataHeatBal->Mixing(Loop).MaxIndoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                        "\" must have a maximum zone temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(8));
                    ErrorsFound = true;
                }
            }
        }

        if (NumAlpha > 8) {
            state.dataHeatBal->Mixing(Loop).MinSourceTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(9));
            if (state.dataHeatBal->Mixing(Loop).MinSourceTempSchedPtr == 0) {
                if ((!lAlphaFieldBlanks(9))) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(9) +
                                        " not found=\"" + cAlphaArgs(9) + "\".");
                    ErrorsFound = true;
                }
            }
            if (state.dataHeatBal->Mixing(Loop).MinSourceTempSchedPtr > 0) {
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(
                        state, state.dataHeatBal->Mixing(Loop).MinSourceTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                    ShowSevereError(
                        state,
                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                            "\" must have a minimum source temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(9));
                    ErrorsFound = true;
                }
            }
        }

        if (NumAlpha > 9) {
            state.dataHeatBal->Mixing(Loop).MaxSourceTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(10));
            if (state.dataHeatBal->Mixing(Loop).MaxSourceTempSchedPtr == 0) {
                if ((!lAlphaFieldBlanks(10))) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(10) +
                                        " not found=\"" + cAlphaArgs(10) + "\".");
                    ErrorsFound = true;
                }
            }
            if (state.dataHeatBal->Mixing(Loop).MaxSourceTempSchedPtr > 0) {
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(
                        state, state.dataHeatBal->Mixing(Loop).MaxSourceTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                    ShowSevereError(
                        state,
                        std::string{RoutineName} + cCurrentModuleObject + " statement =\"" + cAlphaArgs(1) +
                            "\" must have a maximum source temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(10));
                    ErrorsFound = true;
                }
            }
        }

        if (NumAlpha > 10) {
            state.dataHeatBal->Mixing(Loop).MinOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(11));
            if (state.dataHeatBal->Mixing(Loop).MinOutdoorTempSchedPtr == 0) {
                if ((!lAlphaFieldBlanks(11))) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(11) +
                                        " not found=\"" + cAlphaArgs(11) + "\".");
                    ErrorsFound = true;
                }
            }
            if (state.dataHeatBal->Mixing(Loop).MinOutdoorTempSchedPtr > 0) {
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(
                        state, state.dataHeatBal->Mixing(Loop).MinOutdoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                    ShowSevereError(
                        state,
                        std::string{RoutineName} + cCurrentModuleObject + " =\"" + cAlphaArgs(1) +
                            "\" must have a minimum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(11));
                    ErrorsFound = true;
                }
            }
        }

        if (NumAlpha > 11) {
            state.dataHeatBal->Mixing(Loop).MaxOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(12));
            if (state.dataHeatBal->Mixing(Loop).MaxOutdoorTempSchedPtr == 0) {
                if ((!lAlphaFieldBlanks(12))) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(12) +
                                        " not found=\"" + cAlphaArgs(12) + "\".");
                    ErrorsFound = true;
                }
            }
            if (state.dataHeatBal->Mixing(Loop).MaxOutdoorTempSchedPtr > 0) {
                // Check min and max values in the schedule to ensure both values are within the range
                if (!CheckScheduleValueMinMax(
                        state, state.dataHeatBal->Mixing(Loop).MaxOutdoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                    ShowSevereError(
                        state,
                        std::string{RoutineName} + cCurrentModuleObject + " =\"" + cAlphaArgs(1) +
                            "\" must have a maximum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(12));
                    ErrorsFound = true;
                }
            }
        }

        if (state.dataHeatBal->Mixing(Loop).ZonePtr > 0) {
            if (RepVarSet(state.dataHeatBal->Mixing(Loop).ZonePtr)) {
                RepVarSet(state.dataHeatBal->Mixing(Loop).ZonePtr) = false;
                SetupOutputVariable(state,
                                    "Zone Mixing Volume",
                                    OutputProcessor::Unit::m3,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixVolume,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Current Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixVdotCurDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Standard Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixVdotStdDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Mass",
                                    OutputProcessor::Unit::kg,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixMass,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixMdot,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Sensible Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixHeatLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Sensible Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixHeatGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Latent Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixLatentLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Latent Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixLatentGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Total Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixTotalLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Total Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->Mixing(Loop).ZonePtr).MixTotalGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).ZonePtr).Name);
            }
        }
        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSActuator(state,
                             "ZoneMixing",
                             state.dataHeatBal->Mixing(Loop).Name,
                             "Air Exchange Flow Rate",
                             "[m3/s]",
                             state.dataHeatBal->Mixing(Loop).EMSSimpleMixingOn,
                             state.dataHeatBal->Mixing(Loop).EMSimpleMixingFlowRate);
        }
    }

    // allocate MassConservation
    state.dataHeatBal->MassConservation.allocate(state.dataGlobal->NumOfZones);

    // added by BAN, 02/14
    if (state.dataHeatBal->TotMixing > 0) {
        ZoneMixingNum.allocate(state.dataHeatBal->TotMixing);
        // get source zones mixing objects index
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            SourceCount = 0;
            for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
                if (ZoneNum == state.dataHeatBal->Mixing(Loop).FromZone) {
                    SourceCount += 1;
                    ZoneMixingNum(SourceCount) = Loop;
                }
            }
            // save mixing objects index for zones which serve as a source zone
            state.dataHeatBal->MassConservation(ZoneNum).NumSourceZonesMixingObject = SourceCount;
            if (SourceCount > 0) {
                state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingSourcesPtr.allocate(SourceCount);
                for (int Loop = 1; Loop <= SourceCount; ++Loop) {
                    state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingSourcesPtr(Loop) = ZoneMixingNum(Loop);
                }
            }
        }

        // check zones which are used only as a source zones
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            IsSourceZone = false;
            for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
                if (ZoneNum != state.dataHeatBal->Mixing(Loop).FromZone) continue;
                state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone = true;
                for (int Loop1 = 1; Loop1 <= state.dataHeatBal->TotMixing; ++Loop1) {
                    if (ZoneNum == state.dataHeatBal->Mixing(Loop1).ZonePtr) {
                        state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone = false;
                        break;
                    }
                }
            }
        }
        // get receiving zones mixing objects index
        ZoneMixingNum = 0;
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            ReceivingCount = 0;
            for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
                if (ZoneNum == state.dataHeatBal->Mixing(Loop).ZonePtr) {
                    ReceivingCount += 1;
                    ZoneMixingNum(ReceivingCount) = Loop;
                }
            }
            // save mixing objects index for zones which serve as a receiving zone
            state.dataHeatBal->MassConservation(ZoneNum).NumReceivingZonesMixingObject = ReceivingCount;
            if (ReceivingCount > 0) {
                state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingReceivingPtr.allocate(ReceivingCount);
                state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingReceivingFr.allocate(ReceivingCount);
                for (int Loop = 1; Loop <= ReceivingCount; ++Loop) {
                    state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingReceivingPtr(Loop) = ZoneMixingNum(Loop);
                }
            }
            // flag zones used as both source and receiving zone
            if (state.dataHeatBal->MassConservation(ZoneNum).NumSourceZonesMixingObject > 0 &&
                state.dataHeatBal->MassConservation(ZoneNum).NumReceivingZonesMixingObject > 0) {
                state.dataHeatBal->MassConservation(ZoneNum).IsSourceAndReceivingZone = true;
            }
        }
        if (allocated(ZoneMixingNum)) ZoneMixingNum.deallocate();
    }

    // zone mass conservation calculation order starts with receiving zones
    // and then proceeds to source zones
    int Loop2 = 0;
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (!state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone &&
            !state.dataHeatBal->MassConservation(ZoneNum).IsSourceAndReceivingZone) {
            Loop2 += 1;
            state.dataHeatBalFanSys->ZoneReOrder(Loop2) = ZoneNum;
        }
    }
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataHeatBal->MassConservation(ZoneNum).IsSourceAndReceivingZone) {
            Loop2 += 1;
            state.dataHeatBalFanSys->ZoneReOrder(Loop2) = ZoneNum;
        }
    }
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone) {
            Loop2 += 1;
            state.dataHeatBalFanSys->ZoneReOrder(Loop2) = ZoneNum;
        }
    }
    cCurrentModuleObject = "ZoneCrossMixing";
    int inputCrossMixing = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    state.dataHeatBal->TotCrossMixing = inputCrossMixing + state.dataHeatBal->NumAirBoundaryMixing;
    state.dataHeatBal->CrossMixing.allocate(state.dataHeatBal->TotCrossMixing);

    for (int Loop = 1; Loop <= state.dataHeatBal->TotCrossMixing; ++Loop) {

        if (Loop > inputCrossMixing) {
            // Create CrossMixing object from air boundary info
            int airBoundaryIndex = Loop - inputCrossMixing - 1; // zero-based
            int zone1 = state.dataHeatBal->AirBoundaryMixingZone1[airBoundaryIndex];
            int zone2 = state.dataHeatBal->AirBoundaryMixingZone2[airBoundaryIndex];
            state.dataHeatBal->CrossMixing(Loop).Name = fmt::format("Air Boundary Mixing Zones {} and {}", zone1, zone2);
            state.dataHeatBal->CrossMixing(Loop).ZonePtr = zone1;
            state.dataHeatBal->CrossMixing(Loop).SchedPtr = state.dataHeatBal->AirBoundaryMixingSched[airBoundaryIndex];
            state.dataHeatBal->CrossMixing(Loop).DesignLevel = state.dataHeatBal->AirBoundaryMixingVol[airBoundaryIndex];
            state.dataHeatBal->CrossMixing(Loop).FromZone = zone2;
        } else {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Loop,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(state, cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            state.dataHeatBal->CrossMixing(Loop).Name = cAlphaArgs(1);

            state.dataHeatBal->CrossMixing(Loop).ZonePtr = UtilityRoutines::FindItemInList(cAlphaArgs(2), state.dataHeatBal->Zone);
            if (state.dataHeatBal->CrossMixing(Loop).ZonePtr == 0) {
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                    cAlphaFieldNames(2) + "=\"" + cAlphaArgs(2) + "\".");
                ErrorsFound = true;
            }

            state.dataHeatBal->CrossMixing(Loop).SchedPtr = GetScheduleIndex(state, cAlphaArgs(3));
            if (state.dataHeatBal->CrossMixing(Loop).SchedPtr == 0) {
                if (lAlphaFieldBlanks(3)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(3) +
                                        " is required but field is blank.");
                } else {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                        cAlphaFieldNames(3) + "=\"" + cAlphaArgs(3) + "\".");
                }
                ErrorsFound = true;
            }

            // Mixing equipment design level calculation method.
            AirflowSpec flow = static_cast<AirflowSpec>(getEnumerationValue(airflowNamesUC, cAlphaArgs(4))); // NOLINT(modernize-use-auto)
            switch (flow) {
            case AirflowSpec::Flow:
            case AirflowSpec::FlowPerZone:
                state.dataHeatBal->CrossMixing(Loop).DesignLevel = rNumericArgs(1);
                if (lAlphaFieldBlanks(1)) {
                    ShowWarningError(state,
                                     std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                         " specifies " + cNumericFieldNames(1) + ", but that field is blank.  0 Cross Mixing will result.");
                }
                break;

            case AirflowSpec::FlowPerArea:
                if (state.dataHeatBal->CrossMixing(Loop).ZonePtr != 0) {
                    if (rNumericArgs(2) >= 0.0) {
                        state.dataHeatBal->CrossMixing(Loop).DesignLevel =
                            rNumericArgs(2) * state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).FloorArea;
                        if (state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).FloorArea <= 0.0) {
                            ShowWarningError(state,
                                             std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(2) + ", but Zone Floor Area = 0.  0 Cross Mixing will result.");
                        }
                    } else {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1),
                                               rNumericArgs(2)));
                        ErrorsFound = true;
                    }
                }
                if (lAlphaFieldBlanks(2)) {
                    ShowWarningError(state,
                                     std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                         " specifies " + cNumericFieldNames(2) + ", but that field is blank.  0 Cross Mixing will result.");
                }
                break;

            case AirflowSpec::FlowPerPerson:
                if (state.dataHeatBal->CrossMixing(Loop).ZonePtr != 0) {
                    if (rNumericArgs(3) >= 0.0) {
                        state.dataHeatBal->CrossMixing(Loop).DesignLevel =
                            rNumericArgs(3) * state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).TotOccupants;
                        if (state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).TotOccupants <= 0.0) {
                            ShowWarningError(state,
                                             std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(3) +
                                                 ", but Zone Total Occupants = 0.  0 Cross Mixing will result.");
                        }
                    } else {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1),
                                               rNumericArgs(3)));
                        ErrorsFound = true;
                    }
                }
                if (lAlphaFieldBlanks(3)) {
                    ShowWarningError(state,
                                     std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                         " specifies " + cNumericFieldNames(3) + ", but that field is blank.  0 Cross Mixing will result.");
                }
                break;

            case AirflowSpec::AirChanges:
                if (state.dataHeatBal->CrossMixing(Loop).ZonePtr != 0) {
                    if (rNumericArgs(4) >= 0.0) {
                        state.dataHeatBal->CrossMixing(Loop).DesignLevel =
                            rNumericArgs(4) * state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Volume /
                            DataGlobalConstants::SecInHour;
                        if (state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Volume <= 0.0) {
                            ShowWarningError(state,
                                             std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                                 " specifies " + cNumericFieldNames(4) + ", but Zone Volume = 0.  0 Cross Mixing will result.");
                        }
                    } else {
                        ShowSevereError(state,
                                        format("{}{}=\"{}\", invalid flow/person specification [<0.0]={:.3R}",
                                               RoutineName,
                                               cCurrentModuleObject,
                                               cAlphaArgs(1),
                                               rNumericArgs(4)));
                        ErrorsFound = true;
                    }
                }
                if (lAlphaFieldBlanks(4)) {
                    ShowWarningError(state,
                                     std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", " + cAlphaFieldNames(4) +
                                         " specifies " + cNumericFieldNames(4) + ", but that field is blank.  0 Cross Mixing will result.");
                }
                break;

            default:
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                    "\", invalid calculation method=" + cAlphaArgs(4));
                ErrorsFound = true;
            }

            state.dataHeatBal->CrossMixing(Loop).FromZone = UtilityRoutines::FindItemInList(cAlphaArgs(5), state.dataHeatBal->Zone);
            if (state.dataHeatBal->CrossMixing(Loop).FromZone == 0) {
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                    cAlphaFieldNames(5) + "=\"" + cAlphaArgs(5) + "\".");
                ErrorsFound = true;
            }
            state.dataHeatBal->CrossMixing(Loop).DeltaTemperature = rNumericArgs(5);

            if (NumAlpha > 5) {
                state.dataHeatBal->CrossMixing(Loop).DeltaTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(6));
                if (state.dataHeatBal->CrossMixing(Loop).DeltaTempSchedPtr > 0) {
                    if (!lNumericFieldBlanks(5))
                        ShowWarningError(state,
                                         std::string{RoutineName} +
                                             "The Delta Temperature value and schedule are provided. The scheduled temperature will be used in the " +
                                             cCurrentModuleObject + " object = " + cAlphaArgs(1));
                    if (GetScheduleMinValue(state, state.dataHeatBal->CrossMixing(Loop).DeltaTempSchedPtr) < 0.0) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                            " must have a delta temperature equal to or above 0 C defined in the schedule = " + cAlphaArgs(6));
                        ErrorsFound = true;
                    }
                }
            }
            if (state.dataHeatBal->CrossMixing(Loop).DeltaTempSchedPtr == 0 && lNumericFieldBlanks(5) && (!lAlphaFieldBlanks(6))) {
                ShowWarningError(state,
                                 format("{}{}: the value field is blank and schedule field is invalid. The default value will be used ({:.1R}) ",
                                        RoutineName,
                                        cNumericFieldNames(5),
                                        rNumericArgs(5)));
                ShowContinueError(state, "in " + cCurrentModuleObject + " = " + cAlphaArgs(1) + " and the simulation continues...");
            }
            if (!lNumericFieldBlanks(5) && ((!lAlphaFieldBlanks(6)) && state.dataHeatBal->CrossMixing(Loop).DeltaTempSchedPtr == 0)) {
                ShowWarningError(state,
                                 format("{}{} = {} is invalid. The constant value will be used at {:.1R} degrees C ",
                                        RoutineName,
                                        cAlphaFieldNames(6),
                                        cAlphaArgs(6),
                                        rNumericArgs(5)));
                ShowContinueError(state, "in the " + cCurrentModuleObject + " object = " + cAlphaArgs(1) + " and the simulation continues...");
            }

            if (NumAlpha > 6) {
                state.dataHeatBal->CrossMixing(Loop).MinIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(7));
                if (state.dataHeatBal->CrossMixing(Loop).MinIndoorTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(7))) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(7) +
                                            " not found=" + cAlphaArgs(7) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (state.dataHeatBal->CrossMixing(Loop).MinIndoorTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(
                            state, state.dataHeatBal->CrossMixing(Loop).MinIndoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a minimum zone temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(7));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 7) {
                state.dataHeatBal->CrossMixing(Loop).MaxIndoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(8));
                if (state.dataHeatBal->CrossMixing(Loop).MaxIndoorTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(8))) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(8) +
                                            " not found=\"" + cAlphaArgs(8) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (state.dataHeatBal->CrossMixing(Loop).MaxIndoorTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(
                            state, state.dataHeatBal->CrossMixing(Loop).MaxIndoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a maximum zone temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(8));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 8) {
                state.dataHeatBal->CrossMixing(Loop).MinSourceTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(9));
                if (state.dataHeatBal->CrossMixing(Loop).MinSourceTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(9))) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(9) +
                                            " not found=\"" + cAlphaArgs(9) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (state.dataHeatBal->CrossMixing(Loop).MinSourceTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(
                            state, state.dataHeatBal->CrossMixing(Loop).MinSourceTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a minimum source temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(9));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 9) {
                state.dataHeatBal->CrossMixing(Loop).MaxSourceTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(10));
                if (state.dataHeatBal->CrossMixing(Loop).MaxSourceTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(10))) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(10) +
                                            " not found=\"" + cAlphaArgs(9) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (state.dataHeatBal->CrossMixing(Loop).MaxSourceTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(
                            state, state.dataHeatBal->CrossMixing(Loop).MaxSourceTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a maximum source temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(10));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 10) {
                state.dataHeatBal->CrossMixing(Loop).MinOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(11));
                if (state.dataHeatBal->CrossMixing(Loop).MinOutdoorTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(11))) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(11) +
                                            " not found=\"" + cAlphaArgs(9) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (state.dataHeatBal->CrossMixing(Loop).MinOutdoorTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(
                            state, state.dataHeatBal->CrossMixing(Loop).MinOutdoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a minimum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(11));
                        ErrorsFound = true;
                    }
                }
            }

            if (NumAlpha > 11) {
                state.dataHeatBal->CrossMixing(Loop).MaxOutdoorTempSchedPtr = GetScheduleIndex(state, cAlphaArgs(12));
                if (state.dataHeatBal->CrossMixing(Loop).MaxOutdoorTempSchedPtr == 0) {
                    if ((!lAlphaFieldBlanks(12))) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(12) +
                                            " not found=\"" + cAlphaArgs(9) + "\".");
                        ErrorsFound = true;
                    }
                }
                if (state.dataHeatBal->CrossMixing(Loop).MaxOutdoorTempSchedPtr > 0) {
                    // Check min and max values in the schedule to ensure both values are within the range
                    if (!CheckScheduleValueMinMax(
                            state, state.dataHeatBal->CrossMixing(Loop).MaxOutdoorTempSchedPtr, ">=", -MixingTempLimit, "<=", MixingTempLimit)) {
                        ShowSevereError(
                            state,
                            std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                " must have a maximum outdoor temperature between -100C and 100C defined in the schedule = " + cAlphaArgs(12));
                        ErrorsFound = true;
                    }
                }
            }
        }

        if (state.dataHeatBal->CrossMixing(Loop).ZonePtr > 0) {
            if (RepVarSet(state.dataHeatBal->CrossMixing(Loop).ZonePtr)) {
                RepVarSet(state.dataHeatBal->CrossMixing(Loop).ZonePtr) = false;
                SetupOutputVariable(state,
                                    "Zone Mixing Volume",
                                    OutputProcessor::Unit::m3,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixVolume,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Current Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixVdotCurDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Standard Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixVdotStdDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Mass",
                                    OutputProcessor::Unit::kg,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixMass,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixMdot,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Sensible Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixHeatLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Sensible Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixHeatGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Latent Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixLatentLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Latent Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixLatentGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Total Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixTotalLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Total Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).ZonePtr).MixTotalGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).ZonePtr).Name);
            }
        }
        if (state.dataHeatBal->CrossMixing(Loop).FromZone > 0) {
            if (RepVarSet(state.dataHeatBal->CrossMixing(Loop).FromZone)) {
                RepVarSet(state.dataHeatBal->CrossMixing(Loop).FromZone) = false;
                SetupOutputVariable(state,
                                    "Zone Mixing Volume",
                                    OutputProcessor::Unit::m3,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixVolume,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Current Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixVdotCurDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Standard Density Volume Flow Rate",
                                    OutputProcessor::Unit::m3_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixVdotStdDensity,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Mass",
                                    OutputProcessor::Unit::kg,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixMass,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixMdot,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Sensible Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixHeatLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Sensible Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixHeatGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Latent Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixLatentLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Latent Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixLatentGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Total Heat Loss Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixTotalLoss,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
                SetupOutputVariable(state,
                                    "Zone Mixing Total Heat Gain Energy",
                                    OutputProcessor::Unit::J,
                                    state.dataHeatBal->ZnAirRpt(state.dataHeatBal->CrossMixing(Loop).FromZone).MixTotalGain,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Summed,
                                    state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
            }
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            SetupEMSActuator(state,
                             "ZoneCrossMixing",
                             state.dataHeatBal->CrossMixing(Loop).Name,
                             "Air Exchange Flow Rate",
                             "[m3/s]",
                             state.dataHeatBal->CrossMixing(Loop).EMSSimpleMixingOn,
                             state.dataHeatBal->CrossMixing(Loop).EMSimpleMixingFlowRate);
        }
    }

    cCurrentModuleObject = "ZoneRefrigerationDoorMixing";
    state.dataHeatBal->TotRefDoorMixing = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    if (state.dataHeatBal->TotRefDoorMixing > 0) {
        state.dataHeatBal->RefDoorMixing.allocate(state.dataGlobal->NumOfZones);
        for (auto &e : state.dataHeatBal->RefDoorMixing)
            e.NumRefDoorConnections = 0;

        for (int Loop = 1; Loop <= state.dataHeatBal->TotRefDoorMixing; ++Loop) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     cCurrentModuleObject,
                                                                     Loop,
                                                                     cAlphaArgs,
                                                                     NumAlpha,
                                                                     rNumericArgs,
                                                                     NumNumber,
                                                                     IOStat,
                                                                     lNumericFieldBlanks,
                                                                     lAlphaFieldBlanks,
                                                                     cAlphaFieldNames,
                                                                     cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(state, cAlphaArgs(1), cCurrentModuleObject, ErrorsFound);

            NameThisObject = cAlphaArgs(1);

            AlphaNum = 2;
            Zone1Num = UtilityRoutines::FindItemInList(cAlphaArgs(AlphaNum), state.dataHeatBal->Zone);
            if (Zone1Num == 0) {
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                    cAlphaFieldNames(AlphaNum) + "=\"" + cAlphaArgs(AlphaNum) + "\".");
                ErrorsFound = true;
            }

            ++AlphaNum; // 3
            Zone2Num = UtilityRoutines::FindItemInList(cAlphaArgs(AlphaNum), state.dataHeatBal->Zone);
            if (Zone2Num == 0) {
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                    cAlphaFieldNames(AlphaNum) + "=\"" + cAlphaArgs(AlphaNum) + "\".");
                ErrorsFound = true;
            }
            if (Zone1Num == Zone2Num) {
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                    "\", The same zone name has been entered for both sides of a refrigerated door " + cAlphaFieldNames(AlphaNum) +
                                    "=\"" + cAlphaArgs(AlphaNum) + "\".");
                ErrorsFound = true;
            } else if (Zone1Num < Zone2Num) { // zone 1 will come first in soln loop, id zone 2 as mate zone
                ZoneNumA = Zone1Num;
                ZoneNumB = Zone2Num;
            } else if (Zone2Num < Zone1Num) { // zone 2 will come first in soln loop, id zone 1 as mate zone
                ZoneNumA = Zone2Num;
                ZoneNumB = Zone1Num;
            }

            if (!allocated(state.dataHeatBal->RefDoorMixing(ZoneNumA).OpenSchedPtr)) {
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorMixingObjectName.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).OpenSchedPtr.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorHeight.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorArea.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).Protection.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).MateZonePtr.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorMixingOn.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).VolRefDoorFlowRate.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorProtTypeName.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorMixingObjectName = "";
                state.dataHeatBal->RefDoorMixing(ZoneNumA).OpenSchedPtr = 0;
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorHeight = 0.0;
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorArea = 0.0;
                state.dataHeatBal->RefDoorMixing(ZoneNumA).Protection = RefDoorNone;
                state.dataHeatBal->RefDoorMixing(ZoneNumA).MateZonePtr = 0;
                state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorMixingOn = false;
                state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate = 0.0;
                state.dataHeatBal->RefDoorMixing(ZoneNumA).VolRefDoorFlowRate = 0.0;
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorProtTypeName = "";
            } // First refrigeration mixing in this zone

            if (!allocated(state.dataHeatBal->RefDoorMixing(ZoneNumB).OpenSchedPtr)) {
                state.dataHeatBal->RefDoorMixing(ZoneNumB).DoorMixingObjectName.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).OpenSchedPtr.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).DoorHeight.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).DoorArea.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).Protection.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).MateZonePtr.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).EMSRefDoorMixingOn.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).EMSRefDoorFlowRate.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).VolRefDoorFlowRate.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).DoorProtTypeName.allocate(state.dataGlobal->NumOfZones);
                state.dataHeatBal->RefDoorMixing(ZoneNumB).DoorMixingObjectName = "";
                state.dataHeatBal->RefDoorMixing(ZoneNumB).OpenSchedPtr = 0;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).DoorHeight = 0.0;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).DoorArea = 0.0;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).Protection = RefDoorNone;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).MateZonePtr = 0;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).EMSRefDoorMixingOn = false;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).EMSRefDoorFlowRate = 0.0;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).VolRefDoorFlowRate = 0.0;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).DoorProtTypeName = "";
            } // First refrigeration mixing in this zone

            ConnectionNumber = state.dataHeatBal->RefDoorMixing(ZoneNumA).NumRefDoorConnections + 1;
            state.dataHeatBal->RefDoorMixing(ZoneNumA).NumRefDoorConnections = ConnectionNumber;
            state.dataHeatBal->RefDoorMixing(ZoneNumA).ZonePtr = ZoneNumA;
            state.dataHeatBal->RefDoorMixing(ZoneNumA).MateZonePtr(ConnectionNumber) = ZoneNumB;
            state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorMixingObjectName(ConnectionNumber) = NameThisObject;
            // need to make sure same pair of zones is only entered once.
            if (state.dataHeatBal->RefDoorMixing(ZoneNumA).RefDoorMixFlag && state.dataHeatBal->RefDoorMixing(ZoneNumB).RefDoorMixFlag) {
                if (state.dataHeatBal->RefDoorMixing(ZoneNumA).NumRefDoorConnections > 1) {
                    for (ConnectTest = 1; ConnectTest <= (ConnectionNumber - 1); ++ConnectTest) {
                        if (state.dataHeatBal->RefDoorMixing(ZoneNumA).MateZonePtr(ConnectTest) !=
                            state.dataHeatBal->RefDoorMixing(ZoneNumA).MateZonePtr(ConnectionNumber))
                            continue;
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", and " +
                                            state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorMixingObjectName(ConnectTest));
                        ShowContinueError(state,
                                          " Share same pair of zones: \"" + state.dataHeatBal->Zone(ZoneNumA).Name + "\" and \"" +
                                              state.dataHeatBal->Zone(ZoneNumB).Name +
                                              "\". Only one RefrigerationDoorMixing object is allowed for any unique pair of zones.");
                        ErrorsFound = true;
                    } // ConnectTest
                }     // NumRefDoorconnections > 1
            } else {  // Both zones need to be flagged with ref doors
                state.dataHeatBal->RefDoorMixing(ZoneNumA).RefDoorMixFlag = true;
                state.dataHeatBal->RefDoorMixing(ZoneNumB).RefDoorMixFlag = true;
            } // Both zones already flagged with ref doors

            ++AlphaNum; // 4
            if (lAlphaFieldBlanks(AlphaNum)) {
                ShowSevereError(state,
                                std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(AlphaNum) +
                                    " is required but field is blank.");
                ErrorsFound = true;
            } else { //(lAlphaFieldBlanks(AlphaNum)) THEN
                state.dataHeatBal->RefDoorMixing(ZoneNumA).OpenSchedPtr(ConnectionNumber) = GetScheduleIndex(state, cAlphaArgs(AlphaNum));
                if (state.dataHeatBal->RefDoorMixing(ZoneNumA).OpenSchedPtr(ConnectionNumber) == 0) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\", invalid (not found) " +
                                        cAlphaFieldNames(AlphaNum) + "=\"" + cAlphaArgs(AlphaNum) + "\".");
                    ErrorsFound = true;
                } else { // OpenSchedPtr(ConnectionNumber) ne 0)
                    if (!CheckScheduleValueMinMax(
                            state, state.dataHeatBal->RefDoorMixing(ZoneNumA).OpenSchedPtr(ConnectionNumber), ">=", 0.0, "<=", 1.0)) {
                        ShowSevereError(state,
                                        std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"," + cAlphaFieldNames(AlphaNum) +
                                            "=\"" + cAlphaArgs(AlphaNum) + "\" has schedule values < 0 or > 1.");
                        ErrorsFound = true;
                    } // check door opening schedule values between 0 and 1
                }     // OpenSchedPtr(ConnectionNumber) == 0)
            }         //(lAlphaFieldBlanks(AlphaNum)) THEN

            NumbNum = 1;
            if (lAlphaFieldBlanks(NumbNum)) {
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber) = 3.0; // default height of 3 meters
                ShowWarningError(state,
                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + cNumericFieldNames(NumbNum) +
                                     " is blank and the default value of 3.0 will be used.");
            } else {
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber) = rNumericArgs(NumbNum);
                if ((state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber) < 0) ||
                    (state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber) > 50.0)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                        " must have a door height between 0 and 50 meters. ");
                    ErrorsFound = true;
                }
            }

            ++NumbNum; // 2
            if (lAlphaFieldBlanks(NumbNum)) {
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber) = 9.0; // default area of 9 m2
                ShowWarningError(state,
                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + cNumericFieldNames(NumbNum) +
                                     " is blank and the default value of 9 m2 will be used.");
            } else {
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber) = rNumericArgs(NumbNum);
                if ((state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber) < 0) ||
                    (state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber) > 400.0)) {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + " = " + cAlphaArgs(1) +
                                        " must have a door height between 0 and 400 square meters. ");
                    ErrorsFound = true;
                }
            }

            ++AlphaNum; // 5
            // Door protection type.
            if (lAlphaFieldBlanks(AlphaNum)) {
                state.dataHeatBal->RefDoorMixing(ZoneNumA).Protection(ConnectionNumber) = RefDoorNone;  // Default
                state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber) = "None"; // Default
                ShowWarningError(state,
                                 std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) + "\"  " + cAlphaFieldNames(AlphaNum) +
                                     " is blank. Default of no door protection will be used");
            } else {
                if (cAlphaArgs(AlphaNum) == "NONE") {
                    state.dataHeatBal->RefDoorMixing(ZoneNumA).Protection(ConnectionNumber) = RefDoorNone;
                    state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber) = "None";
                } else if (cAlphaArgs(AlphaNum) == "AIRCURTAIN") {
                    state.dataHeatBal->RefDoorMixing(ZoneNumA).Protection(ConnectionNumber) = RefDoorAirCurtain;
                    state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber) = "AirCurtain";
                } else if (cAlphaArgs(AlphaNum) == "STRIPCURTAIN") {
                    state.dataHeatBal->RefDoorMixing(ZoneNumA).Protection(ConnectionNumber) = RefDoorStripCurtain;
                    state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber) = "StripCurtain";
                } else {
                    ShowSevereError(state,
                                    std::string{RoutineName} + cCurrentModuleObject + "=\"" + cAlphaArgs(1) +
                                        "\", invalid calculation method=" + cAlphaArgs(AlphaNum) + " with alphanum of 5: " + cAlphaArgs(5));
                    ErrorsFound = true;
                } // =none, etc.
            }     // Blank

            if (ZoneNumA > 0) {
                if (RepVarSet(ZoneNumA)) {
                    RepVarSet(ZoneNumA) = false;
                    SetupOutputVariable(state,
                                        "Zone Mixing Volume",
                                        OutputProcessor::Unit::m3,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixVolume,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Current Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixVdotCurDensity,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Standard Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixVdotStdDensity,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass",
                                        OutputProcessor::Unit::kg,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixMass,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixMdot,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixHeatLoss,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixHeatGain,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixLatentLoss,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixLatentGain,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixTotalLoss,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumA).MixTotalGain,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumA).Name);
                }
            }
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "ZoneRefDoorMixing",
                                 state.dataHeatBal->RefDoorMixing(ZoneNumA).Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorMixingOn(ConnectionNumber),
                                 state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate(ConnectionNumber));
            }

            if (ZoneNumB > 0) {
                if (RepVarSet(ZoneNumB)) {
                    RepVarSet(ZoneNumB) = false;
                    SetupOutputVariable(state,
                                        "Zone Mixing Volume",
                                        OutputProcessor::Unit::m3,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixVolume,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Current Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixVdotCurDensity,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Standard Density Volume Flow Rate",
                                        OutputProcessor::Unit::m3_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixVdotStdDensity,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass",
                                        OutputProcessor::Unit::kg,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixMass,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Mass Flow Rate",
                                        OutputProcessor::Unit::kg_s,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixMdot,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Average,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixHeatLoss,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Sensible Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixHeatGain,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixLatentLoss,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Latent Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixLatentGain,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Loss Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixTotalLoss,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                    SetupOutputVariable(state,
                                        "Zone Mixing Total Heat Gain Energy",
                                        OutputProcessor::Unit::J,
                                        state.dataHeatBal->ZnAirRpt(ZoneNumB).MixTotalGain,
                                        OutputProcessor::SOVTimeStepType::System,
                                        OutputProcessor::SOVStoreType::Summed,
                                        state.dataHeatBal->Zone(ZoneNumB).Name);
                }
            }
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "ZoneRefDoorMixing",
                                 state.dataHeatBal->RefDoorMixing(ZoneNumB).Name,
                                 "Air Exchange Flow Rate",
                                 "[m3/s]",
                                 state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorMixingOn(ConnectionNumber),
                                 state.dataHeatBal->RefDoorMixing(ZoneNumA).EMSRefDoorFlowRate(ConnectionNumber));
            }

        } // DO Loop=1,TotRefDoorMixing
    }     // TotRefDoorMixing > 0)

    RepVarSet.deallocate();
    cAlphaArgs.deallocate();
    cAlphaFieldNames.deallocate();
    cNumericFieldNames.deallocate();
    rNumericArgs.deallocate();
    lAlphaFieldBlanks.deallocate();
    lNumericFieldBlanks.deallocate();

    TotInfilVentFlow.dimension(state.dataGlobal->NumOfZones, 0.0);

    auto divide_and_print_if_greater_than_zero = [&](const Real64 denominator, const Real64 numerator) {
        if (denominator > 0.0) {
            print(state.files.eio, "{:.3R},", numerator / denominator);
        } else {
            print(state.files.eio, "N/A,");
        }
    };

    for (int Loop = 1; Loop <= state.dataHeatBal->TotInfiltration; ++Loop) {
        if (Loop == 1)
            print(state.files.eio,
                  Format_721,
                  "ZoneInfiltration",
                  "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/Exterior Surface Area {m3/s-m2},ACH - "
                  "Air Changes per Hour,Equation A - Constant Term Coefficient {},Equation B - Temperature Term Coefficient {1/C},Equation C - "
                  "Velocity Term Coefficient {s/m}, Equation D - Velocity Squared Term Coefficient {s2/m2}");

        int ZoneNum = state.dataHeatBal->Infiltration(Loop).ZonePtr;
        if (ZoneNum == 0) {
            print(state.files.eio, Format_722, "Infiltration-Illegal Zone specified", state.dataHeatBal->Infiltration(Loop).Name);
            continue;
        }
        TotInfilVentFlow(ZoneNum) += state.dataHeatBal->Infiltration(Loop).DesignLevel;
        print(state.files.eio,
              Format_720,
              "ZoneInfiltration",
              state.dataHeatBal->Infiltration(Loop).Name,
              GetScheduleName(state, state.dataHeatBal->Infiltration(Loop).SchedPtr),
              state.dataHeatBal->Zone(ZoneNum).Name,
              state.dataHeatBal->Zone(ZoneNum).FloorArea,
              state.dataHeatBal->Zone(ZoneNum).TotOccupants);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Infiltration(Loop).DesignLevel);

        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).FloorArea, state.dataHeatBal->Infiltration(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).ExteriorTotalSurfArea,
                                              state.dataHeatBal->Infiltration(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).Volume,
                                              state.dataHeatBal->Infiltration(Loop).DesignLevel * DataGlobalConstants::SecInHour);

        print(state.files.eio, "{:.3R},", state.dataHeatBal->Infiltration(Loop).ConstantTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Infiltration(Loop).TemperatureTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Infiltration(Loop).VelocityTermCoef);
        print(state.files.eio, "{:.3R}\n", state.dataHeatBal->Infiltration(Loop).VelocitySQTermCoef);
    }

    if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
        for (int Loop = 1; Loop <= state.dataHeatBal->TotInfiltration; ++Loop) {
            int ZoneNum = state.dataHeatBal->Infiltration(Loop).ZonePtr;
            state.dataHeatBal->MassConservation(ZoneNum).InfiltrationPtr = Loop;
        }
    }

    for (int Loop = 1; Loop <= state.dataHeatBal->TotVentilation; ++Loop) {
        if (Loop == 1) {
            print(state.files.eio,
                  Format_721,
                  "ZoneVentilation",
                  "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                  "Changes per Hour,Fan Type {Exhaust;Intake;Natural},Fan Pressure Rise {Pa},Fan Efficiency {},Equation A - Constant Term "
                  "Coefficient {},Equation B - Temperature Term Coefficient {1/C},Equation C - Velocity Term Coefficient {s/m}, Equation D - "
                  "Velocity Squared Term Coefficient {s2/m2},Minimum Indoor Temperature{C}/Schedule,Maximum Indoor "
                  "Temperature{C}/Schedule,Delta Temperature{C}/Schedule,Minimum Outdoor Temperature{C}/Schedule,Maximum Outdoor "
                  "Temperature{C}/Schedule,Maximum WindSpeed{m/s}");
        }

        int ZoneNum = state.dataHeatBal->Ventilation(Loop).ZonePtr;
        if (ZoneNum == 0) {
            print(state.files.eio, Format_722, "Ventilation-Illegal Zone specified", state.dataHeatBal->Ventilation(Loop).Name);
            continue;
        }
        TotInfilVentFlow(ZoneNum) += state.dataHeatBal->Ventilation(Loop).DesignLevel;
        print(state.files.eio,
              Format_720,
              "ZoneVentilation",
              state.dataHeatBal->Ventilation(Loop).Name,
              GetScheduleName(state, state.dataHeatBal->Ventilation(Loop).SchedPtr),
              state.dataHeatBal->Zone(ZoneNum).Name,
              state.dataHeatBal->Zone(ZoneNum).FloorArea,
              state.dataHeatBal->Zone(ZoneNum).TotOccupants);

        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).DesignLevel);

        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).FloorArea, state.dataHeatBal->Ventilation(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).TotOccupants, state.dataHeatBal->Ventilation(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).Volume,
                                              state.dataHeatBal->Ventilation(Loop).DesignLevel * DataGlobalConstants::SecInHour);

        if (state.dataHeatBal->Ventilation(Loop).FanType == DataHeatBalance::VentilationType::Exhaust) {
            print(state.files.eio, "Exhaust,");
        } else if (state.dataHeatBal->Ventilation(Loop).FanType == DataHeatBalance::VentilationType::Intake) {
            print(state.files.eio, "Intake,");
        } else if (state.dataHeatBal->Ventilation(Loop).FanType == DataHeatBalance::VentilationType::Natural) {
            print(state.files.eio, "Natural,");
        } else if (state.dataHeatBal->Ventilation(Loop).FanType == DataHeatBalance::VentilationType::Balanced) {
            print(state.files.eio, "Balanced,");
        } else {
            print(state.files.eio, "UNKNOWN,");
        }
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).FanPressure);
        print(state.files.eio, "{:.1R},", state.dataHeatBal->Ventilation(Loop).FanEfficiency);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).ConstantTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).TemperatureTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).VelocityTermCoef);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Ventilation(Loop).VelocitySQTermCoef);

        // TODO Should this also be prefixed with "Schedule: " like the following ones are?
        if (state.dataHeatBal->Ventilation(Loop).MinIndoorTempSchedPtr > 0) {
            print(state.files.eio, "{},", GetScheduleName(state, state.dataHeatBal->Ventilation(Loop).MinIndoorTempSchedPtr));
        } else {
            print(state.files.eio, "{:.2R},", state.dataHeatBal->Ventilation(Loop).MinIndoorTemperature);
        }

        const auto print_temperature = [&](const int ptr, const Real64 value) {
            if (ptr > 0) {
                print(state.files.eio, "Schedule: {},", GetScheduleName(state, ptr));
            } else {
                print(state.files.eio, "{:.2R},", value);
            }
        };

        print_temperature(state.dataHeatBal->Ventilation(Loop).MaxIndoorTempSchedPtr, state.dataHeatBal->Ventilation(Loop).MaxIndoorTemperature);
        print_temperature(state.dataHeatBal->Ventilation(Loop).DeltaTempSchedPtr, state.dataHeatBal->Ventilation(Loop).DelTemperature);
        print_temperature(state.dataHeatBal->Ventilation(Loop).MinOutdoorTempSchedPtr, state.dataHeatBal->Ventilation(Loop).MinOutdoorTemperature);
        print_temperature(state.dataHeatBal->Ventilation(Loop).MaxOutdoorTempSchedPtr, state.dataHeatBal->Ventilation(Loop).MaxOutdoorTemperature);

        print(state.files.eio, "{:.2R}\n", state.dataHeatBal->Ventilation(Loop).MaxWindSpeed);
    }

    TotMixingFlow.dimension(state.dataGlobal->NumOfZones, 0.0);
    for (int Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
        if (Loop == 1)
            print(state.files.eio,
                  Format_721,
                  "Mixing",
                  "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                  "Changes per Hour,From/Source Zone,Delta Temperature {C}");

        int ZoneNum = state.dataHeatBal->Mixing(Loop).ZonePtr;
        if (ZoneNum == 0) {
            print(state.files.eio, Format_722, "Mixing-Illegal Zone specified", state.dataHeatBal->Mixing(Loop).Name);
            continue;
        }
        TotMixingFlow(ZoneNum) += state.dataHeatBal->Mixing(Loop).DesignLevel;
        print(state.files.eio,
              Format_720,
              "Mixing",
              state.dataHeatBal->Mixing(Loop).Name,
              GetScheduleName(state, state.dataHeatBal->Mixing(Loop).SchedPtr),
              state.dataHeatBal->Zone(ZoneNum).Name,
              state.dataHeatBal->Zone(ZoneNum).FloorArea,
              state.dataHeatBal->Zone(ZoneNum).TotOccupants);
        print(state.files.eio, "{:.3R},", state.dataHeatBal->Mixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).FloorArea, state.dataHeatBal->Mixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).TotOccupants, state.dataHeatBal->Mixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).Volume,
                                              state.dataHeatBal->Mixing(Loop).DesignLevel * DataGlobalConstants::SecInHour);

        print(state.files.eio, "{},", state.dataHeatBal->Zone(state.dataHeatBal->Mixing(Loop).FromZone).Name);
        print(state.files.eio, "{:.2R}\n", state.dataHeatBal->Mixing(Loop).DeltaTemperature);
    }

    for (int Loop = 1; Loop <= state.dataHeatBal->TotCrossMixing; ++Loop) {
        if (Loop == 1) {
            print(state.files.eio,
                  Format_721,
                  "CrossMixing",
                  "Design Volume Flow Rate {m3/s},Volume Flow Rate/Floor Area {m3/s-m2},Volume Flow Rate/person Area {m3/s-person},ACH - Air "
                  "Changes per Hour,From/Source Zone,Delta Temperature {C}");
        }

        int ZoneNum = state.dataHeatBal->CrossMixing(Loop).ZonePtr;
        if (ZoneNum == 0) {
            print(state.files.eio, Format_722, "CrossMixing-Illegal Zone specified", state.dataHeatBal->CrossMixing(Loop).Name);
            continue;
        }
        TotMixingFlow(ZoneNum) += state.dataHeatBal->CrossMixing(Loop).DesignLevel;
        print(state.files.eio,
              Format_720,
              "CrossMixing",
              state.dataHeatBal->CrossMixing(Loop).Name,
              GetScheduleName(state, state.dataHeatBal->CrossMixing(Loop).SchedPtr),
              state.dataHeatBal->Zone(ZoneNum).Name,
              state.dataHeatBal->Zone(ZoneNum).FloorArea,
              state.dataHeatBal->Zone(ZoneNum).TotOccupants);

        print(state.files.eio, "{:.3R},", state.dataHeatBal->CrossMixing(Loop).DesignLevel);

        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).FloorArea, state.dataHeatBal->CrossMixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).TotOccupants, state.dataHeatBal->CrossMixing(Loop).DesignLevel);
        divide_and_print_if_greater_than_zero(state.dataHeatBal->Zone(ZoneNum).Volume,
                                              state.dataHeatBal->CrossMixing(Loop).DesignLevel * DataGlobalConstants::SecInHour);

        print(state.files.eio, "{},", state.dataHeatBal->Zone(state.dataHeatBal->CrossMixing(Loop).FromZone).Name);
        print(state.files.eio, "{:.2R}\n", state.dataHeatBal->CrossMixing(Loop).DeltaTemperature);
    }

    if (state.dataHeatBal->TotRefDoorMixing > 0) {
        static constexpr std::string_view Format_724("! <{} Airflow Stats Nominal>, {}\n");
        print(state.files.eio,
              Format_724,
              "RefrigerationDoorMixing ",
              "Name, Zone 1 Name,Zone 2 Name,Door Opening Schedule Name,Door Height {m},Door Area {m2},Door Protection Type");
        for (ZoneNumA = 1; ZoneNumA <= (state.dataGlobal->NumOfZones - 1); ++ZoneNumA) {
            if (!state.dataHeatBal->RefDoorMixing(ZoneNumA).RefDoorMixFlag) continue;
            for (ConnectionNumber = 1; ConnectionNumber <= state.dataHeatBal->RefDoorMixing(ZoneNumA).NumRefDoorConnections; ++ConnectionNumber) {
                ZoneNumB = state.dataHeatBal->RefDoorMixing(ZoneNumA).MateZonePtr(ConnectionNumber);
                // TotMixingFlow(ZoneNum)=TotMixingFlow(ZoneNum)+RefDoorMixing(Loop)%!DesignLevel
                static constexpr std::string_view Format_723(" {} Airflow Stats Nominal, {},{},{},{},{:.3R},{:.3R},{}\n");
                print(state.files.eio,
                      Format_723,
                      "RefrigerationDoorMixing",
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorMixingObjectName(ConnectionNumber),
                      state.dataHeatBal->Zone(ZoneNumA).Name,
                      state.dataHeatBal->Zone(ZoneNumB).Name,
                      GetScheduleName(state, state.dataHeatBal->RefDoorMixing(ZoneNumA).OpenSchedPtr(ConnectionNumber)),
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorHeight(ConnectionNumber),
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorArea(ConnectionNumber),
                      state.dataHeatBal->RefDoorMixing(ZoneNumA).DoorProtTypeName(ConnectionNumber));
            } // ConnectionNumber
        }     // ZoneNumA
    }         //(TotRefDoorMixing .GT. 0)

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        state.dataHeatBal->Zone(ZoneNum).NominalInfilVent = TotInfilVentFlow(ZoneNum);
        state.dataHeatBal->Zone(ZoneNum).NominalMixing = TotMixingFlow(ZoneNum);
    }

    if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
        // Check for infiltration in zone which are only a mixing source zone
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            if ((state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing &&
                 state.dataHeatBal->MassConservation(ZoneNum).IsOnlySourceZone) &&
                (state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment != DataHeatBalance::InfiltrationFlow::No)) {
                if (state.dataHeatBal->MassConservation(ZoneNum).InfiltrationPtr == 0) {
                    ShowSevereError(
                        state, std::string{RoutineName} + ": Infiltration object is not defined for zone = " + state.dataHeatBal->Zone(ZoneNum).Name);
                    ShowContinueError(state, "Zone air mass flow balance requires infiltration object for source zones of mixing objects");
                }
            }
        }
        // Set up zone air mass balance output variables
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            SetupOutputVariable(state,
                                "Zone Air Mass Balance Supply Mass Flow Rate",
                                OutputProcessor::Unit::kg_s,
                                state.dataHeatBal->MassConservation(ZoneNum).InMassFlowRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Air Mass Balance Exhaust Mass Flow Rate",
                                OutputProcessor::Unit::kg_s,
                                state.dataHeatBal->MassConservation(ZoneNum).ExhMassFlowRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Air Mass Balance Return Mass Flow Rate",
                                OutputProcessor::Unit::kg_s,
                                state.dataHeatBal->MassConservation(ZoneNum).RetMassFlowRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
            if ((state.dataHeatBal->ZoneAirMassFlow.ZoneFlowAdjustment != DataHeatBalance::AdjustmentType::NoAdjustReturnAndMixing) &&
                ((state.dataHeatBal->MassConservation(ZoneNum).NumSourceZonesMixingObject +
                  state.dataHeatBal->MassConservation(ZoneNum).NumReceivingZonesMixingObject) > 0)) {
                SetupOutputVariable(state,
                                    "Zone Air Mass Balance Mixing Receiving Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataHeatBal->MassConservation(ZoneNum).MixingMassFlowRate,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(ZoneNum).Name);
                SetupOutputVariable(state,
                                    "Zone Air Mass Balance Mixing Source Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    state.dataHeatBal->MassConservation(ZoneNum).MixingSourceMassFlowRate,
                                    OutputProcessor::SOVTimeStepType::System,
                                    OutputProcessor::SOVStoreType::Average,
                                    state.dataHeatBal->Zone(ZoneNum).Name);
            }
            if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationTreatment != DataHeatBalance::InfiltrationFlow::No) {
                if (state.dataHeatBal->ZoneAirMassFlow.InfiltrationForZones == DataHeatBalance::InfiltrationZoneType::AllZones ||
                    (state.dataHeatBal->MassConservation(ZoneNum).NumSourceZonesMixingObject > 0)) {
                    if (state.dataHeatBal->MassConservation(ZoneNum).InfiltrationPtr > 0) {
                        SetupOutputVariable(state,
                                            "Zone Air Mass Balance Infiltration Mass Flow Rate",
                                            OutputProcessor::Unit::kg_s,
                                            state.dataHeatBal->MassConservation(ZoneNum).InfiltrationMassFlowRate,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(ZoneNum).Name);
                        SetupOutputVariable(state,
                                            "Zone Air Mass Balance Infiltration Status",
                                            OutputProcessor::Unit::None,
                                            state.dataHeatBal->MassConservation(ZoneNum).IncludeInfilToZoneMassBal,
                                            OutputProcessor::SOVTimeStepType::System,
                                            OutputProcessor::SOVStoreType::Average,
                                            state.dataHeatBal->Zone(ZoneNum).Name);
                    }
                }
            }
        }
    }

    TotInfilVentFlow.deallocate();
    TotMixingFlow.deallocate();
    //           ' Area per Occupant {m2/person}, Occupant per Area {person/m2}, Interior Lighting {W/m2}, ',  &
    //           'Electric Load {W/m2}, Gas Load {W/m2}, Other Load {W/m2}, Hot Water Eq {W/m2}, Outdoor Controlled Baseboard Heat')
}

void GetRoomAirModelParameters(EnergyPlusData &state, bool &errFlag) // True if errors found during this input routine
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   August 2001
    //       MODIFIED       na
    //       RE-ENGINEERED  April 2003, Weixiu Kong
    //                      December 2003, CC

    // PURPOSE OF THIS SUBROUTINE:
    //     Get room air model parameters for all zones at once

    // Using/Aliasing

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas; // States which alpha value to read from a
    // "Number" line
    int NumNumbers; // Number of numbers encountered
    int Status;     // Notes if there was an error in processing the input
    int AirModelNum;
    int NumOfAirModels;
    int ZoneNum;
    bool ErrorsFound;
    bool IsNotOK;

    // Initialize default values for air model parameters
    state.dataRoomAirMod->AirModel.allocate(state.dataGlobal->NumOfZones);

    ErrorsFound = false;
    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;

    cCurrentModuleObject = "RoomAirModelType";
    NumOfAirModels = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    if (NumOfAirModels > state.dataGlobal->NumOfZones) {
        ShowSevereError(state, "Too many " + cCurrentModuleObject + ".  Cannot exceed the number of Zones.");
        ErrorsFound = true;
    }

    for (AirModelNum = 1; AirModelNum <= NumOfAirModels; ++AirModelNum) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 AirModelNum,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNumbers,
                                                                 Status,
                                                                 _,
                                                                 _,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);
        ZoneNum = UtilityRoutines::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataHeatBal->Zone);
        if (ZoneNum != 0) {
            if (!state.dataRoomAirMod->AirModel(ZoneNum).AirModelName.empty()) {
                ShowSevereError(state, "Invalid " + state.dataIPShortCut->cAlphaFieldNames(2) + " = " + state.dataIPShortCut->cAlphaArgs(2));
                ShowContinueError(state, "Entered in " + cCurrentModuleObject + " = " + state.dataIPShortCut->cAlphaArgs(1));
                ShowContinueError(state, "Duplicate zone name, only one type of roomair model is allowed per zone");
                ShowContinueError(state,
                                  "Zone " + state.dataIPShortCut->cAlphaArgs(2) + " was already assigned a roomair model by " + cCurrentModuleObject +
                                      " = " + state.dataRoomAirMod->AirModel(ZoneNum).AirModelName);
                ShowContinueError(state,
                                  format("Air Model Type for zone already set to {}",
                                         DataRoomAirModel::ChAirModel[static_cast<int>(state.dataRoomAirMod->AirModel(ZoneNum).AirModelType)]));
                ShowContinueError(state, "Trying to overwrite with model type = " + state.dataIPShortCut->cAlphaArgs(3));
                ErrorsFound = true;
            }
            state.dataRoomAirMod->AirModel(ZoneNum).AirModelName = state.dataIPShortCut->cAlphaArgs(1);
            state.dataRoomAirMod->AirModel(ZoneNum).ZoneName = state.dataIPShortCut->cAlphaArgs(2);

            state.dataRoomAirMod->AirModel(ZoneNum).AirModelType =
                static_cast<DataRoomAirModel::RoomAirModel>(getEnumerationValue(roomAirModelNamesUC, state.dataIPShortCut->cAlphaArgs(3)));
            switch (state.dataRoomAirMod->AirModel(ZoneNum).AirModelType) {
            case DataRoomAirModel::RoomAirModel::Mixing:
                // nothing to do here actually
                break;

            case DataRoomAirModel::RoomAirModel::Mundt:
                state.dataRoomAirMod->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAirMod->MundtModelUsed = true;
                IsNotOK = false;
                ValidateComponent(state,
                                  "RoomAirSettings:OneNodeDisplacementVentilation",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, "In " + cCurrentModuleObject + '=' + state.dataIPShortCut->cAlphaArgs(1) + '.');
                    ErrorsFound = true;
                }
                break;

            case DataRoomAirModel::RoomAirModel::UCSDDV:
                state.dataRoomAirMod->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAirMod->UCSDModelUsed = true;
                IsNotOK = false;
                ValidateComponent(state,
                                  "RoomAirSettings:ThreeNodeDisplacementVentilation",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, "In " + cCurrentModuleObject + '=' + state.dataIPShortCut->cAlphaArgs(1) + '.');
                    ErrorsFound = true;
                }
                break;

            case DataRoomAirModel::RoomAirModel::UCSDCV:
                state.dataRoomAirMod->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAirMod->UCSDModelUsed = true;
                IsNotOK = false;
                ValidateComponent(state,
                                  "RoomAirSettings:CrossVentilation",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, "In " + cCurrentModuleObject + '=' + state.dataIPShortCut->cAlphaArgs(1) + '.');
                    ErrorsFound = true;
                }
                break;

            case DataRoomAirModel::RoomAirModel::UCSDUFI:
                state.dataRoomAirMod->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAirMod->UCSDModelUsed = true;
                ValidateComponent(state,
                                  "RoomAirSettings:UnderFloorAirDistributionInterior",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, "In " + cCurrentModuleObject + '=' + state.dataIPShortCut->cAlphaArgs(1) + '.');
                    ErrorsFound = true;
                }
                break;

            case DataRoomAirModel::RoomAirModel::UCSDUFE:
                state.dataRoomAirMod->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAirMod->UCSDModelUsed = true;
                ValidateComponent(state,
                                  "RoomAirSettings:UnderFloorAirDistributionExterior",
                                  "zone_name",
                                  state.dataIPShortCut->cAlphaArgs(2),
                                  IsNotOK,
                                  "GetRoomAirModelParameters");
                if (IsNotOK) {
                    ShowContinueError(state, "In " + cCurrentModuleObject + '=' + state.dataIPShortCut->cAlphaArgs(1) + '.');
                    ErrorsFound = true;
                }
                break;

            case DataRoomAirModel::RoomAirModel::UserDefined:
                state.dataRoomAirMod->AirModel(ZoneNum).SimAirModel = true;
                state.dataRoomAirMod->UserDefinedUsed = true;
                break;

            case DataRoomAirModel::RoomAirModel::AirflowNetwork:
                state.dataRoomAirMod->AirModel(ZoneNum).SimAirModel = true;
                if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "AirflowNetwork:SimulationControl") == 0) {
                    ShowSevereError(state,
                                    "In " + cCurrentModuleObject + " = " + state.dataIPShortCut->cAlphaArgs(1) + ": " +
                                        state.dataIPShortCut->cAlphaFieldNames(3) + " = AIRFLOWNETWORK.");
                    ShowContinueError(state,
                                      "This model requires AirflowNetwork:* objects to form a complete network, including "
                                      "AirflowNetwork:Intrazone:Node and AirflowNetwork:Intrazone:Linkage.");
                    ShowContinueError(state, "AirflowNetwork:SimulationControl not found.");
                    ErrorsFound = true;
                }
                break;

            default:
                ShowWarningError(state, "Invalid " + state.dataIPShortCut->cAlphaFieldNames(3) + " = " + state.dataIPShortCut->cAlphaArgs(3));
                ShowContinueError(state, "Entered in " + cCurrentModuleObject + " = " + state.dataIPShortCut->cAlphaArgs(1));
                ShowContinueError(state, "The mixing air model will be used for Zone =" + state.dataIPShortCut->cAlphaArgs(2));
                state.dataRoomAirMod->AirModel(ZoneNum).AirModelType = DataRoomAirModel::RoomAirModel::Mixing;
            }

            state.dataRoomAirMod->AirModel(ZoneNum).TempCoupleScheme =
                static_cast<DataRoomAirModel::CouplingScheme>(getEnumerationValue(couplingSchemeNamesUC, state.dataIPShortCut->cAlphaArgs(4)));
            if (state.dataRoomAirMod->AirModel(ZoneNum).TempCoupleScheme == DataRoomAirModel::CouplingScheme::Invalid) {
                ShowWarningError(state, "Invalid " + state.dataIPShortCut->cAlphaFieldNames(4) + " = " + state.dataIPShortCut->cAlphaArgs(4));
                ShowContinueError(state, "Entered in " + cCurrentModuleObject + " = " + state.dataIPShortCut->cAlphaArgs(1));
                ShowContinueError(state, "The direct coupling scheme will be used for Zone =" + state.dataIPShortCut->cAlphaArgs(2));
                state.dataRoomAirMod->AirModel(ZoneNum).TempCoupleScheme = DataRoomAirModel::CouplingScheme::Direct;
            }

        } else { // Zone Not Found
            ShowSevereError(state, cCurrentModuleObject + ", Zone not found=" + state.dataIPShortCut->cAlphaArgs(2));
            ShowContinueError(state, "occurs in " + cCurrentModuleObject + '=' + state.dataIPShortCut->cAlphaArgs(1));
            ErrorsFound = true;
        }
    } // AirModel_Param_Loop

    for (ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        // this used to be an if (NumOfAirModels == 0) block, but both the IF and the ELSE had the same content, these two lines:
        state.dataRoomAirMod->AirModel(ZoneNum).AirModelName = "MIXING AIR MODEL FOR " + state.dataHeatBal->Zone(ZoneNum).Name;
        state.dataRoomAirMod->AirModel(ZoneNum).ZoneName = state.dataHeatBal->Zone(ZoneNum).Name;
    }

    // Write RoomAir Model details onto EIO file
    static constexpr std::string_view RoomAirHeader("! <RoomAir Model>, Zone Name, Mixing/Mundt/UCSDDV/UCSDCV/UCSDUFI/UCSDUFE/User Defined\n");
    print(state.files.eio, RoomAirHeader);
    for (ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        {
            static constexpr std::string_view RoomAirZoneFmt("RoomAir Model,{},{}\n");

            switch (state.dataRoomAirMod->AirModel(ZoneNum).AirModelType) {
            case DataRoomAirModel::RoomAirModel::Mixing: {
                print(state.files.eio, RoomAirZoneFmt, state.dataHeatBal->Zone(ZoneNum).Name, "Mixing/Well-Stirred");
            } break;
            case DataRoomAirModel::RoomAirModel::Mundt: {
                print(state.files.eio, RoomAirZoneFmt, state.dataHeatBal->Zone(ZoneNum).Name, "OneNodeDisplacementVentilation");
            } break;
            case DataRoomAirModel::RoomAirModel::UCSDDV: {
                print(state.files.eio, RoomAirZoneFmt, state.dataHeatBal->Zone(ZoneNum).Name, "ThreeNodeDisplacementVentilation");
            } break;
            case DataRoomAirModel::RoomAirModel::UCSDCV: {
                print(state.files.eio, RoomAirZoneFmt, state.dataHeatBal->Zone(ZoneNum).Name, "CrossVentilation");
            } break;
            case DataRoomAirModel::RoomAirModel::UCSDUFI: {
                print(state.files.eio, RoomAirZoneFmt, state.dataHeatBal->Zone(ZoneNum).Name, "UnderFloorAirDistributionInterior");
            } break;
            case DataRoomAirModel::RoomAirModel::UCSDUFE: {
                print(state.files.eio, RoomAirZoneFmt, state.dataHeatBal->Zone(ZoneNum).Name, "UnderFloorAirDistributionExterior");
            } break;
            case DataRoomAirModel::RoomAirModel::UserDefined: {
                print(state.files.eio, RoomAirZoneFmt, state.dataHeatBal->Zone(ZoneNum).Name, "UserDefined");
            } break;
            case DataRoomAirModel::RoomAirModel::AirflowNetwork: {
                print(state.files.eio, RoomAirZoneFmt, state.dataHeatBal->Zone(ZoneNum).Name, "AirflowNetwork");
            } break;
            default:
                break;
            }
        }
    }

    if (ErrorsFound) {
        ShowSevereError(state, "Errors found in processing input for " + cCurrentModuleObject);
        errFlag = true;
    }
}

void InitAirHeatBalance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   February 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for  initializations within the
    // air heat balance.

    // Do the Begin Day initializations
    if (state.dataGlobal->BeginDayFlag) {
    }

    // Do the following initializations (every time step):
    InitSimpleMixingConvectiveHeatGains(state);
}

void InitSimpleMixingConvectiveHeatGains(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998
    //       MODIFIED       March 2003, FCW: allow individual window/door venting control
    //       DATE MODIFIED  April 2000
    //                      May 2009, Brent Griffith added EMS override to mixing and cross mixing flows
    //                      renamed routine and did some cleanup
    //                      August 2011, Therese Stovall added refrigeration door mixing flows
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets up the mixing and cross mixing flows

    using ScheduleManager::GetCurrentScheduleValue;
    using ScheduleManager::GetScheduleIndex;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Loop; // local loop index
    int NZ;   // local index for zone number
    int J;    // local index for second zone in refrig door pair

    int ZoneNum;              // zone counter
    Real64 ZoneMixingFlowSum; // sum of zone mixing flows for a zone
    int NumOfMixingObjects;   // number of mixing objects for a receiving zone

    // Select type of airflow calculation
    if (state.dataHeatBal->AirFlowFlag) { // Simplified airflow calculation
        // Process the scheduled Mixing for air heat balance
        for (Loop = 1; Loop <= state.dataHeatBal->TotMixing; ++Loop) {
            NZ = state.dataHeatBal->Mixing(Loop).ZonePtr;
            state.dataHeatBal->Mixing(Loop).DesiredAirFlowRate =
                state.dataHeatBal->Mixing(Loop).DesignLevel * GetCurrentScheduleValue(state, state.dataHeatBal->Mixing(Loop).SchedPtr);
            if (state.dataHeatBal->Mixing(Loop).EMSSimpleMixingOn)
                state.dataHeatBal->Mixing(Loop).DesiredAirFlowRate = state.dataHeatBal->Mixing(Loop).EMSimpleMixingFlowRate;
            state.dataHeatBal->Mixing(Loop).DesiredAirFlowRateSaved = state.dataHeatBal->Mixing(Loop).DesiredAirFlowRate;
        }

        // if zone air mass flow balance enforced calculate the fraction of
        // contribution of each mixing object to a zone mixed flow rate, BAN Feb 2014
        if (state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
            for (ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                ZoneMixingFlowSum = 0.0;
                NumOfMixingObjects = state.dataHeatBal->MassConservation(ZoneNum).NumReceivingZonesMixingObject;
                for (Loop = 1; Loop <= NumOfMixingObjects; ++Loop) {
                    ZoneMixingFlowSum = ZoneMixingFlowSum + state.dataHeatBal->Mixing(Loop).DesignLevel;
                }
                if (ZoneMixingFlowSum > 0.0) {
                    for (Loop = 1; Loop <= NumOfMixingObjects; ++Loop) {
                        state.dataHeatBal->MassConservation(ZoneNum).ZoneMixingReceivingFr(Loop) =
                            state.dataHeatBal->Mixing(Loop).DesignLevel / ZoneMixingFlowSum;
                    }
                }
            }
        }

        // Process the scheduled CrossMixing for air heat balance
        for (Loop = 1; Loop <= state.dataHeatBal->TotCrossMixing; ++Loop) {
            NZ = state.dataHeatBal->CrossMixing(Loop).ZonePtr;
            state.dataHeatBal->CrossMixing(Loop).DesiredAirFlowRate =
                state.dataHeatBal->CrossMixing(Loop).DesignLevel * GetCurrentScheduleValue(state, state.dataHeatBal->CrossMixing(Loop).SchedPtr);
            if (state.dataHeatBal->CrossMixing(Loop).EMSSimpleMixingOn)
                state.dataHeatBal->CrossMixing(Loop).DesiredAirFlowRate = state.dataHeatBal->CrossMixing(Loop).EMSimpleMixingFlowRate;
        }

        // Note - do each Pair a Single time, so must do increment reports for both zones
        //       Can't have a pair that has ZoneA zone number = NumOfZones because organized
        //       in input with lowest zone # first no matter how input in idf

        // Process the scheduled Refrigeration Door mixing for air heat balance
        if (state.dataHeatBal->TotRefDoorMixing > 0) {
            for (NZ = 1; NZ <= (state.dataGlobal->NumOfZones - 1);
                 ++NZ) { // Can't have %ZonePtr==NumOfZones because lesser zone # of pair placed in ZonePtr in input
                if (!state.dataHeatBal->RefDoorMixing(NZ).RefDoorMixFlag) continue;
                if (state.dataHeatBal->RefDoorMixing(NZ).ZonePtr == NZ) {
                    for (J = 1; J <= state.dataHeatBal->RefDoorMixing(NZ).NumRefDoorConnections; ++J) {
                        state.dataHeatBal->RefDoorMixing(NZ).VolRefDoorFlowRate(J) = 0.0;
                        if (state.dataHeatBal->RefDoorMixing(NZ).EMSRefDoorMixingOn(J))
                            state.dataHeatBal->RefDoorMixing(NZ).VolRefDoorFlowRate(J) = state.dataHeatBal->RefDoorMixing(NZ).EMSRefDoorFlowRate(J);
                    }
                }
            }
        } // TotRefDoorMixing

        // Infiltration and ventilation calculations have been moved to a subroutine of CalcAirFlowSimple in HVAC Manager
    }
}

void CalcHeatBalanceAir(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN   na
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the air component of the heat balance.

    // Using/Aliasing
    using HVACManager::ManageHVAC;

    if (state.dataGlobal->externalHVACManager) {
        if (!state.dataGlobal->externalHVACManagerInitialized) {
            initializeForExternalHVACManager(state);
        }
        state.dataGlobal->externalHVACManager(&state);
    } else {
        ManageHVAC(state);
    }

    // Do Final Temperature Calculations for Heat Balance before next Time step
    state.dataHeatBalFanSys->SumHmAW = 0.0;
    state.dataHeatBalFanSys->SumHmARa = 0.0;
    state.dataHeatBalFanSys->SumHmARaW = 0.0;
}

// END Algorithm Section of the Module

void initializeForExternalHVACManager(EnergyPlusData &state)
{
    // this function will ultimately provide a nice series of calls that initialize all the hvac stuff needed
    // to allow an external hvac manager to play nice with E+
    EnergyPlus::ZoneTempPredictorCorrector::InitZoneAirSetPoints(state);
    if (!state.dataZoneEquip->ZoneEquipInputsFilled) {
        EnergyPlus::DataZoneEquipment::GetZoneEquipmentData(state);
        state.dataZoneEquip->ZoneEquipInputsFilled = true;
    }
}

void ReportZoneMeanAirTemp(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   July 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates the report variables for the AirHeatBalance.

    // Using/Aliasing
    using Psychrometrics::PsyTdpFnWPb;
    using ScheduleManager::GetCurrentScheduleValue;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int ZoneLoop;             // Counter for the # of zones (nz)
    int TempControlledZoneID; // index for zone in TempConrolled Zone structure
    Real64 thisMRTFraction;   // temp working value for radiative fraction/weight

    for (ZoneLoop = 1; ZoneLoop <= state.dataGlobal->NumOfZones; ++ZoneLoop) {
        // The mean air temperature is actually ZTAV which is the average
        // temperature of the air temperatures at the system time step for the
        // entire zone time step.
        state.dataHeatBal->ZnAirRpt(ZoneLoop).MeanAirTemp = state.dataHeatBalFanSys->ZTAV(ZoneLoop);
        state.dataHeatBal->ZnAirRpt(ZoneLoop).MeanAirHumRat = state.dataHeatBalFanSys->ZoneAirHumRatAvg(ZoneLoop);
        state.dataHeatBal->ZnAirRpt(ZoneLoop).OperativeTemp = 0.5 * (state.dataHeatBalFanSys->ZTAV(ZoneLoop) + state.dataHeatBal->ZoneMRT(ZoneLoop));
        state.dataHeatBal->ZnAirRpt(ZoneLoop).MeanAirDewPointTemp =
            PsyTdpFnWPb(state, state.dataHeatBal->ZnAirRpt(ZoneLoop).MeanAirHumRat, state.dataEnvrn->OutBaroPress);

        // if operative temperature control is being used, then radiative fraction/weighting
        //  might be defined by user to be something different than 0.5, even scheduled over simulation period
        if (state.dataZoneCtrls->AnyOpTempControl) { // dig further...
            // find TempControlledZoneID from ZoneLoop index
            TempControlledZoneID = state.dataHeatBal->Zone(ZoneLoop).TempControlledZoneIndex;
            if (state.dataHeatBal->Zone(ZoneLoop).IsControlled) {
                if ((state.dataZoneCtrls->TempControlledZone(TempControlledZoneID).OperativeTempControl)) {
                    // is operative temp radiative fraction scheduled or fixed?
                    if (state.dataZoneCtrls->TempControlledZone(TempControlledZoneID).OpTempCntrlModeScheduled) {
                        thisMRTFraction = GetCurrentScheduleValue(
                            state, state.dataZoneCtrls->TempControlledZone(TempControlledZoneID).OpTempRadiativeFractionSched);
                    } else {
                        thisMRTFraction = state.dataZoneCtrls->TempControlledZone(TempControlledZoneID).FixedRadiativeFraction;
                    }
                    state.dataHeatBal->ZnAirRpt(ZoneLoop).ThermOperativeTemp =
                        (1.0 - thisMRTFraction) * state.dataHeatBalFanSys->ZTAV(ZoneLoop) + thisMRTFraction * state.dataHeatBal->ZoneMRT(ZoneLoop);
                }
            }
        }
    }
}

} // namespace EnergyPlus::HeatBalanceAirManager
