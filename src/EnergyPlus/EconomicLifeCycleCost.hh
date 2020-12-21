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

#ifndef EconomicLifeCycleCost_hh_INCLUDED
#define EconomicLifeCycleCost_hh_INCLUDED

// C++ Headers
#include <map>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobalConstants.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

namespace EconomicLifeCycleCost {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS:
    enum class iDiscConv {
        BeginOfYear,
        MidYear,
        EndOfYear,
    };

    enum class iInflAppr {
        ConstantDollar,
        CurrentDollar,
    };

    enum class iDeprMethod {
        MACRS3,
        MACRS5,
        MACRS7,
        MACRS10,
        MACRS15,
        MACRS20,
        Straight27,
        Straight31,
        Straight39,
        Straight40,
        None,
    };

    constexpr int costCatMaintenance(1);
    constexpr int costCatRepair(2);
    constexpr int costCatOperation(3);
    constexpr int costCatReplacement(4);
    constexpr int costCatMinorOverhaul(5);
    constexpr int costCatMajorOverhaul(6);
    constexpr int costCatOtherOperational(7);
    constexpr int costCatConstruction(8);
    constexpr int costCatSalvage(9);
    constexpr int costCatOtherCapital(10);
    constexpr int costCatWater(11);
    constexpr int costCatEnergy(12);
    constexpr int costCatTotEnergy(13);
    constexpr int costCatTotOper(14);
    constexpr int costCatTotCaptl(15);
    constexpr int costCatTotGrand(16);

    constexpr int countOfCostCat(16); // count of the number of cost categories

    // The NIST supplement includes UPV* factors for
    //   Electricity
    //   Natural gas
    //   Distillate oil - FuelOilNo1
    //   Liquified petroleum gas - Propane
    //   Residual oil - FuelOilNo2
    //   Coal

    enum class iStartCosts {
        ServicePeriod,
        BasePeriod,
    };

    extern int numRecurringCosts;

    extern int numNonrecurringCost;

    extern int numUsePriceEscalation;

    extern int numUseAdjustment;

    extern int numCashFlow;
    extern int const skRecurring;
    extern int const skNonrecurring;
    extern int const skResource;
    extern int const skSum;
    extern int const pvkEnergy;
    extern int const pvkNonEnergy;
    extern int const pvkNotComputed;
    extern int numResourcesUsed;

    // present value factors
    extern Array1D<Real64> SPV;

    // arrays related to computing after tax cashflow and present value
    extern Array1D<Real64> DepreciatedCapital;
    extern Array1D<Real64> TaxableIncome;
    extern Array1D<Real64> Taxes;
    extern Array1D<Real64> AfterTaxCashFlow;
    extern Array1D<Real64> AfterTaxPresentValue;

    extern Array1D_string const MonthNames;

    // arrays related to escalated energy costs
    extern Array1D<Real64> EscalatedTotEnergy;
    extern std::map<int, std::map<DataGlobalConstants::ResourceType, Real64>> EscalatedEnergy;

    // SUBROUTINE SPECIFICATIONS FOR MODULE <module_name>:

    // Types

    struct RecurringCostsType
    {
        // Members
        std::string name;            // Name
        std::string lineItem;        // Line Item
        int category;                // Category
        Real64 cost;                 // Cost
        iStartCosts startOfCosts;    // Start of Costs
        int yearsFromStart;          // Years from Start 0 - 100
        int monthsFromStart;         // Months from Start 0 - 11
        int totalMonthsFromStart;    // Total months (12 x years) + months
        int repeatPeriodYears;       // Repeat Period Years 1 - 100
        int repeatPeriodMonths;      // Repeat Period Months 0 - 11
        int totalRepeatPeriodMonths; // Total months (12 x years) + months
        Real64 annualEscalationRate; // Annual escalation rate

        // Default Constructor
        RecurringCostsType()
            : category(costCatMaintenance), cost(0.0), startOfCosts(iStartCosts::ServicePeriod), yearsFromStart(0), monthsFromStart(0),
              totalMonthsFromStart(0), repeatPeriodYears(0), repeatPeriodMonths(0), totalRepeatPeriodMonths(0), annualEscalationRate(0.0)
        {
        }
    };

    struct NonrecurringCostType
    {
        // Members
        std::string name;         // Name
        std::string lineItem;     // Line Item
        int category;             // Category
        Real64 cost;              // Cost
        iStartCosts startOfCosts; // Start of Costs
        int yearsFromStart;       // Years from Start 0 - 100
        int monthsFromStart;      // Months from Start 0 - 11
        int totalMonthsFromStart; // Total months (12 x years) + months

        // Default Constructor
        NonrecurringCostType()
            : category(costCatConstruction), cost(0.0), startOfCosts(iStartCosts::ServicePeriod), yearsFromStart(0), monthsFromStart(0),
              totalMonthsFromStart(0)
        {
        }
    };

    struct UsePriceEscalationType
    {
        // Members
        std::string name;           // Name
        DataGlobalConstants::ResourceType resource; // resource like electricity or natural gas (uses definitions from DataGlobalConstants)
        int escalationStartYear;    // Escalation Start Year 1900-2100
        int escalationStartMonth;   // Escalation Start Month 1 to 12
        Array1D<Real64> Escalation; // Escalation by year, first year is baseDateYear
        // last year is baseDateYear + lengthStudyYears - 1

        // Default Constructor
        UsePriceEscalationType() : resource(DataGlobalConstants::ResourceType::None), escalationStartYear(0), escalationStartMonth(0)
        {
        }
    };

    struct UseAdjustmentType
    {
        // Members
        std::string name;           // Name
        DataGlobalConstants::ResourceType resource;               // resource like electricity or natural gas (uses definitions from DataGlobalConstants)
        Array1D<Real64> Adjustment; // Adjustment by year, first year is baseDateYear
        // last year is baseDateYear + lengthStudyYears - 1

