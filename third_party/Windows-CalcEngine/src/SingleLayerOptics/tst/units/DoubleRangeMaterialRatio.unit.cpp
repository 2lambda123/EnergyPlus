#include <memory>
#include <gtest/gtest.h>

#include "WCESingleLayerOptics.hpp"


using namespace SingleLayerOptics;
using namespace FenestrationCommon;

// Creation of double range material with provided ratio
class TestDoubleRangeMaterialRatio : public testing::Test
{
private:
    std::shared_ptr<CMaterial> m_Material;

protected:
    virtual void SetUp()
    {
        // Solar range material
        const auto Tsol = 0.1;
        const auto Rfsol = 0.7;
        const auto Rbsol = 0.7;

        // Visible range
        const auto Tvis = 0.2;
        const auto Rfvis = 0.6;
        const auto Rbvis = 0.6;

        m_Material = Material::dualBandMaterial(Tsol, Tsol, Rfsol, Rbsol, Tvis, Tvis, Rfvis, Rbvis);
    }

public:
    std::shared_ptr<CMaterial> getMaterial()
    {
        return m_Material;
    };
};

TEST_F(TestDoubleRangeMaterialRatio, TestMaterialProperties)
{
    SCOPED_TRACE("Begin Test: Phi angles creation.");

    std::shared_ptr<CMaterial> aMaterial = getMaterial();

    double T = aMaterial->getProperty(Property::T, Side::Front);

    // Test for solar range first
    EXPECT_NEAR(0.1, T, 1e-6);

    double R = aMaterial->getProperty(Property::R, Side::Front);

    EXPECT_NEAR(0.7, R, 1e-6);

    // Properties at four wavelengths should have been created
    size_t size = 5;

    std::vector<double> Transmittances = aMaterial->getBandProperties(Property::T, Side::Front);

    EXPECT_EQ(size, Transmittances.size());

    std::vector<double> correctResults;
    correctResults.push_back(0);
    correctResults.push_back(0.0039215686274509838);
    correctResults.push_back(0.2);
    correctResults.push_back(0.0039215686274509838);
    correctResults.push_back(0.0039215686274509838);

    for(size_t i = 0; i < size; ++i)
    {
        EXPECT_NEAR(correctResults[i], Transmittances[i], 1e-6);
    }

    std::vector<double> Reflectances = aMaterial->getBandProperties(Property::R, Side::Front);

    EXPECT_EQ(size, Reflectances.size());

    correctResults.clear();
    correctResults.push_back(0);
    correctResults.push_back(0.79607843137254897);
    correctResults.push_back(0.6);
    correctResults.push_back(0.79607843137254897);
    correctResults.push_back(0.79607843137254897);

    for(size_t i = 0; i < size; ++i)
    {
        EXPECT_NEAR(correctResults[i], Reflectances[i], 1e-6);
    }
}
