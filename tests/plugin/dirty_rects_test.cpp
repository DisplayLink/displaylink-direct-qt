// Copyright (C) 2026 Synaptics Incorporated
#include <QRect>
#include <QRegion>

#include <bridge.hpp>
#include <dirty_rects.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace dl::sdk::qpa::plugin::tests
{

namespace
{

MATCHER_P4(IsRect, x, y, width, height, "")
{
    return arg.x == static_cast<unsigned int>(x) && arg.y == static_cast<unsigned int>(y) &&
           arg.width == static_cast<unsigned int>(width) && arg.height == static_cast<unsigned int>(height);
}

QRegion makeDiagonalRegion(int rectCount)
{
    QRegion region;
    for (int i = 0; i < rectCount; ++i)
    {
        region += QRect(2 * i, 2 * i, 1, 1);
    }
    return region;
}

} // namespace

TEST(DirtyRectsTest, EmptyRegionProducesNoRects)
{
    EXPECT_TRUE(toDirtyRects(QRegion(), bridge::MaxDirtyRects).empty());
}

TEST(DirtyRectsTest, SingleRectKeepsPositionAndSize)
{
    EXPECT_THAT(
        toDirtyRects(QRegion(10, 20, 30, 40), bridge::MaxDirtyRects), testing::ElementsAre(IsRect(10, 20, 30, 40))
    );
}

TEST(DirtyRectsTest, DisjointRectsAreForwardedIndividually)
{
    const QRegion region = QRegion(0, 0, 10, 10) + QRegion(50, 60, 5, 5);

    ASSERT_EQ(region.rectCount(), 2);
    EXPECT_THAT(
        toDirtyRects(region, bridge::MaxDirtyRects),
        testing::UnorderedElementsAre(IsRect(0, 0, 10, 10), IsRect(50, 60, 5, 5))
    );
}

TEST(DirtyRectsTest, RectCountAtLimitIsNotCollapsed)
{
    const QRegion region = makeDiagonalRegion(bridge::MaxDirtyRects);

    ASSERT_EQ(region.rectCount(), bridge::MaxDirtyRects);
    EXPECT_EQ(toDirtyRects(region, bridge::MaxDirtyRects).size(), static_cast<size_t>(bridge::MaxDirtyRects));
}

TEST(DirtyRectsTest, RectCountAboveLimitCollapsesToBoundingBox)
{
    const QRegion region = makeDiagonalRegion(bridge::MaxDirtyRects + 1);
    const QRect bounds = region.boundingRect();

    ASSERT_EQ(region.rectCount(), bridge::MaxDirtyRects + 1);
    EXPECT_THAT(
        toDirtyRects(region, bridge::MaxDirtyRects),
        testing::ElementsAre(IsRect(bounds.x(), bounds.y(), bounds.width(), bounds.height()))
    );
}

TEST(DirtyRectsTest, RegionClippedToScreenHasNoNegativeCoordinates)
{
    const QRect screenGeometry(0, 0, 640, 480);
    const QRegion region = QRegion(-5, -5, 20, 20) + QRegion(630, 470, 100, 100);

    const auto rects = toDirtyRects(region.intersected(screenGeometry), bridge::MaxDirtyRects);

    EXPECT_THAT(rects, testing::UnorderedElementsAre(IsRect(0, 0, 15, 15), IsRect(630, 470, 10, 10)));
}

} // namespace dl::sdk::qpa::plugin::tests
