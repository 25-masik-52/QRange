#pragma once

#include <QtCore>

/*!
 * \enum QRangeDirection
 * \brief Enum defining the direction of the range.
 */
enum class QRangeDirection
{
    Minus = -1, /*!< (-1) The beginning of the range is greater than its end. */
    Zero = 0, /*!< (0) The beginning and end of the range are equal. */
    Plus = 1 /*!< (1) The beginning of the range is less than its end. */
};

/*!
 * \enum QRangeSensitivity
 * \brief Enum that determines the sensitivity of certain methods, such as contains().
 */
enum class QRangeSensitivity
{
    Sensitive, /*!< If QRangeSensitivity::Sensitive is set, the method will work with the operators '>' and '<'. */
    Insensitive /*!< If QRangeSensitivity::Insensitive is set, the method will work with '>=' and '<='. */
};

/*!
 * \class QRange
 *
 * \author 25-masik-52
 * \copyright MIT License
 * \version 1.0.0
 * \date 04.11.2024
 */
template <typename T>
class QRange
{
public:
    /*!
     * \brief Constructs a null range. Null ranges are also considered zero.
     * \see QRange::isZero().
     */
    QRange() = default;
    ~QRange() = default;

    /*!
     * \brief QRange(T start, T end) - the constructor with 2 parameters.
     * \param start - the beginning of the range.
     * \param end - the end of the range.
     */
    explicit QRange(T start, T end) : m_start { start }, m_end { end }
    {
        static_assert(is_normal_v, "[QRange::QRange] Typename T is partially or completely non-numeric!");

        if (end - start > 0) m_direction = QRangeDirection::Plus;
        else if (end - start < 0) m_direction = QRangeDirection::Minus;
        else m_direction = QRangeDirection::Zero;
    }

    /*!
     * \brief QRange(std::pair<T, T> start_end) - A constructor that
     * creates a range based on a pair of set values, where the first element of the pair
     * is the beginning of the range, and the second is the end.
     */
    explicit QRange(std::pair<T, T> start_end)
        : QRange{ start_end.first, start_end.second } {}

    /*!
     * \brief QRange(QString start, QString end, uint8_t first_base, uint8_t second_base = 0) - A constructor that works with
     * different calculus systems where:
     * \param start - QString value in a certain calculus systems of the beginning of the range.
     * \param end - QString value in a certain calculus systems of the end of the range.
     * \param first_base - the calculus systems base for the beginning of the range.
     * \param second_base - the calculus systems base for the end of the range.
     * \warning QString format must be only a number in a certain calculus systems.
     * Some kind of "0xFFF" or "#FFF" in the hexadecimal calculus systems will not work
     * and you will get zero range (QRange{0, 0}).
     * \see QRange::toAnotherBase(uint8_t base).
     */
    explicit QRange(QString start, QString end, uint8_t first_base, uint8_t second_base = 0)
        : QRange{ normalizeValue(start, first_base), normalizeValue(end, second_base < 2 ? first_base : second_base) } {}

    /*!
     * \brief QRange - A constructor that takes the beginning of the range in a defined calculus systems,
     * the calculus system itself and offset from the beginning.
     * \param start - QString value in a certain calculus systems of the beginning of the range.
     * \param offset - offset from the beginning of the range.
     * \param base - the calculus system base for the beginning of the range.
     * \see QRange::QRange(QString start, QString end, uint8_t first_base, uint8_t second_base = 0).
     */
    explicit QRange(QString start, T offset, uint8_t base)
        : QRange{ normalizeValue(start, base), normalizeValue(start, base) + offset } {}

