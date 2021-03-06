
#include <iostream>

#include <ros/ros.h>

#include <assert.h>
#include <filter_test/residuals/constant_residual.h>
#include <filter_test/residuals/constant_velocity_residual.h>
#include <filter_test/residuals/position_residual.h>
#include "filter_test/estimator.h"
#include "filter_test/measurement.h"

#include <glog/logging.h>

using namespace tsif;

enum StateDefinition { kStatePosition = 0, kStateVelocity, kStateOrientation };

enum MeasurementDefinition { kMeasPosition = 0, kMeasImu };

class InitStateConstVelocity : public InitStateBase {
 public:
  InitStateConstVelocity() {}
  ~InitStateConstVelocity() {}
  virtual bool init(
      const MeasurementManager& measurement_manager,
      const MeasurementBuffer& measurement_buffer, State* state,
      MatrixX* information) {
    const Timeline& position_timeline =
        measurement_manager.getTimeline(kMeasPosition);
    const PositionMeasurement* position_measurement =
        position_timeline.getMeasurement<PositionMeasurement>(
            measurement_buffer.timestamp_ns);

    state->template setValue<VectorBlock<3>>(
        kStatePosition, position_measurement->position_);
    state->template setValue<VectorBlock<3>>(kStateVelocity, Vector3::Zero());

    information->setIdentity();
    return true;
  }

 private:
};

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Announce this program to the ROS master
  ros::init(argc, argv, "filter_test_node");
  // Start the node resource managers (communication, time, etc)
  ros::NodeHandle nh("~");

  std::vector<BlockTypeId> state_block_types{kVector3, kVector3};
  //  std::vector<size_t> state_names {kPosition, kVelocity, kOrientation};

  Estimator test_estimator(new InitStateConstVelocity());
  test_estimator.defineState(state_block_types);

  Vector3 initial_position(1, 2, 3);
  test_estimator.initStateValue(kStatePosition, initial_position);

  test_estimator.printState();

  ConstantVelocityResidual* test_residual1 = new ConstantVelocityResidual(1, 1);
  std::vector<size_t> first_keys{kStatePosition, kStateVelocity};
  std::vector<size_t> second_keys{kStatePosition, kStateVelocity};
  test_estimator.addResidual(test_residual1, first_keys, second_keys);

  PositionResidual* test_residual2 = new PositionResidual(Matrix3::Identity());
  first_keys = {};
  second_keys = {kStatePosition};
  std::vector<size_t> measurement_keys{kMeasPosition};

  test_estimator.addResidual(
      test_residual2, first_keys, second_keys, measurement_keys);

  test_estimator.printResiduals();
  test_estimator.checkResiduals();
  test_estimator.printTimeline();

  MeasurementBase* position_measurement1 =
      new PositionMeasurement(Vector3(1, 1, 1));
  test_estimator.addMeasurement(kMeasPosition, 140, position_measurement1);
  MeasurementBase* position_measurement2 =
      new PositionMeasurement(Vector3(1, 1, 1));
  test_estimator.addMeasurement(kMeasPosition, 150, position_measurement2);

  test_estimator.printTimeline();

  // Stop the node's resources
  ros::shutdown();
  // Exit tranquilly
  return 0;
}
