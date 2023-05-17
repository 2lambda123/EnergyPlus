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

#ifndef ConvectionCoefficients_hh_INCLUDED
#define ConvectionCoefficients_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array1S.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/ConvectionConstants.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Convect {

    struct HcIntUserCurveStruct
    {
        // Members
        std::string Name; // user's name for object
        RefTemp ReferenceTempType = RefTemp::Invalid;
        int HcFnTempDiffCurveNum = 0;
        int HcFnTempDiffDivHeightCurveNum = 0;
        int HcFnACHCurveNum = 0;
        int HcFnACHDivPerimLengthCurveNum = 0;
    };

    struct HcExtUserCurveStruct
    {
        // Members
        std::string Name;
        int ReferenceTempType = 0;
        bool SuppressRainChange = false;
        RefWind WindSpeedType = RefWind::Invalid;
        int HfFnWindSpeedCurveNum = 0;
        int HnFnTempDiffCurveNum = 0;
        int HnFnTempDiffDivHeightCurveNum = 0;
    };

    struct IntAdaptiveConvAlgoStruct
    {
        // Members
        std::string Name;

        std::array<HcInt, static_cast<int>(IntConvClass::Num)> IntConvClassEqNums = {
            HcInt::FohannoPolidoriVerticalWall, // A3_SimpleBuoy_VertWalls
	    HcInt::AlamdariHammondStableHorizontal, // A3_SimpleBuoy_StableHoriz
	    HcInt::AlamdariHammondUnstableHorizontal, // A3_SimpleBuoy_UnstableHoriz
	    HcInt::WaltonStableHorizontalOrTilt, // A3_SimpleBuoy_StableTilted
	    HcInt::WaltonUnstableHorizontalOrTilt, // A3_SimpleBuoy_UnstableTilted
	    HcInt::ISO15099Windows, // A3_SimpleBuoy_Windows
	    HcInt::KhalifaEq3WallAwayFromHeat, // A1_FloorHeatCeilingCool_VertWalls
	    HcInt::AlamdariHammondStableHorizontal, // A1_FloorHeatCeilingCool_StableHoriz
	    HcInt::KhalifaEq4CeilingAwayFromHeat, // A1_FloorHeatCeilingCool_UnstableHoriz
	    HcInt::AwbiHattonHeatedFloor, // A1_FloorHeatCeilingCool_HeatedFloor
	    HcInt::KaradagChilledCeiling, // A1_FloorHeatCeilingCool_ChilledCeil
	    HcInt::WaltonStableHorizontalOrTilt, // A1_FloorHeatCeilingCool_StableTilted
	    HcInt::WaltonUnstableHorizontalOrTilt, // A1_FloorHeatCeilingCool_UnstableTilted
	    HcInt::ISO15099Windows, // A1_FloorHeatCeilingCool_Windows
	    HcInt::KhalifaEq6NonHeatedWalls, // A2_WallPanelHeat_VertWallsNonHeated
	    HcInt::AwbiHattonHeatedWall, // A2_WallPanelHeat_HeatedVerticalWall
	    HcInt::AlamdariHammondStableHorizontal, // A2_WallPanelHeat_StableHoriz
	    HcInt::KhalifaEq7Ceiling, // A2_WallPanelHeat_UnstableHoriz
	    HcInt::WaltonStableHorizontalOrTilt, // A2_WallPanelHeat_StableTilted
	    HcInt::WaltonUnstableHorizontalOrTilt, // A2_WallPanelHeat_UnstableTilted
	    HcInt::ISO15099Windows, // A2_WallPanelHeat_Windows
	    HcInt::FohannoPolidoriVerticalWall, // B_ConvectiveHeat_VertWall
	    HcInt::KhalifaEq5WallNearHeat, // B_ConvectiveHeat_VertWallNearHeater
	    HcInt::AlamdariHammondStableHorizontal, // B_ConvectiveHeat_StableHoriz
	    HcInt::KhalifaEq7Ceiling, // B_ConvectiveHeat_UnstableHoriz
	    HcInt::WaltonStableHorizontalOrTilt, // B_ConvectiveHeat_StableTilted
	    HcInt::WaltonUnstableHorizontalOrTilt, // B_ConvectiveHeat_UnstableTilted
	    HcInt::ISO15099Windows, // B_ConvectiveHeat_Windows
	    HcInt::GoldsteinNovoselacCeilingDiffuserWalls, // C_CentralAir_Walls
	    HcInt::FisherPedersenCeilDiffuserCeiling, // C_CentralAir_Ceiling
	    HcInt::GoldsteinNovoselacCeilingDiffuserFloor, // C_CentralAir_Floor
	    HcInt::GoldsteinNovoselacCeilingDiffuserWindow, // C_CentralAir_Windows
	    HcInt::KhalifaEq3WallAwayFromHeat, // D_ZoneFanCirc_VertWall
	    HcInt::AlamdariHammondStableHorizontal, // D_ZoneFanCirc_StableHoriz
	    HcInt::KhalifaEq4CeilingAwayFromHeat, // D_ZoneFanCirc_UnstableHoriz
	    HcInt::WaltonStableHorizontalOrTilt, // D_ZoneFanCirc_StableTilted
	    HcInt::WaltonUnstableHorizontalOrTilt, // D_ZoneFanCirc_UnstableTilted
	    HcInt::ISO15099Windows, // D_ZoneFanCirc_Windows
            HcInt::BeausoleilMorrisonMixedAssistingWall, // E_MixedBuoy_AssistFlowWall
	    HcInt::BeausoleilMorrisonMixedOppossingWall, // E_MixedBuoy_OppositeFlowWall
	    HcInt::BeausoleilMorrisonMixedStableFloor, // E_MixedBuoy_StableFloor
	    HcInt::BeausoleilMorrisonMixedUnstableFloor, // E_MixedBuoy_UnstableFloor
	    HcInt::BeausoleilMorrisonMixedStableCeiling, // E_MixedBuoy_StableCeiling
	    HcInt::BeausoleilMorrisonMixedUnstableCeiling, // E_MixedBuoy_UnstableCeiling
	    HcInt::GoldsteinNovoselacCeilingDiffuserWindow // E_MixedBuoy_Windows
	};
	    
        std::array<int, static_cast<int>(IntConvClass::Num)> IntConvClassUserCurveNums = {
            0, // A3_SimpleBuoy_VertWalls
	    0, // A3_SimpleBuoy_StableHoriz
	    0, // A3_SimpleBuoy_UnstableHoriz
	    0, // A3_SimpleBuoy_StableTilted
	    0, // A3_SimpleBuoy_UnstableTilted
	    0, // A3_SimpleBuoy_Windows
	    0, // A1_FloorHeatCeilingCool_VertWalls
	    0, // A1_FloorHeatCeilingCool_StableHoriz
	    0, // A1_FloorHeatCeilingCool_UnstableHoriz
	    0, // A1_FloorHeatCeilingCool_HeatedFloor
	    0, // A1_FloorHeatCeilingCool_ChilledCeil
	    0, // A1_FloorHeatCeilingCool_StableTilted
	    0, // A1_FloorHeatCeilingCool_UnstableTilted
	    0, // A1_FloorHeatCeilingCool_Windows
	    0, // A2_WallPanelHeat_VertWallsNonHeated
	    0, // A2_WallPanelHeat_HeatedVerticalWall
	    0, // A2_WallPanelHeat_StableHoriz
	    0, // A2_WallPanelHeat_UnstableHoriz
	    0, // A2_WallPanelHeat_StableTilted
	    0, // A2_WallPanelHeat_UnstableTilted
	    0, // A2_WallPanelHeat_Windows
	    0, // B_ConvectiveHeat_VertWall
	    0, // B_ConvectiveHeat_VertWallNearHeater
	    0, // B_ConvectiveHeat_StableHoriz
	    0, // B_ConvectiveHeat_UnstableHoriz
	    0, // B_ConvectiveHeat_StableTilted
	    0, // B_ConvectiveHeat_UnstableTilted
	    0, // B_ConvectiveHeat_Windows
	    0, // C_CentralAir_Walls
	    0, // C_CentralAir_Ceiling
	    0, // C_CentralAir_Floor
	    0, // C_CentralAir_Windows
	    0, // D_ZoneFanCirc_VertWall
	    0, // D_ZoneFanCirc_StableHoriz
	    0, // D_ZoneFanCirc_UnstableHoriz
	    0, // D_ZoneFanCirc_StableTilted
	    0, // D_ZoneFanCirc_UnstableTilted
	    0, // D_ZoneFanCirc_Windows
            0, // E_MixedBuoy_AssistFlowWall
	    0, // E_MixedBuoy_OppositeFlowWall
	    0, // E_MixedBuoy_StableFloor
	    0, // E_MixedBuoy_UnstableFloor
	    0, // E_MixedBuoy_StableCeiling
	    0, // E_MixedBuoy_UnstableCeiling
	    0 // E_MixedBuoy_Windows
	};

#ifdef GET_OUT	    
	HcIn SimpleBuoyVertWallEqNum = HcInt::FohannoPolidoriVerticalWall; // InConvClass_A3_VertWalls
        int SimpleBuoyVertWallUserCurveNum = 0;
        HcIn SimpleBuoyStableHorizEqNum = HcInt::AlamdariHammondStableHorizontal; // InConvClass_A3_StableHoriz
        int SimpleBuoyStableHorizUserCurveNum = 0;
        HcIn SimpleBuoyUnstableHorizEqNum = HcInt::AlamdariHammondUnstableHorizontal; // InConvClass_A3_UnstableHoriz
        int SimpleBuoyUnstableHorizUserCurveNum = 0;
        HcIn SimpleBuoyStableTiltedEqNum = HcInt::WaltonStableHorizontalOrTilt; // InConvClass_A3_StableTilted
        int SimpleBuoyStableTiltedUserCurveNum = 0;
        HcIn SimpleBuoyUnstableTiltedEqNum = HcInt::WaltonUnstableHorizontalOrTilt; // InConvClass_A3_UnstableTilted
        int SimpleBuoyUnstableTiltedUserCurveNum = 0;
        HcIn SimpleBuoyWindowsEqNum = HcInt::ISO15099Windows; // InConvClass_A3_Windows
        int SimpleBuoyWindowsUserCurveNum = 0;
        HcIn FloorHeatCeilingCoolVertWallEqNum = HcInt::KhalifaEq3WallAwayFromHeat; // InConvClass_A1_VertWalls
        int FloorHeatCeilingCoolVertWallUserCurveNum = 0;
        HcIn FloorHeatCeilingCoolStableHorizEqNum = HcInt::AlamdariHammondStableHorizontal; // InConvClass_A1_StableHoriz
        int FloorHeatCeilingCoolStableHorizUserCurveNum = 0;
        HcIn FloorHeatCeilingCoolUnstableHorizEqNum = HcInt::KhalifaEq4CeilingAwayFromHeat; // InConvClass_A1_UnstableHoriz
        int FloorHeatCeilingCoolUnstableHorizUserCurveNum = 0;
        HcIn FloorHeatCeilingCoolHeatedFloorEqNum = HcInt::AwbiHattonHeatedFloor; // InConvClass_A1_HeatedFloor
        int FloorHeatCeilingCoolHeatedFloorUserCurveNum = 0;
        HcIn FloorHeatCeilingCoolChilledCeilingEqNum = HcInt::KaradagChilledCeiling; // InConvClass_A1_ChilledCeil
        int FloorHeatCeilingCoolChilledCeilingUserCurveNum = 0;
        HcIn FloorHeatCeilingCoolStableTiltedEqNum = HcInt::WaltonStableHorizontalOrTilt; // InConvClass_A1_StableTilted
        int FloorHeatCeilingCoolStableTiltedUserCurveNum = 0;
        HcIn FloorHeatCeilingCoolUnstableTiltedEqNum = HcInt::WaltonUnstableHorizontalOrTilt; // InConvClass_A1_UnstableTilted
        int FloorHeatCeilingCoolUnstableTiltedUserCurveNum = 0;
        HcIn FloorHeatCeilingCoolWindowsEqNum = HcInt::ISO15099Windows; // InConvClass_A1_Windows
        int FloorHeatCeilingCoolWindowsUserCurveNum = 0;
        HcIn WallPanelHeatVertWallEqNum = HcInt::KhalifaEq6NonHeatedWalls; // InConvClass_A2_VertWallsNonHeated
        int WallPanelHeatVertWallUserCurveNum = 0;
        HcIn WallPanelHeatHeatedWallEqNum = HcInt::AwbiHattonHeatedWall; // InConvClass_A2_HeatedVerticalWall
        int WallPanelHeatHeatedWallUserCurveNum = 0;
        HcIn WallPanelHeatStableHorizEqNum = HcInt::AlamdariHammondStableHorizontal; // InConvClass_A2_StableHoriz
        int WallPanelHeatStableHorizUserCurveNum = 0;
        HcIn WallPanelHeatUnstableHorizEqNum = HcInt::KhalifaEq7Ceiling; // InConvClass_A2_UnstableHoriz
        int WallPanelHeatUnstableHorizUserCurveNum = 0;
        HcIn WallPanelHeatStableTiltedEqNum = HcInt::WaltonStableHorizontalOrTilt; // InConvClass_A2_StableTilted
        int WallPanelHeatStableTiltedUserCurveNum = 0;
        HcIn WallPanelHeatUnstableTiltedEqNum = HcInt::WaltonUnstableHorizontalOrTilt; // InConvClass_A2_UnstableTilted
        int WallPanelHeatUnstableTiltedUserCurveNum = 0;
        HcIn WallPanelHeatWindowsEqNum = HcInt::ISO15099Windows; // InConvClass_A2_Windows
        int WallPanelHeatWindowsUserCurveNum = 0;
        HcIn ConvectiveHeatVertWallEqNum = HcInt::FohannoPolidoriVerticalWall;
        int ConvectiveHeatVertWallUserCurveNum = 0;
        HcIn ConvectiveHeatVertWallNearHeaterEqNum = HcInt::KhalifaEq5WallNearHeat;
        int ConvectiveHeatVertWallNearHeaterUserCurveNum = 0;
        HcIn ConvectiveHeatStableHorizEqNum = HcInt::AlamdariHammondStableHorizontal;
        int ConvectiveHeatStableHorizUserCurveNum = 0;
        HcIn ConvectiveHeatUnstableHorizEqNum = HcInt::KhalifaEq7Ceiling;
        int ConvectiveHeatUnstableHorizUserCurveNum = 0;
        HcIn ConvectiveHeatStableTiltedEqNum = HcInt::WaltonStableHorizontalOrTilt;
        int ConvectiveHeatStableTiltedUserCurveNum = 0;
        HcIn ConvectiveHeatUnstableTiltedEqNum = HcInt::WaltonUnstableHorizontalOrTilt;
        int ConvectiveHeatUnstableTiltedUserCurveNum = 0;
        HcIn ConvectiveHeatWindowsEqNum = HcInt::ISO15099Windows;
        int ConvectiveHeatWindowsUserCurveNum = 0;
        HcIn CentralAirWallEqNum = HcInt::GoldsteinNovoselacCeilingDiffuserWalls;
        int CentralAirWallUserCurveNum = 0;
        HcIn CentralAirCeilingEqNum = HcInt::FisherPedersenCeilDiffuserCeiling;
        int CentralAirCeilingUserCurveNum = 0;
        HcIn CentralAirFloorEqNum = HcInt::GoldsteinNovoselacCeilingDiffuserFloor;
        int CentralAirFloorUserCurveNum = 0;
        HcIn CentralAirWindowsEqNum = HcInt::GoldsteinNovoselacCeilingDiffuserWindow;
        int CentralAirWindowsUserCurveNum = 0;
        HcIn ZoneFanCircVertWallEqNum = HcInt::KhalifaEq3WallAwayFromHeat;
        int ZoneFanCircVertWallUserCurveNum = 0;
        HcIn ZoneFanCircStableHorizEqNum = HcInt::AlamdariHammondStableHorizontal;
        int ZoneFanCircStableHorizUserCurveNum = 0;
        HcIn ZoneFanCircUnstableHorizEqNum = HcInt::KhalifaEq4CeilingAwayFromHeat;
        int ZoneFanCircUnstableHorizUserCurveNum = 0;
        HcIn ZoneFanCircStableTiltedEqNum = HcInt::WaltonStableHorizontalOrTilt;
        int ZoneFanCircStableTiltedUserCurveNum = 0;
        HcIn ZoneFanCircUnstableTiltedEqNum = HcInt::WaltonUnstableHorizontalOrTilt;
        int ZoneFanCircUnstableTiltedUserCurveNum = 0;
        HcIn ZoneFanCircWindowsEqNum = HcInt::ISO15099Windows;
        int ZoneFanCircWindowsUserCurveNum = 0;
        HcIn MixedBuoyAssistingFlowWallEqNum = HcInt::BeausoleilMorrisonMixedAssistingWall;
        int MixedBuoyAssistingFlowWallUserCurveNum = 0;
        HcIn MixedBuoyOpposingFlowWallEqNum = HcInt::BeausoleilMorrisonMixedOppossingWall;
        int MixedBuoyOpposingFlowWallUserCurveNum = 0;
        HcIn MixedStableFloorEqNum = HcInt::BeausoleilMorrisonMixedStableFloor;
        int MixedStableFloorUserCurveNum = 0;
        HcIn MixedUnstableFloorEqNum = HcInt::BeausoleilMorrisonMixedUnstableFloor;
        int MixedUnstableFloorUserCurveNum = 0;
        HcIn MixedStableCeilingEqNum = HcInt::BeausoleilMorrisonMixedStableCeiling;
        int MixedStableCeilingUserCurveNum = 0;
        HcIn MixedUnstableCeilingEqNum = HcInt::BeausoleilMorrisonMixedUnstableCeiling;
        int MixedUnstableCeilingUserCurveNum = 0;
        HcIn MixedWindowsEqNum = HcInt::GoldsteinNovoselacCeilingDiffuserWindow;
        int MixedWindowsUserCurveNum = 0;
#endif // GET_OUT
    };

    struct ExtAdaptiveConvAlgoStruct
    {
        // Members
        std::string Name;
        bool SuppressRainChange = false;

        std::array<HcExt, static_cast<int>(ExtConvClass2::Num)> ExtConvClass2EqNums = {
            HcExt::SparrowWindward, // WindConvection_WindwardWall 
            HcExt::SparrowLeeward, // WindConvection_LeewardWall
            HcExt::ClearRoof, // WindConvection_HorizRoof
            HcExt::NaturalASHRAEVerticalWall, // NaturalConvection_VertWall
            HcExt::NaturalWaltonStableHorizontalOrTilt, // NaturalConvection_StableHorizOrTilt
            HcExt::NaturalWaltonUnstableHorizontalOrTilt // NaturalConvection_StableHorizOrTilt
        };
		    

	std::array<int, static_cast<int>(ExtConvClass2::Num)> ExtConvClass2UserCurveNums = {
            0, // WindConvection_WindwardWall 
            0, // WindConvection_LeewardWall
            0, // WindConvection_HorizRoof
            0, // NaturalConvection_VertWall
            0, // NaturalConvection_StableHorizOrTilt
            0  // NaturalConvection_StableHorizOrTilt
        };
#ifdef GET_OUT

	       
        HcExt HWindWallWindwardEqNum = HcExt::SparrowWindward;
        int HWindWallWindwardUserCurveNum = 0;
        HcExt HWindWallLeewardEqNum = HcExt::SparrowLeeward;
        int HWindWallLeewardUserCurveNum = 0;
        HcExt HWindHorizRoofEqNum = HcExt::ClearRoof;
        int HWindHorizRoofUserCurveNum = 0;
        HcExt HNatVertWallEqNum = HcExt::NaturalASHRAEVerticalWall;
        int HNatVertWallUserCurveNum = 0;
        HcExt HNatStableHorizEqNum = HcExt::NaturalWaltonStableHorizontalOrTilt;
        int HNatStableHorizUserCurveNum = 0;
        HcExt HNatUnstableHorizEqNum = HcExt::NaturalWaltonUnstableHorizontalOrTilt;
        int HNatUnstableHorizUserCurveNum = 0;
#endif // GET_OUT	    
    };

    struct BoundingBoxVertStruct
    {
        // Members
        int SurfNum = 0;
        int VertNum = 0;
        Vector3<Real64> Vertex{0.0, 0.0, 0.0};
    };

    struct RoofGeoCharacteristicsStruct
    {
        // Members
        Real64 Area = 0.0;      // Sum of all roof surface areas
        Real64 Perimeter = 0.0; // Actual perimeter of all roof surfaces, after removing all edges that are used twice (and inserting vertices
                                // to split surfaces as needed)
        Real64 Height = 0.0;    // Weighted average mean vertical height: for each surface, take max - Zmin value,
                                // then do a weighted average by surface area
        Real64 Azimuth = 0.0;   // Weighted average azimuth
        Real64 Tilt = 0.0;      // Weighted average tilt
    };

    struct FacadeGeoCharacteristicsStruct
    {
        // Members
        Real64 AzimuthRangeLow;
        Real64 AzimuthRangeHi;
        Real64 Zmax;
        Real64 Zmin;
        Real64 Ymax;
        Real64 Ymin;
        Real64 Xmax;
        Real64 Xmin;
        Real64 Area;
        Real64 Perimeter;
        Real64 Height;

        // Default Constructor
        FacadeGeoCharacteristicsStruct() = default;

        // Member Constructor
        FacadeGeoCharacteristicsStruct(Real64 const AzimuthRangeLow,
                                       Real64 const AzimuthRangeHi,
                                       Real64 const Zmax,
                                       Real64 const Zmin,
                                       Real64 const Ymax,
                                       Real64 const Ymin,
                                       Real64 const Xmax,
                                       Real64 const Xmin,
                                       Real64 const Area,
                                       Real64 const Perimeter,
                                       Real64 const Height)
            : AzimuthRangeLow(AzimuthRangeLow), AzimuthRangeHi(AzimuthRangeHi), Zmax(Zmax), Zmin(Zmin), Ymax(Ymax), Ymin(Ymin), Xmax(Xmax),
              Xmin(Xmin), Area(Area), Perimeter(Perimeter), Height(Height)
        {
        }
    };

    // Functions

    void
    InitIntConvCoeff(EnergyPlusData &state,
		     const Array1D<Real64> &SurfaceTemperatures,        // Temperature of surfaces for evaluation of HcIn
		     ObjexxFCL::Optional_int_const ZoneToResimulate = _ // if passed in, then only calculate surfaces that have this zone
		     );

    void InitExtConvCoeff(EnergyPlusData &state,
			  int SurfNum,                          // Surface number (in Surface derived type)
			  Real64 HMovInsul,                     // Equivalent convection coefficient of movable insulation
			  Material::SurfaceRoughness Roughness, // Roughness index (1-6), see DataHeatBalance parameters
			  Real64 AbsExt,                        // Exterior thermal absorptance
			  Real64 TempExt,                       // Exterior surface temperature (C)
			  Real64 &HExt,                         // Convection coefficient to exterior air
			  Real64 &HSky,                         // "Convection" coefficient to sky temperature
			  Real64 &HGround,                      // "Convection" coefficient to ground temperature
			  Real64 &HAir                          // Radiation to Air Component
			  );
	
    Real64 CalcHfExteriorSparrow(Real64 SurfWindSpeed,                 // Local wind speed at height of the heat transfer surface (m/s)
                                 Real64 GrossArea,                     // Gross surface area {m2}
                                 Real64 Perimeter,                     // Surface perimeter length {m}
                                 Real64 CosTilt,                       // Cosine of the Surface Tilt Angle
                                 Real64 Azimuth,                       // Facing angle (degrees) of the surface outward normal
                                 Material::SurfaceRoughness Roughness, // Surface roughness index
                                 Real64 WindDirection                  // Wind (compass) direction (degrees)
    );

    bool Windward(Real64 CosTilt,      // Cosine of the surface tilt angle
                  Real64 Azimuth,      // or Facing, Direction the surface outward normal faces (degrees)
                  Real64 WindDirection // Wind direction measured clockwise from geographhic North
    );

    void GetUserConvCoeffs(EnergyPlusData &state);

    void ApplyIntConvValue(EnergyPlusData &state, int surfNum, HcInt model, int userNum);
    void ApplyExtConvValue(EnergyPlusData &state, int surfNum, HcInt model, int userNum);
    void ApplyIntConvValueMulti(EnergyPlusData &state, DataSurfaces::SurfaceFilter surfaceFilter, HcInt model, int userNum);
    void ApplyExtConvValueMulti(EnergyPlusData &state, DataSurfaces::SurfaceFilter surfaceFilter, HcExt model, int userNum);

    Real64 CalcASHRAESimpExtConvCoeff(Material::SurfaceRoughness Roughness, // Integer index for roughness, relates to parameter array indices
                                         Real64 SurfWindSpeed                  // Current wind speed, m/s
    );

    Real64 CalcASHRAESimpleIntConvCoeff(Real64 Tsurf, Real64 Tamb, Real64 cosTilt);

    void CalcASHRAESimpleIntConvCoeff(EnergyPlusData &state,
                                      int SurfNum,                  // surface number for which coefficients are being calculated
                                      Real64 SurfaceTemperature,    // Temperature of surface for evaluation of HcIn
                                      Real64 ZoneMeanAirTemperature // Mean Air Temperature of Zone
    );

    Real64 CalcASHRAETARPNatural(Real64 Tsurf, Real64 Tamb, Real64 cosTilt);

    void CalcASHRAEDetailedIntConvCoeff(EnergyPlusData &state,
                                        int SurfNum,                  // surface number for which coefficients are being calculated
                                        Real64 SurfaceTemperature,    // Temperature of surface for evaluation of HcIn
                                        Real64 ZoneMeanAirTemperature // Mean Air Temperature of Zone
    );

    void CalcDetailedHcInForDVModel(EnergyPlusData &state,
                                    int SurfNum,                                       // surface number for which coefficients are being calculated
                                    const Array1D<Real64> &SurfaceTemperatures,        // Temperature of surfaces for evaluation of HcIn
                                    Array1D<Real64> &HcIn,                             // Interior Convection Coeff Array
                                    ObjexxFCL::Optional<Array1S<Real64> const> Vhc = _ // Velocity array for forced convection coeff calculation
    );

    Real64 CalcZoneSupplyAirTemp(EnergyPlusData &state, int ZoneNum);

    Real64 CalcZoneSystemVolFlowRate(EnergyPlusData &state, int ZoneNum);

    Real64 CalcZoneSystemACH(EnergyPlusData &state, int ZoneNum);

    Real64 CalcCeilingDiffuserACH(EnergyPlusData &state, int ZoneNum);

    Real64 CalcCeilingDiffuserIntConvCoeff(EnergyPlusData &state,
                                           Real64 ACH, // [1/hr] air system air change rate
                                           Real64 Tsurf,
                                           Real64 Tair,
                                           Real64 cosTilt,
                                           Real64 humRat,
                                           Real64 height,
                                           bool isWindow = false);

    void CalcCeilingDiffuserIntConvCoeff(EnergyPlusData &state,
                                         int ZoneNum,
                                         const Array1D<Real64> &SurfaceTemperatures); // zone number for which coefficients are being calculated

    // CalcCeilingDiffuserInletCorr should replace CalcCeilingDiffuser (above), if ZoneTempPredictorCorrector can
    // ever be made to work correctly with the inlet air temperature.

    void CalcCeilingDiffuserInletCorr(EnergyPlusData &state,
                                      int ZoneNum,                         // Zone number
                                      Array1D<Real64> &SurfaceTemperatures // For CalcASHRAEDetailed, if called
    );

    void CalcTrombeWallIntConvCoeff(EnergyPlusData &state,
                                    int ZoneNum,                               // Zone number for which coefficients are being calculated
                                    const Array1D<Real64> &SurfaceTemperatures // Temperature of surfaces for evaluation of HcIn
    );

    void CalcNusselt(EnergyPlusData &state,
                     int SurfNum, // Surface number
                     Real64 asp,  // Aspect ratio: window height to gap width
                     Real64 tso,  // Temperature of gap surface closest to outside (K)
                     Real64 tsi,  // Temperature of gap surface closest to zone (K)
                     Real64 gr,   // Gap gas Grashof number
                     Real64 pr,   // Gap gas Prandtl number
                     Real64 &gnu  // Gap gas Nusselt number
    );

    Real64 SetExtConvCoeff(EnergyPlusData &state, int SurfNum); // Surface Number

    Real64 SetIntConvCoeff(EnergyPlusData &state, int SurfNum); // Surface Number

    Real64 CalcISO15099WindowIntConvCoeff(EnergyPlusData &state,
                                          Real64 SurfaceTemperature, // Temperature of surface for evaluation of HcIn
                                          Real64 AirTemperature,     // Mean Air Temperature of Zone (or adjacent air temperature)
                                          Real64 AirHumRat,          // air humidity ratio
                                          Real64 Height,             // window cavity height [m]
                                          Real64 TiltDeg,            // glazing tilt in degrees
                                          Real64 sineTilt            // sine of glazing tilt
    );

    void CalcISO15099WindowIntConvCoeff(EnergyPlusData &state,
                                        int SurfNum,               // surface number for which coefficients are being calculated
                                        Real64 SurfaceTemperature, // Temperature of surface for evaluation of HcIn
                                        Real64 AirTemperature      // Mean Air Temperature of Zone (or adjacent air temperature)
    );

    RoofGeoCharacteristicsStruct getRoofGeometryInformation(EnergyPlusData &state);

    void SetupAdaptiveConvectionStaticMetaData(EnergyPlusData &state);

    void SetupAdaptiveConvectionRadiantSurfaceData(EnergyPlusData &state);

    void ManageInsideAdaptiveConvectionAlgo(EnergyPlusData &state, int SurfNum); // surface number for which coefficients are being calculated

    void ManageOutsideAdaptiveConvectionAlgo(EnergyPlusData &state,
                                             int SurfNum, // surface number for which coefficients are being calculated
                                             Real64 &Hc   // result for Hc Outside face, becomes HExt.
    );

    Real64 EvaluateInHcModels(EnergyPlusData &state, int SurfNum, HcInt ConvModelEquationNum);

    Real64 EvaluateOutHcModels(EnergyPlusData &state, int SurfNum, HcExt NaturalConvModelEqNum, HcExt ForcedConvModelEqNum);

    void DynamicExtConvSurfaceClassification(EnergyPlusData &state, int SurfNum); // surface number

    void MapExtConvClassificationToHcModels(EnergyPlusData &state, int SurfNum); // surface number

    void DynamicIntConvSurfaceClassification(EnergyPlusData &state, int SurfNum); // surface number

    void MapIntConvClassificationToHcModels(EnergyPlusData &state, int SurfNum); // surface pointer index

    Real64 CalcUserDefinedInsideHcModel(EnergyPlusData &state, int SurfNum, int UserCurveNum);

    Real64 CalcUserDefinedOutsideHcModel(EnergyPlusData &state, int SurfNum, int UserCurveNum);

    //** Begin catalog of Hc equation functions. **** !*************************************************

    inline Real64 CalcASHRAEVerticalWall(Real64 const DeltaTemp) // [C] temperature difference between surface and air
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Aug 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Calculate the model equation attributed to ASHRAE for vertical walls for natural convection

        // REFERENCES:
        // 2.  ASHRAE Handbook of Fundamentals 2001, p. 3.12, Table 5.

        return 1.31 * std::pow(std::abs(DeltaTemp), 1.0 / 3.0);
    }

    inline Real64 CalcWaltonUnstableHorizontalOrTilt(Real64 const DeltaTemp, // [C] temperature difference between surface and air
                                                     Real64 const CosineTilt // Cosine of tilt angle
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Aug 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Calculate the model equation attributed to Walton's TARP program for horizontal
        // and tilted surfaces with enhanced, thermally unstable natural convection

        // METHODOLOGY EMPLOYED:

        // REFERENCES:
        // 1.  Walton, G. N. 1983. Thermal Analysis Research Program (TARP) Reference Manual,
        //     NBSSIR 83-2655, National Bureau of Standards, "Surface Inside Heat Balances", pp 79-80.

        return 9.482 * std::pow(std::abs(DeltaTemp), 1.0 / 3.0) / (7.238 - std::abs(CosineTilt));
    }

    inline Real64 CalcWaltonStableHorizontalOrTilt(Real64 const DeltaTemp, // [C] temperature difference between surface and air
                                                   Real64 const CosineTilt // Cosine of tilt angle
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Aug 2010
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // Calculate the model equation attributed to Walton's TARP program for horizontal
        // and tilted surfaces with reduced, thermally stable natural convection

        // REFERENCES:
        // 1.  Walton, G. N. 1983. Thermal Analysis Research Program (TARP) Reference Manual,
        //     NBSSIR 83-2655, National Bureau of Standards, "Surface Inside Heat Balances", pp 79-80.

        return 1.810 * std::pow(std::abs(DeltaTemp), 1.0 / 3.0) / (1.382 + std::abs(CosineTilt));
    }

    Real64 CalcFisherPedersenCeilDiffuserFloor(EnergyPlusData &state,
                                               Real64 ACH, // [1/hr] air system air change rate
                                               Real64 Tsurf,
                                               Real64 Tair,
                                               Real64 cosTilt,
                                               Real64 humRat,
                                               Real64 height,
                                               bool isWindow = false);

    Real64 CalcFisherPedersenCeilDiffuserCeiling(EnergyPlusData &state,
                                                 Real64 ACH, // [1/hr] air system air change rate
                                                 Real64 Tsurf,
                                                 Real64 Tair,
                                                 Real64 cosTilt,
                                                 Real64 humRat,
                                                 Real64 height,
                                                 bool isWindow = false);

    Real64 CalcFisherPedersenCeilDiffuserWalls(EnergyPlusData &state,
                                               Real64 ACH, // [1/hr] air system air change rate
                                               Real64 Tsurf,
                                               Real64 Tair,
                                               Real64 cosTilt,
                                               Real64 humRat,
                                               Real64 height,
                                               bool isWindow = false);

    Real64 CalcFisherPedersenCeilDiffuserNatConv(
        EnergyPlusData &state, Real64 Hforced, Real64 ACH, Real64 Tsurf, Real64 Tair, Real64 cosTilt, Real64 humRat, Real64 height, bool isWindow);

    Real64 CalcAlamdariHammondUnstableHorizontal(Real64 DeltaTemp,        // [C] temperature difference between surface and air
                                                 Real64 HydraulicDiameter // [m] characteristic size, = (4 * area) / perimeter
    );

    Real64 CalcAlamdariHammondUnstableHorizontal(EnergyPlusData &state,
                                                 Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                 Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                 int SurfNum               // for messages
    );

    Real64 CalcAlamdariHammondStableHorizontal(Real64 DeltaTemp,        // [C] temperature difference between surface and air
                                               Real64 HydraulicDiameter // [m] characteristic size, = (4 * area) / perimeter
    );

    Real64 CalcAlamdariHammondStableHorizontal(EnergyPlusData &state,
                                               Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                               Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                               int SurfNum               // for messages
    );

    Real64 CalcAlamdariHammondVerticalWall(Real64 DeltaTemp, // [C] temperature difference between surface and air
                                           Real64 Height     // [m] characteristic size, = zone height
    );

    Real64 CalcAlamdariHammondVerticalWall(EnergyPlusData &state,
                                           Real64 DeltaTemp, // [C] temperature difference between surface and air
                                           Real64 Height,    // [m] characteristic size, = zone height
                                           int SurfNum       // for messages
    );

    Real64 CalcKhalifaEq3WallAwayFromHeat(Real64 DeltaTemp); // [C] temperature difference between surface and air

    Real64 CalcKhalifaEq4CeilingAwayFromHeat(Real64 DeltaTemp); // [C] temperature difference between surface and air

    Real64 CalcKhalifaEq5WallsNearHeat(Real64 DeltaTemp); // [C] temperature difference between surface and air

    Real64 CalcKhalifaEq6NonHeatedWalls(Real64 DeltaTemp); // [C] temperature difference between surface and air

    Real64 CalcKhalifaEq7Ceiling(Real64 DeltaTemp); // [C] temperature difference between surface and air

    Real64 CalcAwbiHattonHeatedFloor(Real64 DeltaTemp,        // [C] temperature difference between surface and air
                                     Real64 HydraulicDiameter // [m] characteristic size, = (4 * area) / perimeter
    );

    Real64 CalcAwbiHattonHeatedWall(Real64 DeltaTemp,        // [C] temperature difference between surface and air
                                    Real64 HydraulicDiameter // [m] characteristic size, = (4 * area) / perimeter
    );

    Real64 CalcBeausoleilMorrisonMixedAssistedWall(Real64 DeltaTemp,     // [C] temperature difference between surface and air
                                                   Real64 Height,        // [m] characteristic size
                                                   Real64 SurfTemp,      // [C] surface temperature
                                                   Real64 SupplyAirTemp, // [C] temperature of supply air into zone
                                                   Real64 AirChangeRate  // [ACH] [1/hour] supply air ACH for zone
    );

    Real64 CalcBeausoleilMorrisonMixedAssistedWall(EnergyPlusData &state,
                                                   Real64 DeltaTemp, // [C] temperature difference between surface and air
                                                   Real64 Height,    // [m] characteristic size
                                                   Real64 SurfTemp,  // [C] surface temperature
                                                   int ZoneNum       // index of zone for messaging
    );

    Real64 CalcBeausoleilMorrisonMixedOpposingWall(Real64 DeltaTemp,     // [C] temperature difference between surface and air
                                                   Real64 Height,        // [m] characteristic size
                                                   Real64 SurfTemp,      // [C] surface temperature
                                                   Real64 SupplyAirTemp, // [C] temperature of supply air into zone
                                                   Real64 AirChangeRate  // [ACH] [1/hour] supply air ACH for zone
    );

    Real64 CalcBeausoleilMorrisonMixedOpposingWall(EnergyPlusData &state,
                                                   Real64 DeltaTemp, // [C] temperature difference between surface and air
                                                   Real64 Height,    // [m] characteristic size
                                                   Real64 SurfTemp,  // [C] surface temperature
                                                   int ZoneNum       // index of zone for messaging
    );

    Real64 CalcBeausoleilMorrisonMixedStableFloor(Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                  Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                  Real64 SurfTemp,          // [C] surface temperature
                                                  Real64 SupplyAirTemp,     // [C] temperature of supply air into zone
                                                  Real64 AirChangeRate      // [ACH] [1/hour] supply air ACH for zone
    );

    Real64 CalcBeausoleilMorrisonMixedStableFloor(EnergyPlusData &state,
                                                  Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                  Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                  Real64 SurfTemp,          // [C] surface temperature
                                                  int ZoneNum               // index of zone for messaging
    );

    Real64 CalcBeausoleilMorrisonMixedUnstableFloor(Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                    Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                    Real64 SurfTemp,          // [C] surface temperature
                                                    Real64 SupplyAirTemp,     // [C] temperature of supply air into zone
                                                    Real64 AirChangeRate      // [ACH] [1/hour] supply air ACH for zone
    );

    Real64 CalcBeausoleilMorrisonMixedUnstableFloor(EnergyPlusData &state,
                                                    Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                    Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                    Real64 SurfTemp,          // [C] surface temperature
                                                    int ZoneNum               // index of zone for messaging
    );

    Real64 CalcBeausoleilMorrisonMixedStableCeiling(Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                    Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                    Real64 SurfTemp,          // [C] surface temperature
                                                    Real64 SupplyAirTemp,     // [C] temperature of supply air into zone
                                                    Real64 AirChangeRate      // [ACH] [1/hour] supply air ACH for zone
    );

    Real64 CalcBeausoleilMorrisonMixedStableCeiling(EnergyPlusData &state,
                                                    Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                    Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                    Real64 SurfTemp,          // [C] surface temperature
                                                    int ZoneNum               // index of zone for messaging
    );

    Real64 CalcBeausoleilMorrisonMixedUnstableCeiling(Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                      Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                      Real64 SurfTemp,          // [C] surface temperature
                                                      Real64 SupplyAirTemp,     // [C] temperature of supply air into zone
                                                      Real64 AirChangeRate      // [ACH] [1/hour] supply air ACH for zone
    );

    Real64 CalcBeausoleilMorrisonMixedUnstableCeiling(EnergyPlusData &state,
                                                      Real64 DeltaTemp,         // [C] temperature difference between surface and air
                                                      Real64 HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                      Real64 SurfTemp,          // [C] surface temperature
                                                      int ZoneNum               // index of zone for messaging
    );

    Real64 CalcFohannoPolidoriVerticalWall(Real64 DeltaTemp, // [C] temperature difference between surface and air
                                           Real64 Height,    // [m] characteristic size, height of zone
                                           Real64 SurfTemp,  // [C] surface temperature
                                           Real64 QdotConv   // [W/m2] heat flux rate for rayleigh #
    );

    Real64 CallCalcFohannoPolidoriVerticalWall(EnergyPlusData &state,
                                               Real64 DeltaTemp, // [C] temperature difference between surface and air
                                               Real64 Height,    // [m] characteristic size, height of zone
                                               Real64 SurfTemp,  // [C] surface temperature
                                               Real64 QdotConv,  // [W/m2] heat flux rate for rayleigh #
                                               int SurfNum       // for messages
    );

    Real64 CalcKaradagChilledCeiling(Real64 DeltaTemp); // [C] temperature difference between surface and air

    Real64 CalcGoldsteinNovoselacCeilingDiffuserWindow(Real64 AirSystemFlowRate,  // [m3/s] air system flow rate
                                                       Real64 ZoneExtPerimLength, // [m] length of zone perimeter with exterior walls
                                                       Real64 WindWallRatio,      // [ ] fraction of window area to wall area for zone
                                                       IntConvWinLoc WindowLocationType // index for location types
    );

    Real64 CalcGoldsteinNovoselacCeilingDiffuserWindow(EnergyPlusData &state,
                                                       Real64 ZoneExtPerimLength, // [m] length of zone perimeter with exterior walls
                                                       Real64 WindWallRatio,      // [ ] fraction of window area to wall area for zone
                                                       IntConvWinLoc WindowLocationType, // index for location types
                                                       int ZoneNum                                           // for messages
    );

    Real64 CalcGoldsteinNovoselacCeilingDiffuserWall(Real64 AirSystemFlowRate,  // [m3/s] air system flow rate
                                                     Real64 ZoneExtPerimLength, // [m] length of zone perimeter with exterior walls
                                                     IntConvWinLoc WindowLocationType // index for location types
    );

    Real64 CalcGoldsteinNovoselacCeilingDiffuserWall(EnergyPlusData &state,
                                                     Real64 ZoneExtPerimLength, // [m] length of zone perimeter with exterior walls
                                                     IntConvWinLoc WindowLocationType, // index for location types
                                                     int ZoneNum                                           // for messages
    );

    Real64 CalcGoldsteinNovoselacCeilingDiffuserFloor(Real64 AirSystemFlowRate, // [m3/s] air system flow rate
                                                      Real64 ZoneExtPerimLength // [m] length of zone perimeter with exterior walls
    );

    Real64 CalcGoldsteinNovoselacCeilingDiffuserFloor(EnergyPlusData &state,
                                                      Real64 ZoneExtPerimLength, // [m] length of zone perimeter with exterior walls
                                                      int ZoneNum                // for messages
    );

    Real64 CalcSparrowWindward(Material::SurfaceRoughness roughness, Real64 FacePerimeter, Real64 FaceArea, Real64 WindAtZ);

    Real64 CalcSparrowWindward(EnergyPlusData &state, Material::SurfaceRoughness roughness, Real64 FacePerimeter, Real64 FaceArea, Real64 WindAtZ, int SurfNum);

    Real64 CalcSparrowLeeward(Material::SurfaceRoughness roughness, Real64 FacePerimeter, Real64 FaceArea, Real64 WindAtZ);

    Real64 CalcSparrowLeeward(EnergyPlusData &state, Material::SurfaceRoughness roughness, Real64 FacePerimeter, Real64 FaceArea, Real64 WindAtZ, int SurfNum);

    Real64 CalcMoWITTNatural(Real64 DeltaTemp);

    Real64 CalcMoWITTForcedWindward(Real64 WindAtZ);

    Real64 CalcMoWITTForcedLeeward(Real64 WindAtZ);

    Real64 CalcMoWITTWindward(Real64 DeltaTemp, Real64 WindAtZ);

    Real64 CalcMoWITTLeeward(Real64 DeltaTemp, Real64 WindAtZ);

    Real64 CalcDOE2Forced(Real64 SurfaceTemp, Real64 AirTemp, Real64 CosineTilt, Real64 HfSmooth, Material::SurfaceRoughness roughness);

    Real64 CalcDOE2Windward(Real64 SurfaceTemp, Real64 AirTemp, Real64 CosineTilt, Real64 WindAtZ, Material::SurfaceRoughness roughness);

    Real64 CalcDOE2Leeward(Real64 SurfaceTemp, Real64 AirTemp, Real64 CosineTilt, Real64 WindAtZ, Material::SurfaceRoughness roughness);

    Real64 CalcNusseltJurges(Real64 WindAtZ);

    Real64 CalcMcAdams(Real64 WindAtZ);

    Real64 CalcMitchell(Real64 WindAtZ, Real64 LengthScale);

    Real64 CalcMitchell(EnergyPlusData &state, Real64 WindAtZ, Real64 LengthScale, int SurfNum);

    Real64 CalcBlockenWindward(EnergyPlusData &state,
                               Real64 WindAt10m,
                               Real64 WindDir,     // Wind direction measured clockwise from geographic North
                               Real64 SurfAzimuth, // or Facing, Direction the surface outward normal faces (degrees)
                               int SurfNum);

    Real64 CalcWindSurfaceTheta(Real64 const WindDir, Real64 const SurfAzimuth);

    Real64 CalcEmmelVertical(Real64 WindAt10m,
                             Real64 WindDir,      // Wind direction measured clockwise from geographic North
                             Real64 SurfAzimuth); // or Facing, Direction the surface outward normal faces (degrees)

    Real64 CalcEmmelRoof(Real64 WindAt10m,
                         Real64 WindDir,                 // Wind direction measured clockwise from geographic North
                         Real64 LongAxisOutwardAzimuth); // or Facing, Direction the surface outward normal faces (degrees)

    Real64 CalcClearRoof(EnergyPlusData &state,
                         Real64 AirTemp,
                         Real64 WindAtZ,
                         Real64 WindDirect, // Wind direction measured clockwise from geographic North
                         Real64 RoofArea,
                         Real64 RoofPerimeter,
                         Material::SurfaceRoughness roughness);

    Real64 CalcClearRoof(EnergyPlusData &state,
                         int SurfNum,
                         Real64 SurfTemp,
                         Real64 AirTemp,
                         Real64 WindAtZ,
                         Real64 WindDirect, // Wind direction measured clockwise from geographic North
                         Real64 RoofArea,
                         Real64 RoofPerimeter);

    void CalcASTMC1340ConvCoeff(EnergyPlusData &state,
                                int const SurfNum,                  // surface number for which coefficients are being calculated
                                Real64 const SurfaceTemperature,    // Temperature of surface for evaluation of HcIn
                                Real64 const ZoneMeanAirTemperature // Mean Air Temperature of Zone
    );

    Real64
    CalcASTMC1340ConvCoeff(EnergyPlusData &state, int const SurfNum, Real64 const Tsurf, Real64 const Tair, Real64 const Vair, Real64 const Tilt);

    SurfOrientation GetSurfConvOrientation(Real64 const Tilt);

} // namespace Convect