    /*!
     * \brief Gets the start of the range.
     * \return The beginning of the range.
     */
    T start() const { return m_start; }
    /*!
     * \brief Gets the end of the range.
     * \return The end of the range.
     */
    T end() const { return m_end; }
    /*!
     * \brief Gets the lower bound of the range.
     * \return The minimum value between the start and end of the range.
     */
    T lower() const { return std::min(m_start, m_end); }
    /*!
     * \brief Gets the upper bound of the range.
     * \return The maximum value between the start and end of the range.
     */
    T upper() const { return std::max(m_start, m_end); }
    /*!
     * \brief Gets the midpoint of the range.
     * \return The center of the range.
     */
    T middle() const { return static_cast<T>((lower() + upper()) / 2.0); }
    /*!
     * \brief Gets the exact center of the range, regardless of its generic type.
     * \return The precise center of the range.
     */
    double middleAccurate() const { return (lower() + upper()) / 2.0; }
    /*!
     * \brief Gets the length of the range.
     * \return The difference between the beginning and the end of the range.
     */
    T length() const { return std::abs(m_end - m_start); }
    /*!
     * \brief Gets the direction of the range.
     * \return The direction of the range.
     * \see enum QRangeDirection.
     */
    QRangeDirection direction() const { return m_direction; }

    /*!
     * \brief Gets the range in another base.
     * \param base The base for the calculus system of the beginning and end of the range.
     * \return A pair of values in the specified calculus system, where the first value is the beginning
     * and the second is the end of the range.
     */
    std::pair<QString, QString> toAnotherBase(uint8_t base) const
    {
        return { QString::number(m_start, base), QString::number(m_end, base) };
    }

    /*!
     * \brief Sets the start of the range.
     * \param start The beginning of the range.
     * \note This method may change the direction of the range.
     * \see enum QRangeDirection.
     */
    void setStart(const T start) { *this = QRange{ start, m_end }; }
    /*!
     * \brief Sets the end of the range.
     * \param end The end of the range.
     * \note This method may change the direction of the range.
     * \see enum QRangeDirection.
     */
    void setEnd(const T end) { *this = QRange{ m_start, end }; }

    /*!
     * \brief Sets the direction of the range by swapping the start and end.
     * \param direction The QRangeDirection parameter.
     * \warning Direction cannot be changed in a zero-length range.
     * \see enum QRangeDirection.
     */
    void setDirection(const QRangeDirection& direction)
    {
        if (m_direction == QRangeDirection::Zero) {
            qWarning() << "[QRange::setDirection] There is no way to change direction in zero length range!";
            return;
        }

        if (static_cast<int>(direction) > 1 || static_cast<int>(direction) < -1) {
            qWarning() << "[QRange::setDirection] Invalid direction!";
            return;
        }

        if (direction == 0 && m_start != m_end) {
            qWarning() << "[QRange::setDirection] There is no way to set direction to Zero in non-zero length range!";
            return;
        }

        if (m_direction != direction)
            std::swap(m_start, m_end);

        m_direction = direction;
    }

    /*!
     * \see QRange::setDirection(const QRangeDirection& direction).
     * \see enum QRangeDirection.
     */
    void setDirection(int direction)
    {
        switch (direction)
        {
        case -1:
            setDirection(QRangeDirection::Minus);
            break;
        case 0:
            setDirection(QRangeDirection::Zero);
            break;
        case 1:
            setDirection(QRangeDirection::Plus);
            break;
        default:
            qWarning() << "[QRange::setDirection] Invalid direction!";
            break;
        }
    }

    /*!
     * \brief This method changes the values of the start and end of the range relative to the x-axis.
     * \param direction The direction to reverse the range.
     * \code{.cpp}
     * QRange<int> r{-1, 10};
     * r.reverse(Zero);  // r == QRange<int>{1, -10}
     * r.reverse(Minus);  // r == QRange<int>{-1, -10}
     * r.reverse(Plus);  // r == QRange<int>{1, 10}
     * \endcode
     * \see enum QRangeDirection.
     */
    void reverse(const QRangeDirection& direction = QRangeDirection::Zero)
    {
        if (direction == QRangeDirection::Zero)
            *this = QRange{ m_start * -1, m_end * -1 };

        if (direction == QRangeDirection::Minus) {
            auto start = m_start > 0 ? m_start * -1 : m_start;
            auto end = m_end > 0 ? m_end * -1 : m_end;
            *this = QRange{ start, end };
        }

        if (direction == QRangeDirection::Plus) {
            auto start = m_start < 0 ? m_start * -1 : m_start;
            auto end = m_end < 0 ? m_end * -1 : m_end;
            *this = QRange{ start, end };
        }
    }

