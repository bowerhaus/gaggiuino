#include <unity.h>
#include "functional/profiling_phases.h"
#include "sensors_state.h"
#include "utils.h"


Phase pressurePhase(float start, float end, long time) {
  Phase phase = Phase{PHASE_TYPE_PRESSURE, Transition{start, end}, 0.f, PhaseStopConditions{}};
  phase.stopConditions.time = time;
  return phase;
}

Phase presurePhaseWithWeightTarget(float start, float end, long time, float weight) {
  Phase phase = Phase{PHASE_TYPE_PRESSURE, Transition{start, end}, 0.f, PhaseStopConditions{}};
  phase.stopConditions.time = time;
  phase.stopConditions.weight = weight;
  return phase;
}

Phase phaseArray[] = {
    pressurePhase(0, 2, 1000),
    pressurePhase(2, 2, 10000),
    pressurePhase(2, 9, 1000),
    pressurePhase(9, 9, 10000),
    pressurePhase(9, 6, 30000),
};

Phases phases = {5, phaseArray};
PhaseProfiler phaseProfiler = PhaseProfiler(phases);
SensorState state;

void test_current_phase_calculation(void)
{
    phaseProfiler.reset();
    phaseProfiler.updatePhase(0, state);
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getTimeInPhase());

    phaseProfiler.updatePhase(550, state);
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(550, phaseProfiler.getCurrentPhase().getTimeInPhase());

    phaseProfiler.updatePhase(1000, state);
    TEST_ASSERT_EQUAL(1, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(0, phaseProfiler.getCurrentPhase().getTimeInPhase());

    phaseProfiler.updatePhase(5000, state);
    TEST_ASSERT_EQUAL(1, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(4000, phaseProfiler.getCurrentPhase().getTimeInPhase());

    phaseProfiler.updatePhase(30000, state);
    TEST_ASSERT_EQUAL(4, phaseProfiler.getCurrentPhase().getIndex());
    TEST_ASSERT_EQUAL(8000, phaseProfiler.getCurrentPhase().getTimeInPhase());
}

void test_get_pressure_for_phase(void)
{
    Phase phase = pressurePhase(0, 2, 1000);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, phase.getTarget(0));
    TEST_ASSERT_EQUAL_FLOAT(1.0f, phase.getTarget(500));
    TEST_ASSERT_EQUAL_FLOAT(1.5f, phase.getTarget(750));
    TEST_ASSERT_EQUAL_FLOAT(2.0f, phase.getTarget(1000));
}


void test_get_pressure_for_phase_with_negative_change(void)
{
    Phase phase = pressurePhase(9, 6, 3000);

    TEST_ASSERT_EQUAL_FLOAT(9.0f, phase.getTarget(0));
    TEST_ASSERT_EQUAL_FLOAT(8.0f, phase.getTarget(1000));
    TEST_ASSERT_EQUAL_FLOAT(7.0f, phase.getTarget(2000));
    TEST_ASSERT_EQUAL_FLOAT(6.0f, phase.getTarget(3000));
}

void test_get_pressure_for_phase_with_time_larger_than_duration(void)
{
    Phase phase = pressurePhase(9, 6, 3000);

    TEST_ASSERT_EQUAL_FLOAT(6.0f, phase.getTarget(4000));
    TEST_ASSERT_EQUAL_FLOAT(6.0f, phase.getTarget(10000));
}

void test_phases_with_zero_duration_are_skipped(void) {
    Phase array[] = {
      pressurePhase(2, 2, 0),
      pressurePhase(2, 5, 0),
      pressurePhase(2, 5, 0),
      pressurePhase(5, 5, 1000)
    };
    Phases phases = Phases {4, array};
    PhaseProfiler profiler = PhaseProfiler{phases};

    profiler.updatePhase(0, state);
    TEST_ASSERT_EQUAL(3, profiler.getCurrentPhase().getIndex());
}


void test_phases_with_weight_stop_condition(void) {
  SensorState mockedState;
  mockedState.shotWeight = 0.f;
  mockedState.smoothedPumpFlow = 0.f;
  mockedState.weightFlow = 0.f;

  float weightTarget = 0.4f;
  Phase array[] = {
    presurePhaseWithWeightTarget(2, 2, -1, weightTarget),
    pressurePhase(5, 5, 1000)
  };
  Phases phases = Phases {2, array};
  PhaseProfiler profiler = PhaseProfiler{phases};

  mockedState.shotWeight = 0.2f;
  profiler.updatePhase(3000, mockedState);
  TEST_ASSERT_FALSE(profiler.isFinished());

  mockedState.shotWeight = 0.5f;
  profiler.updatePhase(3000, mockedState);
  TEST_ASSERT_FALSE(profiler.isFinished());

  profiler.updatePhase(3500, mockedState);
  TEST_ASSERT_EQUAL(1, profiler.getCurrentPhase().getIndex());
  TEST_ASSERT_EQUAL(500, profiler.getCurrentPhase().getTimeInPhase());
  TEST_ASSERT_FALSE(profiler.isFinished());

  profiler.updatePhase(4100, mockedState);
  TEST_ASSERT_EQUAL(100, profiler.getCurrentPhase().getTimeInPhase());
  TEST_ASSERT_TRUE(profiler.isFinished());
}

void runAllPressureProfilerTests() {
  RUN_TEST(test_current_phase_calculation);
  RUN_TEST(test_get_pressure_for_phase);
  RUN_TEST(test_get_pressure_for_phase_with_negative_change);
  RUN_TEST(test_get_pressure_for_phase_with_time_larger_than_duration);
  RUN_TEST(test_phases_with_zero_duration_are_skipped);
  RUN_TEST(test_phases_with_weight_stop_condition);
}
