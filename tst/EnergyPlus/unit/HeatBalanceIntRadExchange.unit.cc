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

// EnergyPlus::HeatBalFiniteDiffManager Unit Tests

// Google Test Headers
#include <gtest/gtest.h>

// Eigen Headers
#include <Eigen/Dense>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/SimulationManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputFiles.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ElectricPowerServiceManager.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus::HeatBalanceIntRadExchange;

namespace EnergyPlus {

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_CalcInteriorRadExchange)
{
    std::string const idf_objects = delimited_string({
         "Zone,",
         "  Space 0 ZN,                             !- Name",
         "  ,                                       !- Direction of Relative North {deg}",
         "  ,                                       !- X Origin {m}",
         "  ,                                       !- Y Origin {m}",
         "  ,                                       !- Z Origin {m}",
         "  ,                                       !- Type",
         "  ,                                       !- Multiplier",
         "  ,                                       !- Ceiling Height {m}",
         "  ,                                       !- Volume {m3}",
         "  ,                                       !- Floor Area {m2}",
         "  ,                                       !- Zone Inside Convection Algorithm",
         "  ;                                       !- Zone Outside Convection Algorithm",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 1,                              !- Name",
         "  Floor,                                  !- Surface Type",
         "  Typical Insulated Carpeted 6in Slab Floor, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Ground,                                 !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  NoSun,                                  !- Sun Exposure",
         "  NoWind,                                 !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  101.900081235481, -2.49583216635773e-14, 0, !- X,Y,Z Vertex 1 {m}",
         "  101.409404492206, -9.98795456205172, 0, !- X,Y,Z Vertex 2 {m}",
         "  99.9420997476528, -19.8797196616996, 0, !- X,Y,Z Vertex 3 {m}",
         "  97.5122979486201, -29.580032193645, 0,  !- X,Y,Z Vertex 4 {m}",
         "  94.1433994146979, -38.9954728454752, 0, !- X,Y,Z Vertex 5 {m}",
         "  89.8678484803951, -48.0353657767096, 0, !- X,Y,Z Vertex 6 {m}",
         "  84.7268210384629, -56.6126518767123, 0, !- X,Y,Z Vertex 7 {m}",
         "  78.7698279935385, -64.6447271915188, 0, !- X,Y,Z Vertex 8 {m}",
         "  72.0542384450684, -72.0542384450684, 0, !- X,Y,Z Vertex 9 {m}",
         "  64.6447271915188, -78.7698279935386, 0, !- X,Y,Z Vertex 10 {m}",
         "  56.6126518767123, -84.7268210384629, 0, !- X,Y,Z Vertex 11 {m}",
         "  48.0353657767096, -89.8678484803951, 0, !- X,Y,Z Vertex 12 {m}",
         "  38.9954728454752, -94.1433994146979, 0, !- X,Y,Z Vertex 13 {m}",
         "  29.5800321936449, -97.5122979486201, 0, !- X,Y,Z Vertex 14 {m}",
         "  19.8797196616995, -99.9420997476528, 0, !- X,Y,Z Vertex 15 {m}",
         "  9.98795456205167, -101.409404492206, 0, !- X,Y,Z Vertex 16 {m}",
         "  -1.8718741247683e-14, -101.900081235481, 0, !- X,Y,Z Vertex 17 {m}",
         "  -9.98795456205171, -101.409404492206, 0, !- X,Y,Z Vertex 18 {m}",
         "  -19.8797196616996, -99.9420997476528, 0, !- X,Y,Z Vertex 19 {m}",
         "  -29.580032193645, -97.5122979486201, 0, !- X,Y,Z Vertex 20 {m}",
         "  -38.9954728454752, -94.1433994146979, 0, !- X,Y,Z Vertex 21 {m}",
         "  -48.0353657767096, -89.8678484803951, 0, !- X,Y,Z Vertex 22 {m}",
         "  -56.6126518767123, -84.7268210384629, 0, !- X,Y,Z Vertex 23 {m}",
         "  -64.6447271915188, -78.7698279935385, 0, !- X,Y,Z Vertex 24 {m}",
         "  -72.0542384450684, -72.0542384450684, 0, !- X,Y,Z Vertex 25 {m}",
         "  -78.7698279935386, -64.6447271915188, 0, !- X,Y,Z Vertex 26 {m}",
         "  -84.7268210384629, -56.6126518767123, 0, !- X,Y,Z Vertex 27 {m}",
         "  -89.8678484803951, -48.0353657767096, 0, !- X,Y,Z Vertex 28 {m}",
         "  -94.1433994146979, -38.9954728454752, 0, !- X,Y,Z Vertex 29 {m}",
         "  -97.5122979486201, -29.580032193645, 0, !- X,Y,Z Vertex 30 {m}",
         "  -99.9420997476528, -19.8797196616995, 0, !- X,Y,Z Vertex 31 {m}",
         "  -101.409404492206, -9.98795456205172, 0, !- X,Y,Z Vertex 32 {m}",
         "  -101.900081235481, 1.24791608317887e-14, 0, !- X,Y,Z Vertex 33 {m}",
         "  -101.409404492206, 9.98795456205175, 0, !- X,Y,Z Vertex 34 {m}",
         "  -99.9420997476528, 19.8797196616996, 0, !- X,Y,Z Vertex 35 {m}",
         "  -97.5122979486201, 29.580032193645, 0,  !- X,Y,Z Vertex 36 {m}",
         "  -94.1433994146979, 38.9954728454752, 0, !- X,Y,Z Vertex 37 {m}",
         "  -89.8678484803951, 48.0353657767096, 0, !- X,Y,Z Vertex 38 {m}",
         "  -84.7268210384629, 56.6126518767123, 0, !- X,Y,Z Vertex 39 {m}",
         "  -78.7698279935386, 64.6447271915188, 0, !- X,Y,Z Vertex 40 {m}",
         "  -72.0542384450684, 72.0542384450684, 0, !- X,Y,Z Vertex 41 {m}",
         "  -64.6447271915188, 78.7698279935386, 0, !- X,Y,Z Vertex 42 {m}",
         "  -56.6126518767123, 84.7268210384629, 0, !- X,Y,Z Vertex 43 {m}",
         "  -48.0353657767096, 89.8678484803951, 0, !- X,Y,Z Vertex 44 {m}",
         "  -38.9954728454752, 94.1433994146979, 0, !- X,Y,Z Vertex 45 {m}",
         "  -29.580032193645, 97.5122979486201, 0,  !- X,Y,Z Vertex 46 {m}",
         "  -19.8797196616995, 99.9420997476528, 0, !- X,Y,Z Vertex 47 {m}",
         "  -9.98795456205173, 101.409404492206, 0, !- X,Y,Z Vertex 48 {m}",
         "  6.23958041589433e-15, 101.900081235481, 0, !- X,Y,Z Vertex 49 {m}",
         "  9.98795456205174, 101.409404492206, 0,  !- X,Y,Z Vertex 50 {m}",
         "  19.8797196616995, 99.9420997476528, 0,  !- X,Y,Z Vertex 51 {m}",
         "  29.580032193645, 97.5122979486201, 0,   !- X,Y,Z Vertex 52 {m}",
         "  38.9954728454752, 94.1433994146979, 0,  !- X,Y,Z Vertex 53 {m}",
         "  48.0353657767096, 89.8678484803951, 0,  !- X,Y,Z Vertex 54 {m}",
         "  56.6126518767123, 84.7268210384629, 0,  !- X,Y,Z Vertex 55 {m}",
         "  64.6447271915188, 78.7698279935386, 0,  !- X,Y,Z Vertex 56 {m}",
         "  72.0542384450684, 72.0542384450684, 0,  !- X,Y,Z Vertex 57 {m}",
         "  78.7698279935386, 64.6447271915188, 0,  !- X,Y,Z Vertex 58 {m}",
         "  84.7268210384629, 56.6126518767123, 0,  !- X,Y,Z Vertex 59 {m}",
         "  89.8678484803951, 48.0353657767096, 0,  !- X,Y,Z Vertex 60 {m}",
         "  94.1433994146979, 38.9954728454752, 0,  !- X,Y,Z Vertex 61 {m}",
         "  97.5122979486201, 29.580032193645, 0,   !- X,Y,Z Vertex 62 {m}",
         "  99.9420997476528, 19.8797196616995, 0,  !- X,Y,Z Vertex 63 {m}",
         "  101.409404492206, 9.98795456205172, 0;  !- X,Y,Z Vertex 64 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 10,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  64.6447271915188, -78.7698279935386, 12, !- X,Y,Z Vertex 1 {m}",
         "  64.6447271915188, -78.7698279935386, 0, !- X,Y,Z Vertex 2 {m}",
         "  72.0542384450684, -72.0542384450684, 0, !- X,Y,Z Vertex 3 {m}",
         "  72.0542384450684, -72.0542384450684, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 11,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  56.6126518767123, -84.7268210384629, 12, !- X,Y,Z Vertex 1 {m}",
         "  56.6126518767123, -84.7268210384629, 0, !- X,Y,Z Vertex 2 {m}",
         "  64.6447271915188, -78.7698279935386, 0, !- X,Y,Z Vertex 3 {m}",
         "  64.6447271915188, -78.7698279935386, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 12,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  48.0353657767096, -89.8678484803951, 12, !- X,Y,Z Vertex 1 {m}",
         "  48.0353657767096, -89.8678484803951, 0, !- X,Y,Z Vertex 2 {m}",
         "  56.6126518767123, -84.7268210384629, 0, !- X,Y,Z Vertex 3 {m}",
         "  56.6126518767123, -84.7268210384629, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 13,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  38.9954728454752, -94.1433994146979, 12, !- X,Y,Z Vertex 1 {m}",
         "  38.9954728454752, -94.1433994146979, 0, !- X,Y,Z Vertex 2 {m}",
         "  48.0353657767096, -89.8678484803951, 0, !- X,Y,Z Vertex 3 {m}",
         "  48.0353657767096, -89.8678484803951, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 14,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  29.5800321936449, -97.5122979486201, 12, !- X,Y,Z Vertex 1 {m}",
         "  29.5800321936449, -97.5122979486201, 0, !- X,Y,Z Vertex 2 {m}",
         "  38.9954728454752, -94.1433994146979, 0, !- X,Y,Z Vertex 3 {m}",
         "  38.9954728454752, -94.1433994146979, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 15,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  19.8797196616995, -99.9420997476528, 12, !- X,Y,Z Vertex 1 {m}",
         "  19.8797196616995, -99.9420997476528, 0, !- X,Y,Z Vertex 2 {m}",
         "  29.5800321936449, -97.5122979486201, 0, !- X,Y,Z Vertex 3 {m}",
         "  29.5800321936449, -97.5122979486201, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 16,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  9.98795456205167, -101.409404492206, 12, !- X,Y,Z Vertex 1 {m}",
         "  9.98795456205167, -101.409404492206, 0, !- X,Y,Z Vertex 2 {m}",
         "  19.8797196616995, -99.9420997476528, 0, !- X,Y,Z Vertex 3 {m}",
         "  19.8797196616995, -99.9420997476528, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 17,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -1.8718741247683e-14, -101.900081235481, 12, !- X,Y,Z Vertex 1 {m}",
         "  -1.8718741247683e-14, -101.900081235481, 0, !- X,Y,Z Vertex 2 {m}",
         "  9.98795456205167, -101.409404492206, 0, !- X,Y,Z Vertex 3 {m}",
         "  9.98795456205167, -101.409404492206, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 18,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -9.98795456205171, -101.409404492206, 12, !- X,Y,Z Vertex 1 {m}",
         "  -9.98795456205171, -101.409404492206, 0, !- X,Y,Z Vertex 2 {m}",
         "  -1.8718741247683e-14, -101.900081235481, 0, !- X,Y,Z Vertex 3 {m}",
         "  -1.8718741247683e-14, -101.900081235481, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 19,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -19.8797196616996, -99.9420997476528, 12, !- X,Y,Z Vertex 1 {m}",
         "  -19.8797196616996, -99.9420997476528, 0, !- X,Y,Z Vertex 2 {m}",
         "  -9.98795456205171, -101.409404492206, 0, !- X,Y,Z Vertex 3 {m}",
         "  -9.98795456205171, -101.409404492206, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 2,                              !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  101.409404492206, -9.98795456205172, 12, !- X,Y,Z Vertex 1 {m}",
         "  101.409404492206, -9.98795456205172, 0, !- X,Y,Z Vertex 2 {m}",
         "  101.900081235481, -2.49583216635773e-14, 0, !- X,Y,Z Vertex 3 {m}",
         "  101.900081235481, -2.49583216635773e-14, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 20,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -29.580032193645, -97.5122979486201, 12, !- X,Y,Z Vertex 1 {m}",
         "  -29.580032193645, -97.5122979486201, 0, !- X,Y,Z Vertex 2 {m}",
         "  -19.8797196616996, -99.9420997476528, 0, !- X,Y,Z Vertex 3 {m}",
         "  -19.8797196616996, -99.9420997476528, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 21,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -38.9954728454752, -94.1433994146979, 12, !- X,Y,Z Vertex 1 {m}",
         "  -38.9954728454752, -94.1433994146979, 0, !- X,Y,Z Vertex 2 {m}",
         "  -29.580032193645, -97.5122979486201, 0, !- X,Y,Z Vertex 3 {m}",
         "  -29.580032193645, -97.5122979486201, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 22,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -48.0353657767096, -89.8678484803951, 12, !- X,Y,Z Vertex 1 {m}",
         "  -48.0353657767096, -89.8678484803951, 0, !- X,Y,Z Vertex 2 {m}",
         "  -38.9954728454752, -94.1433994146979, 0, !- X,Y,Z Vertex 3 {m}",
         "  -38.9954728454752, -94.1433994146979, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 23,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -56.6126518767123, -84.7268210384629, 12, !- X,Y,Z Vertex 1 {m}",
         "  -56.6126518767123, -84.7268210384629, 0, !- X,Y,Z Vertex 2 {m}",
         "  -48.0353657767096, -89.8678484803951, 0, !- X,Y,Z Vertex 3 {m}",
         "  -48.0353657767096, -89.8678484803951, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 24,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -64.6447271915188, -78.7698279935385, 12, !- X,Y,Z Vertex 1 {m}",
         "  -64.6447271915188, -78.7698279935385, 0, !- X,Y,Z Vertex 2 {m}",
         "  -56.6126518767123, -84.7268210384629, 0, !- X,Y,Z Vertex 3 {m}",
         "  -56.6126518767123, -84.7268210384629, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 25,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -72.0542384450684, -72.0542384450684, 12, !- X,Y,Z Vertex 1 {m}",
         "  -72.0542384450684, -72.0542384450684, 0, !- X,Y,Z Vertex 2 {m}",
         "  -64.6447271915188, -78.7698279935385, 0, !- X,Y,Z Vertex 3 {m}",
         "  -64.6447271915188, -78.7698279935385, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 26,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -78.7698279935386, -64.6447271915188, 12, !- X,Y,Z Vertex 1 {m}",
         "  -78.7698279935386, -64.6447271915188, 0, !- X,Y,Z Vertex 2 {m}",
         "  -72.0542384450684, -72.0542384450684, 0, !- X,Y,Z Vertex 3 {m}",
         "  -72.0542384450684, -72.0542384450684, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 27,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -84.7268210384629, -56.6126518767123, 12, !- X,Y,Z Vertex 1 {m}",
         "  -84.7268210384629, -56.6126518767123, 0, !- X,Y,Z Vertex 2 {m}",
         "  -78.7698279935386, -64.6447271915188, 0, !- X,Y,Z Vertex 3 {m}",
         "  -78.7698279935386, -64.6447271915188, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 28,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -89.8678484803951, -48.0353657767096, 12, !- X,Y,Z Vertex 1 {m}",
         "  -89.8678484803951, -48.0353657767096, 0, !- X,Y,Z Vertex 2 {m}",
         "  -84.7268210384629, -56.6126518767123, 0, !- X,Y,Z Vertex 3 {m}",
         "  -84.7268210384629, -56.6126518767123, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 29,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -94.1433994146979, -38.9954728454752, 12, !- X,Y,Z Vertex 1 {m}",
         "  -94.1433994146979, -38.9954728454752, 0, !- X,Y,Z Vertex 2 {m}",
         "  -89.8678484803951, -48.0353657767096, 0, !- X,Y,Z Vertex 3 {m}",
         "  -89.8678484803951, -48.0353657767096, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 3,                              !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  99.9420997476528, -19.8797196616996, 12, !- X,Y,Z Vertex 1 {m}",
         "  99.9420997476528, -19.8797196616996, 0, !- X,Y,Z Vertex 2 {m}",
         "  101.409404492206, -9.98795456205172, 0, !- X,Y,Z Vertex 3 {m}",
         "  101.409404492206, -9.98795456205172, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 30,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -97.5122979486201, -29.580032193645, 12, !- X,Y,Z Vertex 1 {m}",
         "  -97.5122979486201, -29.580032193645, 0, !- X,Y,Z Vertex 2 {m}",
         "  -94.1433994146979, -38.9954728454752, 0, !- X,Y,Z Vertex 3 {m}",
         "  -94.1433994146979, -38.9954728454752, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 31,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -99.9420997476528, -19.8797196616995, 12, !- X,Y,Z Vertex 1 {m}",
         "  -99.9420997476528, -19.8797196616995, 0, !- X,Y,Z Vertex 2 {m}",
         "  -97.5122979486201, -29.580032193645, 0, !- X,Y,Z Vertex 3 {m}",
         "  -97.5122979486201, -29.580032193645, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 32,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -101.409404492206, -9.98795456205172, 12, !- X,Y,Z Vertex 1 {m}",
         "  -101.409404492206, -9.98795456205172, 0, !- X,Y,Z Vertex 2 {m}",
         "  -99.9420997476528, -19.8797196616995, 0, !- X,Y,Z Vertex 3 {m}",
         "  -99.9420997476528, -19.8797196616995, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 33,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -101.900081235481, 1.24791608317887e-14, 12, !- X,Y,Z Vertex 1 {m}",
         "  -101.900081235481, 1.24791608317887e-14, 0, !- X,Y,Z Vertex 2 {m}",
         "  -101.409404492206, -9.98795456205172, 0, !- X,Y,Z Vertex 3 {m}",
         "  -101.409404492206, -9.98795456205172, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 34,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -101.409404492206, 9.98795456205175, 12, !- X,Y,Z Vertex 1 {m}",
         "  -101.409404492206, 9.98795456205175, 0, !- X,Y,Z Vertex 2 {m}",
         "  -101.900081235481, 1.24791608317887e-14, 0, !- X,Y,Z Vertex 3 {m}",
         "  -101.900081235481, 1.24791608317887e-14, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 35,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -99.9420997476528, 19.8797196616996, 12, !- X,Y,Z Vertex 1 {m}",
         "  -99.9420997476528, 19.8797196616996, 0, !- X,Y,Z Vertex 2 {m}",
         "  -101.409404492206, 9.98795456205175, 0, !- X,Y,Z Vertex 3 {m}",
         "  -101.409404492206, 9.98795456205175, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 36,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -97.5122979486201, 29.580032193645, 12, !- X,Y,Z Vertex 1 {m}",
         "  -97.5122979486201, 29.580032193645, 0,  !- X,Y,Z Vertex 2 {m}",
         "  -99.9420997476528, 19.8797196616996, 0, !- X,Y,Z Vertex 3 {m}",
         "  -99.9420997476528, 19.8797196616996, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 37,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -94.1433994146979, 38.9954728454752, 12, !- X,Y,Z Vertex 1 {m}",
         "  -94.1433994146979, 38.9954728454752, 0, !- X,Y,Z Vertex 2 {m}",
         "  -97.5122979486201, 29.580032193645, 0,  !- X,Y,Z Vertex 3 {m}",
         "  -97.5122979486201, 29.580032193645, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 38,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -89.8678484803951, 48.0353657767096, 12, !- X,Y,Z Vertex 1 {m}",
         "  -89.8678484803951, 48.0353657767096, 0, !- X,Y,Z Vertex 2 {m}",
         "  -94.1433994146979, 38.9954728454752, 0, !- X,Y,Z Vertex 3 {m}",
         "  -94.1433994146979, 38.9954728454752, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 39,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -84.7268210384629, 56.6126518767123, 12, !- X,Y,Z Vertex 1 {m}",
         "  -84.7268210384629, 56.6126518767123, 0, !- X,Y,Z Vertex 2 {m}",
         "  -89.8678484803951, 48.0353657767096, 0, !- X,Y,Z Vertex 3 {m}",
         "  -89.8678484803951, 48.0353657767096, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 4,                              !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  97.5122979486201, -29.580032193645, 12, !- X,Y,Z Vertex 1 {m}",
         "  97.5122979486201, -29.580032193645, 0,  !- X,Y,Z Vertex 2 {m}",
         "  99.9420997476528, -19.8797196616996, 0, !- X,Y,Z Vertex 3 {m}",
         "  99.9420997476528, -19.8797196616996, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 40,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -78.7698279935386, 64.6447271915188, 12, !- X,Y,Z Vertex 1 {m}",
         "  -78.7698279935386, 64.6447271915188, 0, !- X,Y,Z Vertex 2 {m}",
         "  -84.7268210384629, 56.6126518767123, 0, !- X,Y,Z Vertex 3 {m}",
         "  -84.7268210384629, 56.6126518767123, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 41,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -72.0542384450684, 72.0542384450684, 12, !- X,Y,Z Vertex 1 {m}",
         "  -72.0542384450684, 72.0542384450684, 0, !- X,Y,Z Vertex 2 {m}",
         "  -78.7698279935386, 64.6447271915188, 0, !- X,Y,Z Vertex 3 {m}",
         "  -78.7698279935386, 64.6447271915188, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 42,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -64.6447271915188, 78.7698279935386, 12, !- X,Y,Z Vertex 1 {m}",
         "  -64.6447271915188, 78.7698279935386, 0, !- X,Y,Z Vertex 2 {m}",
         "  -72.0542384450684, 72.0542384450684, 0, !- X,Y,Z Vertex 3 {m}",
         "  -72.0542384450684, 72.0542384450684, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 43,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -56.6126518767123, 84.7268210384629, 12, !- X,Y,Z Vertex 1 {m}",
         "  -56.6126518767123, 84.7268210384629, 0, !- X,Y,Z Vertex 2 {m}",
         "  -64.6447271915188, 78.7698279935386, 0, !- X,Y,Z Vertex 3 {m}",
         "  -64.6447271915188, 78.7698279935386, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 44,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -48.0353657767096, 89.8678484803951, 12, !- X,Y,Z Vertex 1 {m}",
         "  -48.0353657767096, 89.8678484803951, 0, !- X,Y,Z Vertex 2 {m}",
         "  -56.6126518767123, 84.7268210384629, 0, !- X,Y,Z Vertex 3 {m}",
         "  -56.6126518767123, 84.7268210384629, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 45,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -38.9954728454752, 94.1433994146979, 12, !- X,Y,Z Vertex 1 {m}",
         "  -38.9954728454752, 94.1433994146979, 0, !- X,Y,Z Vertex 2 {m}",
         "  -48.0353657767096, 89.8678484803951, 0, !- X,Y,Z Vertex 3 {m}",
         "  -48.0353657767096, 89.8678484803951, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 46,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -29.580032193645, 97.5122979486201, 12, !- X,Y,Z Vertex 1 {m}",
         "  -29.580032193645, 97.5122979486201, 0,  !- X,Y,Z Vertex 2 {m}",
         "  -38.9954728454752, 94.1433994146979, 0, !- X,Y,Z Vertex 3 {m}",
         "  -38.9954728454752, 94.1433994146979, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 47,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -19.8797196616995, 99.9420997476528, 12, !- X,Y,Z Vertex 1 {m}",
         "  -19.8797196616995, 99.9420997476528, 0, !- X,Y,Z Vertex 2 {m}",
         "  -29.580032193645, 97.5122979486201, 0,  !- X,Y,Z Vertex 3 {m}",
         "  -29.580032193645, 97.5122979486201, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 48,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  -9.98795456205173, 101.409404492206, 12, !- X,Y,Z Vertex 1 {m}",
         "  -9.98795456205173, 101.409404492206, 0, !- X,Y,Z Vertex 2 {m}",
         "  -19.8797196616995, 99.9420997476528, 0, !- X,Y,Z Vertex 3 {m}",
         "  -19.8797196616995, 99.9420997476528, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 49,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  6.23958041589433e-15, 101.900081235481, 12, !- X,Y,Z Vertex 1 {m}",
         "  6.23958041589433e-15, 101.900081235481, 0, !- X,Y,Z Vertex 2 {m}",
         "  -9.98795456205173, 101.409404492206, 0, !- X,Y,Z Vertex 3 {m}",
         "  -9.98795456205173, 101.409404492206, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 5,                              !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  94.1433994146979, -38.9954728454752, 12, !- X,Y,Z Vertex 1 {m}",
         "  94.1433994146979, -38.9954728454752, 0, !- X,Y,Z Vertex 2 {m}",
         "  97.5122979486201, -29.580032193645, 0,  !- X,Y,Z Vertex 3 {m}",
         "  97.5122979486201, -29.580032193645, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 50,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  9.98795456205174, 101.409404492206, 12, !- X,Y,Z Vertex 1 {m}",
         "  9.98795456205174, 101.409404492206, 0,  !- X,Y,Z Vertex 2 {m}",
         "  6.23958041589433e-15, 101.900081235481, 0, !- X,Y,Z Vertex 3 {m}",
         "  6.23958041589433e-15, 101.900081235481, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 51,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  19.8797196616995, 99.9420997476528, 12, !- X,Y,Z Vertex 1 {m}",
         "  19.8797196616995, 99.9420997476528, 0,  !- X,Y,Z Vertex 2 {m}",
         "  9.98795456205174, 101.409404492206, 0,  !- X,Y,Z Vertex 3 {m}",
         "  9.98795456205174, 101.409404492206, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 52,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  29.580032193645, 97.5122979486201, 12,  !- X,Y,Z Vertex 1 {m}",
         "  29.580032193645, 97.5122979486201, 0,   !- X,Y,Z Vertex 2 {m}",
         "  19.8797196616995, 99.9420997476528, 0,  !- X,Y,Z Vertex 3 {m}",
         "  19.8797196616995, 99.9420997476528, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 53,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  38.9954728454752, 94.1433994146979, 12, !- X,Y,Z Vertex 1 {m}",
         "  38.9954728454752, 94.1433994146979, 0,  !- X,Y,Z Vertex 2 {m}",
         "  29.580032193645, 97.5122979486201, 0,   !- X,Y,Z Vertex 3 {m}",
         "  29.580032193645, 97.5122979486201, 12;  !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 54,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  48.0353657767096, 89.8678484803951, 12, !- X,Y,Z Vertex 1 {m}",
         "  48.0353657767096, 89.8678484803951, 0,  !- X,Y,Z Vertex 2 {m}",
         "  38.9954728454752, 94.1433994146979, 0,  !- X,Y,Z Vertex 3 {m}",
         "  38.9954728454752, 94.1433994146979, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 55,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  56.6126518767123, 84.7268210384629, 12, !- X,Y,Z Vertex 1 {m}",
         "  56.6126518767123, 84.7268210384629, 0,  !- X,Y,Z Vertex 2 {m}",
         "  48.0353657767096, 89.8678484803951, 0,  !- X,Y,Z Vertex 3 {m}",
         "  48.0353657767096, 89.8678484803951, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 56,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  64.6447271915188, 78.7698279935386, 12, !- X,Y,Z Vertex 1 {m}",
         "  64.6447271915188, 78.7698279935386, 0,  !- X,Y,Z Vertex 2 {m}",
         "  56.6126518767123, 84.7268210384629, 0,  !- X,Y,Z Vertex 3 {m}",
         "  56.6126518767123, 84.7268210384629, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 57,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  72.0542384450684, 72.0542384450684, 12, !- X,Y,Z Vertex 1 {m}",
         "  72.0542384450684, 72.0542384450684, 0,  !- X,Y,Z Vertex 2 {m}",
         "  64.6447271915188, 78.7698279935386, 0,  !- X,Y,Z Vertex 3 {m}",
         "  64.6447271915188, 78.7698279935386, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 58,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  78.7698279935386, 64.6447271915188, 12, !- X,Y,Z Vertex 1 {m}",
         "  78.7698279935386, 64.6447271915188, 0,  !- X,Y,Z Vertex 2 {m}",
         "  72.0542384450684, 72.0542384450684, 0,  !- X,Y,Z Vertex 3 {m}",
         "  72.0542384450684, 72.0542384450684, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 59,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  84.7268210384629, 56.6126518767123, 12, !- X,Y,Z Vertex 1 {m}",
         "  84.7268210384629, 56.6126518767123, 0,  !- X,Y,Z Vertex 2 {m}",
         "  78.7698279935386, 64.6447271915188, 0,  !- X,Y,Z Vertex 3 {m}",
         "  78.7698279935386, 64.6447271915188, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 6,                              !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  89.8678484803951, -48.0353657767096, 12, !- X,Y,Z Vertex 1 {m}",
         "  89.8678484803951, -48.0353657767096, 0, !- X,Y,Z Vertex 2 {m}",
         "  94.1433994146979, -38.9954728454752, 0, !- X,Y,Z Vertex 3 {m}",
         "  94.1433994146979, -38.9954728454752, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 60,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  89.8678484803951, 48.0353657767096, 12, !- X,Y,Z Vertex 1 {m}",
         "  89.8678484803951, 48.0353657767096, 0,  !- X,Y,Z Vertex 2 {m}",
         "  84.7268210384629, 56.6126518767123, 0,  !- X,Y,Z Vertex 3 {m}",
         "  84.7268210384629, 56.6126518767123, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 61,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  94.1433994146979, 38.9954728454752, 12, !- X,Y,Z Vertex 1 {m}",
         "  94.1433994146979, 38.9954728454752, 0,  !- X,Y,Z Vertex 2 {m}",
         "  89.8678484803951, 48.0353657767096, 0,  !- X,Y,Z Vertex 3 {m}",
         "  89.8678484803951, 48.0353657767096, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 62,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  97.5122979486201, 29.580032193645, 12,  !- X,Y,Z Vertex 1 {m}",
         "  97.5122979486201, 29.580032193645, 0,   !- X,Y,Z Vertex 2 {m}",
         "  94.1433994146979, 38.9954728454752, 0,  !- X,Y,Z Vertex 3 {m}",
         "  94.1433994146979, 38.9954728454752, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 63,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  99.9420997476528, 19.8797196616995, 12, !- X,Y,Z Vertex 1 {m}",
         "  99.9420997476528, 19.8797196616995, 0,  !- X,Y,Z Vertex 2 {m}",
         "  97.5122979486201, 29.580032193645, 0,   !- X,Y,Z Vertex 3 {m}",
         "  97.5122979486201, 29.580032193645, 12;  !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 64,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  101.409404492206, 9.98795456205172, 12, !- X,Y,Z Vertex 1 {m}",
         "  101.409404492206, 9.98795456205172, 0,  !- X,Y,Z Vertex 2 {m}",
         "  99.9420997476528, 19.8797196616995, 0,  !- X,Y,Z Vertex 3 {m}",
         "  99.9420997476528, 19.8797196616995, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 65,                             !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  101.900081235481, -2.49583216635773e-14, 12, !- X,Y,Z Vertex 1 {m}",
         "  101.900081235481, -2.49583216635773e-14, 0, !- X,Y,Z Vertex 2 {m}",
         "  101.409404492206, 9.98795456205172, 0,  !- X,Y,Z Vertex 3 {m}",
         "  101.409404492206, 9.98795456205172, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 66,                             !- Name",
         "  Roof,                                   !- Surface Type",
         "  Typical IEAD Roof R-20.83,              !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  101.409404492206, 9.98795456205172, 12, !- X,Y,Z Vertex 1 {m}",
         "  99.9420997476528, 19.8797196616995, 12, !- X,Y,Z Vertex 2 {m}",
         "  97.5122979486201, 29.580032193645, 12,  !- X,Y,Z Vertex 3 {m}",
         "  94.1433994146979, 38.9954728454752, 12, !- X,Y,Z Vertex 4 {m}",
         "  89.8678484803951, 48.0353657767096, 12, !- X,Y,Z Vertex 5 {m}",
         "  84.7268210384629, 56.6126518767123, 12, !- X,Y,Z Vertex 6 {m}",
         "  78.7698279935386, 64.6447271915188, 12, !- X,Y,Z Vertex 7 {m}",
         "  72.0542384450684, 72.0542384450684, 12, !- X,Y,Z Vertex 8 {m}",
         "  64.6447271915188, 78.7698279935386, 12, !- X,Y,Z Vertex 9 {m}",
         "  56.6126518767123, 84.7268210384629, 12, !- X,Y,Z Vertex 10 {m}",
         "  48.0353657767096, 89.8678484803951, 12, !- X,Y,Z Vertex 11 {m}",
         "  38.9954728454752, 94.1433994146979, 12, !- X,Y,Z Vertex 12 {m}",
         "  29.580032193645, 97.5122979486201, 12,  !- X,Y,Z Vertex 13 {m}",
         "  19.8797196616995, 99.9420997476528, 12, !- X,Y,Z Vertex 14 {m}",
         "  9.98795456205174, 101.409404492206, 12, !- X,Y,Z Vertex 15 {m}",
         "  6.23958041589433e-15, 101.900081235481, 12, !- X,Y,Z Vertex 16 {m}",
         "  -9.98795456205173, 101.409404492206, 12, !- X,Y,Z Vertex 17 {m}",
         "  -19.8797196616995, 99.9420997476528, 12, !- X,Y,Z Vertex 18 {m}",
         "  -29.580032193645, 97.5122979486201, 12, !- X,Y,Z Vertex 19 {m}",
         "  -38.9954728454752, 94.1433994146979, 12, !- X,Y,Z Vertex 20 {m}",
         "  -48.0353657767096, 89.8678484803951, 12, !- X,Y,Z Vertex 21 {m}",
         "  -56.6126518767123, 84.7268210384629, 12, !- X,Y,Z Vertex 22 {m}",
         "  -64.6447271915188, 78.7698279935386, 12, !- X,Y,Z Vertex 23 {m}",
         "  -72.0542384450684, 72.0542384450684, 12, !- X,Y,Z Vertex 24 {m}",
         "  -78.7698279935386, 64.6447271915188, 12, !- X,Y,Z Vertex 25 {m}",
         "  -84.7268210384629, 56.6126518767123, 12, !- X,Y,Z Vertex 26 {m}",
         "  -89.8678484803951, 48.0353657767096, 12, !- X,Y,Z Vertex 27 {m}",
         "  -94.1433994146979, 38.9954728454752, 12, !- X,Y,Z Vertex 28 {m}",
         "  -97.5122979486201, 29.580032193645, 12, !- X,Y,Z Vertex 29 {m}",
         "  -99.9420997476528, 19.8797196616996, 12, !- X,Y,Z Vertex 30 {m}",
         "  -101.409404492206, 9.98795456205175, 12, !- X,Y,Z Vertex 31 {m}",
         "  -101.900081235481, 1.24791608317887e-14, 12, !- X,Y,Z Vertex 32 {m}",
         "  -101.409404492206, -9.98795456205172, 12, !- X,Y,Z Vertex 33 {m}",
         "  -99.9420997476528, -19.8797196616995, 12, !- X,Y,Z Vertex 34 {m}",
         "  -97.5122979486201, -29.580032193645, 12, !- X,Y,Z Vertex 35 {m}",
         "  -94.1433994146979, -38.9954728454752, 12, !- X,Y,Z Vertex 36 {m}",
         "  -89.8678484803951, -48.0353657767096, 12, !- X,Y,Z Vertex 37 {m}",
         "  -84.7268210384629, -56.6126518767123, 12, !- X,Y,Z Vertex 38 {m}",
         "  -78.7698279935386, -64.6447271915188, 12, !- X,Y,Z Vertex 39 {m}",
         "  -72.0542384450684, -72.0542384450684, 12, !- X,Y,Z Vertex 40 {m}",
         "  -64.6447271915188, -78.7698279935385, 12, !- X,Y,Z Vertex 41 {m}",
         "  -56.6126518767123, -84.7268210384629, 12, !- X,Y,Z Vertex 42 {m}",
         "  -48.0353657767096, -89.8678484803951, 12, !- X,Y,Z Vertex 43 {m}",
         "  -38.9954728454752, -94.1433994146979, 12, !- X,Y,Z Vertex 44 {m}",
         "  -29.580032193645, -97.5122979486201, 12, !- X,Y,Z Vertex 45 {m}",
         "  -19.8797196616996, -99.9420997476528, 12, !- X,Y,Z Vertex 46 {m}",
         "  -9.98795456205171, -101.409404492206, 12, !- X,Y,Z Vertex 47 {m}",
         "  -1.8718741247683e-14, -101.900081235481, 12, !- X,Y,Z Vertex 48 {m}",
         "  9.98795456205167, -101.409404492206, 12, !- X,Y,Z Vertex 49 {m}",
         "  19.8797196616995, -99.9420997476528, 12, !- X,Y,Z Vertex 50 {m}",
         "  29.5800321936449, -97.5122979486201, 12, !- X,Y,Z Vertex 51 {m}",
         "  38.9954728454752, -94.1433994146979, 12, !- X,Y,Z Vertex 52 {m}",
         "  48.0353657767096, -89.8678484803951, 12, !- X,Y,Z Vertex 53 {m}",
         "  56.6126518767123, -84.7268210384629, 12, !- X,Y,Z Vertex 54 {m}",
         "  64.6447271915188, -78.7698279935386, 12, !- X,Y,Z Vertex 55 {m}",
         "  72.0542384450684, -72.0542384450684, 12, !- X,Y,Z Vertex 56 {m}",
         "  78.7698279935385, -64.6447271915188, 12, !- X,Y,Z Vertex 57 {m}",
         "  84.7268210384629, -56.6126518767123, 12, !- X,Y,Z Vertex 58 {m}",
         "  89.8678484803951, -48.0353657767096, 12, !- X,Y,Z Vertex 59 {m}",
         "  94.1433994146979, -38.9954728454752, 12, !- X,Y,Z Vertex 60 {m}",
         "  97.5122979486201, -29.580032193645, 12, !- X,Y,Z Vertex 61 {m}",
         "  99.9420997476528, -19.8797196616996, 12, !- X,Y,Z Vertex 62 {m}",
         "  101.409404492206, -9.98795456205172, 12, !- X,Y,Z Vertex 63 {m}",
         "  101.900081235481, -2.49583216635773e-14, 12; !- X,Y,Z Vertex 64 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 7,                              !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  84.7268210384629, -56.6126518767123, 12, !- X,Y,Z Vertex 1 {m}",
         "  84.7268210384629, -56.6126518767123, 0, !- X,Y,Z Vertex 2 {m}",
         "  89.8678484803951, -48.0353657767096, 0, !- X,Y,Z Vertex 3 {m}",
         "  89.8678484803951, -48.0353657767096, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 8,                              !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  78.7698279935385, -64.6447271915188, 12, !- X,Y,Z Vertex 1 {m}",
         "  78.7698279935385, -64.6447271915188, 0, !- X,Y,Z Vertex 2 {m}",
         "  84.7268210384629, -56.6126518767123, 0, !- X,Y,Z Vertex 3 {m}",
         "  84.7268210384629, -56.6126518767123, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "BuildingSurface:Detailed,",
         "  Surface 9,                              !- Name",
         "  Wall,                                   !- Surface Type",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  Outdoors,                               !- Outside Boundary Condition",
         "  ,                                       !- Outside Boundary Condition Object",
         "  SunExposed,                             !- Sun Exposure",
         "  WindExposed,                            !- Wind Exposure",
         "  ,                                       !- View Factor to Ground",
         "  ,                                       !- Number of Vertices",
         "  72.0542384450684, -72.0542384450684, 12, !- X,Y,Z Vertex 1 {m}",
         "  72.0542384450684, -72.0542384450684, 0, !- X,Y,Z Vertex 2 {m}",
         "  78.7698279935385, -64.6447271915188, 0, !- X,Y,Z Vertex 3 {m}",
         "  78.7698279935385, -64.6447271915188, 12; !- X,Y,Z Vertex 4 {m}",
         "",
         "RunPeriod,",
         "  Run Period 1,                           !- Name",
         "  1,                                      !- Begin Month",
         "  1,                                      !- Begin Day of Month",
         "   ,                                      !- Begin Year",
         "  12,                                     !- End Month",
         "  31,                                     !- End Day of Month",
         "   ,                                      !- End Year",
         "  Sunday,                                 !- Day of Week for Start Day",
         "  No,                                     !- Use Weather File Holidays and Special Days",
         "  No,                                     !- Use Weather File Daylight Saving Period",
         "  No,                                     !- Apply Weekend Holiday Rule",
         "  Yes,                                    !- Use Weather File Rain Indicators",
         "  Yes,                                    !- Use Weather File Snow Indicators",
         "  No;                                     !- Treat Weather as Actual",
         "",
         "GlobalGeometryRules,",
         "  UpperLeftCorner,                        !- Starting Vertex Position",
         "  Counterclockwise,                       !- Vertex Entry Direction",
         "  Relative,                               !- Coordinate System",
         "  Relative,                               !- Daylighting Reference Point Coordinate System",
         "  Relative;                               !- Rectangular Surface Coordinate System",
         "",
         "Material,",
         "  1/2IN Gypsum,                           !- Name",
         "  Smooth,                                 !- Roughness",
         "  0.0127,                                 !- Thickness {m}",
         "  0.16,                                   !- Conductivity {W/m-K}",
         "  784.9,                                  !- Density {kg/m3}",
         "  830.000000000001,                       !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.4,                                    !- Solar Absorptance",
         "  0.4;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  100mm Normalweight concrete floor,      !- Name",
         "  MediumSmooth,                           !- Roughness",
         "  0.1016,                                 !- Thickness {m}",
         "  2.31,                                   !- Conductivity {W/m-K}",
         "  2322,                                   !- Density {kg/m3}",
         "  832,                                    !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  1IN Stucco,                             !- Name",
         "  Smooth,                                 !- Roughness",
         "  0.0253,                                 !- Thickness {m}",
         "  0.691799999999999,                      !- Conductivity {W/m-K}",
         "  1858,                                   !- Density {kg/m3}",
         "  836.999999999999,                       !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.92;                                   !- Visible Absorptance",
         "",
         "Material,",
         "  4 in. Normalweight Concrete Floor,      !- Name",
         "  MediumRough,                            !- Roughness",
         "  0.1016,                                 !- Thickness {m}",
         "  2.31,                                   !- Conductivity {W/m-K}",
         "  2321.99999999999,                       !- Density {kg/m3}",
         "  831.999999999997,                       !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  6 in. Normalweight Concrete Floor,      !- Name",
         "  MediumRough,                            !- Roughness",
         "  0.1524,                                 !- Thickness {m}",
         "  2.31,                                   !- Conductivity {W/m-K}",
         "  2321.99999999999,                       !- Density {kg/m3}",
         "  831.999999999997,                       !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  8 in. Concrete Block Basement Wall,     !- Name",
         "  MediumRough,                            !- Roughness",
         "  0.2032,                                 !- Thickness {m}",
         "  1.326,                                  !- Conductivity {W/m-K}",
         "  1841.99999999999,                       !- Density {kg/m3}",
         "  911.999999999999,                       !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  8IN CONCRETE HW RefBldg,                !- Name",
         "  Rough,                                  !- Roughness",
         "  0.2032,                                 !- Thickness {m}",
         "  1.311,                                  !- Conductivity {W/m-K}",
         "  2240,                                   !- Density {kg/m3}",
         "  836.800000000001,                       !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  F08 Metal surface,                      !- Name",
         "  Smooth,                                 !- Roughness",
         "  0.0008,                                 !- Thickness {m}",
         "  45.2800000000001,                       !- Conductivity {W/m-K}",
         "  7823.99999999999,                       !- Density {kg/m3}",
         "  500,                                    !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  G01 13mm gypsum board,                  !- Name",
         "  Smooth,                                 !- Roughness",
         "  0.0127,                                 !- Thickness {m}",
         "  0.16,                                   !- Conductivity {W/m-K}",
         "  800,                                    !- Density {kg/m3}",
         "  1090,                                   !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.5;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  G05 25mm wood,                          !- Name",
         "  MediumSmooth,                           !- Roughness",
         "  0.0254,                                 !- Thickness {m}",
         "  0.15,                                   !- Conductivity {W/m-K}",
         "  608,                                    !- Density {kg/m3}",
         "  1630,                                   !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.5,                                    !- Solar Absorptance",
         "  0.5;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  M10 200mm concrete block basement wall, !- Name",
         "  MediumRough,                            !- Roughness",
         "  0.2032,                                 !- Thickness {m}",
         "  1.326,                                  !- Conductivity {W/m-K}",
         "  1842,                                   !- Density {kg/m3}",
         "  912,                                    !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  Metal Roof Surface,                     !- Name",
         "  Smooth,                                 !- Roughness",
         "  0.000799999999999998,                   !- Thickness {m}",
         "  45.2799999999999,                       !- Conductivity {W/m-K}",
         "  7823.99999999999,                       !- Density {kg/m3}",
         "  499.999999999996,                       !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  Roof Membrane,                          !- Name",
         "  VeryRough,                              !- Roughness",
         "  0.0095,                                 !- Thickness {m}",
         "  0.16,                                   !- Conductivity {W/m-K}",
         "  1121.29,                                !- Density {kg/m3}",
         "  1460,                                   !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.77,                                   !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material,",
         "  Std Wood 6inch,                         !- Name",
         "  MediumSmooth,                           !- Roughness",
         "  0.15,                                   !- Thickness {m}",
         "  0.12,                                   !- Conductivity {W/m-K}",
         "  540,                                    !- Density {kg/m3}",
         "  1210,                                   !- Specific Heat {J/kg-K}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material:NoMass,",
         "  CP02 CARPET PAD,                        !- Name",
         "  VeryRough,                              !- Roughness",
         "  0.21648,                                !- Thermal Resistance {m2-K/W}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.8;                                    !- Visible Absorptance",
         "",
         "Material:NoMass,",
         "  Nonres_Floor_Insulation,                !- Name",
         "  MediumSmooth,                           !- Roughness",
         "  2.88291975297193,                       !- Thermal Resistance {m2-K/W}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material:NoMass,",
         "  Typical Carpet Pad,                     !- Name",
         "  Smooth,                                 !- Roughness",
         "  0.216479986995276,                      !- Thermal Resistance {m2-K/W}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.8;                                    !- Visible Absorptance",
         "",
         "Material:NoMass,",
         "  Typical Insulation R-0.58,              !- Name",
         "  Smooth,                                 !- Roughness",
         "  0.101874652714525,                      !- Thermal Resistance {m2-K/W}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material:NoMass,",
         "  Typical Insulation R-1.15,              !- Name",
         "  Smooth,                                 !- Roughness",
         "  0.202526711234652,                      !- Thermal Resistance {m2-K/W}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material:NoMass,",
         "  Typical Insulation R-10.11,             !- Name",
         "  Smooth,                                 !- Roughness",
         "  1.78074119041048,                       !- Thermal Resistance {m2-K/W}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material:NoMass,",
         "  Typical Insulation R-13.06,             !- Name",
         "  Smooth,                                 !- Roughness",
         "  2.29929884884436,                       !- Thermal Resistance {m2-K/W}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "Material:NoMass,",
         "  Typical Insulation R-19.72,             !- Name",
         "  Smooth,                                 !- Roughness",
         "  3.47220354893132,                       !- Thermal Resistance {m2-K/W}",
         "  0.9,                                    !- Thermal Absorptance",
         "  0.7,                                    !- Solar Absorptance",
         "  0.7;                                    !- Visible Absorptance",
         "",
         "WindowMaterial:Gas,",
         "  AIR 13MM,                               !- Name",
         "  Air,                                    !- Gas Type",
         "  0.0127;                                 !- Thickness {m}",
         "",
         "WindowMaterial:Glazing,",
         "  Clear 3mm,                              !- Name",
         "  SpectralAverage,                        !- Optical Data Type",
         "  ,                                       !- Window Glass Spectral Data Set Name",
         "  0.00299999999999999,                    !- Thickness {m}",
         "  0.837,                                  !- Solar Transmittance at Normal Incidence",
         "  0.075,                                  !- Front Side Solar Reflectance at Normal Incidence",
         "  0.075,                                  !- Back Side Solar Reflectance at Normal Incidence",
         "  0.898,                                  !- Visible Transmittance at Normal Incidence",
         "  0.081,                                  !- Front Side Visible Reflectance at Normal Incidence",
         "  0.081,                                  !- Back Side Visible Reflectance at Normal Incidence",
         "  0,                                      !- Infrared Transmittance at Normal Incidence",
         "  0.84,                                   !- Front Side Infrared Hemispherical Emissivity",
         "  0.84,                                   !- Back Side Infrared Hemispherical Emissivity",
         "  0.9,                                    !- Conductivity {W/m-K}",
         "  1,                                      !- Dirt Correction Factor for Solar and Visible Transmittance",
         "  No;                                     !- Solar Diffusing",
         "",
         "WindowMaterial:Glazing,",
         "  CLEAR 6MM,                              !- Name",
         "  SpectralAverage,                        !- Optical Data Type",
         "  ,                                       !- Window Glass Spectral Data Set Name",
         "  0.00599999999999998,                    !- Thickness {m}",
         "  0.775,                                  !- Solar Transmittance at Normal Incidence",
         "  0.071,                                  !- Front Side Solar Reflectance at Normal Incidence",
         "  0.071,                                  !- Back Side Solar Reflectance at Normal Incidence",
         "  0.881,                                  !- Visible Transmittance at Normal Incidence",
         "  0.08,                                   !- Front Side Visible Reflectance at Normal Incidence",
         "  0.08,                                   !- Back Side Visible Reflectance at Normal Incidence",
         "  0,                                      !- Infrared Transmittance at Normal Incidence",
         "  0.84,                                   !- Front Side Infrared Hemispherical Emissivity",
         "  0.84,                                   !- Back Side Infrared Hemispherical Emissivity",
         "  0.9,                                    !- Conductivity {W/m-K}",
         "  1,                                      !- Dirt Correction Factor for Solar and Visible Transmittance",
         "  No;                                     !- Solar Diffusing",
         "",
         "WindowMaterial:Glazing,",
         "  REF D CLEAR 6MM,                        !- Name",
         "  SpectralAverage,                        !- Optical Data Type",
         "  ,                                       !- Window Glass Spectral Data Set Name",
         "  0.00599999999999998,                    !- Thickness {m}",
         "  0.429,                                  !- Solar Transmittance at Normal Incidence",
         "  0.308,                                  !- Front Side Solar Reflectance at Normal Incidence",
         "  0.379,                                  !- Back Side Solar Reflectance at Normal Incidence",
         "  0.334,                                  !- Visible Transmittance at Normal Incidence",
         "  0.453,                                  !- Front Side Visible Reflectance at Normal Incidence",
         "  0.505,                                  !- Back Side Visible Reflectance at Normal Incidence",
         "  0,                                      !- Infrared Transmittance at Normal Incidence",
         "  0.84,                                   !- Front Side Infrared Hemispherical Emissivity",
         "  0.82,                                   !- Back Side Infrared Hemispherical Emissivity",
         "  0.9,                                    !- Conductivity {W/m-K}",
         "  1,                                      !- Dirt Correction Factor for Solar and Visible Transmittance",
         "  No;                                     !- Solar Diffusing",
         "",
         "WindowMaterial:SimpleGlazingSystem,",
         "  U 1.17 SHGC 0.49 Simple Glazing,        !- Name",
         "  6.64356810910278,                       !- U-Factor {W/m2-K}",
         "  0.49,                                   !- Solar Heat Gain Coefficient",
         "  0.38;                                   !- Visible Transmittance",
         "",
         "Construction,",
         "  Basement Floor construction,            !- Name",
         "  M10 200mm concrete block basement wall, !- Layer 1",
         "  CP02 CARPET PAD;                        !- Layer 2",
         "",
         "Construction,",
         "  Basement Wall construction,             !- Name",
         "  M10 200mm concrete block basement wall; !- Layer 1",
         "",
         "Construction,",
         "  Floor Adiabatic construction,           !- Name",
         "  CP02 CARPET PAD,                        !- Layer 1",
         "  100mm Normalweight concrete floor,      !- Layer 2",
         "  Nonres_Floor_Insulation;                !- Layer 3",
         "",
         "Construction,",
         "  InteriorFurnishings,                    !- Name",
         "  Std Wood 6inch;                         !- Layer 1",
         "",
         "Construction,",
         "  Typical IEAD Roof R-20.83,              !- Name",
         "  Roof Membrane,                          !- Layer 1",
         "  Typical Insulation R-19.72,             !- Layer 2",
         "  Metal Roof Surface;                     !- Layer 3",
         "",
         "Construction,",
         "  Typical Insulated Basement Mass Wall,   !- Name",
         "  8 in. Concrete Block Basement Wall;     !- Layer 1",
         "",
         "Construction,",
         "  Typical Insulated Carpeted 6in Slab Floor, !- Name",
         "  6 in. Normalweight Concrete Floor,      !- Layer 1",
         "  Typical Carpet Pad;                     !- Layer 2",
         "",
         "Construction,",
         "  Typical Insulated Exterior Mass Floor R-15.63, !- Name",
         "  Typical Insulation R-13.06,             !- Layer 1",
         "  4 in. Normalweight Concrete Floor,      !- Layer 2",
         "  Typical Carpet Pad;                     !- Layer 3",
         "",
         "Construction,",
         "  Typical Insulated Exterior Mass Wall R-12.5, !- Name",
         "  1IN Stucco,                             !- Layer 1",
         "  8IN CONCRETE HW RefBldg,                !- Layer 2",
         "  Typical Insulation R-10.11,             !- Layer 3",
         "  1/2IN Gypsum;                           !- Layer 4",
         "",
         "Construction,",
         "  Typical Insulated Metal Door R-1.43,    !- Name",
         "  F08 Metal surface,                      !- Layer 1",
         "  Typical Insulation R-0.58;              !- Layer 2",
         "",
         "Construction,",
         "  Typical Interior Ceiling,               !- Name",
         "  CP02 CARPET PAD,                        !- Layer 1",
         "  100mm Normalweight concrete floor;      !- Layer 2",
         "",
         "Construction,",
         "  Typical Interior Door,                  !- Name",
         "  G05 25mm wood;                          !- Layer 1",
         "",
         "Construction,",
         "  Typical Interior Floor,                 !- Name",
         "  100mm Normalweight concrete floor,      !- Layer 1",
         "  CP02 CARPET PAD;                        !- Layer 2",
         "",
         "Construction,",
         "  Typical Interior Partition,             !- Name",
         "  G05 25mm wood;                          !- Layer 1",
         "",
         "Construction,",
         "  Typical Interior Wall,                  !- Name",
         "  G01 13mm gypsum board,                  !- Layer 1",
         "  G01 13mm gypsum board;                  !- Layer 2",
         "",
         "Construction,",
         "  Typical Interior Window,                !- Name",
         "  Clear 3mm;                              !- Layer 1",
         "",
         "Construction,",
         "  Typical Overhead Door R-2.0,            !- Name",
         "  Typical Insulation R-1.15;              !- Layer 1",
         "",
         "Construction,",
         "  U 0.48 SHGC 0.40 Dbl Ref-D Clr 6mm/13mm, !- Name",
         "  REF D CLEAR 6MM,                        !- Layer 1",
         "  AIR 13MM,                               !- Layer 2",
         "  CLEAR 6MM;                              !- Layer 3",
         "",
         "Construction,",
         "  U 1.17 SHGC 0.49 Simple Glazing Skylight, !- Name",
         "  U 1.17 SHGC 0.49 Simple Glazing;        !- Layer 1",
         "",
         "InternalMass,",
         "  Space 0 Mass,                           !- Name",
         "  InteriorFurnishings,                    !- Construction Name",
         "  Space 0 ZN,                             !- Zone Name",
         "  65137.496399959;                        !- Surface Area {m2}",
         ""
    });
    ASSERT_TRUE(process_idf(idf_objects));

    int N = 67;
    Array1D<Real64> NetLWRadtoSurf(N);
    DataGlobals::BeginEnvrnFlag = true;
    DataGlobals::BeginSimFlag = true;
    bool ErrorsFound = false;
    DataHeatBalance::ZoneIntGain.allocate(1);

    createFacilityElectricPowerServiceObject();
    HeatBalanceManager::SetPreConstructionInputParameters();
    HeatBalanceManager::GetProjectControlData(state, state.outputFiles, ErrorsFound);
    HeatBalanceManager::GetFrameAndDividerData(ErrorsFound);
    HeatBalanceManager::GetMaterialData(state.dataWindowEquivalentLayer, state.outputFiles, ErrorsFound);
    HeatBalanceManager::GetConstructData(ErrorsFound);
    HeatBalanceManager::GetBuildingData(state, ErrorsFound);

    Psychrometrics::InitializePsychRoutines();

    DataGlobals::TimeStep = 1;
    DataGlobals::TimeStepZone = 1;
    DataGlobals::HourOfDay = 1;
    DataGlobals::NumOfTimeStepInHour = 1;
    DataGlobals::BeginSimFlag = true;
    DataGlobals::BeginEnvrnFlag = true;
    DataEnvironment::OutBaroPress = 100000;

    HeatBalanceSurfaceManager::AllocateSurfaceHeatBalArrays();

    {
        Array1D<Real64> NetLWRadtoSurf_expected{-0.3425735192323175,-0.3425735192323175,-0.3425735192323173,-0.3425735192323183,-0.3425735192323187,-0.3425735192323188,-0.3425735192323182,-0.3425735192323174,-0.3425735192323185,-0.3425735192323167,-0.3425735192323172,-0.342573519232318,-0.3425735192323172,-0.3425735192323177,-0.3425735192323177,-0.3425735192323172,-0.3425735192323181,-0.3425735192323183,-0.3425735192323182,-0.3425735192323173,-0.3425735192323177,-0.3425735192323167,-0.3425735192323183,-0.3425735192323171,-0.3425735192323295,-0.3425735191521347,-0.3425729526425549,-0.342572952561826,-0.3425729525618361,-0.3425729525618371,-0.3425729525618361,-0.3425729525618361,-0.3425735192323177,-0.3425729525618366,-0.3425729525618371,-0.3425729525618368,-0.3425729525618366,-0.3425729525618363,-0.3425729525618365,-0.3425729525602763,-0.3425729525533314,-0.3425729306877724,-0.3425018823817422,-0.3425735192323181,-0.3270671531134264,-0.3272403380435988,-0.3426839358557595,-0.3425729677153186,-0.3425729525652054,-0.3425729525618373,-0.3425729525618367,-0.3425729525618366,-0.3425729525618366,-0.3425729525618367,-0.3425735192323177,-0.3425729525618251,-0.3425729526425549,-0.3425735191521347,-0.3425735192323286,-0.3425735192323175,-0.3425735192323175,-0.3425735192323175,-0.3425735192323176,-0.3425735192323175,-0.09278284035219624,-0.5700925603104247,0.371771871092954};
        Array1D<Real64> Surface_Temps{23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208908,23.89487887621731,23.89487887622465,23.89487887622465,23.89487887622465,23.89487887622465,23.89487887622465,23.89487898208168,23.89487887622465,23.89487887622465,23.89487887622465,23.89487887622465,23.89487887622465,23.89487887622465,23.89487887622438,23.89487887622452,23.89487887689728,23.89487829411545,23.89487898208168,23.90529879087763,23.90529971690676,23.89487530607031,23.89487887567004,23.89487887622426,23.89487887622465,23.89487887622465,23.89487887622465,23.89487887622465,23.89487887622465,23.89487898208168,23.89487887622465,23.89487887621731,23.89487898208908,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.89487898208168,23.84735203238828,23.93059779469495,23.81384335202301};

        CalcInteriorRadExchange(Surface_Temps, 0, NetLWRadtoSurf);

        for (int i = 0; i < N; ++i) {
            EXPECT_NEAR(NetLWRadtoSurf[i], NetLWRadtoSurf_expected[i], 1e-6) << "index: " << i;
        }
    }

    {
        Array1D<Real64> Surface_Temps{23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242635,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242685,23.28065276242551,23.2806527586819,23.28064035615369,23.28065276242635,23.28344365773561,23.28347281048904,23.28066911977259,23.28065276539241,23.28065276242774,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242686,23.28065276242635,23.28065276242686,23.28065276242686,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.28065276242635,23.61746540810387,23.46925015349246,23.46557584975521};
        Array1D<Real64> NetLWRadtoSurf_expected{1.046890058984696,1.046890058984696,1.046890058984695,1.046890058984699,1.046890058984699,1.046890058984699,1.046890058984698,1.046890058984696,1.046890058984697,1.046890058984693,1.046890058984695,1.046890058984698,1.046890058984694,1.046890058984697,1.046890058984697,1.046890058984694,1.046890058984697,1.046890058984698,1.046890058984698,1.046890058984694,1.046890058984696,1.046890058984693,1.046890058984698,1.046890058984695,1.046890058984697,1.046890058984696,1.046890058981969,1.04689005898197,1.046890058981968,1.04689005898197,1.046890058981968,1.046890058981968,1.046890058984696,1.046890058981969,1.04689005898197,1.04689005898197,1.046890058981969,1.046890058981968,1.04689005898197,1.046890058982205,1.046890058964635,1.046890011304996,1.046732038035725,1.046890058984697,1.082388320082436,1.082759648795467,1.047098391317256,1.04689009672507,1.046890058993232,1.046890058981972,1.046890058981969,1.046890058981969,1.046890058981969,1.046890058981969,1.046890058984696,1.046890058981968,1.046890058981969,1.046890058984695,1.046890058984695,1.046890058984696,1.046890058984695,1.046890058984696,1.046890058984696,1.046890058984696,-0.8005193697278674,0.04634571035732391,0.253522273536903};

        CalcInteriorRadExchange(Surface_Temps, 0, NetLWRadtoSurf);

        for (int i = 0; i < N; ++i) {
            EXPECT_NEAR(NetLWRadtoSurf[i], NetLWRadtoSurf_expected[i], 1e-6) << "index: " << i;
        }
    }
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_CalcInverse)
{
    std::vector<double> cinverse67{-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-3.12951e-08,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-0.000833345,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-3.12951e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-3.12951e-08,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-0.000833345,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12956e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-3.12951e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-3.12951e-08,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-0.000833345,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-1.16678e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-3.12951e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12861e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16659e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12861e-08,-1.16659e-08,-0.000833345,-3.12856e-08,-5.60874e-08,-5.6108e-08,-3.12939e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.12856e-08,-3.45369e-08,-3.45369e-08,-1.32337e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-0.000833345,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.60874e-08,-5.61046e-08,-0.000833342,-8.70162e-09,-5.61237e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-5.61046e-08,-6.18331e-08,-6.18331e-08,-1.02307e-07,
        -5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.6108e-08,-5.61251e-08,-8.70162e-09,-0.000833342,-9.5786e-09,-5.61262e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-5.61251e-08,-6.18516e-08,-6.18516e-08,-1.02372e-07,
        -3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.12939e-08,-3.13034e-08,-5.61237e-08,-9.5786e-09,-0.000833345,-1.16691e-08,-3.13038e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.13034e-08,-3.4555e-08,-3.4555e-08,-1.32403e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61262e-08,-1.16691e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13038e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-3.12951e-08,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-1.16678e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-0.000833345,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-3.12951e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12956e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-0.000833345,-1.16678e-08,-3.12956e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-1.16678e-08,-0.000833345,-1.16678e-08,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -1.16678e-08,-3.12956e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12856e-08,-3.12951e-08,-5.61046e-08,-5.61251e-08,-3.13034e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12951e-08,-3.12956e-08,-1.16678e-08,-0.000833345,-3.45458e-08,-3.45458e-08,-1.32363e-07,
        -3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45369e-08,-3.45458e-08,-6.18331e-08,-6.18516e-08,-3.4555e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.08352e-06,-3.75956e-08,-1.41067e-07,
        -3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45369e-08,-3.45458e-08,-6.18331e-08,-6.18516e-08,-3.4555e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.45458e-08,-3.75956e-08,-3.08352e-06,-1.41067e-07,
        -1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32337e-07,-1.32363e-07,-1.02307e-07,-1.02372e-07,-1.32403e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.32363e-07,-1.41067e-07,-1.41067e-07,-1.54923e-06};

    std::vector<double> cmatrix67{-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,8.70559,8.70559,100.789,
        0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,-1200,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,-1200,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,-1200,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0,-1200,0.0282537,0.0669879,0.0670108,0.0282637,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,0.0282537,8.70284,8.70284,100.767,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,-1200,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0669879,0.0670113,-1200,0,0.0670333,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,0.0670113,20.3292,20.3292,75.1877,
        0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670108,0.0670342,0,-1200,0,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,0.0670342,20.3346,20.3346,75.2426,
        0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282637,0.0282746,0.0670333,0,-1200,0,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,0.0282746,8.70866,8.70866,100.821,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,-1200,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0.0282646,0.0282646,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,-1200,0,0.0282646,8.70559,8.70559,100.789,
        0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,0,8.70559,8.70559,100.789,
        0,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282537,0.0282646,0.0670113,0.0670342,0.0282746,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0.0282646,0,-1200,8.70559,8.70559,100.789,
        8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70284,8.70559,20.3292,20.3346,8.70866,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,-325687,2620.5,29368.4,
        8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70284,8.70559,20.3292,20.3346,8.70866,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,8.70559,2620.5,-325687,29368.4,
        100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.767,100.789,75.1877,75.2426,100.821,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,100.789,29368.4,29368.4,-651375};

    std::vector<double> cinverse7{-0.000834781,-1.39589e-05,-1.39589e-05,-1.39589e-05,-1.3606e-05,-1.3606e-05,-1.59713e-05,
        -1.39589e-05,-0.000834781,-1.39589e-05,-1.39589e-05,-1.3606e-05,-1.3606e-05,-1.59713e-05,
        -1.39589e-05,-1.39589e-05,-0.000834781,-1.39589e-05,-1.3606e-05,-1.3606e-05,-1.59713e-05,
        -1.39589e-05,-1.39589e-05,-1.39589e-05,-0.000834781,-1.3606e-05,-1.3606e-05,-1.59713e-05,
        -1.3606e-05,-1.3606e-05,-1.3606e-05,-1.3606e-05,-0.00100141,-1.32592e-05,-1.55829e-05,
        -1.3606e-05,-1.3606e-05,-1.3606e-05,-1.3606e-05,-1.32592e-05,-0.00100141,-1.55829e-05,
        -1.59713e-05,-1.59713e-05,-1.59713e-05,-1.59713e-05,-1.55829e-05,-1.55829e-05,-0.000501584};
    std::vector<double> cmatrix7{-1200,18.2915,18.2915,18.2915,14.8095,14.8095,35.5427,
        18.2915,-1200,18.2915,18.2915,14.8095,14.8095,35.5427,
        18.2915,18.2915,-1200,18.2915,14.8095,14.8095,35.5427,
        18.2915,18.2915,18.2915,-1200,14.8095,14.8095,35.5427,
        14.8095,14.8095,14.8095,14.8095,-1000,11.9874,28.8087,
        14.8095,14.8095,14.8095,14.8095,11.9874,-1000,28.8087,
        35.5427,35.5427,35.5427,35.5427,28.8087,28.8087,-2000};

    int N = 67;
    Array2D<Real64> Cinverse_expected(N, N);
    Array2D<Real64> Cinverse(N, N);
    Array2D<Real64> Cmatrix(N, N);

    for (int i = 0; i < N * N; ++i)
    {
        Cinverse_expected[i] = cinverse67[i];
        Cmatrix[i] = cmatrix67[i];
    }

    // CalcMatrixInverse(Cmatrix, Cinverse);
    Eigen::Map<Eigen::MatrixXd> CMatrixXd(Cmatrix.data(), N, N);
    Eigen::Map<Eigen::MatrixXd> CinverseXd(Cinverse.data(), N, N);
    CinverseXd = CMatrixXd.inverse();

    for (int i = 0; i < N * N; ++i) {
        EXPECT_NEAR(Cinverse[i], Cinverse_expected[i], 1e-6) << "index: " << i;
    }

    N = 7;
    Cinverse_expected = Array2D<Real64>(N, N);
    Cinverse = Array2D<Real64>(N, N);
    Cmatrix = Array2D<Real64>(N, N);

    for (int i = 0; i < N * N; ++i)
    {
        Cinverse_expected[i] = cinverse7[i];
        Cmatrix[i] = cmatrix7[i];
    }

    // CalcMatrixInverse(Cmatrix, Cinverse);
    Eigen::Map<Eigen::MatrixXd> CMatrixXd2(Cmatrix.data(), N, N);
    Eigen::Map<Eigen::MatrixXd> CinverseXd2(Cinverse.data(), N, N);
    CinverseXd2 = CMatrixXd2.inverse();

    for (int i = 0; i < N * N; ++i) {
        EXPECT_NEAR(Cinverse[i], Cinverse_expected[i], 1e-6) << "index: " << i;
    }
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_CalcScriptF)
{
    std::vector<double> a{120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,120,32568.7,32568.7,65137.5};
    std::vector<double> f{0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000267299,0.000267299,0.00154733,
    0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0,0,0.000235447,0.000558232,0.000558423,0.000235531,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000235447,0.000267215,0.000267215,0.00154699,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558232,0.000558427,0,0,0.000558611,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000558427,0.000624193,0.000624193,0.00115429,
    0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558423,0.000558618,0,0,0,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000558618,0.000624358,0.000624358,0.00115513,
    0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235531,0.000235622,0.000558611,0,0,0,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000235622,0.000267393,0.000267393,0.00154782,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0.000235538,0.000235538,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0.000235538,0.000267299,0.000267299,0.00154733,
    0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0,0.000267299,0.000267299,0.00154733,
    0,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235447,0.000235538,0.000558427,0.000558618,0.000235622,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0.000235538,0,0,0.000267299,0.000267299,0.00154733,
    0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725237,0.0725466,0.16941,0.169455,0.0725722,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0,0.0804606,0.450868,
    0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725237,0.0725466,0.16941,0.169455,0.0725722,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0725466,0.0804606,0,0.450868,
    0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839724,0.839908,0.626564,0.627021,0.840175,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.839908,0.901736,0.901736,0};

    std::vector<double> emiss{0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9};

    std::vector<double> scriptf{0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.0911343,0.0911343,0.698366,
    0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.0911343,0.0911343,0.698366,
    0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000304188,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113416,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000304188,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000304188,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113416,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304193,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000304188,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000304188,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113416,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000113411,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000304188,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304101,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113393,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911342,0.0911342,0.698366,
    0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304101,0.000113393,0.000113375,0.000304096,0.00054517,0.00054537,0.000304177,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.000304096,0.0911106,0.0911106,0.698228,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000113416,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.00054517,0.000545336,8.45588e-05,8.45798e-05,0.000545523,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.000545336,0.16312,0.16312,0.539785,
    0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.00054537,0.000545536,8.45798e-05,8.46261e-05,9.3104e-05,0.000545547,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.000545536,0.163169,0.163169,0.540128,
    0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304177,0.000304269,0.000545523,9.3104e-05,0.000113462,0.000113424,0.000304273,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.000304269,0.0911583,0.0911583,0.698574,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545547,0.000113424,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304273,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000304188,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000113411,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113416,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304193,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000304188,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000113411,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.000304188,0.000304188,0.000304188,0.0911343,0.0911343,0.698366,
    0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304193,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113416,0.000113411,0.000304193,0.0911343,0.0911343,0.698366,
    0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000113411,0.000113416,0.000113411,0.0911343,0.0911343,0.698366,
    0.000113411,0.000304193,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304096,0.000304188,0.000545336,0.000545536,0.000304269,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304188,0.000304193,0.000113411,0.000113416,0.0911343,0.0911343,0.698366,
    0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335698,0.000335785,0.000601018,0.000601198,0.000335874,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.0345269,0.0991798,0.744287,
    0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335698,0.000335785,0.000601018,0.000601198,0.000335874,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.000335785,0.0991798,0.0345269,0.744287,
    0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128632,0.00128657,0.000994424,0.000995054,0.00128695,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.00128657,0.372144,0.372144,0.0739395};


    int N = 67;
    Array1D<Real64> A(N);
    Array2D<Real64> F(N, N);
    Array1D<Real64> EMISS(N);
    Array2D<Real64> ScriptF(N, N);
    Array2D<Real64> ScriptF_expected(N, N);

    for (int i = 0; i < N; ++i)
    {
        A[i] = a[i];
        EMISS[i] = emiss[i];
        for (int j = 0; j < N; ++j)
        {
            F[i*N+j] = f[i*N+j];
            ScriptF_expected[i*N+j] = scriptf[i*N+j];
        }
    }

    CalcScriptF(N, A, F, EMISS, ScriptF);

    for (int i = 0; i < N * N; ++i) {
        EXPECT_NEAR(ScriptF[i], ScriptF_expected[i], 1e-6) << "index: " << i;
    }

    a = std::vector<double>{120,120,120,120,100,100,200};
    f = std::vector<double>{0,0.152429,0.152429,0.152429,0.148095,0.148095,0.177713,
        0.152429,0,0.152429,0.152429,0.148095,0.148095,0.177713,
        0.152429,0.152429,0,0.152429,0.148095,0.148095,0.177713,
        0.152429,0.152429,0.152429,0,0.148095,0.148095,0.177713,
        0.123412,0.123412,0.123412,0.123412,0,0.119874,0.144044,
        0.123412,0.123412,0.123412,0.123412,0.119874,0,0.144044,
        0.296189,0.296189,0.296189,0.296189,0.288087,0.288087,0};
    emiss = std::vector<double>{0.9,0.9,0.9,0.9,0.9,0.9,0.9};
    scriptf = std::vector<double>{0.0140668,0.13568,0.13568,0.13568,0.110209,0.110209,0.258736,
        0.13568,0.0140668,0.13568,0.13568,0.110209,0.110209,0.258736,
        0.13568,0.13568,0.0140668,0.13568,0.110209,0.110209,0.258736,
        0.13568,0.13568,0.13568,0.0140668,0.110209,0.110209,0.258736,
        0.132251,0.132251,0.132251,0.132251,0.0114523,0.1074,0.252443,
        0.132251,0.132251,0.132251,0.132251,0.1074,0.0114523,0.252443,
        0.155241,0.155241,0.155241,0.155241,0.126222,0.126222,0.0256649};

    N = 7;
    A = Array1D<Real64>(N);
    F = Array2D<Real64>(N, N);
    EMISS = Array1D<Real64>(N);
    ScriptF = Array2D<Real64>(N, N);
    ScriptF_expected = Array2D<Real64>(N, N);

    for (int i = 0; i < N; ++i)
    {
        A[i] = a[i];
        EMISS[i] = emiss[i];
        for (int j = 0; j < N; ++j)
        {
            F[i*N+j] = f[i*N+j];
            ScriptF_expected[i*N+j] = scriptf[i*N+j];
        }
    }

    CalcScriptF(N, A, F, EMISS, ScriptF);

    for (int i = 0; i < N * N; ++i) {
        EXPECT_NEAR(ScriptF[i], ScriptF_expected[i], 1e-6) << "index: " << i;
    }
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_CarrollMRT) {
    int N;                     // NUMBER OF SURFACES
    Array1D<Real64> A;         // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
    Array1D<Real64> FMRT;      // MRT "VIEW FACTORS"
    Array1D<Real64> EMISS;     // Gray body emissivities
    Array1D<Real64> Fp;        // Gray body radiative resistance

    // Three surfaces of equal size
    N = 3;

    A.allocate(N);
    A(1) = 1.0;
    A(2) = 1.0;
    A(3) = 1.0;

    FMRT.allocate(N);
    CalcFMRT(N, A, FMRT);

    EMISS.allocate(N);
    EMISS(1) = 1.0;
    EMISS(2) = 1.0;
    EMISS(3) = 1.0;

    Fp.allocate(N);
    CalcFp(N, EMISS, FMRT, Fp);

    EXPECT_NEAR(FMRT(1), 1.5, 0.001);
    EXPECT_NEAR(FMRT(2), 1.5, 0.001);
    EXPECT_NEAR(FMRT(3), 1.5, 0.001);

    // Special case where surfaces are equal area (each 50% of total).
    N = 2;

    A.redimension(N);
    A(1) = 1.0;
    A(2) = 1.0;

    FMRT.redimension(N);

    CalcFMRT(N, A, FMRT);

    EXPECT_NEAR(FMRT(1), 2.0, 0.001);
    EXPECT_NEAR(FMRT(2), 2.0, 0.001);

    EMISS.redimension(N);
    EMISS(1) = 1.0;
    EMISS(2) = 1.0;

    Fp.redimension(N);
    CalcFp(N, EMISS, FMRT, Fp);

    // Imbalanced areas
    A(1) = 2.0;
    A(2) = 1.0;

    CalcFMRT(N, A, FMRT);

    std::string const error_string = delimited_string({"   ** Severe  ** Geometry not compatible with Carroll MRT Zone Radiant Exchange method."});
    EXPECT_TRUE(compare_err_stream(error_string, true));

    CalcFp(N, EMISS, FMRT, Fp);
 }

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_FixViewFactorsTest)
{

    int N;                     // NUMBER OF SURFACES
    Array1D<Real64> A;         // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
    Array2D<Real64> F;         // APPROXIMATE DIRECT VIEW FACTOR MATRIX (N X N)
    int ZoneNum;               // Zone number being fixed
    Real64 OriginalCheckValue; // check of SUM(F) - N
    Real64 FixedCheckValue;    // check after fixed of SUM(F) - N
    Real64 FinalCheckValue;    // the one to go with
    int NumIterations;         // number of iterations to fixed
    Real64 RowSum;             // RowSum of Fixed

    N = 3;

    A.allocate(N);
    F.allocate(N, N);

    A(1) = 1.0;
    A(2) = 1.0;
    A(3) = 1.0;

    F(1, 1) = 0.0;
    F(1, 2) = 0.5;
    F(1, 3) = 0.5;
    F(2, 1) = 0.5;
    F(2, 2) = 0.0;
    F(2, 3) = 0.5;
    F(3, 1) = 0.5;
    F(3, 2) = 0.5;
    F(3, 3) = 0.0;

    ZoneNum = 1;

    DataHeatBalance::Zone.allocate(ZoneNum);
    DataHeatBalance::Zone(ZoneNum).Name = "Test";
    DataViewFactorInformation::ZoneRadiantInfo.allocate(ZoneNum);
    DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).Name = DataHeatBalance::Zone(ZoneNum).Name;
    DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).ZoneNums.push_back(ZoneNum);

    FixViewFactors(N,
                   A,
                   F,
                   DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).Name,
                   DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).ZoneNums,
                   OriginalCheckValue,
                   FixedCheckValue,
                   FinalCheckValue,
                   NumIterations,
                   RowSum);

    std::string const error_string = delimited_string({
        "   ** Warning ** Surfaces in Zone/Enclosure=\"Test\" do not define an enclosure.",
        "   **   ~~~   ** Number of surfaces <= 3, view factors are set to force reciprocity but may not fulfill completeness.",
        "   **   ~~~   ** Reciprocity means that radiant exchange between two surfaces will match and not lead to an energy loss.",
        "   **   ~~~   ** Completeness means that all of the view factors between a surface and the other surfaces in a zone add up to unity.",
        "   **   ~~~   ** So, when there are three or less surfaces in a zone, EnergyPlus will make sure there are no losses of energy but",
        "   **   ~~~   ** it will not exchange the full amount of radiation with the rest of the zone as it would if there was a completed "
        "enclosure.",
    });

    EXPECT_TRUE(compare_err_stream(error_string, true));

    // Tests for correction of view factors based on GitHub Issue #5772

    A(1) = 20.0;
    A(2) = 180.0;
    A(3) = 180.0;
    F(1, 1) = 0.0;
    F(1, 2) = 0.5;
    F(1, 3) = 0.5;
    F(2, 1) = 0.1;
    F(2, 2) = 0.0;
    F(2, 3) = 0.9;
    F(3, 1) = 0.1;
    F(3, 2) = 0.9;
    F(3, 3) = 0.0;

    FixViewFactors(N,
        A,
        F,
        DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).Name,
        DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).ZoneNums,
        OriginalCheckValue,
        FixedCheckValue,
        FinalCheckValue,
        NumIterations,
        RowSum);

    EXPECT_NEAR(F(1, 2), 0.07986, 0.001);
    EXPECT_NEAR(F(2, 1), 0.71875, 0.001);
    EXPECT_NEAR(F(3, 2), 0.28125, 0.001);

    A(1) = 100.0;
    A(2) = 100.0;
    A(3) = 200.0;
    F(1, 1) = 0.0;
    F(1, 2) = 1.0 / 3.0;
    F(1, 3) = 2.0 / 3.0;
    F(2, 1) = 1.0 / 3.0;
    F(2, 2) = 0.0;
    F(2, 3) = 2.0 / 3.0;
    F(3, 1) = 0.5;
    F(3, 2) = 0.5;
    F(3, 3) = 0.0;

    FixViewFactors(N,
        A,
        F,
        DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).Name,
        DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).ZoneNums,
        OriginalCheckValue,
        FixedCheckValue,
        FinalCheckValue,
        NumIterations,
        RowSum);

    EXPECT_NEAR(F(1, 2), 0.181818, 0.001);
    EXPECT_NEAR(F(2, 3), 0.25, 0.001);
    EXPECT_NEAR(F(3, 2), 0.5, 0.001);

    A(1) = 100.0;
    A(2) = 150.0;
    A(3) = 200.0;
    F(1, 1) = 0.0;
    F(1, 2) = 150.0 / 350.0;
    F(1, 3) = 200.0 / 350.0;
    F(2, 1) = 1.0 / 3.0;
    F(2, 2) = 0.0;
    F(2, 3) = 2.0 / 3.0;
    F(3, 1) = 0.4;
    F(3, 2) = 0.6;
    F(3, 3) = 0.0;

    FixViewFactors(N,
        A,
        F,
        DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).Name,
        DataViewFactorInformation::ZoneRadiantInfo(ZoneNum).ZoneNums,
        OriginalCheckValue,
        FixedCheckValue,
        FinalCheckValue,
        NumIterations,
        RowSum);

    EXPECT_NEAR(F(1, 2), 0.21466, 0.001);
    EXPECT_NEAR(F(1, 3), 0.25445, 0.001);
    EXPECT_NEAR(F(2, 1), 0.32199, 0.001);
    EXPECT_NEAR(F(2, 3), 0.36832, 0.001);
    EXPECT_NEAR(F(3, 1), 0.50890, 0.001);
    EXPECT_NEAR(F(3, 2), 0.49110, 0.001);

    A.deallocate();
    F.deallocate();
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_UpdateMovableInsulationFlagTest)
{

    bool DidMIChange;
    int SurfNum;

    dataConstruction.Construct.allocate(1);
    dataMaterial.Material.allocate(1);
    DataSurfaces::Surface.allocate(1);

    SurfNum = 1;
    DataSurfaces::Surface(1).MaterialMovInsulInt = 1;
    DataSurfaces::Surface(1).MovInsulIntPresent = false;
    DataSurfaces::Surface(1).MovInsulIntPresentPrevTS = false;
    DataSurfaces::Surface(1).Construction = 1;
    DataSurfaces::Surface(1).MaterialMovInsulInt = 1;
    dataConstruction.Construct(1).InsideAbsorpThermal = 0.9;
    dataMaterial.Material(1).AbsorpThermal = 0.5;
    dataMaterial.Material(1).Resistance = 1.25;
    DataSurfaces::Surface(1).SchedMovInsulInt = -1;
    dataMaterial.Material(1).AbsorpSolar = 0.25;

    // Test 1: Movable insulation present but wasn't in previous time step, also movable insulation emissivity different than base construction
    //         This should result in a true value from the algorithm which will cause interior radiant exchange matrices to be recalculated
    HeatBalanceIntRadExchange::UpdateMovableInsulationFlag(DidMIChange, SurfNum);
    EXPECT_TRUE(DidMIChange);

    // Test 2: Movable insulation present and was also present in previous time step.  This should result in a false value since nothing has changed.
    DataSurfaces::Surface(1).MovInsulIntPresentPrevTS = true;
    HeatBalanceIntRadExchange::UpdateMovableInsulationFlag(DidMIChange, SurfNum);
    EXPECT_TRUE(!DidMIChange);

    // Test 2: Movable insulation present but wasn't in previous time step.  However, the emissivity of the movable insulation and that of the
    // 		   construction are the same so nothing has actually changed.  This should result in a false value.
    DataSurfaces::Surface(1).MovInsulIntPresentPrevTS = false;
    dataMaterial.Material(1).AbsorpThermal = dataConstruction.Construct(1).InsideAbsorpThermal;
    HeatBalanceIntRadExchange::UpdateMovableInsulationFlag(DidMIChange, SurfNum);
    EXPECT_TRUE(!DidMIChange);
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_AlignInputViewFactorsTest)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Zone,",
        "Zone 4;             !- Name",

        "Zone,",
        "Zone 5;             !- Name",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Zone 3,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Perimeter Zones,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",

        "ZoneList,",
        "Perimeter Zones, !- Name",
        "Zone 5, !- Zone 1 Name",
        "Zone 2; !- Zone 2 Name",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Zone 6,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",
        });
    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    DataViewFactorInformation::NumOfRadiantEnclosures = 3;
    DataViewFactorInformation::ZoneRadiantInfo.allocate(3);
    DataViewFactorInformation::ZoneRadiantInfo(1).Name = "Enclosure 1";
    DataViewFactorInformation::ZoneRadiantInfo(1).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 2"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneRadiantInfo(1).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 1"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneRadiantInfo(2).Name = "Enclosure 2";
    DataViewFactorInformation::ZoneRadiantInfo(2).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 4"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneRadiantInfo(2).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 5"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneRadiantInfo(3).Name = "Zone 3";
    DataViewFactorInformation::ZoneRadiantInfo(3).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 3"), DataHeatBalance::Zone, DataGlobals::NumOfZones));

    ErrorsFound = false;
    HeatBalanceIntRadExchange::AlignInputViewFactors("ZoneProperty:UserViewFactors:BySurfaceName", ErrorsFound);
    EXPECT_TRUE(ErrorsFound);
    std::string const error_string = delimited_string({
    "   ** Severe  ** AlignInputViewFactors: ZoneProperty:UserViewFactors:BySurfaceName=\"Perimeter Zones\" found a matching ZoneList, but did not find a matching radiant or solar enclosure with the same zones.",
    "   ** Severe  ** AlignInputViewFactors: ZoneProperty:UserViewFactors:BySurfaceName=\"Zone 6\" did not find a matching radiant or solar enclosure name."
        });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    EXPECT_EQ(DataViewFactorInformation::ZoneRadiantInfo(1).Name, "Enclosure 1");
    EXPECT_EQ(DataViewFactorInformation::ZoneRadiantInfo(2).Name, "Enclosure 2");
    EXPECT_EQ(DataViewFactorInformation::ZoneRadiantInfo(3).Name, "Zone 3");
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_AlignInputViewFactorsTest2)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Zone,",
        "Zone 4;             !- Name",

        "Zone,",
        "Zone 5;             !- Name",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Zone 3,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Perimeter Zones,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",

        "ZoneList,",
        "Perimeter Zones, !- Name",
        "Zone 5, !- Zone 1 Name",
        "Zone 2; !- Zone 2 Name",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Zone 6,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",
        });
    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    DataViewFactorInformation::NumOfSolarEnclosures = 3;
    DataViewFactorInformation::ZoneSolarInfo.allocate(3);
    DataViewFactorInformation::ZoneSolarInfo(1).Name = "Enclosure 1";
    DataViewFactorInformation::ZoneSolarInfo(1).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 2"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneSolarInfo(1).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 5"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneSolarInfo(2).Name = "Enclosure 2";
    DataViewFactorInformation::ZoneSolarInfo(2).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 4"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneSolarInfo(2).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 5"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneSolarInfo(3).Name = "Zone 3";
    DataViewFactorInformation::ZoneSolarInfo(3).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 3"), DataHeatBalance::Zone, DataGlobals::NumOfZones));

    ErrorsFound = false;
    HeatBalanceIntRadExchange::AlignInputViewFactors("ZoneProperty:UserViewFactors:BySurfaceName", ErrorsFound);
    EXPECT_TRUE(ErrorsFound);
    std::string const error_string = delimited_string({
    "   ** Severe  ** AlignInputViewFactors: ZoneProperty:UserViewFactors:BySurfaceName=\"Zone 6\" did not find a matching radiant or solar enclosure name."
        });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    EXPECT_EQ(DataViewFactorInformation::ZoneSolarInfo(1).Name, "Perimeter Zones");
    EXPECT_EQ(DataViewFactorInformation::ZoneSolarInfo(2).Name, "Enclosure 2");
    EXPECT_EQ(DataViewFactorInformation::ZoneSolarInfo(3).Name, "Zone 3");
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_AlignInputViewFactorsTest3)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Zone,",
        "Zone 4;             !- Name",

        "Zone,",
        "Zone 5;             !- Name",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Zone 3,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Perimeter Zones,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",

        "ZoneList,",
        "Perimeter Zones, !- Name",
        "Zone 5, !- Zone 1 Name",
        "Zone 2; !- Zone 2 Name",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Zone 6,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",
        });
    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    DataViewFactorInformation::NumOfSolarEnclosures = 3;
    DataViewFactorInformation::ZoneSolarInfo.allocate(3);
    DataViewFactorInformation::ZoneSolarInfo(1).Name = "Enclosure 1";
    DataViewFactorInformation::ZoneSolarInfo(1).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 2"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneSolarInfo(1).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 1"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneSolarInfo(2).Name = "Enclosure 2";
    DataViewFactorInformation::ZoneSolarInfo(2).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 4"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneSolarInfo(2).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 5"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneSolarInfo(3).Name = "Zone 3";
    DataViewFactorInformation::ZoneSolarInfo(3).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 3"), DataHeatBalance::Zone, DataGlobals::NumOfZones));

    ErrorsFound = false;
    HeatBalanceIntRadExchange::AlignInputViewFactors("ZoneProperty:UserViewFactors:BySurfaceName", ErrorsFound);
    EXPECT_TRUE(ErrorsFound);
    std::string const error_string = delimited_string({
    "   ** Severe  ** AlignInputViewFactors: ZoneProperty:UserViewFactors:BySurfaceName=\"Perimeter Zones\" found a matching ZoneList, but did not find a matching radiant or solar enclosure with the same zones.",
    "   ** Severe  ** AlignInputViewFactors: ZoneProperty:UserViewFactors:BySurfaceName=\"Zone 6\" did not find a matching radiant or solar enclosure name."
        });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    EXPECT_EQ(DataViewFactorInformation::ZoneSolarInfo(1).Name, "Enclosure 1");
    EXPECT_EQ(DataViewFactorInformation::ZoneSolarInfo(2).Name, "Enclosure 2");
    EXPECT_EQ(DataViewFactorInformation::ZoneSolarInfo(3).Name, "Zone 3");
}