    /*!
     * \see QRange::reverse(const QRangeDirection& direction = QRangeDirection::Zero).
     * \see enum QRangeDirection.
     */
    void reverse(int direction)
    {
        switch (direction)
        {
        case -1:
            reverse(QRangeDirection::Minus);
            break;
        case 0:
            reverse(QRangeDirection::Zero);
            break;
        case 1:
            reverse(QRangeDirection::Plus);
            break;
        default:
            qWarning() << "[QRange::reverse] Invalid direction!";
            break;
        }
    }

    /*!
     * \brief isPositive property. Checks the beginning and end of the range relative to the x-axis.
     * \param sensitivity The sensitivity level for the check.
     * \return True if both start and end belong to the set of positive numbers; otherwise, returns false.
     * \see enum QRangeSensitivity.
     */
    bool isPositive(QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return m_start > 0 && m_end > 0;
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return m_start >= 0 && m_end >= 0;
        qFatal("[QRange::isPositive] An unexpected error!");
    }

    /*!
     * \brief isNegative property. Checks the beginning and end of the range relative to the x-axis.
     * \param sensitivity The sensitivity level for the check.
     * \return True if both start and end belong to the set of negative numbers; otherwise, returns false.
     * \see enum QRangeSensitivity.
     */
    bool isNegative(QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return m_start < 0 && m_end < 0;
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return m_start <= 0 && m_end <= 0;
        qFatal("[QRange::isNegative] An unexpected error!");
    }

    /*!
     * \brief isMixed property. Checks the beginning and end of the range relative to the x-axis.
     * \return True if start and end belong to different sets of positive and negative numbers; otherwise, returns false.
     */
    bool isMixed() const { return m_end * m_start < 0; }
    /*!
     * \brief isZero property.
     * \return True if both start and end equal 0; otherwise, returns false.
     */
    bool isZero() const { return m_start == 0 && m_end == 0; }

    /*!
     * \brief This method checks whether a number is contained within the range.
     * \param number The number to check.
     * \param sensitivity Sensitivity level for the check.
     * \return True if the number is within the range; otherwise, returns false.
     * \see enum QRangeSensitivity.
     */
    bool contains(T number, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return lower() < number && number < upper();
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return lower() <= number && number <= upper();
        qFatal("[QRange::contains] An unexpected error!");
    }

    /*!
     * \brief This method checks whether an external range is contained within the source range.
     * \param range The external range to check.
     * \param sensitivity Sensitivity level for the check.
     * \return True if the external range is within the source range; otherwise, returns false.
     * \see enum QRangeSensitivity.
     */
    bool contains(const QRange& range, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return lower() < range.lower() && range.upper() < upper();
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return lower() <= range.lower() && range.upper() <= upper();
        qFatal("[QRange::contains] An unexpected error!");
    }

    /*!
     * \brief This method checks whether the source range is contained within the external range.
     * \param range The external range to check against.
     * \param sensitivity Sensitivity level for the check.
     * \return True if the source range is within the external range; otherwise, returns false.
     * \see enum QRangeSensitivity.
     */
    bool in(const QRange& range, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return range.lower() < lower() && upper() < range.upper();
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return range.lower() <= lower() && upper() <= range.upper();
        qFatal("[QRange::in] An unexpected error!");
    }

