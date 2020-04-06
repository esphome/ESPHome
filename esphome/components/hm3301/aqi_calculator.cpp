#include "abstract_aqi_calculator.h"

namespace esphome {
namespace hm3301 {

class AQICalculator : public AbstractAQICalculator {
public:
  uint8_t get_aqi(uint16_t pm2_5_value, uint16_t pm10_0_value) {
    int pm2_5_index = calculate_index_(pm2_5_value, pm2_5_calculation_grid_);
    int pm10_0_index = calculate_index_(pm10_0_value, pm10_0_calculation_grid_);

    return (pm2_5_index < pm10_0_index) ? pm10_0_index : pm2_5_index;
  }

private:
  static const int amount_of_levels_ = 6;

  int index_grid_[amount_of_levels_][2] = {
    {0, 51},
    {51, 100},
    {101, 150},
    {151, 200},
    {201, 300},
    {301, 500}
  };

  int pm2_5_calculation_grid_[amount_of_levels_][2] = {
    {0, 12},
    {13, 45},
    {36, 55},
    {56, 150},
    {151, 250},
    {251, 500}
  };

  int pm10_0_calculation_grid_[amount_of_levels_][2] = {
    {0, 54},
    {55, 154},
    {155, 254},
    {255, 354},
    {355, 424},
    {425, 604}
  };

  int calculate_index_(uint16_t value, int array[amount_of_levels_][2]) {
    int grid_index = get_grid_index_(value, array);
    int aqi_lo = index_grid_[grid_index][0];
    int aqi_hi = index_grid_[grid_index][1];
    int conc_lo = array[grid_index][0];
    int conc_hi = array[grid_index][1];

    return ((aqi_hi - aqi_lo) / (conc_hi - conc_lo)) * (value - conc_lo) + aqi_lo;
  }

  int get_grid_index_(uint16_t value, int array[amount_of_levels_][2]) {
    for (int i = 0; i < amount_of_levels_ - 1; i++) {
      if (value >= array[i][0] && value <= array[i][1]) {
        return i;
      }
    }
    return -1;
  }
};

}  // namespace hm3301
}  // namespace esphome