        // Default Constructor
        UseAdjustmentType() : resource(DataGlobalConstants::ResourceType::None)
        {
        }
    };

    struct CashFlowType
    {
        // Members
        std::string name;         // Name - just for labeling output - use Category for aggregation
        int SourceKind;           // 1=recurring, 2=nonrecurring, 3=resource
        DataGlobalConstants::ResourceType Resource;             // resource like electricity or natural gas (uses definitions from DataGlobalConstants)
        int Category;             // uses "costCat" constants above
        Array1D<Real64> mnAmount; // cashflow dollar amount by month, first year is baseDateYear
        // last year is baseDateYear + lengthStudyYears - 1
        Array1D<Real64> yrAmount;  // cashflow dollar amount by year, first year is baseDateYear
        int pvKind;                // kind of present value 1=energy, 2=non-energy,3=not computed but summed
        Real64 presentValue;       // total present value for cashflow
        Real64 orginalCost;        // original cost from recurring, non-recurring or energy cost
        Array1D<Real64> yrPresVal; // present value by year, first year is baseDateYear

        // Default Constructor
        CashFlowType() : SourceKind(0), Resource(DataGlobalConstants::ResourceType::None), Category(0), pvKind(0), presentValue(0.), orginalCost(0.)
        {
        }
    };

    // Object Data
    extern Array1D<RecurringCostsType> RecurringCosts;
    extern Array1D<NonrecurringCostType> NonrecurringCost;
    extern Array1D<UsePriceEscalationType> UsePriceEscalation;
    extern Array1D<UseAdjustmentType> UseAdjustment;
    extern Array1D<CashFlowType> CashFlow;

    // Functions

    void GetInputForLifeCycleCost(EnergyPlusData &state);

    void ComputeLifeCycleCostAndReport(EnergyPlusData &state);

    //======================================================================================================================
    //======================================================================================================================

    //    GET INPUT ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void GetInputLifeCycleCostParameters(EnergyPlusData &state);

    void GetInputLifeCycleCostRecurringCosts(EnergyPlusData &state);

    void GetInputLifeCycleCostNonrecurringCost(EnergyPlusData &state);

    void GetInputLifeCycleCostUsePriceEscalation(EnergyPlusData &state);

    void GetInputLifeCycleCostUseAdjustment(EnergyPlusData &state);

    int MonthToMonthNumber(std::string const &inMonthString, int const inDefaultMonth);

    //======================================================================================================================
    //======================================================================================================================

    //    COMPUTATION ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void ExpressAsCashFlows(EnergyPlusData &state);

    void ComputeEscalatedEnergyCosts(EnergyPlusData &state);

    void ComputePresentValue(EnergyPlusData &state);

    void ComputeTaxAndDepreciation(EnergyPlusData &state);

    //======================================================================================================================
    //======================================================================================================================

    //    OUTPUT ROUTINES

    //======================================================================================================================
    //======================================================================================================================

    void WriteTabularLifeCycleCostReport(EnergyPlusData &state);

    void clear_state();

} // namespace EconomicLifeCycleCost

struct EconomicLifeCycleCostData : BaseGlobalStruct {
    // related to LifeCycleCost:Parameters
    bool LCCparamPresent = false; // If a LifeCycleCost:Parameters object is present
    std::string LCCname;          // Name
    EconomicLifeCycleCost::iDiscConv discountConvention = EconomicLifeCycleCost::iDiscConv::EndOfYear;     // Discounting Convention
    EconomicLifeCycleCost::iInflAppr inflationApproach = EconomicLifeCycleCost::iInflAppr::ConstantDollar; // Inflation Approach
    Real64 realDiscountRate = 0.0;                                                                         // Real Discount Rate
    Real64 nominalDiscountRate = 0.0;                                                                      // Nominal Discount Rate
    Real64 inflation = 0.0;                                                                                // Inflation
    int baseDateMonth = 0;                                                                                 // Base Date Month (1=Jan, 12=Dec)
    int baseDateYear = 0;                                                                                  // Base Date Year  1900-2100
    int serviceDateMonth = 0;                                                                              // Service Date Month (1=Jan, 12=Dec)
    int serviceDateYear = 0;                                                                               // Service Date Year 1900-2100
    int lengthStudyYears = 0;                                                                              // Length of Study Period in Years
    int lengthStudyTotalMonths = 0; // Length of Study expressed in months (years x 12)
    Real64 taxRate = 0.0;           // Tax rate
    EconomicLifeCycleCost::iDeprMethod depreciationMethod = EconomicLifeCycleCost::iDeprMethod::None; // Depreciation Method
    // derived
    int lastDateMonth = 0; // Last Date Month (the month before the base date month)
    int lastDateYear = 0;  // Last Date Year (base date year + length of study period in years)

    void clear_state() override
    {
        this->LCCparamPresent = false;
        this->LCCname.clear();
        this->discountConvention = EconomicLifeCycleCost::iDiscConv::EndOfYear;
        this->inflationApproach = EconomicLifeCycleCost::iInflAppr::ConstantDollar;
        this->realDiscountRate = 0.0;
        this->nominalDiscountRate = 0.0;
        this->inflation = 0.0;
        this->baseDateMonth = 0;
        this->baseDateYear = 0;
        this->serviceDateMonth = 0;
        this->serviceDateYear = 0;
        this->lengthStudyYears = 0;
        this->lengthStudyTotalMonths = 0;
        this->taxRate = 0.0;
        this->depreciationMethod = EconomicLifeCycleCost::iDeprMethod::None;
        this->lastDateMonth = 0;
        this->lastDateYear = 0;
    }
};

} // namespace EnergyPlus

#endif