    /*!
     * \brief This method checks whether one range overlaps with another.
     * \param range The range to check for overlap.
     * \param sensitivity Sensitivity level for the check.
     * \return True if part of the source range overlaps with the external range; otherwise, returns false.
     * \see enum QRangeSensitivity.
     */
    bool overlays(const QRange& range, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        return contains(range.m_start, sensitivity) || contains(range.m_end, sensitivity) || range.contains(m_start, sensitivity) || range.contains(m_end, sensitivity);
    }

    /*!
     * \brief This method finds the minimum length range from a set of ranges.
     * \param ranges
     * A collection of QRanges in formats described by ArgsType.
     * \return The minimum length range.
     * \see ArgsType.
     */
    template <typename... Args>
    static QRange min(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.length() < r2.length();
        });
    }

    /*!
     * \brief This method finds the maximum length range from a set of ranges.
     * \param ranges
     * A collection of QRanges in formats described by ArgsType.
     * \return The maximum length range.
     * \see ArgsType.
     */
    template <typename... Args>
    static QRange max(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.length() > r2.length();
        });
    }

    /*!
     * \brief This method finds the minimum range on the x-axis from a set of ranges.
     * \param ranges
     * A collection of QRanges in formats described by ArgsType.
     * \return The minimum range on the x-axis.
     * \see ArgsType.
     */
    template <typename... Args>
    static QRange minX(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.lower() < r2.lower();
        });
    }

    /*!
     * \brief This method finds the maximum range on the x-axis from a set of ranges.
     * \param ranges
     * A collection of QRanges in formats described by ArgsType.
     * \return The maximum range on the x-axis.
     * \see ArgsType.
     */
    template <typename... Args>
    static QRange maxX(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.upper() > r2.upper();
        });
    }

    /*!
     * \brief This method finds the range that is closest to minus infinity on the x-axis.
     * \param ranges
     * A collection of QRanges in formats described by ArgsType.
     * \return The range that is closest to minus infinity on the x-axis.
     * \code{.cpp}
     * QRange<int> r{0, 10};
     * QRange<int> r1{1, 3};
     * auto res = QRange::minMidX(r, r1);  // res == r1
     * \endcode
     * \see ArgsType.
     */
    template <typename... Args>
    static QRange minMidX(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.middleAccurate() < r2.middleAccurate();
        });
    }

    /*!
     * \brief This method finds the range that is closest to plus infinity on the x-axis.
     * \param ranges
     * A collection of QRanges in formats described by ArgsType.
     * \return The range that is closest to plus infinity on the x-axis.
     * \code{.cpp}
     * QRange<int> r{1, 11};
     * QRange<int> r1{8, 10};
     * auto res = QRange::maxMidX(r, r1);  // res == r1
     * \endcode
     * \see ArgsType.
     */
    template <typename... Args>
    static QRange maxMidX(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.middleAccurate() > r2.middleAccurate();
        });
    }

    /*!
     * \brief Method converts all ranges in the list to a number list.
     * \param ranges
     * QList of ranges.
     * \return Number list.
     */
    static QList<T> brake(const QList<QRange>& ranges)
    {
        QList<T> result;
        for (const auto& range : ranges)
        {
            result.append(range.m_start);
            result.append(range.m_end);
        }
        return result;
    }

    /*!
     * \brief Method converts a list of numbers to the list of ranges. If
     * there is an even number of elements, it will take each 2 numbers and create QRanges, where the first
     * number will be the start of the range and the second will be the end. If there is
     * an odd number of elements, the last QRange will start and end with the value of the last element.
     * \param numbers
     * \return QList of ranges.
     */
    static QList<QRange> merge(const QList<T>& numbers)
    {
        QList<QRange> result;
        auto editableNumbers = numbers;
        while (!editableNumbers.isEmpty())
        {
            auto first = editableNumbers.takeFirst();
            if (editableNumbers.isEmpty()) {
                result.append(QRange{ first, first });
                continue;
            }

            auto second = editableNumbers.takeFirst();
            result.append(QRange{ first, second });
        }
        return result;
    }

    /*!
     * \brief Method cuts the range into equal parts, the number of which is determined by \param number.
     * \param range
     * The QRange that should be cut.
     * \return Calculated QList of ranges.
     * \warning If number is greater than the range length, the method may behave unexpectedly.
     */
    static QList<QRange> cut(const QRange& range, int number)
    {
        QList<QRange> result;

        if (number == 0)
            return { range };

        if (number < 0)
            qFatal("[QRange::cut] Incorrect number!");

        if (number > range.length())
            qWarning() << "[QRange::cut] number > range.length(), so method can work strange";

        T step = range.length() / number;
        step = range.direction() == QRangeDirection::Minus ? step * -1 : step;
        T currentStart = range.start();
        for (int i = 0; i < number; ++i)
        {
            T currentEnd = (i == number - 1) ? range.end() : currentStart + step;
            result.emplace_back(QRange{ currentStart, currentEnd });
            currentStart = currentEnd;
        }

        return result;
    }

    /*!
     * \brief Method connects ranges from a QList to a single range.
     * \param ranges
     * \param sensitivity. If QRangeSensitivity::Insensitive is set, the resulting
     * range will have the same direction as most ranges with the same direction.
     * \warning If QRangeSensitivity::Sensitive is set, ensure that the ranges do not
     * have gaps between them and that all of them have the same direction!
     * \return Calculated range.
     * \code{.cpp}
     * QRange<int> r{1, 10};
     * QRange<int> r1{-7, 0};
     * QRange<int> r2{10, 1};
     * QRange<int> r3{10, 15};
     * auto res = QRange<int>::combine({r, r1}, Sensitive);  // gap error
     * auto res1 = QRange<int>::combine({r, r2}, Sensitive);  // direction error
     * auto res2 = QRange<int>::combine({r, r3}, Sensitive);  // res2 == QRange{1, 15}
     * auto res3 = QRange<int>::combine({r, r1, r2, r3}, Insensitive);  // res3 == QRange{-7, 15}
     * \endcode
     * \see enum QRangeSensitivity
     */
    static QRange combine(const QList<QRange>& ranges, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive)
    {
        auto sortedRanges = ranges;
        QRange result;

        if (sensitivity == QRangeSensitivity::Sensitive) {
            std::sort(sortedRanges.begin(), sortedRanges.end());
            result = sortedRanges.first();

            for (const auto& range : sortedRanges) {
                if (result == range)
                    continue;

                if (result.m_direction != range.m_direction
                    && result.m_direction != QRangeDirection::Zero
                    && range.m_direction != QRangeDirection::Zero)
                    qFatal("[QRange::combine] Ranges have different directions!");

                if (!result.contains(range.m_start, QRangeSensitivity::Insensitive))
                    qFatal("[QRange::combine] There is a gap between ranges!");

                result.setEnd(sortedRanges[sortedRanges.count() - 1].m_end);
            }
        } else if (sensitivity == QRangeSensitivity::Insensitive) {
            result = sortedRanges.first();

            const auto numbers = QRange::brake(sortedRanges);
            const auto [min, max] = std::minmax_element(numbers.begin(), numbers.end());
            result = QRange{ *min, *max };

            if (result.m_direction != QRangeDirection::Zero) {
                int countDirectionNegative = 0;
                int countDirectionPositive = 0;

                for (const QRange& range : sortedRanges) {
                    if (range.m_direction == QRangeDirection::Minus)
                        countDirectionNegative++;
                    else if (range.m_direction == QRangeDirection::Plus)
                        countDirectionPositive++;
                }

                if (countDirectionNegative > countDirectionPositive)
                    result.setDirection(QRangeDirection::Minus);
                else
                    result.setDirection(QRangeDirection::Plus);
            }
        }

        return result;
    }

    /*!
     * \brief Operators +, -, *, /, % perform the appropriate mathematical operations with
     * the start and the end of the range.
     * \param number
     * \return New QRange with calculated values of the start and end of the range.
     */
    QRange operator+(T number) const { return QRange{ m_start + number, m_end + number }; }
    QRange operator-(T number) const { return QRange{ m_start - number, m_end - number }; }
    QRange operator*(T number) const { return QRange{ m_start * number, m_end * number }; }
    QRange operator/(T number) const { return QRange{ m_start / number, m_end / number }; }
    QRange operator%(T number) const { return QRange{ m_start % number, m_end % number }; }

    /*!
     * \brief Operator + works the same way as the set union operator.
     * \param range
     * \return Calculated ranges.
     * \see Fundamentals of mathematical analysis. (http://www.mathprofi.ru/mnozhestva.html)
     */
    std::pair<QRange, QRange> operator+(const QRange& range) const
    {
        if (!overlays(range, QRangeSensitivity::Insensitive))
            return { *this, range };
        if (contains(range))
            return { *this, *this };
        if (range.contains(*this))
            return { range, range };

        auto min = std::min({ m_start, m_end, range.m_start, range.m_end });
        auto max = std::max({ m_start, m_end, range.m_start, range.m_end });
        auto res = QRange{ min, max };
        const auto direction = length() >= range.length()? m_direction : range.m_direction;
        res.setDirection(direction);

        return { res, res };
    }

    /*!
     * \brief Operator - works the same way as the set difference operator.
     * \param range
     * \return Calculated ranges.
     * \see Fundamentals of mathematical analysis. (http://www.mathprofi.ru/mnozhestva.html)
     */
    std::pair<QRange, QRange> operator-(const QRange& range) const
    {
        if (!overlays(range, QRangeSensitivity::Insensitive) || range.isZero())
            return { *this, *this };

        if (in(range)) {
            qWarning() << "[QRange::operator-] The subtracted is greater than the reduced";
            return range - *this;
        }

        if (contains(range)) {
            auto resFirst = QRange{ lower(), range.lower() };
            auto resSecond = QRange{ range.upper(), upper() };
            resFirst.setDirection(m_direction);
            resSecond.setDirection(m_direction);
            return { resFirst, resSecond };
        }

        if (lower() == range.lower() && upper() == range.upper()) {
            auto res = QRange{};
            return { res, res };
        }

        const auto direction = length() >= range.length()? m_direction : range.m_direction;
        if (middleAccurate() > range.middleAccurate()) {
            auto res = QRange{ range.upper(), upper() };
            res.setDirection(direction);
            return { res, res };
        } else if (middleAccurate() < range.middleAccurate()) {
            auto res = QRange{ lower(), range.lower() };
            res.setDirection(direction);
            return { res, res };
        }

        qFatal("[QRange::operator-] An unexpected error!");
    }

    /*!
     * \brief Operator * works the same way as the set intersection operator.
     * \param range
     * \return Calculated ranges.
     * \see Fundamentals of mathematical analysis. (http://www.mathprofi.ru/mnozhestva.html)
     */
    QRange operator*(const QRange& range) const
    {
        if (isZero() || range.isZero() || !overlays(range, QRangeSensitivity::Insensitive))
            qFatal("[QRange::operator*] The result is ∅!");

        if (contains(range, QRangeSensitivity::Insensitive))
            return range;
        if (range.contains(*this, QRangeSensitivity::Insensitive))
            return *this;
        if (overlays(range, QRangeSensitivity::Insensitive)) {
            auto list = QList<T>{ m_start, m_end, range.m_start, range.m_end };
            auto [min, max] = std::minmax_element(list.begin(), list.end());
            list.removeOne(*min);
            list.removeOne(*max);
            auto res = QRange{ list.front(), list.back() };
            if (res.length() != 0) {
                auto direction = m_direction * range.m_direction;
                res.setDirection(direction);
            }

            return res;
        }
        qFatal("[QRange::operator*] An unexpected error!");
    }

    /*!
     * \brief Operator / works the same way as the set complement operator.
     * \param range
     * \return Calculated ranges.
     * \see Fundamentals of mathematical analysis. (http://www.mathprofi.ru/mnozhestva.html)
     */
    std::pair<QRange, QRange> operator/(const QRange& range) const
    {
        if (!overlays(range, QRangeSensitivity::Insensitive)
            || range.isZero()
            || in(range)
            || contains(range)
            || (lower() == range.lower() && upper() == range.upper()))
            return *this - range;

        const auto direction = m_direction * range.m_direction;;
        if (middleAccurate() > range.middleAccurate()) {
            auto resFirst = QRange{ range.lower(), lower() };
            auto resSecond = QRange{ range.upper(), upper() };
            resFirst.setDirection(direction);
            resSecond.setDirection(direction);
            return { resFirst, resSecond };
        } else if (middleAccurate() < range.middleAccurate()) {
            auto resFirst = QRange{ lower(), range.lower() };
            auto resSecond = QRange{ upper(), range.upper() };
            resFirst.setDirection(direction);
            resSecond.setDirection(direction);
            return { resFirst, resSecond };
        }

        qFatal("[QRange::operator/] An unexpected error!");
    }

    /*!
     * \brief Operators >, <.
     * \param range
     * \return True if the ranges being compared are closer or further to plus infinity on the
     * x-axis, respectively; otherwise, return false.
     */
    bool operator>(const QRange& range) const { return maxMidX(range, *this) == *this; }
    bool operator<(const QRange& range) const { return minMidX(range, *this) == *this; }

    /*!
     * \brief Operators ==, !=.
     * \param range
     * \return True if the ranges, taking into account the direction, are equal or unequal, respectively;
     * otherwise, return false.
     */
    bool operator==(const QRange& range) const
    {
        return m_start == range.m_start && m_end == range.m_end && m_direction == range.m_direction;
    }
    bool operator!=(const QRange& range) const
    {
        return m_start != range.m_start || m_end != range.m_end || m_direction != range.m_direction;
    }

    /*!
     * \brief Operators >=, <=.
     * \param range
     * \return True for the operator >= if true for the operators > and ==; otherwise, return false.
     * \return True for the operator <= if true for the operators < and ==; otherwise, return false.
     */
    bool operator>=(const QRange& range) const { return *this > range || *this == range; }
    bool operator<=(const QRange& range) const { return *this < range || *this == range; }

    friend std::ostream& operator<<(std::ostream &os, const QRange& range)
    {
        return os << range.getRangeInfo().toStdString();
    }

    friend QDebug operator<<(QDebug dbg, const QRange& range)
    {
        return dbg << range.getRangeInfo();
    }

