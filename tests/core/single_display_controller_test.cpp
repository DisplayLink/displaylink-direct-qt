// Copyright (C) 2026 Synaptics Incorporated
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <single_display_controller.hpp>

#include "common.hpp"

namespace dl::sdk::qpa::tests
{

class SingleDisplayControllerTest : public ::testing::Test
{
};

TEST_F(SingleDisplayControllerTest, FirstDisplayBecomesActive)
{
    std::string attachedDisplay;
    core::SingleDisplayController controller(
        [&](const auto& display)
        {
            attachedDisplay = display->id();
            return true;
        },
        [](const auto&) {}
    );

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));

    EXPECT_EQ(attachedDisplay, DISPLAY_1);
    EXPECT_EQ(controller.activeDisplayId(), DISPLAY_1);
    EXPECT_EQ(controller.candidateCount(), 0);
}

TEST_F(SingleDisplayControllerTest, ActiveDisplayRemainsSticky)
{
    int attachCount = 0;
    core::SingleDisplayController controller(
        [&](const auto&)
        {
            ++attachCount;
            return true;
        },
        [](const auto&) {}
    );

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_2));

    EXPECT_EQ(attachCount, 1);
    EXPECT_EQ(controller.activeDisplayId(), DISPLAY_1);
    EXPECT_EQ(controller.candidateCount(), 1);
}

TEST_F(SingleDisplayControllerTest, RemovingActiveDisplayUsesOldestCandidate)
{
    std::vector<std::string> attachments;
    std::string detachedDisplay;
    core::SingleDisplayController controller(
        [&](const auto& display)
        {
            attachments.push_back(display->id());
            return true;
        },
        [&](const auto& displayId) { detachedDisplay = displayId; }
    );

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_2));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_3));
    controller.onDisplayRemoved(DISPLAY_1);

    EXPECT_EQ(detachedDisplay, DISPLAY_1);
    EXPECT_THAT(attachments, testing::ElementsAre(DISPLAY_1, DISPLAY_2));
    EXPECT_EQ(controller.activeDisplayId(), DISPLAY_2);
    EXPECT_EQ(controller.candidateCount(), 1);
}

TEST_F(SingleDisplayControllerTest, RemovingCandidateDoesNotAffectActiveDisplay)
{
    int detachCount = 0;
    core::SingleDisplayController controller([](const auto&) { return true; }, [&](const auto&) { ++detachCount; });

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_2));
    controller.onDisplayRemoved(DISPLAY_2);

    EXPECT_EQ(detachCount, 0);
    EXPECT_EQ(controller.activeDisplayId(), DISPLAY_1);
    EXPECT_EQ(controller.candidateCount(), 0);
}

TEST_F(SingleDisplayControllerTest, FailedCandidateDoesNotBlockFailover)
{
    std::vector<std::string> attachmentAttempts;
    core::SingleDisplayController controller(
        [&](const auto& display)
        {
            const std::string id = display->id();
            attachmentAttempts.push_back(id);
            return id != DISPLAY_2;
        },
        [](const auto&) {}
    );

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_2));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_3));
    controller.onDisplayRemoved(DISPLAY_1);

    EXPECT_THAT(attachmentAttempts, testing::ElementsAre(DISPLAY_1, DISPLAY_2, DISPLAY_3));
    EXPECT_EQ(controller.activeDisplayId(), DISPLAY_3);
    EXPECT_EQ(controller.candidateCount(), 1);
}

TEST_F(SingleDisplayControllerTest, DuplicateActiveDisplayIsIgnored)
{
    int attachCount = 0;
    core::SingleDisplayController controller(
        [&](const auto&)
        {
            ++attachCount;
            return true;
        },
        [](const auto&) {}
    );

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));

    EXPECT_EQ(attachCount, 1);
    EXPECT_EQ(controller.activeDisplayId(), DISPLAY_1);
    EXPECT_EQ(controller.candidateCount(), 0);
}

TEST_F(SingleDisplayControllerTest, DuplicateCandidateIsIgnored)
{
    core::SingleDisplayController controller([](const auto&) { return true; }, [](const auto&) {});

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_2));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_2));

    EXPECT_EQ(controller.candidateCount(), 1);
}

TEST_F(SingleDisplayControllerTest, FailedFirstDisplayIsQueuedAsCandidate)
{
    core::SingleDisplayController controller([](const auto&) { return false; }, [](const auto&) {});

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));

    EXPECT_EQ(controller.activeDisplayId(), "");
    EXPECT_EQ(controller.candidateCount(), 1);
}

TEST_F(SingleDisplayControllerTest, RemovingUnknownDisplayIsNoOp)
{
    int detachCount = 0;
    core::SingleDisplayController controller([](const auto&) { return true; }, [&](const auto&) { ++detachCount; });

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_2));
    controller.onDisplayRemoved(DISPLAY_GENERIC);

    EXPECT_EQ(detachCount, 0);
    EXPECT_EQ(controller.activeDisplayId(), DISPLAY_1);
    EXPECT_EQ(controller.candidateCount(), 1);
}

TEST_F(SingleDisplayControllerTest, AddingNullDisplayIsIgnored)
{
    int attachCount = 0;
    core::SingleDisplayController controller(
        [&](const auto&)
        {
            ++attachCount;
            return true;
        },
        [](const auto&) {}
    );

    controller.onDisplayAdded(nullptr);

    EXPECT_EQ(attachCount, 0);
    EXPECT_EQ(controller.activeDisplayId(), "");
    EXPECT_EQ(controller.candidateCount(), 0);
}

TEST_F(SingleDisplayControllerTest, AllCandidatesFailingDuringFailoverLeavesNoActiveDisplay)
{
    core::SingleDisplayController controller(
        [](const auto& display) { return display->id() == DISPLAY_1; }, [](const auto&) {}
    );

    controller.onDisplayAdded(createMockDisplay(DISPLAY_1));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_2));
    controller.onDisplayAdded(createMockDisplay(DISPLAY_3));
    controller.onDisplayRemoved(DISPLAY_1);

    EXPECT_EQ(controller.activeDisplayId(), "");
    EXPECT_EQ(controller.candidateCount(), 2);
}

} // namespace dl::sdk::qpa::tests
