#include "pvsamv1_common_data.h"

char solar_resource_path[256] = {};
char solar_resource_path_15_min[256] = {};
char load_profile_path[256] = {};
char target_power_path[256] = {};
char sell_rate_path[256] = {};
char subarray1_shading[256] = {};
char subarray2_shading[256] = {};
char temperature_path[256] = {};
char solar_resource_path_15min_fail[256] = {};

int n1 = sprintf(solar_resource_path,
                 "%s/test/input_cases/pvsamv1_data/USA AZ Phoenix (TMY2).csv",
                 SSCDIR);
int n2 = sprintf(
    load_profile_path,
    "%s/test/input_cases/pvsamv1_data/pvsamv1_residential_load.csv", SSCDIR);
int n3 = sprintf(
    target_power_path,
    "%s/test/input_cases/pvsamv1_data/pvsamv1_batt_target_power.csv", SSCDIR);
int n4 = sprintf(sell_rate_path,
                 "%s/test/input_cases/pvsamv1_data/pvsamv1_ur_ts_sell_rate.csv",
                 SSCDIR);
int n5 =
    sprintf(solar_resource_path_15_min,
            "%s/test/input_cases/pvsamv1_data/LosAngeles_WeatherFile_15min.csv",
            SSCDIR);
int n6 = sprintf(
    subarray1_shading,
    "%s/test/input_cases/pvsamv1_data/subarray1_shading_timestep.csv", SSCDIR);
int n7 = sprintf(
    subarray2_shading,
    "%s/test/input_cases/pvsamv1_data/subarray2_shading_timestep.csv", SSCDIR);
int n8 = sprintf(
    temperature_path,
    "%s/test/input_cases/battery_data/batt_room_temperature_celsius_60min.csv",
    SSCDIR);
int n9 = sprintf(solar_resource_path_15min_fail,
                 "%s/test/input_cases/pvsamv1_data/"
                 "LosAngeles_WeatherFile_15min_timestamp_fail.csv",
                 SSCDIR);

/**
 *  Default data for no-financial pvsamv1 run that can be further modified
 */