protected:
    T m_start{ 0 };
    T m_end{ 0 };
    QRangeDirection m_direction{ Zero };

    /*!
     * \brief The library works with certain types of numeric data described below:
     * - int
     * - long
     * - long long
     * - unsigned int
     * - unsigned long
     * - unsigned long long
     * - float
     * - double
     * - long double
     *
     * \note When working with floating-point numbers, you will not be able to use
     * methods that work with different number systems. This is due to the fact
     * that in standard Qt and C++ there is no simple conversion of floating-point
     * numbers in a number system other than decimal to decimal.
     */
    static const bool is_normal_v = std::is_same<T, int>::value
                                    || std::is_same<T, long>::value
                                    || std::is_same<T, long long>::value
                                    || std::is_same<T, unsigned int>::value
                                    || std::is_same<T, unsigned long>::value
                                    || std::is_same<T, unsigned long long>::value
                                    || std::is_same<T, float>::value
                                    || std::is_same<T, double>::value
                                    || std::is_same<T, long double>::value;

    /*!
     * \brief QRange also has several functions with a variable number of parameters.
     * These functions use the alias ArgsType.
     * This allows you to pass in functions (such as QRange::min()) both single
     * objects and arrays.
     * That is, you can simultaneously pass QRange and QList<QRange> to the
     * QRange::min() function and it will calculate the result correctly.
     */
    using ArgsType = std::variant<QRange,
                                  std::pair<QRange, QRange>,
                                  QList<QRange>,
                                  std::list<QRange>,
                                  std::vector<QRange>>;

