#include "qrange.h"

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(TestQRangeConstructors, Default)
{
    QRange<int> range{};
    ASSERT_EQ(range.start(), 0);
    ASSERT_EQ(range.end(), 0);
    ASSERT_EQ(range.direction(), QRangeDirection::Zero);
}

TEST(TestQRangeConstructors, DefaultParameters)
{
    QRange<int> range{1, 10};
    ASSERT_EQ(range.start(), 1);
    ASSERT_EQ(range.end(), 10);
    ASSERT_EQ(range.direction(), QRangeDirection::Plus);
}

TEST(TestQRangeConstructors, Pair)
{
    QRange<int> range{std::pair<int, int>{ 1, 10 }};
    ASSERT_EQ(range.start(), 1);
    ASSERT_EQ(range.end(), 10);
    ASSERT_EQ(range.direction(), QRangeDirection::Plus);
}

TEST(TestQRangeConstructors, AnotherBaseDefault)
{
    QRange<int> range{"10101010", "A9", 2, 16};
    ASSERT_EQ(range.start(), 170);
    ASSERT_EQ(range.end(), 169);
    ASSERT_EQ(range.direction(), QRangeDirection::Minus);
}

TEST(TestQRangeConstructors, AnotherBaseOffset)
{
    QRange<int> range{"800", 1024 * 8, 16};
    ASSERT_EQ(range.start(), 2048);
    ASSERT_EQ(range.end(), 10240);
    ASSERT_EQ(range.direction(), QRangeDirection::Plus);
}

TEST(TestQRangeGetters, All)
{
    QRange<int> range{ 10, 1 };
    
    ASSERT_EQ(range.start(), 10);
    ASSERT_EQ(range.end(), 1);
    
    ASSERT_EQ(range.lower(), 1);
    ASSERT_EQ(range.upper(), 10);
    
    ASSERT_EQ(range.middle(), 5);
    ASSERT_EQ(range.middleAccurate(), 5.5);
    
    ASSERT_EQ(range.length(), 9);
    ASSERT_EQ(range.direction(), QRangeDirection::Minus);
    
    auto pair = range.toAnotherBase(2);
    
    ASSERT_STREQ(pair.first.toStdString().c_str(), "1010");
    ASSERT_STREQ(pair.second.toStdString().c_str(), "1");
}

TEST(TestQRangeSetters, All)
{
    QRange<int> range{ 10, 1 };
    
    range.setStart(5);
    ASSERT_EQ(range.start(), 5);
    ASSERT_EQ(range.end(), 1);
    ASSERT_EQ(range.direction(), QRangeDirection::Minus);
    
    range.setEnd(10);
    ASSERT_EQ(range.start(), 5);
    ASSERT_EQ(range.end(), 10);
    ASSERT_EQ(range.direction(), QRangeDirection::Plus);
    
    range.setDirection(-10);
    range.setDirection(0);
    
    range.setDirection(-1);
    ASSERT_EQ(range.start(), 10);
    ASSERT_EQ(range.end(), 5);
    ASSERT_EQ(range.direction(), QRangeDirection::Minus);
}

TEST(TestQRangeProperties, All)
{
    QRange<float> range_zr{};
    QRange<float> range_ng{ -1.5, -0.1 };
    QRange<int> range_pl{ 0, 5 };
    QRange<int> range_mx{ -1, 1 };
    
    ASSERT_EQ(range_zr.isZero(), true);
    ASSERT_EQ(range_zr.isNegative(), false);
    ASSERT_EQ(range_zr.isPositive(), false);
    ASSERT_EQ(range_zr.isMixed(), false);
    
    ASSERT_EQ(range_ng.isZero(), false);
    ASSERT_EQ(range_ng.isNegative(), true);
    ASSERT_EQ(range_ng.isPositive(), false);
    ASSERT_EQ(range_ng.isMixed(), false);
    
    ASSERT_EQ(range_pl.isZero(), false);
    ASSERT_EQ(range_pl.isNegative(), false);
    ASSERT_EQ(range_pl.isPositive(QRangeSensitivity::Insensitive), true);
    ASSERT_EQ(range_pl.isMixed(), false);
    
    ASSERT_EQ(range_mx.isZero(), false);
    ASSERT_EQ(range_mx.isNegative(), false);
    ASSERT_EQ(range_mx.isPositive(), false);
    ASSERT_EQ(range_mx.isMixed(), true);
}

