#pragma once

#include <QtCore>

enum QRangeDirection { Minus = -1, Zero = 0, Plus = 1 };
enum QRangeSensitivity { Sensitive, Insensitive };

template <typename T>
class QRange
{
public:
    QRange() = default;
    ~QRange() = default;
    
    explicit QRange(T start, T end) : m_start { start }, m_end { end }
    {
        static_assert(is_normal_v, "[QRange::QRange] Typename T is partially or completely non-numeric!");
        
        if (end - start > 0) m_direction = QRangeDirection::Plus;
        else if (end - start < 0) m_direction = QRangeDirection::Minus;
        else m_direction = QRangeDirection::Zero;
    }
    
    explicit QRange(std::pair<T, T> start_end)
        : QRange{ start_end.first, start_end.second } {}
    
    explicit QRange(QString start, QString end, uint8_t first_base, uint8_t second_base = 0)
        : QRange{ normalizeValue(start, first_base), normalizeValue(end, second_base < 2 ? first_base : second_base) } {}
    
    explicit QRange(QString start, T offset, uint8_t base)
        : QRange{ normalizeValue(start, base), normalizeValue(start, base) + offset } {}
    
    T start() const { return m_start; }
    T end() const { return m_end; }
    T lower() const { return std::min(m_start, m_end); }
    T upper() const { return std::max(m_start, m_end); }
    T middle() const { return static_cast<T>((lower() + upper()) / 2.0); }
    double middleAccurate() const { return (lower() + upper()) / 2.0; }
    T length() const { return std::abs(m_end - m_start); }
    QRangeDirection direction() const { return m_direction; }

    std::pair<QString, QString> toAnotherBase(uint8_t base) const
    {
        return { QString::number(m_start, base), QString::number(m_end, base) };
    }

    void setStart(const T start) { *this = QRange{ start, m_end }; }
    void setEnd(const T end) { *this = QRange{ m_start, end }; }

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

    bool isPositive(QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return m_start > 0 && m_end > 0;
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return m_start >= 0 && m_end >= 0;
        qFatal("[QRange::isPositive] An unexpected error!");
    }

    bool isNegative(QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return m_start < 0 && m_end < 0;
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return m_start <= 0 && m_end <= 0;
        qFatal("[QRange::isNegative] An unexpected error!");
    }

    bool isMixed() const { return m_end * m_start < 0; } 
    bool isZero() const { return m_start == 0 && m_end == 0; }

    bool contains(T number, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return lower() < number && number < upper();
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return lower() <= number && number <= upper();
        qFatal("[QRange::contains] An unexpected error!");
    }

    bool contains(const QRange& range, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return lower() < range.lower() && range.upper() < upper();
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return lower() <= range.lower() && range.upper() <= upper();
        qFatal("[QRange::contains] An unexpected error!");
    }

    bool in(const QRange& range, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        if (sensitivity == QRangeSensitivity::Sensitive)
            return range.lower() < lower() && upper() < range.upper();
        else if (sensitivity == QRangeSensitivity::Insensitive)
            return range.lower() <= lower() && upper() <= range.upper();
        qFatal("[QRange::in] An unexpected error!");
    }

    bool overlays(const QRange& range, QRangeSensitivity sensitivity = QRangeSensitivity::Sensitive) const
    {
        return contains(range.m_start, sensitivity) || contains(range.m_end, sensitivity) || range.contains(m_start, sensitivity) || range.contains(m_end, sensitivity);
    }

    template <typename... Args>
    static QRange min(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.length() < r2.length();
        });
    }
    
    template <typename... Args>
    static QRange max(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.length() > r2.length();
        });
    }
    
    template <typename... Args>
    static QRange minX(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.lower() < r2.lower();
        });
    }
    
    template <typename... Args>
    static QRange maxX(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.upper() > r2.upper();
        });
    }
    
    template <typename... Args>
    static QRange minMidX(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.middleAccurate() < r2.middleAccurate();
        });
    }
    
    template <typename... Args>
    static QRange maxMidX(const Args&... ranges)
    {
        return compareRanges(unpackRanges(ranges...), [](const QRange& r1, const QRange& r2) {
            return r1.middleAccurate() > r2.middleAccurate();
        });
    }
    
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
    
    QRange operator+(T number) const { return QRange{ m_start + number, m_end + number }; }
    QRange operator-(T number) const { return QRange{ m_start - number, m_end - number }; }
    QRange operator*(T number) const { return QRange{ m_start * number, m_end * number }; }
    QRange operator/(T number) const { return QRange{ m_start / number, m_end / number }; }
    QRange operator%(T number) const { return QRange{ m_start % number, m_end % number }; }
    
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
    
    bool operator>(const QRange& range) const { return maxMidX(range, *this) == *this; }
    bool operator<(const QRange& range) const { return minMidX(range, *this) == *this; }
    
    bool operator==(const QRange& range) const
    {
        return m_start == range.m_start && m_end == range.m_end && m_direction == range.m_direction;
    }
    bool operator!=(const QRange& range) const
    {
        return m_start != range.m_start || m_end != range.m_end || m_direction != range.m_direction;
    }
    
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
    
    static const bool is_normal_v = std::is_same<T, int>::value
                       || std::is_same<T, long>::value
                       || std::is_same<T, long long>::value
                       || std::is_same<T, unsigned int>::value
                       || std::is_same<T, unsigned long>::value
                       || std::is_same<T, unsigned long long>::value
                       || std::is_same<T, float>::value
                       || std::is_same<T, double>::value
                       || std::is_same<T, long double>::value;
    
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