TEST_F(EnergyPlusFixture, HeatBalanceIntRadExchange_AlignInputViewFactorsTest4)
{

    std::string const idf_objects = delimited_string({
        "Zone,",
        "Zone 1;             !- Name",

        "Zone,",
        "Zone 2;             !- Name",

        "Zone,",
        "Zone 3;             !- Name",

        "Zone,",
        "Zone 4;             !- Name",

        "Zone,",
        "Zone 5;             !- Name",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Zone 3,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Perimeter Zones,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",

        "ZoneList,",
        "Perimeter Zones, !- Name",
        "Zone 5, !- Zone 1 Name",
        "Zone 2; !- Zone 2 Name",

        "ZoneProperty:UserViewFactors:BySurfaceName,",
        "Zone 6,",
        "SB51,SB51,0.000000,",
        "SB51,SB52,2.672021E-002,",
        "SB51,SB53,8.311358E-002,",
        "SB51,SB54,2.672021E-002;",
        });
    ASSERT_TRUE(process_idf(idf_objects));
    bool ErrorsFound = false;
    HeatBalanceManager::GetZoneData(ErrorsFound);
    EXPECT_FALSE(ErrorsFound);

    DataViewFactorInformation::NumOfRadiantEnclosures = 3;
    DataViewFactorInformation::ZoneRadiantInfo.allocate(3);
    DataViewFactorInformation::ZoneRadiantInfo(1).Name = "Enclosure 1";
    DataViewFactorInformation::ZoneRadiantInfo(1).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 2"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneRadiantInfo(1).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 5"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneRadiantInfo(2).Name = "Enclosure 2";
    DataViewFactorInformation::ZoneRadiantInfo(2).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 4"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneRadiantInfo(2).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 5"), DataHeatBalance::Zone, DataGlobals::NumOfZones));
    DataViewFactorInformation::ZoneRadiantInfo(3).Name = "Zone 3";
    DataViewFactorInformation::ZoneRadiantInfo(3).ZoneNums.push_back(UtilityRoutines::FindItemInList(
        UtilityRoutines::MakeUPPERCase("Zone 3"), DataHeatBalance::Zone, DataGlobals::NumOfZones));

    ErrorsFound = false;
    HeatBalanceIntRadExchange::AlignInputViewFactors("ZoneProperty:UserViewFactors:BySurfaceName", ErrorsFound);
    EXPECT_TRUE(ErrorsFound);
    std::string const error_string = delimited_string({
    "   ** Severe  ** AlignInputViewFactors: ZoneProperty:UserViewFactors:BySurfaceName=\"Zone 6\" did not find a matching radiant or solar enclosure name."
        });
    EXPECT_TRUE(compare_err_stream(error_string, true));

    EXPECT_EQ(DataViewFactorInformation::ZoneRadiantInfo(1).Name, "Perimeter Zones");
    EXPECT_EQ(DataViewFactorInformation::ZoneRadiantInfo(2).Name, "Enclosure 2");
    EXPECT_EQ(DataViewFactorInformation::ZoneRadiantInfo(3).Name, "Zone 3");
}

} // namespace EnergyPlus
