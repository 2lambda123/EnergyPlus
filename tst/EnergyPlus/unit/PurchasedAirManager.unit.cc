// EnergyPlus, Copyright (c) 1996-2021, The Board of Trustees of the University of Illinois,
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

// EnergyPlus::PurchasedAirManager (Ideal Loads) Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include "Fixtures/EnergyPlusFixture.hh"
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/PurchasedAirManager.hh>
#include <EnergyPlus/RuntimeLanguageProcessor.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZonePlenum.hh>

using namespace EnergyPlus;
using namespace ObjexxFCL;
using namespace EnergyPlus::DataHeatBalance;
using namespace EnergyPlus::DataHeatBalFanSys;
using namespace EnergyPlus::DataHVACGlobals;
using namespace EnergyPlus::DataLoopNode;
using namespace EnergyPlus::DataSizing;
using namespace EnergyPlus::DataSurfaces;
using namespace EnergyPlus::DataZoneEnergyDemands;
using namespace EnergyPlus::DataZoneEquipment;
using namespace EnergyPlus::HeatBalanceManager;
using namespace EnergyPlus::PurchasedAirManager;
using namespace EnergyPlus::ZoneEquipmentManager;
using namespace EnergyPlus::ZonePlenum;

class ZoneIdealLoadsTest : public EnergyPlusFixture
{
public:
    int IdealLoadsSysNum = 1;
    int NumNodes = 1; // number of zone inlet and zone exhaust nodes
    bool ErrorsFound = false;

protected:
    virtual void SetUp()
    {
        EnergyPlusFixture::SetUp(); // Sets up the base fixture first.

        DataHeatBalFanSys::ZoneThermostatSetPointHi.allocate(1);
        DataHeatBalFanSys::ZoneThermostatSetPointHi(1) = 23.9; // 75F
        DataHeatBalFanSys::ZoneThermostatSetPointLo.allocate(1);
        DataHeatBalFanSys::ZoneThermostatSetPointLo(1) = 23.0; // 73.4F

        FinalZoneSizing.allocate(1);
        ZoneEqSizing.allocate(1);
        CurZoneEqNum = 1;
        CurSysNum = 0;
        ZoneEqSizing(CurZoneEqNum).SizingMethod.allocate(25);
        ZoneSizingRunDone = true;

        state->dataZoneEnergyDemand->ZoneSysEnergyDemand.allocate(1);
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).TotalOutputRequired = 1000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToHeatingSP = 1000.0;
        state->dataZoneEnergyDemand->ZoneSysEnergyDemand(1).OutputRequiredToCoolingSP = 2000.0;
        state->dataZoneEnergyDemand->ZoneSysMoistureDemand.allocate(1);
        NonAirSystemResponse.allocate(1);
        SysDepZoneLoads.allocate(1);
        MassConservation.allocate(1);
        ZoneIntGain.allocate(1);
        SurfaceWindow.allocate(1);
        RefrigCaseCredit.allocate(1);
        ZoneLatentGain.allocate(1);

        TempControlType.allocate(1);
        TempControlType(1) = DataHVACGlobals::SingleHeatingSetPoint;
        state->dataZoneEnergyDemand->CurDeadBandOrSetback.allocate(1);
        state->dataZoneEnergyDemand->DeadBandOrSetback.allocate(1);
        state->dataZoneEnergyDemand->DeadBandOrSetback(1) = false;

        ZoneAirHumRat.allocate(1);
        ZoneAirHumRat(1) = 0.07;

        state->dataZoneEquip->ZoneEquipInputsFilled = false;
    }

    virtual void TearDown()
    {
        EnergyPlusFixture::TearDown(); // Remember to tear down the base fixture after cleaning up derived fixture!
    }
};

