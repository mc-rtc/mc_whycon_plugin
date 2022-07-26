#pragma once

#include <mc_control/CompletionCriteria.h>
#include <mc_control/fsm/Controller.h>
#include <mc_whycon_plugin/TaskUpdater.h>

#include <mc_solver/BoundedSpeedConstr.h>
#include <mc_tasks/BSplineTrajectoryTask.h>
#include <mc_tasks/LookAtTask.h>
#include <mc_tasks/PositionBasedVisServoTask.h>

#include <mc_whycon_plugin/WhyConSubscriber.h>

namespace whycon_plugin
{

/** Approach the selected bracket */
struct ApproachVisualServoing : mc_control::fsm::State
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  void start(mc_control::fsm::Controller & ctl) override;

  bool run(mc_control::fsm::Controller & ctl) override;

  void teardown(mc_control::fsm::Controller & ctl) override;

private:
  sva::PTransformd robotMarkerToFrameOffset(const mc_control::fsm::Controller & ctl) const;

  // Visual servoing target:
  // First compute the relative transform between the target marker and gripper
  // marker so that the robot frame is at the target frame (+offset) at the
  // end of the PBVS task convergence
  sva::PTransformd targetMarkerToFrameOffset(const mc_control::fsm::Controller & ctl) const;

  bool updatePBVSTask(mc_control::fsm::Controller & ctl);

  /** Look halfway between the expected marker pose and the marker pose on the
   * robot */
  void updateLookAt(const mc_control::fsm::Controller & ctl);
  void setBoundedSpeed(mc_control::fsm::Controller & ctl, double speed);
  void pause(mc_control::fsm::Controller & ctl);
  void resume(mc_control::fsm::Controller & ctl);
  void enableVisualServoing(mc_control::fsm::Controller & ctl);

private:
  /** Estimation of the bracket position obtained from the vision system */
  sva::PTransformd X_0_bracket_;
  std::string robotFrame_;
  std::string targetFrame_;
  sva::PTransformd targetFrameOffset_ = sva::PTransformd::Identity();
  std::string robotMarkerName_;
  std::string targetMarkerName_;

  /* Offset relative to the target frame where the
   * visual servoing task is to drive the robot */
  sva::PTransformd targetOffset_ = sva::PTransformd::Identity();

  std::shared_ptr<WhyConSubscriber> subscriber_ = nullptr;

  /** Spline task used to drive the gripper above
   * XXX: to be repaced with a spline with exact waypoints in the future
   **/
  std::shared_ptr<mc_tasks::BSplineTrajectoryTask> task_;
  /** Completion criteria for the approach spline */
  mc_control::CompletionCriteria taskCrit_;
  /** Waypoints */
  std::vector<sva::PTransformd> waypoints_;
  /* Whether to perform the approach phase */
  bool useApproach_ = true;
  /* Whether to use the visual servoing phase */
  bool useVisualServoing_ = true;

  /** Visual servoing task used to adjust the gripper x/y position */
  std::shared_ptr<mc_tasks::PositionBasedVisServoTask> pbvsTask_;
  std::shared_ptr<mc_solver::BoundedSpeedConstr> constr_;
  /* stiffness of the visual servoing task
   * Note that in case of non-convergence this stiffness will gradually increase
   * until convergence up to maxStiffness_
   **/
  double stiffness_ = 2;
  double maxStiffness_ = 2;
  /** actual max speed */
  double maxSpeed_ = 0.01;
  /* desired max speed */
  double maxSpeedDesired_ = 0.01;
  /** Whether visual servoing needs to be manually triggered */
  bool manualConfirmation_ = true;
  /** Evaluation threshold for the task */
  double evalTh_ = 0.02;
  /** Speed threshold for the task */
  double speedTh_ = 0.02;

  /** Task used to keep the vision system active */
  std::shared_ptr<mc_tasks::LookAtTask> lookAt_;
  /** User enables visual servoing approach */
  bool userEnableVS_ = false;
  /** True if the initial positioning is done */
  bool posDone_ = false;
  /** True if visual servoing was previously paused */
  bool vsResume_ = false;
  /** True when the marker was visible at the previous update of PBVS */
  bool wasVisible_ = true;
  bool visible_ = false;
  /** True if the task is paused */
  bool vsPaused_ = false;
  /** True if visual servoing is done */
  bool vsDone_ = false;
  std::vector<std::string> category_;

private:
  size_t iter_ = 0;
  size_t waypoint_ = 0;
  void nextWaypoint();
  void configureThresholds(const std::string & phase);
};

} // namespace whycon_plugin