struct ConvectionCoefficientsData : BaseGlobalStruct
{

    bool GetUserSuppliedConvectionCoeffs = true;  // Get user input first call for Init
    Real64 CubeRootOfOverallBuildingVolume = 0.0; // building meta data. cube root of the volume of all the zones
    Real64 RoofLongAxisOutwardAzimuth = 0.0;      // roof surfaces meta data. outward normal azimuth for longest roof edge

    int BMMixedAssistedWallErrorIDX1 = 0;
    int BMMixedAssistedWallErrorIDX2 = 0;
    int BMMixedOpposingWallErrorIDX1 = 0;
    int BMMixedOpposingWallErrorIDX2 = 0;
    int BMMixedStableFloorErrorIDX1 = 0;
    int BMMixedStableFloorErrorIDX2 = 0;
    int BMMixedUnstableFloorErrorIDX1 = 0;
    int BMMixedUnstableFloorErrorIDX2 = 0;
    int BMMixedStableCeilingErrorIDX1 = 0;
    int BMMixedStableCeilingErrorIDX2 = 0;
    int BMMixedUnstableCeilingErrorIDX1 = 0;
    int BMMixedUnstableCeilingErrorIDX2 = 0;
    int AHUnstableHorizontalErrorIDX = 0;
    int AHStableHorizontalErrorIDX = 0;
    int AHVerticalWallErrorIDX = 0;
    int CalcFohannoPolidoriVerticalWallErrorIDX = 0;
    int CalcGoldsteinNovoselacCeilingDiffuserWindowErrorIDX1 = 0;
    int CalcGoldsteinNovoselacCeilingDiffuserWindowErrorIDX2 = 0;
    int CalcGoldsteinNovoselacCeilingDiffuserWallErrorIDX1 = 0;
    int CalcGoldsteinNovoselacCeilingDiffuserWallErrorIDX2 = 0;
    int CalcGoldsteinNovoselacCeilingDiffuserFloorErrorIDX = 0;
    int CalcSparrowWindwardErrorIDX = 0;
    int CalcSparrowLeewardErrorIDX = 0;
    int CalcBlockenWindwardErrorIDX = 0;
    int CalcClearRoofErrorIDX = 0;
    int CalcMitchellErrorIDX = 0;