TEST_F(EnergyPlusFixture, SizePurchasedAirTest_Test1)
{

    int PurchAirNum = 1;
    ZoneEqSizing.allocate(1);
    CurZoneEqNum = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero in Sizer
    ZoneEqSizing(CurZoneEqNum).SizingMethod.allocate(24);
    CurSysNum = 0;

    FinalZoneSizing.allocate(1);
    FinalZoneSizing(CurZoneEqNum).MinOA = 0.0;
    FinalZoneSizing(CurZoneEqNum).OutTempAtHeatPeak = 5.0;
    FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow = 1.0;
    FinalZoneSizing(CurZoneEqNum).DesHeatCoilInTemp = 30.0;
    FinalZoneSizing(CurZoneEqNum).ZoneTempAtHeatPeak = 30.0;
    FinalZoneSizing(CurZoneEqNum).HeatDesTemp = 80.0;
    FinalZoneSizing(CurZoneEqNum).HeatDesHumRat = 0.008;
    FinalZoneSizing(CurZoneEqNum).DesHeatMassFlow = FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow * state->dataEnvrn->StdRhoAir;

    FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow = 2.0;
    FinalZoneSizing(CurZoneEqNum).DesCoolCoilInTemp = 60.0;
    FinalZoneSizing(CurZoneEqNum).OutTempAtCoolPeak = 70.0;
    FinalZoneSizing(CurZoneEqNum).CoolDesTemp = 50.0;
    FinalZoneSizing(CurZoneEqNum).CoolDesHumRat = 0.008;
    FinalZoneSizing(CurZoneEqNum).DesCoolCoilInHumRat = 0.010;
    FinalZoneSizing(CurZoneEqNum).DesCoolMassFlow = FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow * state->dataEnvrn->StdRhoAir;

    state->dataPurchasedAirMgr->PurchAir.allocate(10);
    state->dataPurchasedAirMgr->PurchAirNumericFields.allocate(10);
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames.allocate(8);
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(5) = "Maximum Heating Air Flow Rate";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(6) = "Maximum Sensible Heating Capacity";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(7) = "Maximum Cooling Air Flow Rate";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(8) = "Maximum Total Cooling Capacity";

    ZoneSizingRunDone = true;

    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).HeatingLimit = LimitType::LimitFlowRateAndCapacity;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatVolFlowRate = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatSensCap = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).CoolingLimit = LimitType::LimitFlowRateAndCapacity;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolVolFlowRate = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolTotCap = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).cObjectName = "ZONEHVAC:IDEALLOADSAIRSYSTEM";
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).Name = "Ideal Loads 1";

    SizePurchasedAir(*state, PurchAirNum);
    EXPECT_DOUBLE_EQ(1.0, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatVolFlowRate);
    EXPECT_NEAR(50985.58, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatSensCap, 0.1);
    EXPECT_DOUBLE_EQ(2.0, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolVolFlowRate);
    EXPECT_NEAR(30844.14, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolTotCap, 0.1);
}

TEST_F(EnergyPlusFixture, SizePurchasedAirTest_Test2)
{

    int PurchAirNum = 1;
    ZoneEqSizing.allocate(1);
    CurZoneEqNum = 1;
    state->dataEnvrn->StdRhoAir = 1.0; // Prevent divide by zero in Sizer
    ZoneEqSizing(CurZoneEqNum).SizingMethod.allocate(24);
    CurSysNum = 0;

    FinalZoneSizing.allocate(1);
    FinalZoneSizing(CurZoneEqNum).MinOA = 0.5;
    FinalZoneSizing(CurZoneEqNum).OutTempAtHeatPeak = 5.0;
    FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow = 1.0;
    FinalZoneSizing(CurZoneEqNum).DesHeatCoilInTemp = 30.0; // this isn't used so don't change it
    FinalZoneSizing(CurZoneEqNum).ZoneTempAtHeatPeak = 30.0;
    FinalZoneSizing(CurZoneEqNum).HeatDesTemp = 80.0;
    FinalZoneSizing(CurZoneEqNum).HeatDesHumRat = 0.008;
    FinalZoneSizing(CurZoneEqNum).DesHeatMassFlow = FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow * state->dataEnvrn->StdRhoAir;

    FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow = 2.0;
    FinalZoneSizing(CurZoneEqNum).DesCoolCoilInTemp = 65.0; // this is used, so make it higher
    FinalZoneSizing(CurZoneEqNum).OutTempAtCoolPeak = 70.0; // this is not currently used for cooling
    FinalZoneSizing(CurZoneEqNum).CoolDesTemp = 50.0;
    FinalZoneSizing(CurZoneEqNum).CoolDesHumRat = 0.008;
    FinalZoneSizing(CurZoneEqNum).DesCoolCoilInHumRat = 0.010;
    FinalZoneSizing(CurZoneEqNum).DesCoolMassFlow = FinalZoneSizing(CurZoneEqNum).DesCoolVolFlow * state->dataEnvrn->StdRhoAir;

    state->dataPurchasedAirMgr->PurchAir.allocate(10);
    state->dataPurchasedAirMgr->PurchAirNumericFields.allocate(10);
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames.allocate(8);
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(5) = "Maximum Heating Air Flow Rate";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(6) = "Maximum Sensible Heating Capacity";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(7) = "Maximum Cooling Air Flow Rate";
    state->dataPurchasedAirMgr->PurchAirNumericFields(PurchAirNum).FieldNames(8) = "Maximum Total Cooling Capacity";

    ZoneSizingRunDone = true;

    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).HeatingLimit = LimitType::LimitFlowRateAndCapacity;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatVolFlowRate = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatSensCap = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).CoolingLimit = LimitType::LimitFlowRateAndCapacity;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolVolFlowRate = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolTotCap = AutoSize;
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).cObjectName = "ZONEHVAC:IDEALLOADSAIRSYSTEM";
    state->dataPurchasedAirMgr->PurchAir(PurchAirNum).Name = "Ideal Loads 1";

    SizePurchasedAir(*state, PurchAirNum);
    EXPECT_DOUBLE_EQ(1.0, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatVolFlowRate);
    EXPECT_NEAR(63731.97, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxHeatSensCap, 0.1); // larger than test 1 above
    EXPECT_DOUBLE_EQ(2.0, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolVolFlowRate);
    EXPECT_NEAR(41078.43, state->dataPurchasedAirMgr->PurchAir(PurchAirNum).MaxCoolTotCap, 0.1); // larger than test1 above
}