TEST(TestQRangeMethods, QRangeReverse)
{
    QRange<float> range_1{ -1, 5 };
    QRange<int> range_2{ 5, -1 };
    
    range_1.reverse(QRangeDirection::Plus);
    range_2.reverse(-1);
    
    ASSERT_EQ(range_1.start(), 1);
    ASSERT_EQ(range_1.end(), 5);
    ASSERT_EQ(range_1.direction(), QRangeDirection::Plus);
    
    ASSERT_EQ(range_2.start(), -5);
    ASSERT_EQ(range_2.end(), -1);
    ASSERT_EQ(range_2.direction(), QRangeDirection::Plus);
}

TEST(TestQRangeMethods, QRangeEnclosure)
{
    QRange<int> range_large{ 1, 10 };
    QRange<int> source_range{ 1, 10 };
    QRange<int> range_small{ 1, 5 };
    QRange<int> range_overlay_0{ -3, 1 };
    
    ASSERT_EQ(source_range.contains(1), false);
    ASSERT_EQ(source_range.contains(1, QRangeSensitivity::Insensitive), true);
    
    ASSERT_EQ(source_range.contains(range_small), false);
    ASSERT_EQ(source_range.contains(range_small, QRangeSensitivity::Insensitive), true);
    
    ASSERT_EQ(source_range.in(range_large), false);
    ASSERT_EQ(source_range.in(range_large, QRangeSensitivity::Insensitive), true);
    
    ASSERT_EQ(source_range.overlays(range_overlay_0), false);
    ASSERT_EQ(range_overlay_0.overlays(source_range, QRangeSensitivity::Insensitive), true);
}

TEST(TestQRangeStatic, QRangeMinMax)
{
    auto res_min = QRange<int>::min(QRange<int>{ 1, 3 },
                     QList<QRange<int>>{ QRange<int>{ 0, 8 }, QRange<int>{ -8, -10 } },
                     std::pair<QRange<int>, QRange<int>>{ QRange<int>{ 8, 3 }, QRange<int>{ -6, -1 } },
                     std::vector<QRange<int>>{ QRange<int>{ 9, -1 }, QRange<int>{ 0, 0 } });
    
    ASSERT_EQ(res_min.start(), 0);
    ASSERT_EQ(res_min.end(), 0);
    ASSERT_EQ(res_min.direction(), QRangeDirection::Zero);
    
    //////////////////////////////////////////////////
    
    auto res_max  = QRange<int>::max(QRange<int>{ 1, 3 },
                                QList<QRange<int>>{ QRange<int>{ 0, 8 }, QRange<int>{ -8, -10 } },
                                std::pair<QRange<int>, QRange<int>>{ QRange<int>{ 8, 3 }, QRange<int>{ -6, -1 } },
                                std::vector<QRange<int>>{ QRange<int>{ 9, -1 }, QRange<int>{ 0, 0 } });
    
    ASSERT_EQ(res_max.start(), 9);
    ASSERT_EQ(res_max.end(), -1);
    ASSERT_EQ(res_max.direction(), QRangeDirection::Minus);
    
    //////////////////////////////////////////////////
    
    auto res_minX = QRange<int>::minX(QRange<int>{ 1, 3 },
                                QList<QRange<int>>{ QRange<int>{ 0, 8 }, QRange<int>{ -8, -10 } },
                                std::pair<QRange<int>, QRange<int>>{ QRange<int>{ 8, 3 }, QRange<int>{ -6, -1 } },
                                std::vector<QRange<int>>{ QRange<int>{ 9, -1 }, QRange<int>{ 0, 0 } });
    
    ASSERT_EQ(res_minX.start(), -8);
    ASSERT_EQ(res_minX.end(), -10);
    ASSERT_EQ(res_minX.direction(), QRangeDirection::Minus);
    
    //////////////////////////////////////////////////
    
    auto res_maxX = QRange<int>::maxX(QRange<int>{ 1, 3 },
                                QList<QRange<int>>{ QRange<int>{ 0, 8 }, QRange<int>{ -8, -10 } },
                                std::pair<QRange<int>, QRange<int>>{ QRange<int>{ 8, 3 }, QRange<int>{ -6, -1 } },
                                std::vector<QRange<int>>{ QRange<int>{ 9, -1 }, QRange<int>{ 0, 0 } });
    
    ASSERT_EQ(res_maxX.start(), 9);
    ASSERT_EQ(res_maxX.end(), -1);
    ASSERT_EQ(res_maxX.direction(), QRangeDirection::Minus);
    
    //////////////////////////////////////////////////
    
    auto res_minMidX = QRange<int>::minMidX(QRange<int>{ 1, 3 },
                                      QList<QRange<int>>{ QRange<int>{ 0, 8 }, QRange<int>{ -8, -10 } },
                                      std::pair<QRange<int>, QRange<int>>{ QRange<int>{ 8, 3 }, QRange<int>{ -6, -1 } },
                                      std::vector<QRange<int>>{ QRange<int>{ 9, -1 }, QRange<int>{ 0, 0 } });
    
    ASSERT_EQ(res_minMidX.start(), -8);
    ASSERT_EQ(res_minMidX.end(), -10);
    ASSERT_EQ(res_minMidX.direction(), QRangeDirection::Minus);
    
    //////////////////////////////////////////////////
    
    auto res_maxMidX = QRange<int>::maxMidX(QRange<int>{ 1, 3 },
                                      QList<QRange<int>>{ QRange<int>{ 0, 8 }, QRange<int>{ -8, -10 } },
                                      std::pair<QRange<int>, QRange<int>>{ QRange<int>{ 8, 3 }, QRange<int>{ -6, -1 } },
                                      std::vector<QRange<int>>{ QRange<int>{ 9, -1 }, QRange<int>{ 0, 0 } });
    
    ASSERT_EQ(res_maxMidX.start(), 8);
    ASSERT_EQ(res_maxMidX.end(), 3);
    ASSERT_EQ(res_maxMidX.direction(), QRangeDirection::Minus);
}

