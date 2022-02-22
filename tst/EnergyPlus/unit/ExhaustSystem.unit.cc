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

// EnergyPlus::Heat Recovery Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "EnergyPlus/DataAirLoop.hh"
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/IOFiles.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
// #include <EnergyPlus/ReturnAirPathManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SimAirServingZones.hh>
#include <EnergyPlus/SimulationManager.hh>

#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/ExhaustAirSystemManager.hh>
#include <EnergyPlus/HVACFan.hh>
#include <EnergyPlus/MixerComponent.hh>

using namespace EnergyPlus;
using namespace DataEnvironment;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataHVACGlobals;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataAirSystems;
using namespace EnergyPlus::Fans;
using namespace EnergyPlus::HeatRecovery;
using namespace EnergyPlus::OutputProcessor;
using namespace EnergyPlus::Psychrometrics;
using namespace EnergyPlus::SimAirServingZones;
// using namespace EnergyPlus::ReturnAirPathManager;
using namespace EnergyPlus::SimulationManager;

TEST_F(EnergyPlusFixture, ExhaustSystemInputTest)
{
    std::string const idf_objects = delimited_string({
        "! Zone1,",
        "! Zone2,",
        "! Zone3,",
        "! Zone4,",

        "AirLoopHVAC:ZoneMixer,",
        "    Mixer1,   !-Name",
        "    Central_ExhFan_1_Inlet,     !-Outlet Node Name",
        "    Zone1 Exhaust Outlet Node,  !-Inlet 1 Node Name",
        "    Zone4 Exhaust Outlet Node;  !-Inlet 2 Node Name",

        "AirLoopHVAC:ZoneMixer,",
        "    Mixer2, !-Name",
        "    Central_ExhFan_2_Inlet,    !-Outlet Node Name",
        "    Zone2 Exhaust Outlet Node, !-Inlet 1 Node Name",
        "    Zone3 Exhaust Outlet Node; !-Inlet 2 Node Name",

        "Fan:SystemModel,",
        "    CentralExhaustFan1,      !- Name",
        "    Omni_Sched,              !- Availability Schedule Name",
        "    Central_ExhFan_1_Inlet,  !- Air Inlet Node Name",
        "    Central_ExhFan_1_Outlet, !- Air Outlet Node Name",
        "    1,                       !- Design Maximum Air Flow Rate {m3/s}",
        "    Discrete,                !- Speed Control Method",
        "    0.2,                     !- Electric Power Minimum Flow Rate Fraction",
        "    10,                      !- Design Pressure Rise {Pa}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Air Stream Fraction",
        "    autosize,                !- Design Electric Power Consumption {W}",
        "    PowerPerFlowPerPressure, !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    1.66667,                 !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.7,                     !- Fan Total Efficiency",
        "    ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "    ,                        !- Night Ventilation Mode Pressure Rise {Pa}",
        "    ,                        !- Night Ventilation Mode Flow Fraction",
        "    ,                        !- Motor Loss Zone Name",
        "    ,                        !- Motor Loss Radiative Fraction",
        "    General,                 !- End-Use Subcategory",
        "    1;                       !- Number of Speeds",

        "Fan:SystemModel,",
        "    CentralExhaustFan2,      !- Name",
        "    Omni_Sched,              !- Availability Schedule Name",
        "    Central_ExhFan_2_Inlet,  !- Air Inlet Node Name",
        "    Central_ExhFan_2_Outlet, !- Air Outlet Node Name",
        "    1,                       !- Design Maximum Air Flow Rate {m3/s}",
        "    Discrete,                !- Speed Control Method",
        "    0.2,                     !- Electric Power Minimum Flow Rate Fraction",
        "    15,                      !- Design Pressure Rise {Pa}",
        "    0.9,                     !- Motor Efficiency",
        "    1,                       !- Motor In Air Stream Fraction",
        "    autosize,                !- Design Electric Power Consumption {W}",
        "    PowerPerFlowPerPressure, !- Design Power Sizing Method",
        "    ,                        !- Electric Power Per Unit Flow Rate {W/(m3/s)}",
        "    1.66667,                 !- Electric Power Per Unit Flow Rate Per Unit Pressure {W/((m3/s)-Pa)}",
        "    0.7,                     !- Fan Total Efficiency",
        "    ,                        !- Electric Power Function of Flow Fraction Curve Name",
        "    ,                        !- Night Ventilation Mode Pressure Rise {Pa}",
        "    ,                        !- Night Ventilation Mode Flow Fraction",
        "    ,                        !- Motor Loss Zone Name",
        "    ,                        !- Motor Loss Radiative Fraction",
        "    General,                 !- End-Use Subcategory",
        "    1;                       !- Number of Speeds",

        "AirLoopHVAC:ExhaustSystem,",
        "    Central Exhaust 1,     !-Name",
        "    Mixer1,                !-AirLoopHVAC:ZoneMixer Name",
        "    Fan:SystemModel,       !-Fan Object Type",
        "    CentralExhaustFan1;    !-Fan Name",

        "AirLoopHVAC:ExhaustSystem,",
        "    Central Exhaust 2,     !-Name",
        "    Mixer2,                !-AirLoopHVAC:ZoneMixer Name",
        "    Fan:SystemModel,       !-Fan Object Type",
        "    CentralExhaustFan2;    !-Fan Name",

        "ZoneHVAC:ExhaustControl,",
        "    Zone1 Exhaust Control,              !-Name",
        "    HVACOperationSchd,                  !- Availability Schedule Name",
        "    Zone1,                              !- Zone Name",
        "    Zone1 Exhaust Node,                 !- Inlet Node Name",
        "    Zone1 Exhaust Oulet Node,           !- Outlet Node Name",
        "    0.1,                                !- Design Flow Rate {m3/s}",
        "    Scheduled,                          !- Flow Control Type (Scheduled, or FollowSupply)",
        "    Zone1Exh Exhaust Flow Frac Sched,   !- Flow Fraction Schedule Name",
        "    ,                                   !- Supply Node or NodeList Name (used with FollowSupply control type)",
        "    ,                                   !- Minimum Zone Temperature Limit Schedule Name",
        "    Zone1Exh Min Exhaust Flow Frac Sched,   !- Minimum Flow Fraction Schedule Name",
        "    Zone1Exh FlowBalancedSched;         !-Balanced Exhaust Fraction Schedule Name",

        "ZoneHVAC:ExhaustControl,",
        "    Zone2 Exhaust Control,              !-Name",
        "    HVACOperationSchd,                  !- Availability Schedule Name",
        "    Zone2,                              !- Zone Name",
        "    Zone2 Exhaust Node,                 !- Inlet Node Name",
        "    Zone2 Exhaust Outlet Node,          !- Outlet Node Name",
        "    autosize,                                !- Design Flow Rate {m3/s}",
        // "    FollowSupply,                       !- Flow Control Type (Scheduled, or FollowSupply)",
        "    Scheduled,",
        "    ,                                   !- Flow Fraction Schedule Name",
        // "    Zone2Exh_SupplyNodeList,           !- Supply Node or NodeList Name (used with FollowSupply control type)",
        "    ,",
        "    ,                                   !- Minimum Zone Temperature Limit Schedule Name",
        "    Zone2Exh Min Exhaust Flow Frac Sched,   !- Minimum Flow Fraction Schedule Name",
        "    Zone2Exh FlowBalancedSched;         !-Balanced Exhaust Fraction Schedule Name",

        "ZoneHVAC:ExhaustControl,",
        "    Zone3 Exhaust Control,              !-Name",
        "    HVACOperationSchd,                  !- Availability Schedule Name",
        "    Zone3,                              !- Zone Name",
        "    Zone3 Exhaust Node,                 !- Inlet Node Name",
        "    Zone3 Exhaust Outlet Node,          !- Outlet Node Name",
        "    0.3,                                !- Design Flow Rate {m3/s}",
        "    Scheduled,                          !- Flow Control Type (Scheduled, or FollowSupply)",
        "    Zone3Exh Exhaust Flow Frac Sched,   !- Flow Fraction Schedule Name",
        "    ,                                   !- Supply Node or NodeList Name (used with FollowSupply control type)",
        "    ,                                   !- Minimum Zone Temperature Limit Schedule Name",
        "    Zone3Exh Min Exhaust Flow Frac Sched,   !- Minimum Flow Fraction Schedule Name",
        "    Zone3Exh FlowBalancedSched;         !-Balanced Exhaust Fraction Schedule Name",

        "ZoneHVAC:ExhaustControl,",
        "    Zone4 Exhaust Control,              !-Name",
        "    HVACOperationSchd,                  !- Availability Schedule Name",
        "    Zone4,                              !- Zone Name",
        "    Zone4 Exhaust Node,                 !- Inlet Node Name",
        "    Zone4 Exhaust Outlet Node,          !- Outlet Node Name",
        "    0.4,                                !- Design Flow Rate {m3/s}",
        // "! may consider an autosize here,",
        "    Scheduled,                          !- Flow Control Type (Scheduled, or FollowSupply)",
        "    Zone4Exh Exhaust Flow Frac Sched,   !- Flow Fraction Schedule Name",
        "    ,                                   !- Supply Node or NodeList Name (used with FollowSupply control type)",
        "    Zone4_MinZoneTempLimitSched,        !- Minimum Zone Temperature Limit Schedule Name",
        "    Zone4Exh Min Exhaust Flow Frac Sched,   !- Minimum Flow Fraction Schedule Name",
        "    Zone4Exh FlowBalancedSched;         !-Balanced Exhaust Fraction Schedule Name",

        "Schedule:Compact,",
        "    Omni_Sched,              !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "Schedule:Compact,",
        "    HVACOperationSchd,       !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "Schedule:Compact,",
        "    Zone1Exh Exhaust Flow Frac Sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "Schedule:Compact,",
        "    Zone1_MinZoneTempLimitSched,             !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 20;        !- Field 3",

        "Schedule:Compact,",
        "    Zone1Exh Min Exhaust Flow Frac Sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.2;       !- Field 3",

        "Schedule:Compact,",
        "    Zone1Exh_FlowBalancedSched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.2;       !- Field 3",

        "Schedule:Compact,",
        "    Zone2Exh Exhaust Flow Frac Sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "Schedule:Compact,",
        "    Zone2_MinZoneTempLimitSched,             !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 20;        !- Field 3",

        "Schedule:Compact,",
        "    Zone2Exh Min Exhaust Flow Frac Sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.2;       !- Field 3",

        "Schedule:Compact,",
        "    Zone2Exh_FlowBalancedSched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.2;       !- Field 3",

        "Schedule:Compact,",
        "    Zone3Exh Exhaust Flow Frac Sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "Schedule:Compact,",
        "    Zone3_MinZoneTempLimitSched,             !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 20;        !- Field 3",

        "Schedule:Compact,",
        "    Zone3Exh Min Exhaust Flow Frac Sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.2;       !- Field 3",

        "Schedule:Compact,",
        "    Zone3Exh_FlowBalancedSched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.2;       !- Field 3",

        "Schedule:Compact,",
        "    Zone4Exh Exhaust Flow Frac Sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00,1.0;        !- Field 3",

        "Schedule:Compact,",
        "    Zone4_MinZoneTempLimitSched,             !- Name",
        "    ,                        !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 20;        !- Field 3",

        "Schedule:Compact,",
        "    Zone4Exh Min Exhaust Flow Frac Sched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.2;       !- Field 3",

        "Schedule:Compact,",
        "    Zone4Exh_FlowBalancedSched,             !- Name",
        "    Fraction,                !- Schedule Type Limits Name",
        "    Through: 12/31,          !- Field 1",
        "    For: AllDays,            !- Field 2",
        "    Until: 24:00, 0.2;       !- Field 3",

        "ScheduleTypeLimits,",
        "    Fraction,                !- Name",
        "    0.0,                     !- Lower Limit Value",
        "    1.0,                     !- Upper Limit Value",
        "    CONTINUOUS;              !- Numeric Type",
    });

    // Preset some elements
    state->dataHeatBal->Zone.allocate(4);
    state->dataHeatBal->Zone(1).Name = "ZONE1";
    state->dataHeatBal->Zone(2).Name = "ZONE2";
    state->dataHeatBal->Zone(3).Name = "ZONE3";
    state->dataHeatBal->Zone(4).Name = "ZONE4";

    state->dataSize->FinalZoneSizing.allocate(4);
    state->dataSize->FinalZoneSizing(2).MinOA = 0.25;

    // state->dataMixerComponent->MixerCond.allocate(2);
    // state->dataMixerComponent->MixerCond(1).MixerName = "MIXER1";
    // state->dataMixerComponent->MixerCond(2).MixerName = "MIXER2";
    // state->dataHVACFan->fanObjs.emplace_back(new HVACFan::FanSystem(*state, "CentralExhaustFan1"));
    // state->dataHVACFan->fanObjs.emplace_back(new HVACFan::FanSystem(*state, "CentralExhaustFan2"));

    ASSERT_TRUE(process_idf(idf_objects));
    ScheduleManager::ProcessScheduleInput(*state);

    // Call the processing codes
    ExhaustAirSystemManager::GetZoneExhaustControlInput(*state);

    ExhaustAirSystemManager::GetExhaustAirSystemInput(*state);

    // Expected input values:
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(1).ZoneName, "ZONE1");
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(2).ZoneName, "ZONE2");
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(3).ZoneName, "ZONE3");
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(4).ZoneName, "ZONE4");

    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(1).ZoneNum, 1);
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(2).ZoneNum, 2);
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(3).ZoneNum, 3);
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(4).ZoneNum, 4);

    // Expected input value:
    EXPECT_NEAR(state->dataZoneEquip->ZoneExhaustControlSystem(1).DesignExhaustFlowRate, 0.1, 1e-5);

    // autosized value
    EXPECT_NEAR(state->dataZoneEquip->ZoneExhaustControlSystem(2).DesignExhaustFlowRate, 0.25, 1e-5);

    // Expected input values:
    EXPECT_NEAR(state->dataZoneEquip->ZoneExhaustControlSystem(3).DesignExhaustFlowRate, 0.3, 1e-5);
    EXPECT_NEAR(state->dataZoneEquip->ZoneExhaustControlSystem(4).DesignExhaustFlowRate, 0.4, 1e-5);

    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(1).FlowControlTypeNum, 0);
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(2).FlowControlTypeNum, 0);
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(3).FlowControlTypeNum, 0);
    EXPECT_EQ(state->dataZoneEquip->ZoneExhaustControlSystem(4).FlowControlTypeNum, 0);

    EXPECT_EQ(state->dataZoneEquip->NumExhaustAirSystems, 2);
    EXPECT_EQ(state->dataZoneEquip->ExhaustAirSystem(1).Name, "CENTRAL EXHAUST 1");
    EXPECT_EQ(state->dataZoneEquip->ExhaustAirSystem(2).Name, "CENTRAL EXHAUST 2");

    EXPECT_EQ(state->dataZoneEquip->ExhaustAirSystem(1).ZoneMixerName, "MIXER1");
    EXPECT_EQ(state->dataZoneEquip->ExhaustAirSystem(2).ZoneMixerName, "MIXER2");

    EXPECT_EQ(state->dataZoneEquip->ExhaustAirSystem(1).ZoneMixerIndex, 1);
    EXPECT_EQ(state->dataZoneEquip->ExhaustAirSystem(2).ZoneMixerIndex, 2);

    EXPECT_EQ(state->dataZoneEquip->ExhaustAirSystem(1).CentralFanName, "CENTRALEXHAUSTFAN1");
    EXPECT_EQ(state->dataZoneEquip->ExhaustAirSystem(2).CentralFanName, "CENTRALEXHAUSTFAN2");
}