TEST_F(EnergyPlusFixture, IdealLoadsAirSystem_GetInput)
{
    std::string const idf_objects = delimited_string({
        "ZoneHVAC:IdealLoadsAirSystem,",
        "ZONE 1 Ideal Loads, !- Name",
        ", !- Availability Schedule Name",
        "ZONE 1 INLETS, !- Zone Supply Air Node Name",
        ", !- Zone Exhaust Air Node Name",
        ", !- System Inlet Air Node Name",
        "50, !- Maximum Heating Supply Air Temperature{ C }",
        "13, !- Minimum Cooling Supply Air Temperature{ C }",
        "0.015, !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "0.009, !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "NoLimit, !- Heating Limit",
        "autosize, !- Maximum Heating Air Flow Rate{ m3 / s }",
        ", !- Maximum Sensible Heating Capacity{ W }",
        "NoLimit, !- Cooling Limit",
        "autosize, !- Maximum Cooling Air Flow Rate{ m3 / s }",
        ", !- Maximum Total Cooling Capacity{ W }",
        ", !- Heating Availability Schedule Name",
        ", !- Cooling Availability Schedule Name",
        "ConstantSupplyHumidityRatio, !- Dehumidification Control Type",
        ", !- Cooling Sensible Heat Ratio{ dimensionless }",
        "ConstantSupplyHumidityRatio, !- Humidification Control Type",
        ", !- Design Specification Outdoor Air Object Name",
        ", !- Outdoor Air Inlet Node Name",
        ", !- Demand Controlled Ventilation Type",
        ", !- Outdoor Air Economizer Type",
        ", !- Heat Recovery Type",
        ", !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "; !- Latent Heat Recovery Effectiveness{ dimensionless }",
    });

    ASSERT_TRUE(process_idf(idf_objects));

    state->dataGlobal->DoWeathSim = true;

    GetPurchasedAir(*state);

    auto & PurchAir(state->dataPurchasedAirMgr->PurchAir);
    EXPECT_EQ(PurchAir.size(), 1u);
    EXPECT_EQ(PurchAir(1).Name, "ZONE 1 IDEAL LOADS");
    EXPECT_EQ(PurchAir(1).MaxHeatSuppAirTemp, 50.0);
    EXPECT_EQ(PurchAir(1).MinCoolSuppAirTemp, 13.0);
    EXPECT_EQ(PurchAir(1).MaxHeatSuppAirHumRat, 0.015);
    EXPECT_EQ(PurchAir(1).MinCoolSuppAirHumRat, 0.009);
    EXPECT_EQ(PurchAir(1).HeatingLimit, LimitType::NoLimit);
    EXPECT_EQ(PurchAir(1).CoolingLimit, LimitType::NoLimit);
    EXPECT_EQ(PurchAir(1).DehumidCtrlType, HumControl::ConstantSupplyHumidityRatio);
    EXPECT_EQ(PurchAir(1).HumidCtrlType, HumControl::ConstantSupplyHumidityRatio);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_PlenumTest)
{

    std::string const idf_objects = delimited_string({

        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "Zone,",
        "  PLENUM ZONE,                    !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  Plenum Outlet Node,             !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "AirLoopHVAC:ReturnPlenum,",
        "  DOAS Zone Return Plenum,        !- Name",
        "  PLENUM ZONE,                    !- Zone Name",
        "  Plenum Node,                    !- Zone Node Name", // illegal use of non-unique zone node name
        "  Plenum Outlet Node,             !- Outlet Node Name",
        "  ,                               !- Induced Air Outlet Node or NodeList Name",
        "  Zone Exhaust Node;              !- Inlet 1 Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    Zone(1).SurfaceFirst = 1;
    Zone(1).SurfaceLast = 1;
    ScheduleManager::Schedule.allocate(1);
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    bool FirstHVACIteration(true);
    bool SimZone(true);
    bool SimAir(false);
    ManageZoneEquipment(*state,
                        FirstHVACIteration,
                        SimZone,
                        SimAir); // read zone equipment configuration and list objects and simulate ideal loads air system
    auto & PurchAir(state->dataPurchasedAirMgr->PurchAir);
    EXPECT_EQ(PurchAir(1).Name, "ZONE 1 IDEAL LOADS");
    // Ideal loads air system found the plenum it is attached to
    EXPECT_EQ(PurchAir(1).ReturnPlenumIndex, 1);
    // The ideal loads air system inlet air node is equal to the zone return plenum outlet node
    EXPECT_EQ(PurchAir(1).PlenumExhaustAirNodeNum, state->dataZonePlenum->ZoneRetPlenCond(1).OutletNode);
    // The ideal loads air system ZoneSupplyAirNodeNum is equal to the zone air inlet node
    EXPECT_EQ(PurchAir(1).ZoneSupplyAirNodeNum, state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1));
    // The ideal loads air system ZoneExhaustAirNodeNum is equal to the zone exhaust air node num
    EXPECT_EQ(PurchAir(1).ZoneExhaustAirNodeNum, state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1));
    // The zone exhaust air node is equal to the zone return plenum inlet air node
    EXPECT_EQ(state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1), state->dataZonePlenum->ZoneRetPlenCond(1).InletNode(1));
    // The ideal loads air system has a non-zero mass flow rate
    EXPECT_GT(PurchAir(1).SupplyAirMassFlowRate, 0.0);
    // The ideal loads air system mass flow rate is equal to all nodes attached to this system
    EXPECT_EQ(PurchAir(1).SupplyAirMassFlowRate, Node(PurchAir(1).ZoneSupplyAirNodeNum).MassFlowRate);
    EXPECT_EQ(PurchAir(1).SupplyAirMassFlowRate, Node(PurchAir(1).ZoneExhaustAirNodeNum).MassFlowRate);
    EXPECT_EQ(PurchAir(1).SupplyAirMassFlowRate, Node(PurchAir(1).PlenumExhaustAirNodeNum).MassFlowRate);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_ExhaustNodeTest)
{

    std::string const idf_objects = delimited_string({

        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,             !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "AirLoopHVAC:ReturnPlenum,",
        "  DOAS Zone Return Plenum,        !- Name",
        "  PLENUM ZONE,                    !- Zone Name",
        "  Plenum Node,                    !- Zone Node Name", // illegal use of non-unique zone node name
        "  Plenum Outlet Node,             !- Outlet Node Name",
        "  ,                               !- Induced Air Outlet Node or NodeList Name",
        "  Zone Exhaust Node;              !- Inlet 1 Node Name",

    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    Zone(1).SurfaceFirst = 1;
    Zone(1).SurfaceLast = 1;
    ScheduleManager::Schedule.allocate(1);
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    bool FirstHVACIteration(true);
    bool SimZone(true);
    bool SimAir(false);
    ManageZoneEquipment(*state,
                        FirstHVACIteration,
                        SimZone,
                        SimAir); // read zone equipment configuration and list objects and simulate ideal loads air system

    auto & PurchAir(state->dataPurchasedAirMgr->PurchAir);
    EXPECT_EQ(PurchAir(1).Name, "ZONE 1 IDEAL LOADS");
    // Ideal loads air system found the plenum it is attached to
    EXPECT_EQ(PurchAir(1).SupplyAirMassFlowRate, Node(PurchAir(1).ZoneSupplyAirNodeNum).MassFlowRate);
    EXPECT_EQ(PurchAir(1).SupplyAirMassFlowRate, Node(PurchAir(1).ZoneExhaustAirNodeNum).MassFlowRate);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_EMSOverrideTest)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,             !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  NoLimit,                        !- Heating Limit",
        "  autosize,                       !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  autosize,                       !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",

        "  Output:EnergyManagementSystem,                                                                ",
        "    Verbose,                 !- Actuator Availability Dictionary Reporting                      ",
        "    Verbose,                 !- Internal Variable Availability Dictionary Reporting             ",
        "    Verbose;                 !- EMS Runtime Language Debug Output Level                         ",

        "EnergyManagementSystem:Actuator,",
        "Mdot,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air Mass Flow Rate;",

        "EnergyManagementSystem:Actuator,",
        "Tsupply,",
        "ZONE 1 IDEAL LOADS,",
        "Ideal Loads Air System,",
        "Air TEMPERATURE;",

        "EnergyManagementSystem:Sensor,",
        "ZoneAirTemp,",
        "EAST ZONE,",
        "Zone Mean Air Temperature;",

        "EnergyManagementSystem:OutputVariable,",
        "MassstromIdealLoad_EMS, ! - Name",
        "Mdot, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:OutputVariable,",
        "SupplyTempIdealLoad_EMS, ! - Name",
        "Tsupply, ! - EMS Variable Name",
        "Averaged, ! - Type of Data in Variable",
        "SystemTimeStep; ! - Update Frequency",

        "EnergyManagementSystem:ProgramCallingManager,",
        "Test inside HVAC system iteration Loop,",
        "InsideHVACSystemIterationLoop,",
        "Test_InsideHVACSystemIterationLoop;",

        "EnergyManagementSystem:Program,",
        "Test_InsideHVACSystemIterationLoop,",
        "set Mdot = 0.1;",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    Zone(1).SurfaceFirst = 1;
    Zone(1).SurfaceLast = 1;
    ScheduleManager::Schedule.allocate(1);
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors
    state->dataZoneEquip->ZoneEquipConfig.allocate(1);

    state->dataZoneEquip->ZoneEquipConfig(1).IsControlled = true;
    state->dataZoneEquip->ZoneEquipConfig(1).NumInletNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).InletNode(1) = 1;

    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode.allocate(1);
    state->dataZoneEquip->ZoneEquipConfig(1).NumExhaustNodes = 1;
    state->dataZoneEquip->ZoneEquipConfig(1).ExhaustNode(1) = 2;
    state->dataGlobal->TimeStepZone = 0.25;

    EMSManager::CheckIfAnyEMS(*state); // get EMS input
    EMSManager::GetEMSInput(*state);
    state->dataEMSMgr->FinishProcessingUserInput = true;

    bool FirstHVACIteration(true);

    if (state->dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(*state);
        state->dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideMdotOn = true;
    state->dataPurchasedAirMgr->PurchAir(1).EMSOverrideSupplyTempOn = true;
    DataLoopNode::Node(2).Temp = 25.0;
    DataLoopNode::Node(2).HumRat = 0.001;

    InitPurchasedAir(*state, 1, FirstHVACIteration, 1, 1);
    Real64 SysOutputProvided;
    Real64 MoistOutputProvided;

    CalcPurchAirLoads(*state, 1, SysOutputProvided, MoistOutputProvided, 1, 1);

    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).EMSValueMassFlowRate, 0.0);
    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).EMSValueSupplyTemp, 0.0);
}

