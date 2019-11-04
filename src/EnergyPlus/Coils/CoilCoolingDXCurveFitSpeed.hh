// EnergyPlus, Copyright (c) 1996-2019, The Board of Trustees of the University of Illinois,
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


#ifndef ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITSPEED
#define ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITSPEED

#include <string>
#include <vector>

#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

class CoilCoolingDXCurveFitOperatingMode;

class CoilCoolingDXCurveFitSpeedInputSpecification
{

public:
    std::string name;
    Real64 gross_rated_total_cooling_capacity_ratio_to_nominal;
    Real64 gross_rated_sensible_heat_ratio;
    Real64 gross_rated_cooling_COP;
    Real64 evaporator_air_flow_fraction;
    Real64 condenser_air_flow_fraction;
    Real64 active_fraction_of_coil_face_area;
    Real64 rated_evaporative_condenser_pump_power_fraction;
    Real64 rated_evaporator_fan_power_per_volume_flow_rate;
    Real64 evaporative_condenser_effectiveness;
    std::string total_cooling_capacity_function_of_temperature_curve_name;
    std::string total_cooling_capacity_function_of_air_flow_fraction_curve_name;
    std::string energy_input_ratio_function_of_temperature_curve_name;
    std::string energy_input_ratio_function_of_air_flow_fraction_curve_name;
    std::string part_load_fraction_correlation_curve_name;
    Real64 rated_waste_heat_fraction_of_power_input;
    std::string waste_heat_function_of_temperature_curve_name;
    std::string sensible_heat_ratio_modifier_function_of_temperature_curve_name;
    std::string sensible_heat_ratio_modifier_function_of_flow_fraction_curve_name;
};

class CoilCoolingDXCurveFitSpeed
{
    std::string const object_name = "Coil:Cooling:DX:CurveFit:Speed";

public:
    CoilCoolingDXCurveFitSpeed() = default;

    explicit CoilCoolingDXCurveFitSpeed(const std::string& name);

    void instantiateFromInputSpec(const CoilCoolingDXCurveFitSpeedInputSpecification& input_data);

    CoilCoolingDXCurveFitSpeedInputSpecification original_input_specs;
    CoilCoolingDXCurveFitOperatingMode *parentMode = nullptr;
    std::string name;

    int indexCapFT = 0;
    int indexCapFFF = 0;
    int indexEIRFT = 0;
    int indexEIRFFF = 0;
    int indexPLRFPLF = 0;
    int indexWHFT = 0;
    int indexWHFFF = 0;
    int indexSHRFT = 0;
    int indexSHRFFF = 0;

    // speed class inputs
    Real64 PLR = 0.0;

    Real64 CondInletTemp = 0.0;            // condenser inlet node temp or outdoor temp if no condenser node {C}
    Real64 ambPressure = 0.0;              // outdoor pressure {Pa]
    Real64 AirFF = 0.0;                    // ratio of air mass flow rate to rated air mass flow rate

                                     //        Real64 RatedTotCap; // rated total capacity at speed {W}
    Real64 RatedAirMassFlowRate = 0.0;     // rated air mass flow rate at speed {kg/s}
    Real64 RatedCondAirMassFlowRate = 0.0; // rated condenser air mass flow rate at speed {kg/s}
    Real64 RatedSHR = 0.0;                 // rated sensible heat ratio at speed
    Real64 RatedCBF = 0.0;                 // rated coil bypass factor at speed
    Real64 RatedEIR = 0.0;                 // rated energy input ratio at speed {W/W}
    Real64 AirMassFlow = 0.0;              // coil inlet air mass flow rate {kg/s}
    int FanOpMode = 0;                   // fan operating mode, constant or cycling fan

    // speed class outputs
    Real64 FullLoadPower = 0.0; // full load power at speed {W}
    Real64 RTF = 0.0;           // coil runtime fraction at speed

    // other data members
    Real64 rated_total_capacity = 0.0;
    Real64 evap_air_flow_rate = 0.0;
    Real64 condenser_air_flow_rate = 0.0;
    Real64 gross_shr = 0.0;
    Real64 active_fraction_of_face_coil_area = 0.0;
    Real64 rated_evap_fan_power_per_volume_flow_rate = 0.0;
    Real64 evap_condenser_pump_power_fraction = 0.0;
    Real64 evap_condenser_effectiveness = 0.0;
    Real64 rated_waste_heat_fraction_of_power_input = 0.0;

    // rating data
    Real64 RatedInletAirTemp = 26.6667;        // 26.6667C or 80F
    Real64 RatedInletWetBulbTemp = 19.44;    // 19.44 or 67F
    Real64 RatedInletAirHumRat = 0.01125;      // Humidity ratio corresponding to 80F dry bulb/67F wet bulb
    Real64 RatedOutdoorAirTemp = 35.0;      // 35 C or 95F
    Real64 DryCoilOutletHumRatioMin = 0.00001; // dry coil outlet minimum hum ratio kgH2O/kgdry air

	bool mySizeFlag = true;

    void CalcSpeedOutput(
        const DataLoopNode::NodeData &inletNode, DataLoopNode::NodeData &outletNode, Real64 &PLR, int &fanOpMode, Real64 condInletTemp);
    void sizeSpeed();

    Real64 CalcBypassFactor(Real64 tdb, Real64 w, Real64 h, Real64 p);

private:
    bool processCurve(const std::string& curveName,
                      int &curveIndex,
                      std::vector<int> validDims,
                      const std::string& routineName,
                      const std::string& fieldName,
                      Real64 Var1,               // required 1st independent variable
                      Optional<Real64 const> Var2 = _, // 2nd independent variable
                      Optional<Real64 const> Var3 = _, // 3rd independent variable
                      Optional<Real64 const> Var4 = _, // 4th independent variable
                      Optional<Real64 const> Var5 = _);
};
} // namespace EnergyPlus

#endif // ENERGYPLUS_COILS_COILCOOLINGDXCURVEFITSPEED