void pvsamv_nofinancial_default(ssc_data_t &data) {
  ssc_data_set_number(data, "analysis_period", 1);
  ssc_data_set_string(data, "solar_resource_file", solar_resource_path);
  ssc_data_set_number(data, "transformer_no_load_loss", 0);
  ssc_data_set_number(data, "transformer_load_loss", 0);
  ssc_data_set_number(data, "en_snow_model", 0);
  ssc_data_set_number(data, "system_capacity", 4.6928696632385254);
  ssc_data_set_number(data, "use_wf_albedo", 0);
  ssc_number_t p_albedo[12] = {
      0.20000000298023224, 0.20000000298023224, 0.20000000298023224,
      0.20000000298023224, 0.20000000298023224, 0.20000000298023224,
      0.20000000298023224, 0.20000000298023224, 0.20000000298023224,
      0.20000000298023224, 0.20000000298023224, 0.20000000298023224};
  ssc_data_set_array(data, "albedo", p_albedo, 12);
  ssc_data_set_number(data, "irrad_mode", 0);
  ssc_data_set_number(data, "sky_model", 2);
  ssc_data_set_number(data, "subarray1_modules_per_string", 7);
  ssc_data_set_number(data, "subarray2_modules_per_string", 7);
  ssc_data_set_number(data, "subarray3_modules_per_string", 7);
  ssc_data_set_number(data, "subarray4_modules_per_string", 7);
  ssc_data_set_number(data, "inverter_count", 1);
  ssc_data_set_number(data, "enable_mismatch_vmax_calc", 0);
  ssc_data_set_number(data, "subarray1_nstrings", 2);
  ssc_data_set_number(data, "subarray1_tilt", 20);
  ssc_data_set_number(data, "subarray1_tilt_eq_lat", 0);
  ssc_data_set_number(data, "subarray1_azimuth", 180);
  ssc_data_set_number(data, "subarray1_track_mode", 0);
  ssc_data_set_number(data, "subarray1_rotlim", 45);
  ssc_data_set_number(data, "subarray1_shade_mode", 0);
  ssc_data_set_number(data, "subarray1_gcr", 0.30000001192092896);
  ssc_number_t p_subarray1_monthly_tilt[12] = {40, 40, 40, 20, 20, 20,
                                               20, 20, 20, 40, 40, 40};
  ssc_data_set_array(data, "subarray1_monthly_tilt", p_subarray1_monthly_tilt,
                     12);
  ssc_number_t p_subarray1_soiling[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  ssc_data_set_array(data, "subarray1_soiling", p_subarray1_soiling, 12);
  ssc_data_set_number(data, "subarray1_rear_irradiance_loss", 0);
  ssc_data_set_number(data, "subarray1_mismatch_loss", 2);
  ssc_data_set_number(data, "subarray1_diodeconn_loss", 0.5);
  ssc_data_set_number(data, "subarray1_dcwiring_loss", 2);
  ssc_data_set_number(data, "subarray1_tracking_loss", 0);
  ssc_data_set_number(data, "subarray1_nameplate_loss", 0);
  ssc_data_set_number(data, "subarray2_rear_irradiance_loss", 0);
  ssc_data_set_number(data, "subarray2_mismatch_loss", 2);
  ssc_data_set_number(data, "subarray2_diodeconn_loss", 0.5);
  ssc_data_set_number(data, "subarray2_dcwiring_loss", 2);
  ssc_data_set_number(data, "subarray2_tracking_loss", 0);
  ssc_data_set_number(data, "subarray2_nameplate_loss", 0);
  ssc_data_set_number(data, "subarray3_rear_irradiance_loss", 0);
  ssc_data_set_number(data, "subarray3_mismatch_loss", 2);
  ssc_data_set_number(data, "subarray3_diodeconn_loss", 0.5);
  ssc_data_set_number(data, "subarray3_dcwiring_loss", 2);
  ssc_data_set_number(data, "subarray3_tracking_loss", 0);
  ssc_data_set_number(data, "subarray3_nameplate_loss", 0);
  ssc_data_set_number(data, "subarray4_rear_irradiance_loss", 0);
  ssc_data_set_number(data, "subarray4_mismatch_loss", 2);
  ssc_data_set_number(data, "subarray4_diodeconn_loss", 0.5);
  ssc_data_set_number(data, "subarray4_dcwiring_loss", 2);
  ssc_data_set_number(data, "subarray4_tracking_loss", 0);
  ssc_data_set_number(data, "subarray4_nameplate_loss", 0);
  ssc_data_set_number(data, "dcoptimizer_loss", 0);
  ssc_data_set_number(data, "acwiring_loss", 1);
  ssc_data_set_number(data, "transmission_loss", 0);
  ssc_data_set_number(data, "subarray1_mod_orient", 0);
  ssc_data_set_number(data, "subarray1_nmodx", 7);
  ssc_data_set_number(data, "subarray1_nmody", 2);
  ssc_data_set_number(data, "subarray1_backtrack", 0);
  ssc_data_set_number(data, "subarray2_enable", 0);
  ssc_data_set_number(data, "subarray2_nstrings", 0);
  ssc_data_set_number(data, "subarray2_tilt", 20);
  ssc_data_set_number(data, "subarray2_tilt_eq_lat", 0);
  ssc_data_set_number(data, "subarray2_azimuth", 180);
  ssc_data_set_number(data, "subarray2_track_mode", 0);
  ssc_data_set_number(data, "subarray2_rotlim", 45);
  ssc_data_set_number(data, "subarray2_shade_mode", 0);
  ssc_data_set_number(data, "subarray2_gcr", 0.30000001192092896);
  ssc_number_t p_subarray2_monthly_tilt[12] = {40, 40, 40, 20, 20, 20,
                                               20, 20, 20, 40, 40, 40};
  ssc_data_set_array(data, "subarray2_monthly_tilt", p_subarray2_monthly_tilt,
                     12);
  ssc_number_t p_subarray2_soiling[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  ssc_data_set_array(data, "subarray2_soiling", p_subarray2_soiling, 12);
  ssc_data_set_number(data, "subarray2_mod_orient", 0);
  ssc_data_set_number(data, "subarray2_nmodx", 9);
  ssc_data_set_number(data, "subarray2_nmody", 2);
  ssc_data_set_number(data, "subarray2_backtrack", 0);
  ssc_data_set_number(data, "subarray3_enable", 0);
  ssc_data_set_number(data, "subarray3_nstrings", 0);
  ssc_data_set_number(data, "subarray3_tilt", 20);
  ssc_data_set_number(data, "subarray3_tilt_eq_lat", 0);
  ssc_data_set_number(data, "subarray3_azimuth", 180);
  ssc_data_set_number(data, "subarray3_track_mode", 0);
  ssc_data_set_number(data, "subarray3_rotlim", 45);
  ssc_data_set_number(data, "subarray3_shade_mode", 0);
  ssc_data_set_number(data, "subarray3_gcr", 0.30000001192092896);
  ssc_number_t p_subarray3_monthly_tilt[12] = {40, 40, 40, 20, 20, 20,
                                               20, 20, 20, 40, 40, 40};
  ssc_data_set_array(data, "subarray3_monthly_tilt", p_subarray3_monthly_tilt,
                     12);
  ssc_number_t p_subarray3_soiling[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  ssc_data_set_array(data, "subarray3_soiling", p_subarray3_soiling, 12);
  ssc_data_set_number(data, "subarray3_mod_orient", 0);
  ssc_data_set_number(data, "subarray3_nmodx", 9);
  ssc_data_set_number(data, "subarray3_nmody", 2);
  ssc_data_set_number(data, "subarray3_backtrack", 0);
  ssc_data_set_number(data, "subarray4_enable", 0);
  ssc_data_set_number(data, "subarray4_nstrings", 0);
  ssc_data_set_number(data, "subarray4_tilt", 20);
  ssc_data_set_number(data, "subarray4_tilt_eq_lat", 0);
  ssc_data_set_number(data, "subarray4_azimuth", 180);
  ssc_data_set_number(data, "subarray4_track_mode", 0);
  ssc_data_set_number(data, "subarray4_rotlim", 45);
  ssc_data_set_number(data, "subarray4_shade_mode", 0);
  ssc_data_set_number(data, "subarray4_gcr", 0.30000001192092896);
  ssc_number_t p_subarray4_monthly_tilt[12] = {40, 40, 40, 20, 20, 20,
                                               20, 20, 20, 40, 40, 40};
  ssc_data_set_array(data, "subarray4_monthly_tilt", p_subarray4_monthly_tilt,
                     12);
  ssc_number_t p_subarray4_soiling[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  ssc_data_set_array(data, "subarray4_soiling", p_subarray4_soiling, 12);
  ssc_data_set_number(data, "subarray4_mod_orient", 0);
  ssc_data_set_number(data, "subarray4_nmodx", 9);
  ssc_data_set_number(data, "subarray4_nmody", 2);
  ssc_data_set_number(data, "subarray4_backtrack", 0);
  ssc_data_set_number(data, "module_model", 1);
  ssc_data_set_number(data, "module_aspect_ratio", 1.7000000476837158);
  ssc_data_set_number(data, "spe_area", 0.74074000120162964);
  ssc_data_set_number(data, "spe_rad0", 200);
  ssc_data_set_number(data, "spe_rad1", 400);
  ssc_data_set_number(data, "spe_rad2", 600);
  ssc_data_set_number(data, "spe_rad3", 800);
  ssc_data_set_number(data, "spe_rad4", 1000);
  ssc_data_set_number(data, "spe_eff0", 13.5);
  ssc_data_set_number(data, "spe_eff1", 13.5);
  ssc_data_set_number(data, "spe_eff2", 13.5);
  ssc_data_set_number(data, "spe_eff3", 13.5);
  ssc_data_set_number(data, "spe_eff4", 13.5);
  ssc_data_set_number(data, "spe_reference", 4);
  ssc_data_set_number(data, "spe_module_structure", 0);
  ssc_data_set_number(data, "spe_a", -3.559999942779541);
  ssc_data_set_number(data, "spe_b", -0.075000002980232239);
  ssc_data_set_number(data, "spe_dT", 3);
  ssc_data_set_number(data, "spe_temp_coeff", -0.5);
  ssc_data_set_number(data, "spe_fd", 1);
  ssc_data_set_number(data, "spe_vmp", 30);
  ssc_data_set_number(data, "spe_voc", 36);
  ssc_data_set_number(data, "spe_is_bifacial", 0);
  ssc_data_set_number(data, "spe_bifacial_transmission_factor", 0);
  ssc_data_set_number(data, "spe_bifaciality", 0);
  ssc_data_set_number(data, "spe_bifacial_ground_clearance_height", 0);
  ssc_data_set_number(data, "cec_is_bifacial", 0);
  ssc_data_set_number(data, "cec_bifacial_transmission_factor", 0);
  ssc_data_set_number(data, "cec_bifaciality", 0);
  ssc_data_set_number(data, "cec_bifacial_ground_clearance_height", 0);
  ssc_data_set_number(data, "cec_area", 1.6310000419616699);
  ssc_data_set_number(data, "cec_a_ref", 2.4200999736785889);
  ssc_data_set_number(data, "cec_adjust", 5.0100002288818359);
  ssc_data_set_number(data, "cec_alpha_sc", 0.0024920001160353422);
  ssc_data_set_number(data, "cec_beta_oc", -0.16975000500679016);
  ssc_data_set_number(data, "cec_gamma_r", -0.31000000238418579);
  ssc_data_set_number(data, "cec_i_l_ref", 6.2369999885559082);
  ssc_data_set_number(data, "cec_i_mp_ref", 5.8499999046325684);
  ssc_data_set_number(data, "cec_i_o_ref", 3.9799999233813832e-12);
  ssc_data_set_number(data, "cec_i_sc_ref", 6.2300000190734863);
  ssc_data_set_number(data, "cec_n_s", 96);
  ssc_data_set_number(data, "cec_r_s", 0.49900001287460327);
  ssc_data_set_number(data, "cec_r_sh_ref", 457.1199951171875);
  ssc_data_set_number(data, "cec_t_noct", 46.400001525878906);
  ssc_data_set_number(data, "cec_v_mp_ref", 57.299999237060547);
  ssc_data_set_number(data, "cec_v_oc_ref", 67.900001525878906);
  ssc_data_set_number(data, "cec_temp_corr_mode", 0);
  ssc_data_set_number(data, "cec_standoff", 6);
  ssc_data_set_number(data, "cec_height", 0);
  ssc_data_set_number(data, "cec_mounting_config", 0);
  ssc_data_set_number(data, "cec_heat_transfer", 0);
  ssc_data_set_number(data, "cec_mounting_orientation", 0);
  ssc_data_set_number(data, "cec_gap_spacing", 0.05000000074505806);
  ssc_data_set_number(data, "cec_module_width", 1);
  ssc_data_set_number(data, "cec_module_length", 1.6310000419616699);
  ssc_data_set_number(data, "cec_array_rows", 1);
  ssc_data_set_number(data, "cec_array_cols", 10);
  ssc_data_set_number(data, "cec_backside_temp", 20);
  ssc_data_set_number(data, "cec_transient_thermal_model_unit_mass", 11.09186);
  ssc_data_set_number(data, "6par_celltech", 1);
  ssc_data_set_number(data, "6par_is_bifacial", 0);
  ssc_data_set_number(data, "6par_bifacial_transmission_factor", 0);
  ssc_data_set_number(data, "6par_bifaciality", 0);
  ssc_data_set_number(data, "6par_bifacial_ground_clearance_height", 0);
  ssc_data_set_number(data, "6par_vmp", 30);
  ssc_data_set_number(data, "6par_imp", 6);
  ssc_data_set_number(data, "6par_voc", 37);
  ssc_data_set_number(data, "6par_isc", 7);
  ssc_data_set_number(data, "6par_bvoc", -0.10999999940395355);
  ssc_data_set_number(data, "6par_aisc", 0.0040000001899898052);
  ssc_data_set_number(data, "6par_gpmp", -0.40999999642372131);
  ssc_data_set_number(data, "6par_nser", 60);
  ssc_data_set_number(data, "6par_area", 1.2999999523162842);
  ssc_data_set_number(data, "6par_tnoct", 46);
  ssc_data_set_number(data, "6par_standoff", 6);
  ssc_data_set_number(data, "6par_mounting", 0);
  ssc_data_set_number(data, "6par_transient_thermal_model_unit_mass", 11.09186);
  ssc_data_set_number(data, "snl_module_structure", 0);
  ssc_data_set_number(data, "snl_a", -3.619999885559082);
  ssc_data_set_number(data, "snl_b", -0.075000002980232239);
  ssc_data_set_number(data, "snl_dtc", 3);
  ssc_data_set_number(data, "snl_ref_a", -3.619999885559082);
  ssc_data_set_number(data, "snl_ref_b", -0.075000002980232239);
  ssc_data_set_number(data, "snl_ref_dT", 3);
  ssc_data_set_number(data, "snl_fd", 1);
  ssc_data_set_number(data, "snl_a0", 0.94045001268386841);
  ssc_data_set_number(data, "snl_a1", 0.052641000598669052);
  ssc_data_set_number(data, "snl_a2", -0.0093897003680467606);
  ssc_data_set_number(data, "snl_a3", 0.00072622997686266899);
  ssc_data_set_number(data, "snl_a4", -1.9937999240937643e-05);
  ssc_data_set_number(data, "snl_aimp", -0.0003800000122282654);
  ssc_data_set_number(data, "snl_aisc", 0.00060999998822808266);
  ssc_data_set_number(data, "snl_area", 1.2439999580383301);
  ssc_data_set_number(data, "snl_b0", 1);
  ssc_data_set_number(data, "snl_b1", -0.0024379999376833439);
  ssc_data_set_number(data, "snl_b2", 0.00031030000536702573);
  ssc_data_set_number(data, "snl_b3", -1.2460000107239466e-05);
  ssc_data_set_number(data, "snl_b4", 2.1099999969464989e-07);
  ssc_data_set_number(data, "snl_b5", -1.3600000015046021e-09);
  ssc_data_set_number(data, "snl_bvmpo", -0.13899999856948853);
  ssc_data_set_number(data, "snl_bvoco", -0.13600000739097595);
  ssc_data_set_number(data, "snl_c0", 1.0039000511169434);
  ssc_data_set_number(data, "snl_c1", -0.0038999998942017555);
  ssc_data_set_number(data, "snl_c2", 0.29106599092483521);
  ssc_data_set_number(data, "snl_c3", -4.7354598045349121);
  ssc_data_set_number(data, "snl_c4", 0.99419999122619629);
  ssc_data_set_number(data, "snl_c5", 0.0057999999262392521);
  ssc_data_set_number(data, "snl_c6", 1.0722999572753906);
  ssc_data_set_number(data, "snl_c7", -0.072300001978874207);
  ssc_data_set_number(data, "snl_impo", 5.25);
  ssc_data_set_number(data, "snl_isco", 5.75);
  ssc_data_set_number(data, "snl_ixo", 5.6500000953674316);
  ssc_data_set_number(data, "snl_ixxo", 3.8499999046325684);
  ssc_data_set_number(data, "snl_mbvmp", 0);
  ssc_data_set_number(data, "snl_mbvoc", 0);
  ssc_data_set_number(data, "snl_n", 1.2209999561309814);
  ssc_data_set_number(data, "snl_series_cells", 72);
  ssc_data_set_number(data, "snl_vmpo", 40);
  ssc_data_set_number(data, "snl_voco", 47.700000762939453);
  ssc_data_set_number(data, "snl_transient_thermal_model_unit_mass", 11.09186);
  ssc_data_set_number(data, "sd11par_nser", 116);
  ssc_data_set_number(data, "sd11par_area", 0.72000002861022949);
  ssc_data_set_number(data, "sd11par_AMa0", 0.94169998168945313);
  ssc_data_set_number(data, "sd11par_AMa1", 0.065159998834133148);
  ssc_data_set_number(data, "sd11par_AMa2", -0.020220000296831131);
  ssc_data_set_number(data, "sd11par_AMa3", 0.0021899999119341373);
  ssc_data_set_number(data, "sd11par_AMa4", -9.1000001702923328e-05);
  ssc_data_set_number(data, "sd11par_glass", 0);
  ssc_data_set_number(data, "sd11par_tnoct", 44.900001525878906);
  ssc_data_set_number(data, "sd11par_standoff", 6);
  ssc_data_set_number(data, "sd11par_mounting", 0);
  ssc_data_set_number(data, "sd11par_Vmp0", 64.599998474121094);
  ssc_data_set_number(data, "sd11par_Imp0", 1.0500000715255737);
  ssc_data_set_number(data, "sd11par_Voc0", 87);
  ssc_data_set_number(data, "sd11par_Isc0", 1.1799999475479126);
  ssc_data_set_number(data, "sd11par_alphaIsc", 0.00047200100379996002);
  ssc_data_set_number(data, "sd11par_n", 1.4507100582122803);
  ssc_data_set_number(data, "sd11par_Il", 1.1895099878311157);
  ssc_data_set_number(data, "sd11par_Io", 2.0852199966725493e-09);
  ssc_data_set_number(data, "sd11par_Egref", 0.73766797780990601);
  ssc_data_set_number(data, "sd11par_d1", 13.550399780273438);
  ssc_data_set_number(data, "sd11par_d2", -0.076973497867584229);
  ssc_data_set_number(data, "sd11par_d3", 0.23732699453830719);
  ssc_data_set_number(data, "sd11par_c1", 1930.1500244140625);
  ssc_data_set_number(data, "sd11par_c2", 474.6400146484375);
  ssc_data_set_number(data, "sd11par_c3", 1.4874600172042847);
  ssc_data_set_number(data, "inverter_model", 0);
  ssc_data_set_number(data, "mppt_low_inverter", 250);
  ssc_data_set_number(data, "mppt_hi_inverter", 480);
  ssc_data_set_number(data, "inv_num_mppt", 1);
  ssc_data_set_number(data, "subarray1_mppt_input", 1);
  ssc_data_set_number(data, "subarray2_mppt_input", 1);
  ssc_data_set_number(data, "subarray3_mppt_input", 1);
  ssc_data_set_number(data, "subarray4_mppt_input", 1);
  ssc_data_set_number(data, "inv_snl_c0", -3.1800000215298496e-06);
  ssc_data_set_number(data, "inv_snl_c1", -5.1200000598328188e-05);
  ssc_data_set_number(data, "inv_snl_c2", 0.00098400004208087921);
  ssc_data_set_number(data, "inv_snl_c3", -0.0015099999727681279);
  ssc_data_set_number(data, "inv_snl_paco", 3800);
  ssc_data_set_number(data, "inv_snl_pdco", 3928.1142578125);
  ssc_data_set_number(data, "inv_snl_pnt", 0.99000000953674316);
  ssc_data_set_number(data, "inv_snl_pso", 19.451622009277344);
  ssc_data_set_number(data, "inv_snl_vdco", 398.49667358398438);
  ssc_data_set_number(data, "inv_snl_vdcmax", 600);
  ssc_data_set_number(data, "inv_cec_cg_c0", -3.1751958431414096e-06);
  ssc_data_set_number(data, "inv_cec_cg_c1", -5.1231381803518161e-05);
  ssc_data_set_number(data, "inv_cec_cg_c2", 0.0009835963137447834);
  ssc_data_set_number(data, "inv_cec_cg_c3", -0.0015077980933710933);
  ssc_data_set_number(data, "inv_cec_cg_paco", 3800);
  ssc_data_set_number(data, "inv_cec_cg_pdco", 3928.11376953125);
  ssc_data_set_number(data, "inv_cec_cg_pnt", 0.99000000953674316);
  ssc_data_set_number(data, "inv_cec_cg_psco", 19.448383331298828);
  ssc_data_set_number(data, "inv_cec_cg_vdco", 398.49661254882813);
  ssc_data_set_number(data, "inv_cec_cg_vdcmax", 600);
  ssc_data_set_number(data, "inv_ds_paco", 4000);
  ssc_data_set_number(data, "inv_ds_eff", 96);
  ssc_data_set_number(data, "inv_ds_pnt", 1);
  ssc_data_set_number(data, "inv_ds_pso", 0);
  ssc_data_set_number(data, "inv_ds_vdco", 310);
  ssc_data_set_number(data, "inv_ds_vdcmax", 600);
  ssc_data_set_number(data, "inv_pd_paco", 4000);
  ssc_data_set_number(data, "inv_pd_pdco", 4210.5263671875);
  ssc_number_t p_inv_pd_partload[251] = {0,
                                         0.40400001406669617,
                                         0.80800002813339233,
                                         1.2120000123977661,
                                         1.6160000562667847,
                                         2.0199999809265137,
                                         2.4240000247955322,
                                         2.8280000686645508,
                                         3.2320001125335693,
                                         3.6359999179840088,
                                         4.0399999618530273,
                                         4.4439997673034668,
                                         4.8480000495910645,
                                         5.2519998550415039,
                                         5.6560001373291016,
                                         6.059999942779541,
                                         6.4640002250671387,
                                         6.8680000305175781,
                                         7.2719998359680176,
                                         7.6760001182556152,
                                         8.0799999237060547,
                                         8.4840002059936523,
                                         8.8879995346069336,
                                         9.2919998168945313,
                                         9.6960000991821289,
                                         10.100000381469727,
                                         10.503999710083008,
                                         10.907999992370605,
                                         11.312000274658203,
                                         11.715999603271484,
                                         12.119999885559082,
                                         12.52400016784668,
                                         12.928000450134277,
                                         13.331999778747559,
                                         13.736000061035156,
                                         14.140000343322754,
                                         14.543999671936035,
                                         14.947999954223633,
                                         15.35200023651123,
                                         15.755999565124512,
                                         16.159999847412109,
                                         16.563999176025391,
                                         16.968000411987305,
                                         17.371999740600586,
                                         17.775999069213867,
                                         18.180000305175781,
                                         18.583999633789063,
                                         18.988000869750977,
                                         19.392000198364258,
                                         19.795999526977539,
                                         20.200000762939453,
                                         20.604000091552734,
                                         21.007999420166016,
                                         21.41200065612793,
                                         21.815999984741211,
                                         22.219999313354492,
                                         22.624000549316406,
                                         23.027999877929688,
                                         23.431999206542969,
                                         23.836000442504883,
                                         24.239999771118164,
                                         24.643999099731445,
                                         25.048000335693359,
                                         25.451999664306641,
                                         25.856000900268555,
                                         26.260000228881836,
                                         26.663999557495117,
                                         27.068000793457031,
                                         27.472000122070313,
                                         27.875999450683594,
                                         28.280000686645508,
                                         28.684000015258789,
                                         29.08799934387207,
                                         29.492000579833984,
                                         29.895999908447266,
                                         30.299999237060547,
                                         30.704000473022461,
                                         31.107999801635742,
                                         31.511999130249023,
                                         31.916000366210938,
                                         32.319999694824219,
                                         32.7239990234375,
                                         33.127998352050781,
                                         33.532001495361328,
                                         33.936000823974609,
                                         34.340000152587891,
                                         34.743999481201172,
                                         35.147998809814453,
                                         35.551998138427734,
                                         35.956001281738281,
                                         36.360000610351563,
                                         36.763999938964844,
                                         37.167999267578125,
                                         37.571998596191406,
                                         37.976001739501953,
                                         38.380001068115234,
                                         38.784000396728516,
                                         39.187999725341797,
                                         39.591999053955078,
                                         39.995998382568359,
                                         40.400001525878906,
                                         40.804000854492188,
                                         41.208000183105469,
                                         41.61199951171875,
                                         42.015998840332031,
                                         42.419998168945313,
                                         42.824001312255859,
                                         43.228000640869141,
                                         43.631999969482422,
                                         44.035999298095703,
                                         44.439998626708984,
                                         44.844001770019531,
                                         45.248001098632813,
                                         45.652000427246094,
                                         46.055999755859375,
                                         46.459999084472656,
                                         46.863998413085938,
                                         47.268001556396484,
                                         47.672000885009766,
                                         48.076000213623047,
                                         48.479999542236328,
                                         48.883998870849609,
                                         49.287998199462891,
                                         49.692001342773438,
                                         50.096000671386719,
                                         50.5,
                                         50.903999328613281,
                                         51.307998657226563,
                                         51.712001800537109,
                                         52.116001129150391,
                                         52.520000457763672,
                                         52.923999786376953,
                                         53.327999114990234,
                                         53.731998443603516,
                                         54.136001586914063,
                                         54.540000915527344,
                                         54.944000244140625,
                                         55.347999572753906,
                                         55.751998901367188,
                                         56.155998229980469,
                                         56.560001373291016,
                                         56.964000701904297,
                                         57.368000030517578,
                                         57.771999359130859,
                                         58.175998687744141,
                                         58.580001831054688,
                                         58.984001159667969,
                                         59.38800048828125,
                                         59.791999816894531,
                                         60.195999145507813,
                                         60.599998474121094,
                                         61.004001617431641,
                                         61.408000946044922,
                                         61.812000274658203,
                                         62.215999603271484,
                                         62.619998931884766,
                                         63.023998260498047,
                                         63.428001403808594,
                                         63.832000732421875,
                                         64.236000061035156,
                                         64.639999389648438,
                                         65.043998718261719,
                                         65.447998046875,
                                         65.851997375488281,
                                         66.255996704101563,
                                         66.660003662109375,
                                         67.064002990722656,
                                         67.468002319335938,
                                         67.872001647949219,
                                         68.2760009765625,
                                         68.680000305175781,
                                         69.083999633789063,
                                         69.487998962402344,
                                         69.891998291015625,
                                         70.295997619628906,
                                         70.699996948242188,
                                         71.103996276855469,
                                         71.508003234863281,
                                         71.912002563476563,
                                         72.316001892089844,
                                         72.720001220703125,
                                         73.124000549316406,
                                         73.527999877929688,
                                         73.931999206542969,
                                         74.33599853515625,
                                         74.739997863769531,
                                         75.143997192382813,
                                         75.547996520996094,
                                         75.952003479003906,
                                         76.356002807617188,
                                         76.760002136230469,
                                         77.16400146484375,
                                         77.568000793457031,
                                         77.972000122070313,
                                         78.375999450683594,
                                         78.779998779296875,
                                         79.183998107910156,
                                         79.587997436523438,
                                         79.991996765136719,
                                         80.396003723144531,
                                         80.800003051757813,
                                         81.204002380371094,
                                         81.608001708984375,
                                         82.012001037597656,
                                         82.416000366210938,
                                         82.819999694824219,
                                         83.2239990234375,
                                         83.627998352050781,
                                         84.031997680664063,
                                         84.435997009277344,
                                         84.839996337890625,
                                         85.244003295898438,
                                         85.648002624511719,
                                         86.052001953125,
                                         86.456001281738281,
                                         86.860000610351563,
                                         87.263999938964844,
                                         87.667999267578125,
                                         88.071998596191406,
                                         88.475997924804688,
                                         88.879997253417969,
                                         89.28399658203125,
                                         89.688003540039063,
                                         90.092002868652344,
                                         90.496002197265625,
                                         90.900001525878906,
                                         91.304000854492188,
                                         91.708000183105469,
                                         92.11199951171875,
                                         92.515998840332031,
                                         92.919998168945313,
                                         93.323997497558594,
                                         93.727996826171875,
                                         94.132003784179688,
                                         94.536003112792969,
                                         94.94000244140625,
                                         95.344001770019531,
                                         95.748001098632813,
                                         96.152000427246094,
                                         96.555999755859375,
                                         96.959999084472656,
                                         97.363998413085938,
                                         97.767997741699219,
                                         98.1719970703125,
                                         98.575996398925781,
                                         98.980003356933594,
                                         99.384002685546875,
                                         99.788002014160156,
                                         100.19200134277344,
                                         100.59600067138672,
                                         101};
  ssc_data_set_array(data, "inv_pd_partload", p_inv_pd_partload, 251);
  ssc_number_t p_inv_pd_efficiency[251] = {0,
                                           0,
                                           34.419998168945313,
                                           55.200000762939453,
                                           65.589996337890625,
                                           71.819999694824219,
                                           75.970001220703125,
                                           78.94000244140625,
                                           81.169998168945313,
                                           82.900001525878906,
                                           84.279998779296875,
                                           85.419998168945313,
                                           86.360000610351563,
                                           87.160003662109375,
                                           87.839996337890625,
                                           88.44000244140625,
                                           88.949996948242188,
                                           89.410003662109375,
                                           89.819999694824219,
                                           90.180000305175781,
                                           90.510002136230469,
                                           90.80999755859375,
                                           91.080001831054688,
                                           91.319999694824219,
                                           91.550003051757813,
                                           91.75,
                                           91.949996948242188,
                                           92.120002746582031,
                                           92.290000915527344,
                                           92.44000244140625,
                                           92.580001831054688,
                                           92.720001220703125,
                                           92.839996337890625,
                                           92.959999084472656,
                                           93.069999694824219,
                                           93.169998168945313,
                                           93.269996643066406,
                                           93.370002746582031,
                                           93.449996948242188,
                                           93.540000915527344,
                                           93.620002746582031,
                                           93.69000244140625,
                                           93.760002136230469,
                                           93.830001831054688,
                                           93.900001525878906,
                                           93.959999084472656,
                                           94.019996643066406,
                                           94.080001831054688,
                                           94.129997253417969,
                                           94.180000305175781,
                                           94.230003356933594,
                                           94.279998779296875,
                                           94.330001831054688,
                                           94.370002746582031,
                                           94.419998168945313,
                                           94.459999084472656,
                                           94.5,
                                           94.540000915527344,
                                           94.569999694824219,
                                           94.610000610351563,
                                           94.639999389648438,
                                           94.680000305175781,
                                           94.709999084472656,
                                           94.739997863769531,
                                           94.769996643066406,
                                           94.800003051757813,
                                           94.830001831054688,
                                           94.860000610351563,
                                           94.889999389648438,
                                           94.910003662109375,
                                           94.94000244140625,
                                           94.959999084472656,
                                           94.980003356933594,
                                           95.010002136230469,
                                           95.029998779296875,
                                           95.050003051757813,
                                           95.069999694824219,
                                           95.089996337890625,
                                           95.110000610351563,
                                           95.129997253417969,
                                           95.150001525878906,
                                           95.169998168945313,
                                           95.19000244140625,
                                           95.209999084472656,
                                           95.230003356933594,
                                           95.239997863769531,
                                           95.260002136230469,
                                           95.279998779296875,
                                           95.290000915527344,
                                           95.30999755859375,
                                           95.319999694824219,
                                           95.339996337890625,
                                           95.349998474121094,
                                           95.360000610351563,
                                           95.379997253417969,
                                           95.389999389648438,
                                           95.400001525878906,
                                           95.419998168945313,
                                           95.430000305175781,
                                           95.44000244140625,
                                           95.449996948242188,
                                           95.470001220703125,
                                           95.480003356933594,
                                           95.489997863769531,
                                           95.5,
                                           95.510002136230469,
                                           95.519996643066406,
                                           95.529998779296875,
                                           95.540000915527344,
                                           95.550003051757813,
                                           95.55999755859375,
                                           95.569999694824219,
                                           95.580001831054688,
                                           95.589996337890625,
                                           95.599998474121094,
                                           95.610000610351563,
                                           95.620002746582031,
                                           95.629997253417969,
                                           95.639999389648438,
                                           95.639999389648438,
                                           95.650001525878906,
                                           95.660003662109375,
                                           95.669998168945313,
                                           95.680000305175781,
                                           95.680000305175781,
                                           95.69000244140625,
                                           95.699996948242188,
                                           95.709999084472656,
                                           95.709999084472656,
                                           95.720001220703125,
                                           95.730003356933594,
                                           95.730003356933594,
                                           95.739997863769531,
                                           95.75,
                                           95.75,
                                           95.760002136230469,
                                           95.769996643066406,
                                           95.769996643066406,
                                           95.779998779296875,
                                           95.779998779296875,
                                           95.790000915527344,
                                           95.800003051757813,
                                           95.800003051757813,
                                           95.80999755859375,
                                           95.80999755859375,
                                           95.819999694824219,
                                           95.819999694824219,
                                           95.830001831054688,
                                           95.830001831054688,
                                           95.839996337890625,
                                           95.839996337890625,
                                           95.849998474121094,
                                           95.849998474121094,
                                           95.860000610351563,
                                           95.860000610351563,
                                           95.870002746582031,
                                           95.870002746582031,
                                           95.879997253417969,
                                           95.879997253417969,
                                           95.889999389648438,
                                           95.889999389648438,
                                           95.889999389648438,
                                           95.900001525878906,
                                           95.900001525878906,
                                           95.910003662109375,
                                           95.910003662109375,
                                           95.910003662109375,
                                           95.919998168945313,
                                           95.919998168945313,
                                           95.930000305175781,
                                           95.930000305175781,
                                           95.930000305175781,
                                           95.94000244140625,
                                           95.94000244140625,
                                           95.94000244140625,
                                           95.949996948242188,
                                           95.949996948242188,
                                           95.959999084472656,
                                           95.959999084472656,
                                           95.959999084472656,
                                           95.970001220703125,
                                           95.970001220703125,
                                           95.970001220703125,
                                           95.980003356933594,
                                           95.980003356933594,
                                           95.980003356933594,
                                           95.980003356933594,
                                           95.989997863769531,
                                           95.989997863769531,
                                           95.989997863769531,
                                           96,
                                           96,
                                           96,
                                           96.010002136230469,
                                           96.010002136230469,
                                           96.010002136230469,
                                           96.010002136230469,
                                           96.019996643066406,
                                           96.019996643066406,
                                           96.019996643066406,
                                           96.019996643066406,
                                           96.029998779296875,
                                           96.029998779296875,
                                           96.029998779296875,
                                           96.029998779296875,
                                           96.040000915527344,
                                           96.040000915527344,
                                           96.040000915527344,
                                           96.040000915527344,
                                           96.050003051757813,
                                           96.050003051757813,
                                           96.050003051757813,
                                           96.050003051757813,
                                           96.05999755859375,
                                           96.05999755859375,
                                           96.05999755859375,
                                           96.05999755859375,
                                           96.05999755859375,
                                           96.069999694824219,
                                           96.069999694824219,
                                           96.069999694824219,
                                           96.069999694824219,
                                           96.069999694824219,
                                           96.080001831054688,
                                           96.080001831054688,
                                           96.080001831054688,
                                           96.080001831054688,
                                           96.080001831054688,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.120002746582031,
                                           96.120002746582031,
                                           96.120002746582031,
                                           96.120002746582031,
                                           96.120002746582031};
  ssc_data_set_array(data, "inv_pd_efficiency", p_inv_pd_efficiency, 251);
  ssc_data_set_number(data, "inv_pd_pnt", 0);
  ssc_data_set_number(data, "inv_pd_vdco", 310);
  ssc_data_set_number(data, "inv_pd_vdcmax", 600);
  ssc_data_set_number(data, "adjust:constant", 0);
  ssc_data_set_number(data, "dc_adjust:constant", 0);
  ssc_data_set_number(data, "inv_snl_eff_cec", 96.636932373046875);
  ssc_data_set_number(data, "inv_pd_eff", 95);
  ssc_data_set_number(data, "inv_cec_cg_eff_cec", 96.636306762695313);
  ssc_number_t matrix_inv_tdc[3] = {1., 52.8, -0.021};
  ssc_data_set_matrix(data, "inv_tdc_cec_db", matrix_inv_tdc, 1, 3);
  ssc_data_set_matrix(data, "inv_tdc_cec_cg", matrix_inv_tdc, 1, 3);
  ssc_data_set_matrix(data, "inv_tdc_plc", matrix_inv_tdc, 1, 3);
  ssc_data_set_matrix(data, "inv_tdc_ds", matrix_inv_tdc, 1, 3);
}

/**
 *  Default data for belpe run that can be further modified
 *  Includes a critical load for resiliency. Will increase runtimes if used
 */
void belpe_default(ssc_data_t &data) {
  ssc_data_set_number(data, "en_belpe", 0);
  set_array(data, "load", load_profile_path, 8760);
  ssc_number_t p_load_escalation[1] = {0};
  ssc_data_set_array(data, "load_escalation", p_load_escalation, 1);
  set_array(data, "crit_load", load_profile_path, 8760);
  ssc_data_set_string(data, "solar_resource_file", solar_resource_path);
  ssc_data_set_number(data, "floor_area", 2000);
  ssc_data_set_number(data, "Stories", 2);
  ssc_data_set_number(data, "YrBuilt", 1980);
  ssc_data_set_number(data, "Retrofits", 0);
  ssc_data_set_number(data, "Occupants", 4);
  ssc_number_t p_Occ_Schedule[24] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ssc_data_set_array(data, "Occ_Schedule", p_Occ_Schedule, 24);
  ssc_data_set_number(data, "THeat", 68);
  ssc_data_set_number(data, "TCool", 76);
  ssc_data_set_number(data, "THeatSB", 68);
  ssc_data_set_number(data, "TCoolSB", 76);
  ssc_number_t p_T_Sched[24] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ssc_data_set_array(data, "T_Sched", p_T_Sched, 24);
  ssc_data_set_number(data, "en_heat", 1);
  ssc_data_set_number(data, "en_cool", 1);
  ssc_data_set_number(data, "en_fridge", 1);
  ssc_data_set_number(data, "en_range", 1);
  ssc_data_set_number(data, "en_dish", 1);
  ssc_data_set_number(data, "en_wash", 1);
  ssc_data_set_number(data, "en_dry", 1);
  ssc_data_set_number(data, "en_mels", 1);
  ssc_number_t p_Monthly_util[12] = {725,  630,  665,  795,  1040, 1590,
                                     1925, 1730, 1380, 1080, 635,  715};
  ssc_data_set_array(data, "Monthly_util", p_Monthly_util, 12);
}

/**
 * Default battery data that can be further modified
 */
void battery_data_default(ssc_data_t &data) {
  ssc_data_set_number(data, "en_batt", 0);
  ssc_data_set_number(data, "batt_replacement_option", 0);
  ssc_data_set_number(data, "adjust:constant", 0);
  ssc_data_set_number(data, "dc_adjust:constant", 0);
  ssc_data_set_number(data, "batt_chem", 1);
  ssc_data_set_number(data, "inv_snl_eff_cec", 96.636932373046875);
  ssc_data_set_number(data, "inv_pd_eff", 95);
  ssc_data_set_number(data, "inv_cec_cg_eff_cec", 96.636306762695313);
  ssc_data_set_number(data, "batt_inverter_efficiency_cutoff", 90);
  ssc_data_set_number(data, "batt_ac_or_dc", 1);
  ssc_data_set_number(data, "batt_dc_dc_efficiency", 99);
  ssc_data_set_number(data, "batt_dc_ac_efficiency", 96);
  ssc_data_set_number(data, "batt_ac_dc_efficiency", 96);
  ssc_data_set_number(data, "batt_inverter_efficiency_cutoff", 90);
  ssc_data_set_number(data, "batt_initial_SOC", 50);
  set_array(data, "batt_room_temperature_celsius", temperature_path, 8760);

  ssc_data_set_number(data, "batt_meter_position", 0);
  ssc_number_t p_batt_losses[1] = {0};
  ssc_data_set_array(data, "batt_losses", p_batt_losses, 1);
  ssc_number_t p_batt_losses_charging[12] = {0, 0, 0, 0, 0, 0,
                                             0, 0, 0, 0, 0, 0};
  ssc_data_set_array(data, "batt_losses_charging", p_batt_losses_charging, 12);
  ssc_number_t p_batt_losses_discharging[12] = {0, 0, 0, 0, 0, 0,
                                                0, 0, 0, 0, 0, 0};
  ssc_data_set_array(data, "batt_losses_discharging", p_batt_losses_discharging,
                     12);
  ssc_number_t p_batt_losses_idle[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ssc_data_set_array(data, "batt_losses_idle", p_batt_losses_idle, 12);
  ssc_data_set_number(data, "batt_loss_choice", 0);
  ssc_data_set_number(data, "batt_current_choice", 0);
  ssc_data_set_number(data, "batt_computed_strings", 88);
  ssc_data_set_number(data, "batt_computed_series", 14);
  ssc_data_set_number(data, "batt_computed_bank_capacity", 9.9791994094848633);
  ssc_data_set_number(data, "batt_current_charge_max", 99);
  ssc_data_set_number(data, "batt_current_discharge_max", 99);
  ssc_data_set_number(data, "batt_power_charge_max_kwdc", 4.9895997047424316);
  ssc_data_set_number(data, "batt_power_discharge_max_kwdc",
                      4.9895997047424316);
  ssc_data_set_number(data, "batt_power_charge_max_kwac", 4.9895997047424316);
  ssc_data_set_number(data, "batt_power_discharge_max_kwac",
                      4.9895997047424316);
  ssc_data_set_number(data, "batt_voltage_choice", 0);
  ssc_data_set_number(data, "batt_Vfull", 4.0999999046325684);
  ssc_data_set_number(data, "batt_Vexp", 4.0500001907348633);
  ssc_data_set_number(data, "batt_Vnom", 3.4000000953674316);
  ssc_data_set_number(data, "batt_Vnom_default", 3.5999999046325684);
  ssc_data_set_number(data, "batt_Qfull", 2.25);
  ssc_data_set_number(data, "batt_Qfull_flow", 198);
  ssc_data_set_number(data, "batt_Qexp", 0.040049999952316284);
  ssc_data_set_number(data, "batt_Qnom", 2.0002501010894775);
  ssc_data_set_number(data, "batt_C_rate", 0.20000000298023224);
  ssc_data_set_number(data, "batt_resistance", 0.0002);
  ssc_number_t p_batt_voltage_matrix[2] = {0, 0};
  ssc_data_set_matrix(data, "batt_voltage_matrix", p_batt_voltage_matrix, 1, 2);
  ssc_data_set_number(data, "LeadAcid_q20_computed", 198);
  ssc_data_set_number(data, "LeadAcid_q10_computed", 184.13999938964844);
  ssc_data_set_number(data, "LeadAcid_qn_computed", 118.80000305175781);
  ssc_data_set_number(data, "LeadAcid_tn", 1);
  ssc_data_set_number(data, "batt_minimum_SOC", 15);
  ssc_data_set_number(data, "batt_maximum_SOC", 95);
  ssc_data_set_number(data, "batt_minimum_modetime", 10);
  ssc_data_set_number(data, "batt_life_model", 0);
  ssc_number_t p_batt_lifetime_matrix[18] = {20, 0,     100, 20, 5000, 80,
                                             20, 10000, 60,  80, 0,    100,
                                             80, 1000,  80,  80, 2000, 60};
  ssc_data_set_matrix(data, "batt_lifetime_matrix", p_batt_lifetime_matrix, 6,
                      3);
  ssc_data_set_number(data, "batt_replacement_capacity", 50);
  ssc_data_set_number(data, "batt_calendar_choice", 1);
  ssc_number_t p_batt_calendar_lifetime_matrix[6] = {0,  100,  3650,
                                                     80, 7300, 50};
  ssc_data_set_matrix(data, "batt_calendar_lifetime_matrix",
                      p_batt_calendar_lifetime_matrix, 3, 2);
  ssc_data_set_number(data, "batt_calendar_q0", 1.0199999809265137);
  ssc_data_set_number(data, "batt_calendar_a", 0.0026599999982863665);
  ssc_data_set_number(data, "batt_calendar_b", -7280);
  ssc_data_set_number(data, "batt_calendar_c", 930);
  ssc_data_set_number(data, "batt_mass", 50.571121215820313);
  ssc_data_set_number(data, "batt_surface_area", 0.4374);
  ssc_data_set_number(data, "batt_Cp", 1004);
  ssc_data_set_number(data, "batt_h_to_ambient", 20);
  ssc_data_set_number(data, "T_room", 20);
  ssc_number_t p_cap_vs_temp[8] = {-10, 60, 0, 80, 25, 100, 40, 100};
  ssc_data_set_matrix(data, "cap_vs_temp", p_cap_vs_temp, 4, 2);
  ssc_number_t p_dispatch_manual_charge[6] = {1, 1, 1, 0, 0, 0};
  ssc_data_set_array(data, "dispatch_manual_charge", p_dispatch_manual_charge,
                     6);
  ssc_number_t p_dispatch_manual_discharge[6] = {0, 0, 1, 0, 0, 0};
  ssc_data_set_array(data, "dispatch_manual_discharge",
                     p_dispatch_manual_discharge, 6);
  ssc_number_t p_dispatch_manual_gridcharge[6] = {0, 1, 0, 0, 0, 0};
  ssc_data_set_array(data, "dispatch_manual_gridcharge",
                     p_dispatch_manual_gridcharge, 6);
  ssc_number_t p_dispatch_manual_percent_discharge[2] = {15, 0};
  ssc_data_set_array(data, "dispatch_manual_percent_discharge",
                     p_dispatch_manual_percent_discharge, 2);
  ssc_number_t p_dispatch_manual_percent_gridcharge[2] = {25, 0};
  ssc_data_set_array(data, "dispatch_manual_percent_gridcharge",
                     p_dispatch_manual_percent_gridcharge, 2);
  ssc_number_t p_dispatch_manual_sched[288] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1, 1};
  ssc_data_set_matrix(data, "dispatch_manual_sched", p_dispatch_manual_sched,
                      12, 24);
  ssc_number_t p_dispatch_manual_sched_weekend[288] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ssc_data_set_matrix(data, "dispatch_manual_sched_weekend",
                      p_dispatch_manual_sched_weekend, 12, 24);
  set_array(data, "batt_target_power", target_power_path, 8760);
  ssc_number_t p_batt_target_power_monthly[12] = {1, 1, 1, 1, 1, 1,
                                                  1, 1, 1, 1, 1, 1};
  ssc_data_set_array(data, "batt_target_power_monthly",
                     p_batt_target_power_monthly, 12);
  ssc_data_set_number(data, "batt_target_choice", 0);
  ssc_data_set_number(data, "batt_dispatch_choice", 3);
  ssc_data_set_number(data, "batt_cycle_cost_choice", 0);
  ssc_number_t p_batt_cycle_cost[1] = {0.1};
  ssc_data_set_array(data, "batt_cycle_cost", p_batt_cycle_cost, 1);
}

/**
 *  Default data for pvsamv1 residential run that can be further modified
 *  Also runs battery_data_default
 */
void pvsamv1_with_residential_default(ssc_data_t &data) {
  ssc_data_set_number(data, "transformer_no_load_loss", 0);
  ssc_data_set_number(data, "transformer_load_loss", 0);
  ssc_data_set_number(data, "system_use_lifetime_output", 0);
  ssc_data_set_number(data, "analysis_period", 25);
  ssc_number_t p_dc_degradation[1] = {0.5};
  ssc_data_set_array(data, "dc_degradation", p_dc_degradation, 1);
  ssc_data_set_number(data, "en_dc_lifetime_losses", 0);
  ssc_number_t p_dc_lifetime_losses[1] = {0};
  ssc_data_set_array(data, "dc_lifetime_losses", p_dc_lifetime_losses, 1);
  ssc_data_set_number(data, "en_ac_lifetime_losses", 0);
  ssc_number_t p_ac_lifetime_losses[1] = {0};
  ssc_data_set_array(data, "ac_lifetime_losses", p_ac_lifetime_losses, 1);
  ssc_data_set_number(data, "en_snow_model", 0);
  ssc_data_set_number(data, "system_capacity", 4.6928696632385254);
  ssc_data_set_number(data, "use_wf_albedo", 0);
  ssc_number_t p_albedo[12] = {
      0.20000000298023224, 0.20000000298023224, 0.20000000298023224,
      0.20000000298023224, 0.20000000298023224, 0.20000000298023224,
      0.20000000298023224, 0.20000000298023224, 0.20000000298023224,
      0.20000000298023224, 0.20000000298023224, 0.20000000298023224};
  ssc_data_set_array(data, "albedo", p_albedo, 12);
  ssc_data_set_number(data, "irrad_mode", 0);
  ssc_data_set_number(data, "sky_model", 2);
  ssc_data_set_number(data, "inverter_count", 1);
  ssc_data_set_number(data, "enable_mismatch_vmax_calc", 0);
  ssc_data_set_number(data, "subarray1_nstrings", 2);
  ssc_data_set_number(data, "subarray1_modules_per_string", 7);
  ssc_data_set_number(data, "subarray1_mppt_input", 1);
  ssc_data_set_number(data, "subarray1_tilt", 20);
  ssc_data_set_number(data, "subarray1_tilt_eq_lat", 0);
  ssc_data_set_number(data, "subarray1_azimuth", 180);
  ssc_data_set_number(data, "subarray1_track_mode", 0);
  ssc_data_set_number(data, "subarray1_rotlim", 45);
  ssc_data_set_number(data, "subarray1_shade_mode", 0);
  ssc_data_set_number(data, "subarray1_gcr", 0.30000001192092896);
  ssc_number_t p_subarray1_monthly_tilt[12] = {40, 40, 40, 20, 20, 20,
                                               20, 20, 20, 40, 40, 40};
  ssc_data_set_array(data, "subarray1_monthly_tilt", p_subarray1_monthly_tilt,
                     12);
  ssc_number_t p_subarray1_soiling[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  ssc_data_set_array(data, "subarray1_soiling", p_subarray1_soiling, 12);
  ssc_data_set_number(data, "subarray1_rear_irradiance_loss", 0);
  ssc_data_set_number(data, "subarray1_mismatch_loss", 2);
  ssc_data_set_number(data, "subarray1_diodeconn_loss", 0.5);
  ssc_data_set_number(data, "subarray1_dcwiring_loss", 2);
  ssc_data_set_number(data, "subarray1_tracking_loss", 0);
  ssc_data_set_number(data, "subarray1_nameplate_loss", 0);
  ssc_data_set_number(data, "subarray2_rear_irradiance_loss", 0);
  ssc_data_set_number(data, "subarray2_mismatch_loss", 2);
  ssc_data_set_number(data, "subarray2_diodeconn_loss", 0.5);
  ssc_data_set_number(data, "subarray2_dcwiring_loss", 2);
  ssc_data_set_number(data, "subarray2_tracking_loss", 0);
  ssc_data_set_number(data, "subarray2_nameplate_loss", 0);
  ssc_data_set_number(data, "subarray3_rear_irradiance_loss", 0);
  ssc_data_set_number(data, "subarray3_mismatch_loss", 2);
  ssc_data_set_number(data, "subarray3_diodeconn_loss", 0.5);
  ssc_data_set_number(data, "subarray3_dcwiring_loss", 2);
  ssc_data_set_number(data, "subarray3_tracking_loss", 0);
  ssc_data_set_number(data, "subarray3_nameplate_loss", 0);
  ssc_data_set_number(data, "subarray4_rear_irradiance_loss", 0);
  ssc_data_set_number(data, "subarray4_mismatch_loss", 2);
  ssc_data_set_number(data, "subarray4_diodeconn_loss", 0.5);
  ssc_data_set_number(data, "subarray4_dcwiring_loss", 2);
  ssc_data_set_number(data, "subarray4_tracking_loss", 0);
  ssc_data_set_number(data, "subarray4_nameplate_loss", 0);
  ssc_data_set_number(data, "dcoptimizer_loss", 0);
  ssc_data_set_number(data, "acwiring_loss", 1);
  ssc_data_set_number(data, "transmission_loss", 0);
  ssc_data_set_number(data, "subarray1_mod_orient", 0);
  ssc_data_set_number(data, "subarray1_nmodx", 7);
  ssc_data_set_number(data, "subarray1_nmody", 2);
  ssc_data_set_number(data, "subarray1_backtrack", 0);
  ssc_data_set_number(data, "subarray2_enable", 0);
  ssc_data_set_number(data, "subarray2_nstrings", 0);
  ssc_data_set_number(data, "subarray2_modules_per_string", 7);
  ssc_data_set_number(data, "subarray2_mppt_input", 1);
  ssc_data_set_number(data, "subarray2_tilt", 20);
  ssc_data_set_number(data, "subarray2_tilt_eq_lat", 0);
  ssc_data_set_number(data, "subarray2_azimuth", 180);
  ssc_data_set_number(data, "subarray2_track_mode", 0);
  ssc_data_set_number(data, "subarray2_rotlim", 45);
  ssc_data_set_number(data, "subarray2_shade_mode", 0);
  ssc_data_set_number(data, "subarray2_gcr", 0.30000001192092896);
  ssc_number_t p_subarray2_monthly_tilt[12] = {40, 40, 40, 20, 20, 20,
                                               20, 20, 20, 40, 40, 40};
  ssc_data_set_array(data, "subarray2_monthly_tilt", p_subarray2_monthly_tilt,
                     12);
  ssc_number_t p_subarray2_soiling[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  ssc_data_set_array(data, "subarray2_soiling", p_subarray2_soiling, 12);
  ssc_data_set_number(data, "subarray2_mod_orient", 0);
  ssc_data_set_number(data, "subarray2_nmodx", 9);
  ssc_data_set_number(data, "subarray2_nmody", 2);
  ssc_data_set_number(data, "subarray2_backtrack", 0);
  ssc_data_set_number(data, "subarray3_enable", 0);
  ssc_data_set_number(data, "subarray3_nstrings", 0);
  ssc_data_set_number(data, "subarray3_modules_per_string", 7);
  ssc_data_set_number(data, "subarray3_mppt_input", 1);
  ssc_data_set_number(data, "subarray3_tilt", 20);
  ssc_data_set_number(data, "subarray3_tilt_eq_lat", 0);
  ssc_data_set_number(data, "subarray3_azimuth", 180);
  ssc_data_set_number(data, "subarray3_track_mode", 0);
  ssc_data_set_number(data, "subarray3_rotlim", 45);
  ssc_data_set_number(data, "subarray3_shade_mode", 0);
  ssc_data_set_number(data, "subarray3_gcr", 0.30000001192092896);
  ssc_number_t p_subarray3_monthly_tilt[12] = {40, 40, 40, 20, 20, 20,
                                               20, 20, 20, 40, 40, 40};
  ssc_data_set_array(data, "subarray3_monthly_tilt", p_subarray3_monthly_tilt,
                     12);
  ssc_number_t p_subarray3_soiling[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  ssc_data_set_array(data, "subarray3_soiling", p_subarray3_soiling, 12);
  ssc_data_set_number(data, "subarray3_mod_orient", 0);
  ssc_data_set_number(data, "subarray3_nmodx", 9);
  ssc_data_set_number(data, "subarray3_nmody", 2);
  ssc_data_set_number(data, "subarray3_backtrack", 0);
  ssc_data_set_number(data, "subarray4_enable", 0);
  ssc_data_set_number(data, "subarray4_nstrings", 0);
  ssc_data_set_number(data, "subarray4_modules_per_string", 7);
  ssc_data_set_number(data, "subarray4_mppt_input", 1);
  ssc_data_set_number(data, "subarray4_tilt", 20);
  ssc_data_set_number(data, "subarray4_tilt_eq_lat", 0);
  ssc_data_set_number(data, "subarray4_azimuth", 180);
  ssc_data_set_number(data, "subarray4_track_mode", 0);
  ssc_data_set_number(data, "subarray4_rotlim", 45);
  ssc_data_set_number(data, "subarray4_shade_mode", 0);
  ssc_data_set_number(data, "subarray4_gcr", 0.30000001192092896);
  ssc_number_t p_subarray4_monthly_tilt[12] = {40, 40, 40, 20, 20, 20,
                                               20, 20, 20, 40, 40, 40};
  ssc_data_set_array(data, "subarray4_monthly_tilt", p_subarray4_monthly_tilt,
                     12);
  ssc_number_t p_subarray4_soiling[12] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
  ssc_data_set_array(data, "subarray4_soiling", p_subarray4_soiling, 12);
  ssc_data_set_number(data, "subarray4_mod_orient", 0);
  ssc_data_set_number(data, "subarray4_nmodx", 9);
  ssc_data_set_number(data, "subarray4_nmody", 2);
  ssc_data_set_number(data, "subarray4_backtrack", 0);
  ssc_data_set_number(data, "module_model", 1);
  ssc_data_set_number(data, "module_aspect_ratio", 1.7000000476837158);
  ssc_data_set_number(data, "spe_area", 0.74074000120162964);
  ssc_data_set_number(data, "spe_rad0", 200);
  ssc_data_set_number(data, "spe_rad1", 400);
  ssc_data_set_number(data, "spe_rad2", 600);
  ssc_data_set_number(data, "spe_rad3", 800);
  ssc_data_set_number(data, "spe_rad4", 1000);
  ssc_data_set_number(data, "spe_eff0", 13.5);
  ssc_data_set_number(data, "spe_eff1", 13.5);
  ssc_data_set_number(data, "spe_eff2", 13.5);
  ssc_data_set_number(data, "spe_eff3", 13.5);
  ssc_data_set_number(data, "spe_eff4", 13.5);
  ssc_data_set_number(data, "spe_reference", 4);
  ssc_data_set_number(data, "spe_module_structure", 0);
  ssc_data_set_number(data, "spe_a", -3.559999942779541);
  ssc_data_set_number(data, "spe_b", -0.075000002980232239);
  ssc_data_set_number(data, "spe_dT", 3);
  ssc_data_set_number(data, "spe_temp_coeff", -0.5);
  ssc_data_set_number(data, "spe_fd", 1);
  ssc_data_set_number(data, "spe_vmp", 30);
  ssc_data_set_number(data, "spe_voc", 36);
  ssc_data_set_number(data, "spe_is_bifacial", 0);
  ssc_data_set_number(data, "spe_bifacial_transmission_factor", 0);
  ssc_data_set_number(data, "spe_bifaciality", 0);
  ssc_data_set_number(data, "spe_bifacial_ground_clearance_height", 0);
  ssc_data_set_number(data, "cec_is_bifacial", 0);
  ssc_data_set_number(data, "cec_bifacial_transmission_factor", 0);
  ssc_data_set_number(data, "cec_bifaciality", 0);
  ssc_data_set_number(data, "cec_bifacial_ground_clearance_height", 0);
  ssc_data_set_number(data, "cec_area", 1.6310000419616699);
  ssc_data_set_number(data, "cec_a_ref", 2.4200999736785889);
  ssc_data_set_number(data, "cec_adjust", 5.0100002288818359);
  ssc_data_set_number(data, "cec_alpha_sc", 0.0024920001160353422);
  ssc_data_set_number(data, "cec_beta_oc", -0.16975000500679016);
  ssc_data_set_number(data, "cec_gamma_r", -0.31000000238418579);
  ssc_data_set_number(data, "cec_i_l_ref", 6.2369999885559082);
  ssc_data_set_number(data, "cec_i_mp_ref", 5.8499999046325684);
  ssc_data_set_number(data, "cec_i_o_ref", 3.9799999233813832e-12);
  ssc_data_set_number(data, "cec_i_sc_ref", 6.2300000190734863);
  ssc_data_set_number(data, "cec_n_s", 96);
  ssc_data_set_number(data, "cec_r_s", 0.49900001287460327);
  ssc_data_set_number(data, "cec_r_sh_ref", 457.1199951171875);
  ssc_data_set_number(data, "cec_t_noct", 46.400001525878906);
  ssc_data_set_number(data, "cec_v_mp_ref", 57.299999237060547);
  ssc_data_set_number(data, "cec_v_oc_ref", 67.900001525878906);
  ssc_data_set_number(data, "cec_temp_corr_mode", 0);
  ssc_data_set_number(data, "cec_standoff", 6);
  ssc_data_set_number(data, "cec_height", 0);
  ssc_data_set_number(data, "cec_mounting_config", 0);
  ssc_data_set_number(data, "cec_heat_transfer", 0);
  ssc_data_set_number(data, "cec_mounting_orientation", 0);
  ssc_data_set_number(data, "cec_gap_spacing", 0.05000000074505806);
  ssc_data_set_number(data, "cec_module_width", 1);
  ssc_data_set_number(data, "cec_module_length", 1.6310000419616699);
  ssc_data_set_number(data, "cec_array_rows", 1);
  ssc_data_set_number(data, "cec_array_cols", 10);
  ssc_data_set_number(data, "cec_backside_temp", 20);
  ssc_data_set_number(data, "cec_transient_thermal_model_unit_mass", 11.09186);
  ssc_data_set_number(data, "6par_is_bifacial", 0);
  ssc_data_set_number(data, "6par_bifacial_transmission_factor", 0);
  ssc_data_set_number(data, "6par_bifaciality", 0);
  ssc_data_set_number(data, "6par_bifacial_ground_clearance_height", 0);
  ssc_data_set_number(data, "6par_celltech", 1);
  ssc_data_set_number(data, "6par_vmp", 30);
  ssc_data_set_number(data, "6par_imp", 6);
  ssc_data_set_number(data, "6par_voc", 37);
  ssc_data_set_number(data, "6par_isc", 7);
  ssc_data_set_number(data, "6par_bvoc", -0.10999999940395355);
  ssc_data_set_number(data, "6par_aisc", 0.0040000001899898052);
  ssc_data_set_number(data, "6par_gpmp", -0.40999999642372131);
  ssc_data_set_number(data, "6par_nser", 60);
  ssc_data_set_number(data, "6par_area", 1.2999999523162842);
  ssc_data_set_number(data, "6par_tnoct", 46);
  ssc_data_set_number(data, "6par_standoff", 6);
  ssc_data_set_number(data, "6par_mounting", 0);
  ssc_data_set_number(data, "6par_transient_thermal_model_unit_mass", 11.09186);
  ssc_data_set_number(data, "snl_module_structure", 0);
  ssc_data_set_number(data, "snl_a", -3.619999885559082);
  ssc_data_set_number(data, "snl_b", -0.075000002980232239);
  ssc_data_set_number(data, "snl_dtc", 3);
  ssc_data_set_number(data, "snl_ref_a", -3.619999885559082);
  ssc_data_set_number(data, "snl_ref_b", -0.075000002980232239);
  ssc_data_set_number(data, "snl_ref_dT", 3);
  ssc_data_set_number(data, "snl_fd", 1);
  ssc_data_set_number(data, "snl_a0", 0.94045001268386841);
  ssc_data_set_number(data, "snl_a1", 0.052641000598669052);
  ssc_data_set_number(data, "snl_a2", -0.0093897003680467606);
  ssc_data_set_number(data, "snl_a3", 0.00072622997686266899);
  ssc_data_set_number(data, "snl_a4", -1.9937999240937643e-05);
  ssc_data_set_number(data, "snl_aimp", -0.0003800000122282654);
  ssc_data_set_number(data, "snl_aisc", 0.00060999998822808266);
  ssc_data_set_number(data, "snl_area", 1.2439999580383301);
  ssc_data_set_number(data, "snl_b0", 1);
  ssc_data_set_number(data, "snl_b1", -0.0024379999376833439);
  ssc_data_set_number(data, "snl_b2", 0.00031030000536702573);
  ssc_data_set_number(data, "snl_b3", -1.2460000107239466e-05);
  ssc_data_set_number(data, "snl_b4", 2.1099999969464989e-07);
  ssc_data_set_number(data, "snl_b5", -1.3600000015046021e-09);
  ssc_data_set_number(data, "snl_bvmpo", -0.13899999856948853);
  ssc_data_set_number(data, "snl_bvoco", -0.13600000739097595);
  ssc_data_set_number(data, "snl_c0", 1.0039000511169434);
  ssc_data_set_number(data, "snl_c1", -0.0038999998942017555);
  ssc_data_set_number(data, "snl_c2", 0.29106599092483521);
  ssc_data_set_number(data, "snl_c3", -4.7354598045349121);
  ssc_data_set_number(data, "snl_c4", 0.99419999122619629);
  ssc_data_set_number(data, "snl_c5", 0.0057999999262392521);
  ssc_data_set_number(data, "snl_c6", 1.0722999572753906);
  ssc_data_set_number(data, "snl_c7", -0.072300001978874207);
  ssc_data_set_number(data, "snl_impo", 5.25);
  ssc_data_set_number(data, "snl_isco", 5.75);
  ssc_data_set_number(data, "snl_ixo", 5.6500000953674316);
  ssc_data_set_number(data, "snl_ixxo", 3.8499999046325684);
  ssc_data_set_number(data, "snl_mbvmp", 0);
  ssc_data_set_number(data, "snl_mbvoc", 0);
  ssc_data_set_number(data, "snl_n", 1.2209999561309814);
  ssc_data_set_number(data, "snl_series_cells", 72);
  ssc_data_set_number(data, "snl_vmpo", 40);
  ssc_data_set_number(data, "snl_voco", 47.700000762939453);
  ssc_data_set_number(data, "snl_transient_thermal_model_unit_mass", 11.09186);
  ssc_data_set_number(data, "sd11par_nser", 116);
  ssc_data_set_number(data, "sd11par_area", 0.72000002861022949);
  ssc_data_set_number(data, "sd11par_AMa0", 0.94169998168945313);
  ssc_data_set_number(data, "sd11par_AMa1", 0.065159998834133148);
  ssc_data_set_number(data, "sd11par_AMa2", -0.020220000296831131);
  ssc_data_set_number(data, "sd11par_AMa3", 0.0021899999119341373);
  ssc_data_set_number(data, "sd11par_AMa4", -9.1000001702923328e-05);
  ssc_data_set_number(data, "sd11par_glass", 0);
  ssc_data_set_number(data, "sd11par_tnoct", 44.900001525878906);
  ssc_data_set_number(data, "sd11par_standoff", 6);
  ssc_data_set_number(data, "sd11par_mounting", 0);
  ssc_data_set_number(data, "sd11par_Vmp0", 64.599998474121094);
  ssc_data_set_number(data, "sd11par_Imp0", 1.0500000715255737);
  ssc_data_set_number(data, "sd11par_Voc0", 87);
  ssc_data_set_number(data, "sd11par_Isc0", 1.1799999475479126);
  ssc_data_set_number(data, "sd11par_alphaIsc", 0.00047200100379996002);
  ssc_data_set_number(data, "sd11par_n", 1.4507100582122803);
  ssc_data_set_number(data, "sd11par_Il", 1.1895099878311157);
  ssc_data_set_number(data, "sd11par_Io", 2.0852199966725493e-09);
  ssc_data_set_number(data, "sd11par_Egref", 0.73766797780990601);
  ssc_data_set_number(data, "sd11par_d1", 13.550399780273438);
  ssc_data_set_number(data, "sd11par_d2", -0.076973497867584229);
  ssc_data_set_number(data, "sd11par_d3", 0.23732699453830719);
  ssc_data_set_number(data, "sd11par_c1", 1930.1500244140625);
  ssc_data_set_number(data, "sd11par_c2", 474.6400146484375);
  ssc_data_set_number(data, "sd11par_c3", 1.4874600172042847);
  ssc_data_set_number(data, "inverter_model", 0);
  ssc_data_set_number(data, "mppt_low_inverter", 250);
  ssc_data_set_number(data, "mppt_hi_inverter", 480);
  ssc_data_set_number(data, "inv_num_mppt", 1);
  ssc_data_set_number(data, "inv_snl_c0", -3.1800000215298496e-06);
  ssc_data_set_number(data, "inv_snl_c1", -5.1200000598328188e-05);
  ssc_data_set_number(data, "inv_snl_c2", 0.00098400004208087921);
  ssc_data_set_number(data, "inv_snl_c3", -0.0015099999727681279);
  ssc_data_set_number(data, "inv_snl_paco", 3800);
  ssc_data_set_number(data, "inv_snl_pdco", 3928.1142578125);
  ssc_data_set_number(data, "inv_snl_pnt", 0.99000000953674316);
  ssc_data_set_number(data, "inv_snl_pso", 19.451622009277344);
  ssc_data_set_number(data, "inv_snl_vdco", 398.49667358398438);
  ssc_data_set_number(data, "inv_snl_vdcmax", 600);
  ssc_data_set_number(data, "inv_cec_cg_c0", -3.1751958431414096e-06);
  ssc_data_set_number(data, "inv_cec_cg_c1", -5.1231381803518161e-05);
  ssc_data_set_number(data, "inv_cec_cg_c2", 0.0009835963137447834);
  ssc_data_set_number(data, "inv_cec_cg_c3", -0.0015077980933710933);
  ssc_data_set_number(data, "inv_cec_cg_paco", 3800);
  ssc_data_set_number(data, "inv_cec_cg_pdco", 3928.11376953125);
  ssc_data_set_number(data, "inv_cec_cg_pnt", 0.99000000953674316);
  ssc_data_set_number(data, "inv_cec_cg_psco", 19.448383331298828);
  ssc_data_set_number(data, "inv_cec_cg_vdco", 398.49661254882813);
  ssc_data_set_number(data, "inv_cec_cg_vdcmax", 600);
  ssc_data_set_number(data, "inv_ds_paco", 4000);
  ssc_data_set_number(data, "inv_ds_eff", 96);
  ssc_data_set_number(data, "inv_ds_pnt", 1);
  ssc_data_set_number(data, "inv_ds_pso", 0);
  ssc_data_set_number(data, "inv_ds_vdco", 310);
  ssc_data_set_number(data, "inv_ds_vdcmax", 600);
  ssc_data_set_number(data, "inv_pd_paco", 4000);
  ssc_data_set_number(data, "inv_pd_pdco", 4210.5263671875);
  ssc_number_t p_inv_pd_partload[251] = {0,
                                         0.40400001406669617,
                                         0.80800002813339233,
                                         1.2120000123977661,
                                         1.6160000562667847,
                                         2.0199999809265137,
                                         2.4240000247955322,
                                         2.8280000686645508,
                                         3.2320001125335693,
                                         3.6359999179840088,
                                         4.0399999618530273,
                                         4.4439997673034668,
                                         4.8480000495910645,
                                         5.2519998550415039,
                                         5.6560001373291016,
                                         6.059999942779541,
                                         6.4640002250671387,
                                         6.8680000305175781,
                                         7.2719998359680176,
                                         7.6760001182556152,
                                         8.0799999237060547,
                                         8.4840002059936523,
                                         8.8879995346069336,
                                         9.2919998168945313,
                                         9.6960000991821289,
                                         10.100000381469727,
                                         10.503999710083008,
                                         10.907999992370605,
                                         11.312000274658203,
                                         11.715999603271484,
                                         12.119999885559082,
                                         12.52400016784668,
                                         12.928000450134277,
                                         13.331999778747559,
                                         13.736000061035156,
                                         14.140000343322754,
                                         14.543999671936035,
                                         14.947999954223633,
                                         15.35200023651123,
                                         15.755999565124512,
                                         16.159999847412109,
                                         16.563999176025391,
                                         16.968000411987305,
                                         17.371999740600586,
                                         17.775999069213867,
                                         18.180000305175781,
                                         18.583999633789063,
                                         18.988000869750977,
                                         19.392000198364258,
                                         19.795999526977539,
                                         20.200000762939453,
                                         20.604000091552734,
                                         21.007999420166016,
                                         21.41200065612793,
                                         21.815999984741211,
                                         22.219999313354492,
                                         22.624000549316406,
                                         23.027999877929688,
                                         23.431999206542969,
                                         23.836000442504883,
                                         24.239999771118164,
                                         24.643999099731445,
                                         25.048000335693359,
                                         25.451999664306641,
                                         25.856000900268555,
                                         26.260000228881836,
                                         26.663999557495117,
                                         27.068000793457031,
                                         27.472000122070313,
                                         27.875999450683594,
                                         28.280000686645508,
                                         28.684000015258789,
                                         29.08799934387207,
                                         29.492000579833984,
                                         29.895999908447266,
                                         30.299999237060547,
                                         30.704000473022461,
                                         31.107999801635742,
                                         31.511999130249023,
                                         31.916000366210938,
                                         32.319999694824219,
                                         32.7239990234375,
                                         33.127998352050781,
                                         33.532001495361328,
                                         33.936000823974609,
                                         34.340000152587891,
                                         34.743999481201172,
                                         35.147998809814453,
                                         35.551998138427734,
                                         35.956001281738281,
                                         36.360000610351563,
                                         36.763999938964844,
                                         37.167999267578125,
                                         37.571998596191406,
                                         37.976001739501953,
                                         38.380001068115234,
                                         38.784000396728516,
                                         39.187999725341797,
                                         39.591999053955078,
                                         39.995998382568359,
                                         40.400001525878906,
                                         40.804000854492188,
                                         41.208000183105469,
                                         41.61199951171875,
                                         42.015998840332031,
                                         42.419998168945313,
                                         42.824001312255859,
                                         43.228000640869141,
                                         43.631999969482422,
                                         44.035999298095703,
                                         44.439998626708984,
                                         44.844001770019531,
                                         45.248001098632813,
                                         45.652000427246094,
                                         46.055999755859375,
                                         46.459999084472656,
                                         46.863998413085938,
                                         47.268001556396484,
                                         47.672000885009766,
                                         48.076000213623047,
                                         48.479999542236328,
                                         48.883998870849609,
                                         49.287998199462891,
                                         49.692001342773438,
                                         50.096000671386719,
                                         50.5,
                                         50.903999328613281,
                                         51.307998657226563,
                                         51.712001800537109,
                                         52.116001129150391,
                                         52.520000457763672,
                                         52.923999786376953,
                                         53.327999114990234,
                                         53.731998443603516,
                                         54.136001586914063,
                                         54.540000915527344,
                                         54.944000244140625,
                                         55.347999572753906,
                                         55.751998901367188,
                                         56.155998229980469,
                                         56.560001373291016,
                                         56.964000701904297,
                                         57.368000030517578,
                                         57.771999359130859,
                                         58.175998687744141,
                                         58.580001831054688,
                                         58.984001159667969,
                                         59.38800048828125,
                                         59.791999816894531,
                                         60.195999145507813,
                                         60.599998474121094,
                                         61.004001617431641,
                                         61.408000946044922,
                                         61.812000274658203,
                                         62.215999603271484,
                                         62.619998931884766,
                                         63.023998260498047,
                                         63.428001403808594,
                                         63.832000732421875,
                                         64.236000061035156,
                                         64.639999389648438,
                                         65.043998718261719,
                                         65.447998046875,
                                         65.851997375488281,
                                         66.255996704101563,
                                         66.660003662109375,
                                         67.064002990722656,
                                         67.468002319335938,
                                         67.872001647949219,
                                         68.2760009765625,
                                         68.680000305175781,
                                         69.083999633789063,
                                         69.487998962402344,
                                         69.891998291015625,
                                         70.295997619628906,
                                         70.699996948242188,
                                         71.103996276855469,
                                         71.508003234863281,
                                         71.912002563476563,
                                         72.316001892089844,
                                         72.720001220703125,
                                         73.124000549316406,
                                         73.527999877929688,
                                         73.931999206542969,
                                         74.33599853515625,
                                         74.739997863769531,
                                         75.143997192382813,
                                         75.547996520996094,
                                         75.952003479003906,
                                         76.356002807617188,
                                         76.760002136230469,
                                         77.16400146484375,
                                         77.568000793457031,
                                         77.972000122070313,
                                         78.375999450683594,
                                         78.779998779296875,
                                         79.183998107910156,
                                         79.587997436523438,
                                         79.991996765136719,
                                         80.396003723144531,
                                         80.800003051757813,
                                         81.204002380371094,
                                         81.608001708984375,
                                         82.012001037597656,
                                         82.416000366210938,
                                         82.819999694824219,
                                         83.2239990234375,
                                         83.627998352050781,
                                         84.031997680664063,
                                         84.435997009277344,
                                         84.839996337890625,
                                         85.244003295898438,
                                         85.648002624511719,
                                         86.052001953125,
                                         86.456001281738281,
                                         86.860000610351563,
                                         87.263999938964844,
                                         87.667999267578125,
                                         88.071998596191406,
                                         88.475997924804688,
                                         88.879997253417969,
                                         89.28399658203125,
                                         89.688003540039063,
                                         90.092002868652344,
                                         90.496002197265625,
                                         90.900001525878906,
                                         91.304000854492188,
                                         91.708000183105469,
                                         92.11199951171875,
                                         92.515998840332031,
                                         92.919998168945313,
                                         93.323997497558594,
                                         93.727996826171875,
                                         94.132003784179688,
                                         94.536003112792969,
                                         94.94000244140625,
                                         95.344001770019531,
                                         95.748001098632813,
                                         96.152000427246094,
                                         96.555999755859375,
                                         96.959999084472656,
                                         97.363998413085938,
                                         97.767997741699219,
                                         98.1719970703125,
                                         98.575996398925781,
                                         98.980003356933594,
                                         99.384002685546875,
                                         99.788002014160156,
                                         100.19200134277344,
                                         100.59600067138672,
                                         101};
  ssc_data_set_array(data, "inv_pd_partload", p_inv_pd_partload, 251);
  ssc_number_t p_inv_pd_efficiency[251] = {0,
                                           0,
                                           34.419998168945313,
                                           55.200000762939453,
                                           65.589996337890625,
                                           71.819999694824219,
                                           75.970001220703125,
                                           78.94000244140625,
                                           81.169998168945313,
                                           82.900001525878906,
                                           84.279998779296875,
                                           85.419998168945313,
                                           86.360000610351563,
                                           87.160003662109375,
                                           87.839996337890625,
                                           88.44000244140625,
                                           88.949996948242188,
                                           89.410003662109375,
                                           89.819999694824219,
                                           90.180000305175781,
                                           90.510002136230469,
                                           90.80999755859375,
                                           91.080001831054688,
                                           91.319999694824219,
                                           91.550003051757813,
                                           91.75,
                                           91.949996948242188,
                                           92.120002746582031,
                                           92.290000915527344,
                                           92.44000244140625,
                                           92.580001831054688,
                                           92.720001220703125,
                                           92.839996337890625,
                                           92.959999084472656,
                                           93.069999694824219,
                                           93.169998168945313,
                                           93.269996643066406,
                                           93.370002746582031,
                                           93.449996948242188,
                                           93.540000915527344,
                                           93.620002746582031,
                                           93.69000244140625,
                                           93.760002136230469,
                                           93.830001831054688,
                                           93.900001525878906,
                                           93.959999084472656,
                                           94.019996643066406,
                                           94.080001831054688,
                                           94.129997253417969,
                                           94.180000305175781,
                                           94.230003356933594,
                                           94.279998779296875,
                                           94.330001831054688,
                                           94.370002746582031,
                                           94.419998168945313,
                                           94.459999084472656,
                                           94.5,
                                           94.540000915527344,
                                           94.569999694824219,
                                           94.610000610351563,
                                           94.639999389648438,
                                           94.680000305175781,
                                           94.709999084472656,
                                           94.739997863769531,
                                           94.769996643066406,
                                           94.800003051757813,
                                           94.830001831054688,
                                           94.860000610351563,
                                           94.889999389648438,
                                           94.910003662109375,
                                           94.94000244140625,
                                           94.959999084472656,
                                           94.980003356933594,
                                           95.010002136230469,
                                           95.029998779296875,
                                           95.050003051757813,
                                           95.069999694824219,
                                           95.089996337890625,
                                           95.110000610351563,
                                           95.129997253417969,
                                           95.150001525878906,
                                           95.169998168945313,
                                           95.19000244140625,
                                           95.209999084472656,
                                           95.230003356933594,
                                           95.239997863769531,
                                           95.260002136230469,
                                           95.279998779296875,
                                           95.290000915527344,
                                           95.30999755859375,
                                           95.319999694824219,
                                           95.339996337890625,
                                           95.349998474121094,
                                           95.360000610351563,
                                           95.379997253417969,
                                           95.389999389648438,
                                           95.400001525878906,
                                           95.419998168945313,
                                           95.430000305175781,
                                           95.44000244140625,
                                           95.449996948242188,
                                           95.470001220703125,
                                           95.480003356933594,
                                           95.489997863769531,
                                           95.5,
                                           95.510002136230469,
                                           95.519996643066406,
                                           95.529998779296875,
                                           95.540000915527344,
                                           95.550003051757813,
                                           95.55999755859375,
                                           95.569999694824219,
                                           95.580001831054688,
                                           95.589996337890625,
                                           95.599998474121094,
                                           95.610000610351563,
                                           95.620002746582031,
                                           95.629997253417969,
                                           95.639999389648438,
                                           95.639999389648438,
                                           95.650001525878906,
                                           95.660003662109375,
                                           95.669998168945313,
                                           95.680000305175781,
                                           95.680000305175781,
                                           95.69000244140625,
                                           95.699996948242188,
                                           95.709999084472656,
                                           95.709999084472656,
                                           95.720001220703125,
                                           95.730003356933594,
                                           95.730003356933594,
                                           95.739997863769531,
                                           95.75,
                                           95.75,
                                           95.760002136230469,
                                           95.769996643066406,
                                           95.769996643066406,
                                           95.779998779296875,
                                           95.779998779296875,
                                           95.790000915527344,
                                           95.800003051757813,
                                           95.800003051757813,
                                           95.80999755859375,
                                           95.80999755859375,
                                           95.819999694824219,
                                           95.819999694824219,
                                           95.830001831054688,
                                           95.830001831054688,
                                           95.839996337890625,
                                           95.839996337890625,
                                           95.849998474121094,
                                           95.849998474121094,
                                           95.860000610351563,
                                           95.860000610351563,
                                           95.870002746582031,
                                           95.870002746582031,
                                           95.879997253417969,
                                           95.879997253417969,
                                           95.889999389648438,
                                           95.889999389648438,
                                           95.889999389648438,
                                           95.900001525878906,
                                           95.900001525878906,
                                           95.910003662109375,
                                           95.910003662109375,
                                           95.910003662109375,
                                           95.919998168945313,
                                           95.919998168945313,
                                           95.930000305175781,
                                           95.930000305175781,
                                           95.930000305175781,
                                           95.94000244140625,
                                           95.94000244140625,
                                           95.94000244140625,
                                           95.949996948242188,
                                           95.949996948242188,
                                           95.959999084472656,
                                           95.959999084472656,
                                           95.959999084472656,
                                           95.970001220703125,
                                           95.970001220703125,
                                           95.970001220703125,
                                           95.980003356933594,
                                           95.980003356933594,
                                           95.980003356933594,
                                           95.980003356933594,
                                           95.989997863769531,
                                           95.989997863769531,
                                           95.989997863769531,
                                           96,
                                           96,
                                           96,
                                           96.010002136230469,
                                           96.010002136230469,
                                           96.010002136230469,
                                           96.010002136230469,
                                           96.019996643066406,
                                           96.019996643066406,
                                           96.019996643066406,
                                           96.019996643066406,
                                           96.029998779296875,
                                           96.029998779296875,
                                           96.029998779296875,
                                           96.029998779296875,
                                           96.040000915527344,
                                           96.040000915527344,
                                           96.040000915527344,
                                           96.040000915527344,
                                           96.050003051757813,
                                           96.050003051757813,
                                           96.050003051757813,
                                           96.050003051757813,
                                           96.05999755859375,
                                           96.05999755859375,
                                           96.05999755859375,
                                           96.05999755859375,
                                           96.05999755859375,
                                           96.069999694824219,
                                           96.069999694824219,
                                           96.069999694824219,
                                           96.069999694824219,
                                           96.069999694824219,
                                           96.080001831054688,
                                           96.080001831054688,
                                           96.080001831054688,
                                           96.080001831054688,
                                           96.080001831054688,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.089996337890625,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.099998474121094,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.110000610351563,
                                           96.120002746582031,
                                           96.120002746582031,
                                           96.120002746582031,
                                           96.120002746582031,
                                           96.120002746582031};
  ssc_data_set_array(data, "inv_pd_efficiency", p_inv_pd_efficiency, 251);
  ssc_data_set_number(data, "inv_pd_pnt", 0);
  ssc_data_set_number(data, "inv_pd_vdco", 310);
  ssc_data_set_number(data, "inv_pd_vdcmax", 600);
  ssc_number_t matrix_inv_tdc[3] = {1., 52.8, -0.021};
  ssc_data_set_matrix(data, "inv_tdc_cec_db", matrix_inv_tdc, 1, 3);
  ssc_data_set_matrix(data, "inv_tdc_cec_cg", matrix_inv_tdc, 1, 3);
  ssc_data_set_matrix(data, "inv_tdc_plc", matrix_inv_tdc, 1, 3);
  ssc_data_set_matrix(data, "inv_tdc_ds", matrix_inv_tdc, 1, 3);

  battery_data_default(data);
}

/**
 *  Default data for utility_rate5 run that can be further modified
 */
void utility_rate5_default(ssc_data_t &data) {
  ssc_data_set_number(data, "inflation_rate", 2.5);
  ssc_number_t p_degradation[1] = {0.5};
  ssc_data_set_array(data, "degradation", p_degradation, 1);
  ssc_number_t p_load_escalation[1] = {0};
  ssc_data_set_array(data, "load_escalation", p_load_escalation, 1);
  ssc_number_t p_rate_escalation[1] = {0};
  ssc_data_set_array(data, "rate_escalation", p_rate_escalation, 1);
  ssc_data_set_number(data, "ur_metering_option", 0);
  ssc_data_set_number(data, "ur_nm_yearend_sell_rate", 0.027890000492334366);
  ssc_data_set_number(data, "ur_monthly_fixed_charge", 16.680000305175781);
  ssc_data_set_number(data, "ur_monthly_min_charge", 0);
  ssc_data_set_number(data, "ur_annual_min_charge", 0);
  ssc_data_set_number(data, "ur_en_ts_sell_rate", 0);
  set_array(data, "ur_ts_sell_rate", sell_rate_path, 8760);
  ssc_number_t p_ur_ec_sched_weekday[288] = {
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4};
  ssc_data_set_matrix(data, "ur_ec_sched_weekday", p_ur_ec_sched_weekday, 12,
                      24);
  ssc_number_t p_ur_ec_sched_weekend[288] = {
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
  ssc_data_set_matrix(data, "ur_ec_sched_weekend", p_ur_ec_sched_weekend, 12,
                      24);
  ssc_number_t p_ur_ec_tou_mat[24] = {1,
                                      1,
                                      9.9999996802856925e+37,
                                      0,
                                      0.26686999201774597,
                                      0.26686999201774597,
                                      2,
                                      1,
                                      9.9999996802856925e+37,
                                      0,
                                      0.083279997110366821,
                                      0.083279997110366821,
                                      3,
                                      1,
                                      9.9999996802856925e+37,
                                      0,
                                      0.22056999802589417,
                                      0.22056999802589417,
                                      4,
                                      1,
                                      9.9999996802856925e+37,
                                      0,
                                      0.083259999752044678,
                                      0.083259999752044678};
  ssc_data_set_matrix(data, "ur_ec_tou_mat", p_ur_ec_tou_mat, 4, 6);
  ssc_data_set_number(data, "ur_dc_enable", 0);
  ssc_number_t p_ur_dc_sched_weekday[288] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ssc_data_set_matrix(data, "ur_dc_sched_weekday", p_ur_dc_sched_weekday, 12,
                      24);
  ssc_number_t p_ur_dc_sched_weekend[288] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ssc_data_set_matrix(data, "ur_dc_sched_weekend", p_ur_dc_sched_weekend, 12,
                      24);
  ssc_number_t p_ur_dc_tou_mat[4] = {1, 1, 9.9999996802856925e+37, 0};
  ssc_data_set_matrix(data, "ur_dc_tou_mat", p_ur_dc_tou_mat, 1, 4);
  ssc_number_t p_ur_dc_flat_mat[48] = {
      0,  1, 9.9999996802856925e+37, 0, 1,  1, 9.9999996802856925e+37, 0,
      2,  1, 9.9999996802856925e+37, 0, 3,  1, 9.9999996802856925e+37, 0,
      4,  1, 9.9999996802856925e+37, 0, 5,  1, 9.9999996802856925e+37, 0,
      6,  1, 9.9999996802856925e+37, 0, 7,  1, 9.9999996802856925e+37, 0,
      8,  1, 9.9999996802856925e+37, 0, 9,  1, 9.9999996802856925e+37, 0,
      10, 1, 9.9999996802856925e+37, 0, 11, 1, 9.9999996802856925e+37, 0};
  ssc_data_set_matrix(data, "ur_dc_flat_mat", p_ur_dc_flat_mat, 12, 4);
}

/**
 *  Default data for cashloan run that can be further modified
 */
void cashloan_default(ssc_data_t &data) {
  ssc_number_t p_federal_tax_rate[1] = {30};
  ssc_data_set_array(data, "federal_tax_rate", p_federal_tax_rate, 1);
  ssc_number_t p_state_tax_rate[1] = {7};
  ssc_data_set_array(data, "state_tax_rate", p_state_tax_rate, 1);
  ssc_data_set_number(data, "property_tax_rate", 1);
  ssc_data_set_number(data, "prop_tax_cost_assessed_percent", 100);
  ssc_data_set_number(data, "prop_tax_assessed_decline", 0);
  ssc_data_set_number(data, "real_discount_rate", 5.5);
  ssc_data_set_number(data, "insurance_rate", 1);
  ssc_data_set_number(data, "loan_term", 25);
  ssc_data_set_number(data, "loan_rate", 5);
  ssc_data_set_number(data, "debt_fraction", 100);
  ssc_number_t p_om_fixed[1] = {0};
  ssc_data_set_array(data, "om_fixed", p_om_fixed, 1);
  ssc_data_set_number(data, "om_fixed_escal", 0);
  ssc_number_t p_om_production[1] = {0};
  ssc_data_set_array(data, "om_production", p_om_production, 1);
  ssc_data_set_number(data, "om_production_escal", 0);
  ssc_number_t p_om_capacity[1] = {20};
  ssc_data_set_array(data, "om_capacity", p_om_capacity, 1);
  ssc_data_set_number(data, "om_capacity_escal", 0);
  ssc_number_t p_om_fuel_cost[1] = {0};
  ssc_data_set_array(data, "om_fuel_cost", p_om_fuel_cost, 1);
  ssc_data_set_number(data, "om_fuel_cost_escal", 0);
  ssc_data_set_number(data, "itc_fed_amount", 0);
  ssc_data_set_number(data, "itc_fed_amount_deprbas_fed", 1);
  ssc_data_set_number(data, "itc_fed_amount_deprbas_sta", 1);
  ssc_data_set_number(data, "itc_sta_amount", 0);
  ssc_data_set_number(data, "itc_sta_amount_deprbas_fed", 0);
  ssc_data_set_number(data, "itc_sta_amount_deprbas_sta", 0);
  ssc_data_set_number(data, "itc_fed_percent", 30);
  ssc_data_set_number(data, "itc_fed_percent_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "itc_fed_percent_deprbas_fed", 1);
  ssc_data_set_number(data, "itc_fed_percent_deprbas_sta", 1);
  ssc_data_set_number(data, "itc_sta_percent", 25);
  ssc_data_set_number(data, "itc_sta_percent_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "itc_sta_percent_deprbas_fed", 0);
  ssc_data_set_number(data, "itc_sta_percent_deprbas_sta", 0);
  ssc_number_t p_ptc_fed_amount[1] = {0};
  ssc_data_set_array(data, "ptc_fed_amount", p_ptc_fed_amount, 1);
  ssc_data_set_number(data, "ptc_fed_term", 10);
  ssc_data_set_number(data, "ptc_fed_escal", 0);
  ssc_number_t p_ptc_sta_amount[1] = {0};
  ssc_data_set_array(data, "ptc_sta_amount", p_ptc_sta_amount, 1);
  ssc_data_set_number(data, "ptc_sta_term", 10);
  ssc_data_set_number(data, "ptc_sta_escal", 0);
  ssc_data_set_number(data, "ibi_fed_amount", 0);
  ssc_data_set_number(data, "ibi_fed_amount_tax_fed", 1);
  ssc_data_set_number(data, "ibi_fed_amount_tax_sta", 1);
  ssc_data_set_number(data, "ibi_fed_amount_deprbas_fed", 0);
  ssc_data_set_number(data, "ibi_fed_amount_deprbas_sta", 0);
  ssc_data_set_number(data, "ibi_sta_amount", 0);
  ssc_data_set_number(data, "ibi_sta_amount_tax_fed", 1);
  ssc_data_set_number(data, "ibi_sta_amount_tax_sta", 1);
  ssc_data_set_number(data, "ibi_sta_amount_deprbas_fed", 0);
  ssc_data_set_number(data, "ibi_sta_amount_deprbas_sta", 0);
  ssc_data_set_number(data, "ibi_uti_amount", 0);
  ssc_data_set_number(data, "ibi_uti_amount_tax_fed", 1);
  ssc_data_set_number(data, "ibi_uti_amount_tax_sta", 1);
  ssc_data_set_number(data, "ibi_uti_amount_deprbas_fed", 0);
  ssc_data_set_number(data, "ibi_uti_amount_deprbas_sta", 0);
  ssc_data_set_number(data, "ibi_oth_amount", 0);
  ssc_data_set_number(data, "ibi_oth_amount_tax_fed", 1);
  ssc_data_set_number(data, "ibi_oth_amount_tax_sta", 1);
  ssc_data_set_number(data, "ibi_oth_amount_deprbas_fed", 0);
  ssc_data_set_number(data, "ibi_oth_amount_deprbas_sta", 0);
  ssc_data_set_number(data, "ibi_fed_percent", 0);
  ssc_data_set_number(data, "ibi_fed_percent_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "ibi_fed_percent_tax_fed", 1);
  ssc_data_set_number(data, "ibi_fed_percent_tax_sta", 1);
  ssc_data_set_number(data, "ibi_fed_percent_deprbas_fed", 0);
  ssc_data_set_number(data, "ibi_fed_percent_deprbas_sta", 0);
  ssc_data_set_number(data, "ibi_sta_percent", 0);
  ssc_data_set_number(data, "ibi_sta_percent_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "ibi_sta_percent_tax_fed", 1);
  ssc_data_set_number(data, "ibi_sta_percent_tax_sta", 1);
  ssc_data_set_number(data, "ibi_sta_percent_deprbas_fed", 0);
  ssc_data_set_number(data, "ibi_sta_percent_deprbas_sta", 0);
  ssc_data_set_number(data, "ibi_uti_percent", 0);
  ssc_data_set_number(data, "ibi_uti_percent_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "ibi_uti_percent_tax_fed", 1);
  ssc_data_set_number(data, "ibi_uti_percent_tax_sta", 1);
  ssc_data_set_number(data, "ibi_uti_percent_deprbas_fed", 0);
  ssc_data_set_number(data, "ibi_uti_percent_deprbas_sta", 0);
  ssc_data_set_number(data, "ibi_oth_percent", 0);
  ssc_data_set_number(data, "ibi_oth_percent_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "ibi_oth_percent_tax_fed", 1);
  ssc_data_set_number(data, "ibi_oth_percent_tax_sta", 1);
  ssc_data_set_number(data, "ibi_oth_percent_deprbas_fed", 0);
  ssc_data_set_number(data, "ibi_oth_percent_deprbas_sta", 0);
  ssc_data_set_number(data, "cbi_fed_amount", 0);
  ssc_data_set_number(data, "cbi_fed_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "cbi_fed_tax_fed", 1);
  ssc_data_set_number(data, "cbi_fed_tax_sta", 1);
  ssc_data_set_number(data, "cbi_fed_deprbas_fed", 0);
  ssc_data_set_number(data, "cbi_fed_deprbas_sta", 0);
  ssc_data_set_number(data, "cbi_sta_amount", 0);
  ssc_data_set_number(data, "cbi_sta_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "cbi_sta_tax_fed", 1);
  ssc_data_set_number(data, "cbi_sta_tax_sta", 1);
  ssc_data_set_number(data, "cbi_sta_deprbas_fed", 0);
  ssc_data_set_number(data, "cbi_sta_deprbas_sta", 0);
  ssc_data_set_number(data, "cbi_uti_amount", 0);
  ssc_data_set_number(data, "cbi_uti_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "cbi_uti_tax_fed", 1);
  ssc_data_set_number(data, "cbi_uti_tax_sta", 1);
  ssc_data_set_number(data, "cbi_uti_deprbas_fed", 0);
  ssc_data_set_number(data, "cbi_uti_deprbas_sta", 0);
  ssc_data_set_number(data, "cbi_oth_amount", 0);
  ssc_data_set_number(data, "cbi_oth_maxvalue", 9.9999996802856925e+37);
  ssc_data_set_number(data, "cbi_oth_tax_fed", 1);
  ssc_data_set_number(data, "cbi_oth_tax_sta", 1);
  ssc_data_set_number(data, "cbi_oth_deprbas_fed", 0);
  ssc_data_set_number(data, "cbi_oth_deprbas_sta", 0);
  ssc_number_t p_pbi_fed_amount[1] = {0};
  ssc_data_set_array(data, "pbi_fed_amount", p_pbi_fed_amount, 1);
  ssc_data_set_number(data, "pbi_fed_term", 0);
  ssc_data_set_number(data, "pbi_fed_escal", 0);
  ssc_data_set_number(data, "pbi_fed_tax_fed", 1);
  ssc_data_set_number(data, "pbi_fed_tax_sta", 1);
  ssc_number_t p_pbi_sta_amount[1] = {0};
  ssc_data_set_array(data, "pbi_sta_amount", p_pbi_sta_amount, 1);
  ssc_data_set_number(data, "pbi_sta_term", 0);
  ssc_data_set_number(data, "pbi_sta_escal", 0);
  ssc_data_set_number(data, "pbi_sta_tax_fed", 1);
  ssc_data_set_number(data, "pbi_sta_tax_sta", 1);
  ssc_number_t p_pbi_uti_amount[1] = {0};
  ssc_data_set_array(data, "pbi_uti_amount", p_pbi_uti_amount, 1);
  ssc_data_set_number(data, "pbi_uti_term", 0);
  ssc_data_set_number(data, "pbi_uti_escal", 0);
  ssc_data_set_number(data, "pbi_uti_tax_fed", 1);
  ssc_data_set_number(data, "pbi_uti_tax_sta", 1);
  ssc_number_t p_pbi_oth_amount[1] = {0};
  ssc_data_set_array(data, "pbi_oth_amount", p_pbi_oth_amount, 1);
  ssc_data_set_number(data, "pbi_oth_term", 0);
  ssc_data_set_number(data, "pbi_oth_escal", 0);
  ssc_data_set_number(data, "pbi_oth_tax_fed", 1);
  ssc_data_set_number(data, "pbi_oth_tax_sta", 1);
  ssc_data_set_number(data, "battery_per_kWh", 500);
  ssc_number_t p_replacement_cost[1] = {500};
  ssc_data_set_array(data, "om_replacement_cost1", p_replacement_cost, 1);
  ssc_data_set_number(data, "om_replacement_cost_escal", 0);
  ssc_data_set_number(data, "market", 0);
  ssc_data_set_number(data, "mortgage", 1);
  ssc_data_set_number(data, "total_installed_cost", 13758.3671875);
  ssc_data_set_number(data, "salvage_percentage", 0);
}

void setup_residential_utility_rates(ssc_data_t &data) {
  ssc_data_set_number(data, "en_electricity_rates", 1);
  ssc_number_t p_ur_ts_buy_rate[1] = {0};
  ssc_data_set_array(data, "ur_ts_buy_rate", p_ur_ts_buy_rate, 1);
  ssc_number_t p_ur_ec_sched_weekday[288] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 1, 1, 1, 1};
  ssc_data_set_matrix(data, "ur_ec_sched_weekday", p_ur_ec_sched_weekday, 12,
                      24);
  ssc_number_t p_ur_ec_sched_weekend[288] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ssc_data_set_matrix(data, "ur_ec_sched_weekend", p_ur_ec_sched_weekend, 12,
                      24);
  ssc_number_t p_ur_ec_tou_mat[24] = {1,
                                      1,
                                      9.9999999999999998e+37,
                                      0,
                                      0.10000000000000001,
                                      0,
                                      2,
                                      1,
                                      9.9999999999999998e+37,
                                      0,
                                      0.050000000000000003,
                                      0,
                                      3,
                                      1,
                                      9.9999999999999998e+37,
                                      0,
                                      0.20000000000000001,
                                      0,
                                      4,
                                      1,
                                      9.9999999999999998e+37,
                                      0,
                                      0.25,
                                      0};
  ssc_data_set_matrix(data, "ur_ec_tou_mat", p_ur_ec_tou_mat, 4, 6);
  ssc_number_t p_ppa_price_input[1] = {0.089999999999999997};
  ssc_data_set_array(data, "ppa_price_input", p_ppa_price_input, 1);
  ssc_data_set_number(data, "ppa_multiplier_model", 1);
  ssc_data_set_number(data, "inflation_rate", 2.5);
  ssc_number_t p_degradation[1] = {0};
  ssc_data_set_array(data, "degradation", p_degradation, 1);
  ssc_number_t p_rate_escalation[1] = {0};
  ssc_data_set_array(data, "rate_escalation", p_rate_escalation, 1);
  ssc_data_set_number(data, "ur_metering_option", 0);
  ssc_data_set_number(data, "ur_nm_yearend_sell_rate", 0.027890000000000002);
  ssc_data_set_number(data, "ur_monthly_fixed_charge", 0);
  ssc_data_set_number(data, "ur_monthly_min_charge", 0);
  ssc_data_set_number(data, "ur_annual_min_charge", 0);
  ssc_number_t ur_ts_sell_rate[1] = {0};
  ssc_data_set_array(data, "ur_ts_sell_rate", ur_ts_sell_rate, 1);
  ssc_data_set_number(data, "ur_dc_enable", 0);
  ssc_number_t p_ur_dc_sched_weekday[288] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ssc_data_set_matrix(data, "ur_dc_sched_weekday", p_ur_dc_sched_weekday, 12,
                      24);
  ssc_number_t p_ur_dc_sched_weekend[288] = {
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  ssc_data_set_matrix(data, "ur_dc_sched_weekend", p_ur_dc_sched_weekend, 12,
                      24);
  ssc_number_t p_ur_dc_tou_mat[8] = {1, 1, 9.9999999999999998e+37, 0,
                                     2, 1, 9.9999999999999998e+37, 0};
  ssc_data_set_matrix(data, "ur_dc_tou_mat", p_ur_dc_tou_mat, 2, 4);
  ssc_number_t p_ur_dc_flat_mat[48] = {
      0,  1, 9.9999999999999998e+37, 0, 1,  1, 9.9999999999999998e+37, 0,
      2,  1, 9.9999999999999998e+37, 0, 3,  1, 9.9999999999999998e+37, 0,
      4,  1, 9.9999999999999998e+37, 0, 5,  1, 9.9999999999999998e+37, 0,
      6,  1, 9.9999999999999998e+37, 0, 7,  1, 9.9999999999999998e+37, 0,
      8,  1, 9.9999999999999998e+37, 0, 9,  1, 9.9999999999999998e+37, 0,
      10, 1, 9.9999999999999998e+37, 0, 11, 1, 9.9999999999999998e+37, 0};
  ssc_data_set_matrix(data, "ur_dc_flat_mat", p_ur_dc_flat_mat, 12, 4);
};