private:
    T normalizeValue(const QString& value, uint8_t base)
    {
        bool ok = false;

        if constexpr (std::is_integral<T>::value)
        {
            if (std::is_same_v<T, int>)
                return value.toInt(&ok, base);
            else if (std::is_same_v<T, long>)
                return value.toLong(&ok, base);
            else if (std::is_same_v<T, long long>)
                return value.toLongLong(&ok, base);
            else if (std::is_same_v<T, unsigned int>)
                return value.toUInt(&ok, base);
            else if (std::is_same_v<T, unsigned long>)
                return value.toULong(&ok, base);
            else if (std::is_same_v<T, unsigned long long>)
                return value.toULongLong(&ok, base);
            else {
                qWarning() << "[QRange::normalizeValue] Unsupported integer type.";
                return 0;
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if (std::is_same_v<T, float>)
                return value.toFloat(&ok);
            else if (std::is_same_v<T, double>)
                return value.toDouble(&ok);
            else if (std::is_same_v<T, long double>)
                return static_cast<long double>(value.toDouble(&ok));
            else {
                qWarning() << "[QRange::normalizeValue] Unsupported floating-point type.";
                return 0.0;
            }
        }

        if (!ok) {
            qCritical() << QString{ "[QRange::normalizeValue] Failed to convert string: %1!" }.arg(value);
            return 0;
        }
    }

    template <typename... Args>
    static QList<QRange> unpackRanges(const Args&... args)
    {
        auto ranges = QList<QRange>{};
        std::initializer_list<ArgsType> initArgs{ args... };
        for (const auto& arg : initArgs)
            unpackRange(ranges, arg);
        return ranges;
    }

    static void unpackRange(QList<QRange>& allRanges, const ArgsType& arg)
    {
        std::visit([&](const auto& v) {
            using VT = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<VT, QRange>)
                allRanges.append(v);
            else if constexpr (std::is_same_v<VT, std::pair<QRange, QRange>>)
                allRanges.append({ v.first, v.second });
            else if constexpr (std::is_same_v<VT, QList<QRange>>
                               || std::is_same_v<VT, std::list<QRange>>
                               || std::is_same_v<VT, std::vector<QRange>>)
                allRanges.append(QList<QRange>{ v.begin(), v.end() });
            else
                qWarning() << "[QRange::unpackRanges] Unexpected variant type";
        }, arg);
    }

    template <typename Comparator>
    static QRange compareRanges(const QList<QRange>& ranges, Comparator cmp)
    {
        if (ranges.empty()) {
            qWarning() << "[QRange::compareRanges] There is nothing to compare";
            return QRange{};
        }

        auto result = ranges.first();

        for (const auto& currentRange : ranges)
            if (cmp(currentRange, result))
                result = currentRange;

        return result;
    }

    QString getRangeInfo() const
    {
        QString direction{"error"};
        if (m_direction == QRangeDirection::Minus)
            direction = "<--";
        else if (m_direction == QRangeDirection::Zero)
            direction = "--><--";
        else if (m_direction == QRangeDirection::Plus)
            direction = "-->";
        QString property{"error"};
        if (isPositive(QRangeSensitivity::Insensitive))
            property = "+";
        else if (isNegative(QRangeSensitivity::Insensitive))
            property = "-";
        else if (isMixed())
            property = m_start < 0 ? "-+" : "+-";
        else if (isZero())
            property = "0";
        return QString{ "[%1, %2]: {length: %3, direction: %4, property: %5}" }
            .arg(start())
            .arg(end())
            .arg(length())
            .arg(direction)
            .arg(property);
    }
};
