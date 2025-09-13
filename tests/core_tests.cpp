#include <gtest/gtest.h>
#include "core/sampling.hpp"

// Test fixture for sampling tests
class SamplingTest : public ::testing::Test {
protected:
    std::vector<int> data;
    void SetUp() override {
        for (int i = 0; i < 1000; ++i) {
            data.push_back(i);
        }
    }
};

TEST_F(SamplingTest, ReservoirSampleShouldHaveCorrectSize) {
    aqe::core::ReservoirSample<int> sampler(100);
    for (int item : data) {
        sampler.add(item);
    }
    auto sample = sampler.getSample();
    ASSERT_EQ(sample.size(), 100);
}

TEST_F(SamplingTest, SimpleRandomSampleIsApproximate) {
    aqe::core::SimpleRandomSampling<int> sampler(0.1); // 10% rate
    for (int item : data) {
        sampler.add(item);
    }
    auto sample = sampler.getSample();
    // Should be roughly 10% of 1000, we'll test for a reasonable range
    ASSERT_GT(sample.size(), 50);
    ASSERT_LT(sample.size(), 150);
}