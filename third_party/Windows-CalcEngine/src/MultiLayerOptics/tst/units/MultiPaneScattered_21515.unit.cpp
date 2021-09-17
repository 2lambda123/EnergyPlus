#include <memory>
#include <gtest/gtest.h>

#include "WCESpectralAveraging.hpp"
#include "WCEMultiLayerOptics.hpp"
#include "WCESingleLayerOptics.hpp"
#include "WCECommon.hpp"


using namespace SingleLayerOptics;
using namespace FenestrationCommon;
using namespace SpectralAveraging;
using namespace MultiLayerOptics;

// Example on how to create scattered multilayer.

class MultiPaneScattered_21515 : public testing::Test
{
private:
    std::unique_ptr<CMultiLayerScattered> m_Layer;

    CSeries loadSolarRadiationFile()
    {
        // Full ASTM E891-87 Table 1 (Solar radiation)
        CSeries aSolarRadiation(
          {{0.3000, 0.0},    {0.3050, 3.4},    {0.3100, 15.6},   {0.3150, 41.1},   {0.3200, 71.2},
           {0.3250, 100.2},  {0.3300, 152.4},  {0.3350, 155.6},  {0.3400, 179.4},  {0.3450, 186.7},
           {0.3500, 212.0},  {0.3600, 240.5},  {0.3700, 324.0},  {0.3800, 362.4},  {0.3900, 381.7},
           {0.4000, 556.0},  {0.4100, 656.3},  {0.4200, 690.8},  {0.4300, 641.9},  {0.4400, 798.5},
           {0.4500, 956.6},  {0.4600, 990.0},  {0.4700, 998.0},  {0.4800, 1046.1}, {0.4900, 1005.1},
           {0.5000, 1026.7}, {0.5100, 1066.7}, {0.5200, 1011.5}, {0.5300, 1084.9}, {0.5400, 1082.4},
           {0.5500, 1102.2}, {0.5700, 1087.4}, {0.5900, 1024.3}, {0.6100, 1088.8}, {0.6300, 1062.1},
           {0.6500, 1061.7}, {0.6700, 1046.2}, {0.6900, 859.2},  {0.7100, 1002.4}, {0.7180, 816.9},
           {0.7244, 842.8},  {0.7400, 971.0},  {0.7525, 956.3},  {0.7575, 942.2},  {0.7625, 524.8},
           {0.7675, 830.7},  {0.7800, 908.9},  {0.8000, 873.4},  {0.8160, 712.0},  {0.8237, 660.2},
           {0.8315, 765.5},  {0.8400, 799.8},  {0.8600, 815.2},  {0.8800, 778.3},  {0.9050, 630.4},
           {0.9150, 565.2},  {0.9250, 586.4},  {0.9300, 348.1},  {0.9370, 224.2},  {0.9480, 271.4},
           {0.9650, 451.2},  {0.9800, 549.7},  {0.9935, 630.1},  {1.0400, 582.9},  {1.0700, 539.7},
           {1.1000, 366.2},  {1.1200, 98.1},   {1.1300, 169.5},  {1.1370, 118.7},  {1.1610, 301.9},
           {1.1800, 406.8},  {1.2000, 375.2},  {1.2350, 423.6},  {1.2900, 365.7},  {1.3200, 223.4},
           {1.3500, 30.1},   {1.3950, 1.4},    {1.4425, 51.6},   {1.4625, 97.0},   {1.4770, 97.3},
           {1.4970, 167.1},  {1.5200, 239.3},  {1.5390, 248.8},  {1.5580, 249.3},  {1.5780, 222.3},
           {1.5920, 227.3},  {1.6100, 210.5},  {1.6300, 224.7},  {1.6460, 215.9},  {1.6780, 202.8},
           {1.7400, 158.2},  {1.8000, 28.6},   {1.8600, 1.8},    {1.9200, 1.1},    {1.9600, 19.7},
           {1.9850, 84.9},   {2.0050, 25.0},   {2.0350, 92.5},   {2.0650, 56.3},   {2.1000, 82.7},
           {2.1480, 76.2},   {2.1980, 66.4},   {2.2700, 65.0},   {2.3600, 57.6},   {2.4500, 19.8},
           {2.4940, 17.0},   {2.5370, 3.0},    {2.9410, 4.0},    {2.9730, 7.0},    {3.0050, 6.0},
           {3.0560, 3.0},    {3.1320, 5.0},    {3.1560, 18.0},   {3.2040, 1.2},    {3.2450, 3.0},
           {3.3170, 12.0},   {3.3440, 3.0},    {3.4500, 12.2},   {3.5730, 11.0},   {3.7650, 9.0},
           {4.0450, 6.9}

          });

        return aSolarRadiation;
    }