TEST_F(ZoneIdealLoadsTest, IdealLoads_NoCapacityTest)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "  EAST ZONE,                      !- Name",
        "  0,                              !- Direction of Relative North{ deg }",
        "  0,                              !- X Origin{ m }",
        "  0,                              !- Y Origin{ m }",
        "  0,                              !- Z Origin{ m }",
        "  1,                              !- Type",
        "  1,                              !- Multiplier",
        "  autocalculate,                  !- Ceiling Height{ m }",
        "  autocalculate;                  !- Volume{ m3 }",

        "ZoneHVAC:IdealLoadsAirSystem,",
        "  ZONE 1 IDEAL LOADS,             !- Name",
        "  ,                               !- Availability Schedule Name",
        "  Zone Inlet Node,                !- Zone Supply Air Node Name",
        "  Zone Exhaust Node,              !- Zone Exhaust Air Node Name",
        "  ,                               !- System Inlet Air Node Name",
        "  50,                             !- Maximum Heating Supply Air Temperature{ C }",
        "  13,                             !- Minimum Cooling Supply Air Temperature{ C }",
        "  0.015,                          !- Maximum Heating Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  0.009,                          !- Minimum Cooling Supply Air Humidity Ratio{ kgWater / kgDryAir }",
        "  LimitCapacity,                  !- Heating Limit",
        "  ,                               !- Maximum Heating Air Flow Rate{ m3 / s }",
        "  0,                              !- Maximum Sensible Heating Capacity{ W }",
        "  NoLimit,                        !- Cooling Limit",
        "  ,                               !- Maximum Cooling Air Flow Rate{ m3 / s }",
        "  ,                               !- Maximum Total Cooling Capacity{ W }",
        "  ,                               !- Heating Availability Schedule Name",
        "  ,                               !- Cooling Availability Schedule Name",
        "  ConstantSupplyHumidityRatio,    !- Dehumidification Control Type",
        "  ,                               !- Cooling Sensible Heat Ratio{ dimensionless }",
        "  ConstantSupplyHumidityRatio,    !- Humidification Control Type",
        "  ,                               !- Design Specification Outdoor Air Object Name",
        "  ,                               !- Outdoor Air Inlet Node Name",
        "  ,                               !- Demand Controlled Ventilation Type",
        "  ,                               !- Outdoor Air Economizer Type",
        "  ,                               !- Heat Recovery Type",
        "  ,                               !- Sensible Heat Recovery Effectiveness{ dimensionless }",
        "  ;                               !- Latent Heat Recovery Effectiveness{ dimensionless }",

        "ZoneHVAC:EquipmentConnections,",
        "  EAST ZONE,                      !- Zone Name",
        "  ZoneEquipment,                  !- Zone Conditioning Equipment List Name",
        "  Zone Inlet Node,                !- Zone Air Inlet Node or NodeList Name",
        "  Zone Exhaust Node,              !- Zone Air Exhaust Node or NodeList Name",
        "  Zone Node,                      !- Zone Air Node Name",
        "  Zone Outlet Node;               !- Zone Return Air Node Name",

        "ZoneHVAC:EquipmentList,",
        "  ZoneEquipment,                  !- Name",
        "  SequentialLoad,                 !- Load Distribution Scheme",
        "  ZoneHVAC:IdealLoadsAirSystem,   !- Zone Equipment 1 Object Type",
        "  ZONE 1 IDEAL LOADS,             !- Zone Equipment 1 Name",
        "  1,                              !- Zone Equipment 1 Cooling Sequence",
        "  1;                              !- Zone Equipment 1 Heating or No - Load Sequence",
    });

    ASSERT_TRUE(process_idf(idf_objects)); // read idf objects

    state->dataGlobal->DoWeathSim = true;

    bool ErrorsFound = false;
    GetZoneData(*state, ErrorsFound);
    Zone(1).SurfaceFirst = 1;
    Zone(1).SurfaceLast = 1;
    ScheduleManager::Schedule.allocate(1);
    AllocateHeatBalArrays(*state);
    EXPECT_FALSE(ErrorsFound); // expect no errors

    bool FirstHVACIteration(true);
    bool SimZone(true);
    bool SimAir(false);

    ManageZoneEquipment(*state,
                        FirstHVACIteration,
                        SimZone,
                        SimAir); // read zone equipment configuration and list objects and simulate ideal loads air system


    if (state->dataPurchasedAirMgr->GetPurchAirInputFlag) {
        GetPurchasedAir(*state);
        state->dataPurchasedAirMgr->GetPurchAirInputFlag = false;
    }

    InitPurchasedAir(*state, 1, FirstHVACIteration, 1, 1);
    Real64 SysOutputProvided;
    Real64 MoistOutputProvided;

    CalcPurchAirLoads(*state, 1, SysOutputProvided, MoistOutputProvided, 1, 1);

    EXPECT_EQ(SysOutputProvided, 0.0);
    // #8365 Supply air mass flow rate should be zero during heating mode when capacity is limited to zero
    EXPECT_EQ(state->dataPurchasedAirMgr->PurchAir(1).SupplyAirMassFlowRate, 0.0);
}