TEST(TestQRangeStatic, QRangeBrakeMerge)
{
    QList<QRange<int>> ranges{ QRange<int>{ 1, 5 }, QRange<int>{ -9, 10 } };
    QList<int> numbers = QRange<int>::brake(ranges);
    
    ASSERT_THAT((std::vector<int>{ numbers.begin(), numbers.end() }),
                ElementsAre(1, 5, -9, 10));
    
    numbers.append(8);
    QList<QRange<int>> ranges_merged = QRange<int>::merge(numbers);
    
    ASSERT_EQ(ranges_merged.size(), 3);
    ASSERT_EQ(ranges_merged.at(2).start(), 8);
    ASSERT_EQ(ranges_merged.at(2).end(), 8);
    ASSERT_EQ(ranges_merged.at(2).direction(), QRangeDirection::Zero);
}

TEST(TestQRangeStatic, QRangeCut)
{
    QRange<int> range_minus{ 25, -25 };
    auto ranges_minus = QRange<int>::cut(range_minus, 5);
    
    ASSERT_EQ(ranges_minus.size(), 5);
    ASSERT_EQ(ranges_minus.at(1).start(), 15);
    ASSERT_EQ(ranges_minus.at(1).end(), 5);
    ASSERT_EQ(ranges_minus.at(1).direction(), QRangeDirection::Minus);
    
    //////////////////////////////////////////////////
    
    QRange<int> range_plus{ -25, 25 };
    auto ranges_plus = QRange<int>::cut(range_plus, 5);
    
    ASSERT_EQ(ranges_plus.size(), 5);
    ASSERT_EQ(ranges_plus.at(1).start(), -15);
    ASSERT_EQ(ranges_plus.at(1).end(), -5);
    ASSERT_EQ(ranges_plus.at(1).direction(), QRangeDirection::Plus);
}