    std::shared_ptr<CSpectralSampleData> loadSampleData_NFRC_21515()
    {
        auto aMeasurements_21515 = CSpectralSampleData::create(
          {{0.300, 0.0028, 0.1026, 0.1058}, {0.305, 0.0029, 0.1049, 0.1067},
           {0.310, 0.0032, 0.1001, 0.1014}, {0.315, 0.0204, 0.0988, 0.0996},
           {0.320, 0.2642, 0.1075, 0.1081}, {0.325, 0.4844, 0.1230, 0.1233},
           {0.330, 0.5577, 0.1283, 0.1286}, {0.335, 0.5905, 0.1297, 0.1302},
           {0.340, 0.6176, 0.1311, 0.1312}, {0.345, 0.6433, 0.1321, 0.1323},
           {0.350, 0.6701, 0.1329, 0.1331}, {0.355, 0.6945, 0.1337, 0.1338},
           {0.360, 0.7161, 0.1346, 0.1349}, {0.365, 0.7346, 0.1353, 0.1351},
           {0.370, 0.7513, 0.1354, 0.1354}, {0.375, 0.7639, 0.1358, 0.1355},
           {0.380, 0.7787, 0.1356, 0.1358}, {0.385, 0.7896, 0.1358, 0.1357},
           {0.390, 0.7983, 0.1356, 0.1358}, {0.395, 0.8075, 0.1359, 0.1360},
           {0.400, 0.8175, 0.1360, 0.1359}, {0.410, 0.8327, 0.1359, 0.1360},
           {0.420, 0.8421, 0.1354, 0.1355}, {0.430, 0.8487, 0.1348, 0.1346},
           {0.440, 0.8543, 0.1340, 0.1340}, {0.450, 0.8580, 0.1335, 0.1334},
           {0.460, 0.8605, 0.1326, 0.1328}, {0.470, 0.8633, 0.1323, 0.1321},
           {0.480, 0.8655, 0.1313, 0.1313}, {0.490, 0.8669, 0.1306, 0.1307},
           {0.500, 0.8683, 0.1301, 0.1300}, {0.510, 0.8694, 0.1298, 0.1298},
           {0.520, 0.8706, 0.1288, 0.1288}, {0.530, 0.8721, 0.1284, 0.1283},
           {0.540, 0.8730, 0.1280, 0.1278}, {0.550, 0.8737, 0.1273, 0.1272},
           {0.560, 0.8744, 0.1265, 0.1266}, {0.570, 0.8755, 0.1265, 0.1264},
           {0.580, 0.8758, 0.1263, 0.1260}, {0.590, 0.8771, 0.1257, 0.1256},
           {0.600, 0.8772, 0.1254, 0.1254}, {0.610, 0.8779, 0.1250, 0.1250},
           {0.620, 0.8779, 0.1248, 0.1249}, {0.630, 0.8783, 0.1247, 0.1244},
           {0.640, 0.8789, 0.1244, 0.1242}, {0.650, 0.8792, 0.1241, 0.1239},
           {0.660, 0.8793, 0.1239, 0.1240}, {0.670, 0.8792, 0.1234, 0.1233},
           {0.680, 0.8793, 0.1234, 0.1234}, {0.690, 0.8801, 0.1231, 0.1234},
           {0.700, 0.8799, 0.1233, 0.1232}, {0.710, 0.8807, 0.1235, 0.1235},
           {0.720, 0.8808, 0.1233, 0.1229}, {0.730, 0.8811, 0.1226, 0.1225},
           {0.740, 0.8800, 0.1227, 0.1228}, {0.750, 0.8811, 0.1229, 0.1229},
           {0.760, 0.8826, 0.1225, 0.1225}, {0.770, 0.8829, 0.1224, 0.1226},
           {0.780, 0.8811, 0.1226, 0.1230}, {0.790, 0.8826, 0.1226, 0.1224},
           {0.800, 0.8817, 0.1218, 0.1218}, {0.810, 0.8823, 0.1226, 0.1225},
           {0.820, 0.8814, 0.1223, 0.1224}, {0.830, 0.8816, 0.1225, 0.1223},
           {0.840, 0.8824, 0.1217, 0.1211}, {0.850, 0.8794, 0.1220, 0.1216},
           {0.860, 0.8826, 0.1244, 0.1241}, {0.870, 0.8862, 0.1206, 0.1206},
           {0.880, 0.8868, 0.1188, 0.1199}, {0.890, 0.8877, 0.1184, 0.1191},
           {0.900, 0.8856, 0.1195, 0.1179}, {0.910, 0.8874, 0.1190, 0.1194},
           {0.920, 0.8876, 0.1190, 0.1185}, {0.930, 0.8854, 0.1189, 0.1191},
           {0.940, 0.8882, 0.1185, 0.1197}, {0.950, 0.8862, 0.1206, 0.1178},
           {0.960, 0.8874, 0.1183, 0.1186}, {0.970, 0.8859, 0.1189, 0.1179},
           {0.980, 0.8859, 0.1195, 0.1191}, {0.990, 0.8889, 0.1195, 0.1190},
           {1.000, 0.8856, 0.1190, 0.1190}, {1.050, 0.8866, 0.1177, 0.1186},
           {1.100, 0.8874, 0.1158, 0.1175}, {1.150, 0.8847, 0.1173, 0.1195},
           {1.200, 0.8862, 0.1173, 0.1173}, {1.250, 0.8881, 0.1164, 0.1171},
           {1.300, 0.8868, 0.1188, 0.1173}, {1.350, 0.8829, 0.1177, 0.1184},
           {1.400, 0.8769, 0.1167, 0.1156}, {1.450, 0.8812, 0.1173, 0.1180},
           {1.500, 0.8848, 0.1146, 0.1172}, {1.550, 0.8869, 0.1155, 0.1180},
           {1.600, 0.8839, 0.1162, 0.1181}, {1.650, 0.8005, 0.1047, 0.1050},
           {1.700, 0.8410, 0.1125, 0.1118}, {1.750, 0.8626, 0.1146, 0.1135},
           {1.800, 0.8585, 0.1163, 0.1142}, {1.850, 0.8662, 0.1142, 0.1141},
           {1.900, 0.8559, 0.1135, 0.1144}, {1.950, 0.8520, 0.1133, 0.1130},
           {2.000, 0.8676, 0.1155, 0.1144}, {2.050, 0.8601, 0.1149, 0.1137},
           {2.100, 0.8376, 0.1113, 0.1105}, {2.150, 0.7541, 0.0993, 0.0987},
           {2.200, 0.8167, 0.1074, 0.1074}, {2.250, 0.6418, 0.0876, 0.0853},
           {2.300, 0.6259, 0.0872, 0.0841}, {2.350, 0.5889, 0.0824, 0.0843},
           {2.400, 0.6081, 0.0847, 0.0857}, {2.450, 0.4717, 0.0757, 0.0735},
           {2.500, 0.6857, 0.0956, 0.0900}});

        return aMeasurements_21515;
    }

protected:
    virtual void SetUp()
    {
        // Create material from samples
        auto thickness = 0.18e-3;   // [m]
        auto aMaterial_21515 = SingleLayerOptics::Material::nBandMaterial(
          loadSampleData_NFRC_21515(), thickness, MaterialType::Monolithic, WavelengthRange::Solar);

        CScatteringLayer Layer21515 = CScatteringLayer::createSpecularLayer(aMaterial_21515);

        // Equivalent BSDF layer
        m_Layer = CMultiLayerScattered::create(Layer21515);

        CSeries solarRadiation{loadSolarRadiationFile()};
        m_Layer->setSourceData(solarRadiation);
    }

public:
    CMultiLayerScattered & getLayer()
    {
        return *m_Layer;
    };
};

TEST_F(MultiPaneScattered_21515, TestSpecular1)
{
    SCOPED_TRACE("Begin Test: Laminate layer - Angular Dependency.");

    const double minLambda = 0.3;
    const double maxLambda = 2.5;

    CMultiLayerScattered & aLayer = getLayer();

    Side aSide = Side::Front;
    std::vector<double> thetaAngles{0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
    double phi = 0;

    const std::vector<double> correctResults{0.865999,
                                             0.863266,
                                             0.862713,
                                             0.860582,
                                             0.854166,
                                             0.837063,
                                             0.793620,
                                             0.685154,
                                             0.436450,
                                             0.000000};

    std::vector<double> results;

    for(const auto & theta : thetaAngles)
    {
        results.push_back(aLayer.getPropertySimple(
          minLambda, maxLambda, PropertySimple::T, aSide, Scattering::DirectDirect, theta, phi));
    }


    EXPECT_EQ(results.size(), correctResults.size());

    for(size_t i = 0u; i < correctResults.size(); ++i)
    {
        EXPECT_NEAR(results[i], correctResults[i], 1e-6);
    }
}