    // move random statics so they can be reset for unit tests
    bool NodeCheck = true;
    bool ActiveSurfaceCheck = true;
    bool MyEnvirnFlag = true;
    bool FirstRoofSurf = true;
    int ActiveWallCount = 0;
    Real64 ActiveWallArea = 0.0;
    int ActiveCeilingCount = 0;
    Real64 ActiveCeilingArea = 0.0;
    int ActiveFloorCount = 0;
    Real64 ActiveFloorArea = 0.0;

    // Object Data
    Convect::IntAdaptiveConvAlgoStruct IntAdaptiveConvAlgo; // stores rules for Hc model equations
    Convect::ExtAdaptiveConvAlgoStruct ExtAdaptiveConvAlgo;
    Array1D<Convect::HcIntUserCurveStruct> HcIntUserCurve;
    Array1D<Convect::HcExtUserCurveStruct> HcExtUserCurve;
    Convect::RoofGeoCharacteristicsStruct RoofGeo;

    Convect::FacadeGeoCharacteristicsStruct NorthFacade = {332.5, 22.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Convect::FacadeGeoCharacteristicsStruct NorthEastFacade = {22.5, 67.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Convect::FacadeGeoCharacteristicsStruct EastFacade = {67.5, 112.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Convect::FacadeGeoCharacteristicsStruct SouthEastFacade = {112.5, 157.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Convect::FacadeGeoCharacteristicsStruct SouthFacade = {157.5, 202.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Convect::FacadeGeoCharacteristicsStruct SouthWestFacade = {202.5, 247.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Convect::FacadeGeoCharacteristicsStruct WestFacade = {247.5, 287.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Convect::FacadeGeoCharacteristicsStruct NorthWestFacade = {287.5, 332.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    void clear_state() override
    {
       new (this) ConvectionCoefficientsData();
    }
};

} // namespace EnergyPlus

#endif