TEST(TestQRangeStatic, QRangeCombine)
{
    QList<QRange<int>> ranges_ideal{ QRange<int>{ -8, -4 }, QRange<int>{ -4, 3 }, QRange<int>{ 3, 6 } };
    
    QList<QRange<int>> ranges_not_ideal{ QRange<int>{ -4, -8 }, QRange<int>{ 0, 0 }, QRange<int>{ 9, 1 } };
    
    auto r_i = QRange<int>::combine(ranges_ideal);
    // QRange<int>::combine(ranges_not_ideal);
    
    auto r_n_i = QRange<int>::combine(ranges_ideal, QRangeSensitivity::Insensitive);
    auto r_n_i_1 = QRange<int>::combine(ranges_not_ideal, QRangeSensitivity::Insensitive);
    
    ASSERT_EQ(r_i.start(), -8);
    ASSERT_EQ(r_i.end(), 6);
    ASSERT_EQ(r_i.direction(), QRangeDirection::Plus);
    
    ASSERT_EQ(r_n_i.start(), -8);
    ASSERT_EQ(r_n_i.end(), 6);
    ASSERT_EQ(r_n_i.direction(), QRangeDirection::Plus);
    
    ASSERT_EQ(r_n_i_1.start(), 9);
    ASSERT_EQ(r_n_i_1.end(), -8);
    ASSERT_EQ(r_n_i_1.direction(), QRangeDirection::Minus);
}

TEST(TestQRangeOperators, Number)
{
    QRange<int> range{ 1, 5 };
    
    ASSERT_EQ((range + 1).start(), 2);
    ASSERT_EQ((range + 1).end(), 6);
    ASSERT_EQ((range + 1).direction(), QRangeDirection::Plus);
    
    ASSERT_EQ((range - 1).start(), 0);
    ASSERT_EQ((range - 1).end(), 4);
    ASSERT_EQ((range - 1).direction(), QRangeDirection::Plus);
    
    ASSERT_EQ((range * 2).start(), 2);
    ASSERT_EQ((range * 2).end(), 10);
    ASSERT_EQ((range * 2).direction(), QRangeDirection::Plus);
    
    ASSERT_EQ((range / 2).start(), 0);
    ASSERT_EQ((range / 2).end(), 2);
    ASSERT_EQ((range / 2).direction(), QRangeDirection::Plus);
    
    ASSERT_EQ((range % 2).start(), 1);
    ASSERT_EQ((range % 2).end(), 1);
    ASSERT_EQ((range % 2).direction(), QRangeDirection::Zero);
}

TEST(TestQRangeOperators, Range)
{
    QRange<int> range_source{ 1, 10 };
    QRange<int> range{ -5, 5 };
    
    auto unification = range_source + range;
    auto intersection = range_source - range;
    auto difference = range_source * range;
    auto complement = range_source / range;
    
    ASSERT_EQ(unification.first.start(), -5);
    ASSERT_EQ(unification.first.end(), 10);
    ASSERT_EQ(unification.first.direction(), QRangeDirection::Plus);
    
    //////////////////////////////////////////////////
    
    ASSERT_EQ(intersection.first.start(), 5);
    ASSERT_EQ(intersection.first.end(), 10);
    ASSERT_EQ(intersection.first.direction(), QRangeDirection::Plus);
    
    //////////////////////////////////////////////////
    
    ASSERT_EQ(difference.start(), 1);
    ASSERT_EQ(difference.end(), 5);
    ASSERT_EQ(difference.direction(), QRangeDirection::Plus);
    
    //////////////////////////////////////////////////
    
    ASSERT_EQ(complement.first.start(), -5);
    ASSERT_EQ(complement.first.end(), 1);
    ASSERT_EQ(complement.first.direction(), QRangeDirection::Plus);
    
    ASSERT_EQ(complement.second.start(), 5);
    ASSERT_EQ(complement.second.end(), 10);
    ASSERT_EQ(complement.second.direction(), QRangeDirection::Plus);
}

TEST(TestQRangeOperators, Сomparison)
{
    QRange<int> range_1{ 1, 10 };
    QRange<int> range_2{ 1, 5 };
    
    ASSERT_EQ(range_1 == range_1, true);
    ASSERT_EQ(range_1 != range_2, true);
    ASSERT_EQ(range_1 > range_2, true);
    ASSERT_EQ(range_1 < range_2, false);
    
    ASSERT_EQ(range_1 >= range_2, true);
    ASSERT_EQ(range_1 <= range_2, false);
}
