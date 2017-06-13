/*
 * filter.h
 *
 *  Created on: 02.06.2017
 *      Author: burrimi
 */

#ifndef INCLUDE_FILTER_TEST_FILTER_H_
#define INCLUDE_FILTER_TEST_FILTER_H_

#include <list>
#include <iostream>

#include "filter_test/block.h"
#include "filter_test/defines.h"
#include "filter_test/measurement_manager.h"
#include "filter_test/residual.h"
#include "filter_test/state.h"
#include "filter_test/helper_functions.h"

struct ResidualContainer {
  std::vector<int> first_keys;
  std::vector<int> second_keys;
  std::vector<int> measurement_keys;
  ResidualBase* residual_;
};

class Filter {
 public:
  Filter():total_residual_dimension_(0), first_run_(true), timestamp_previous_update_ns_(-1) {}

  ~Filter() {
    for(ResidualContainer& current_residual:residuals_) {
      delete current_residual.residual_;
    }
  }

  // state related stuff
  std::vector<BlockType> state_types_;
  //std::vector<std::string> state_names_;

  State first_state_;
  State second_state_;

  MatrixX information_;
  VectorX residual_vector_;
  MatrixX jacobian_wrt_state1_;
  MatrixX jacobian_wrt_state2_;

  MeasurementManager measurement_manager_;

  // everything related to the residuals
  std::vector<ResidualContainer> residuals_;
  int total_residual_dimension_;

  bool defineState(std::vector<BlockType> state_types);

  void initStateValue(const int key, const VectorXRef& value);

  // Adds a residual and takes ownership of the residual.
  // Classical KF measurements only depend on the second state and therefore only contain second_keys.
  bool addResidual(ResidualBase* residual, std::vector<int> first_keys,
                   std::vector<int> second_keys, std::vector<int> measurement_keys = std::vector<int>());

  void addMeasurement(int timeline_key, int timestamp_ns, MeasurementBase* measurement);


  void printState();

  void printTimeline();

  void printResiduals();

  void checkResiduals();

  void step();

 private:

  inline std::vector<BlockBase*> getBlocks(const State& state, const std::vector<int>& keys) {
    std::vector<BlockBase*> blocks;
    for(int current_key:keys) {
      blocks.emplace_back(state.state_blocks_[current_key]);
    }
    return blocks;
  }

  inline std::vector<MatrixXRef> getJacobianBlocks(MatrixX& jacobian, const std::vector<int>& keys, const int& residual_index, const int& residual_dimension) {
    std::vector<MatrixXRef> jacobian_blocks;
    for(int current_key:keys) {
  //    MatrixXRef test = jacobian.block(residual_index, current_key, residual_dimension, first_state_.minimal_dimension_);
      const int& state_index = first_state_.getAccumulatedMinimalDimension(current_key);
      jacobian_blocks.emplace_back(jacobian.block(residual_index, state_index, residual_dimension, first_state_.minimal_dimension_));
    }
    return jacobian_blocks;
  }

  bool init();
  void update(const int timestamp_ns);

  bool first_run_;
  int timestamp_previous_update_ns_;
};



#endif /* INCLUDE_FILTER_TEST_FILTER_H_ */