#include "stdafx.h"
#include "ShipRoute.h"
#include "Vector.h"
#include <iostream>

namespace {
    // Structure to define the chunk header for route data serialization.
    // Contains the version number and line count.
    struct ChunkHeader {
        enum : uint32_t {
            k_Version1 = 1,  // Version 1 of the route data
        };
        const uint32_t version = k_Version1;  // Version of the data chunk
        uint32_t lineCount = 0;  // Number of lines in the route
    };

    const float k_worldLoopThreshold = 0.5f;  // Threshold to handle world wrapping (when crossing the world boundary)

    // Converts a normalized point to a denormalized point with actual coordinates
    inline POINT s_denormalizedPoint(const NormalizedPoint& point)
    {
        POINT p = {
            static_cast<long>(::round(point.x() * k_worldWidth)),
            static_cast<long>(::round(point.y() * k_worldHeight)),
        };
        return p;
    }

    // Calculates the length of a route line (sum of distances between consecutive points)
    inline double s_calcLineLength(const ShipRoute::Line& line)
    {
        double length = 0.0;

        for (auto it = line.begin(); it != line.end(); ++it) {
            auto p1 = *it;
            if (std::next(it) == line.end()) {
                break;
            }
            auto p2 = *std::next(it);
            auto vector = Vector(s_denormalizedPoint(p1), s_denormalizedPoint(p2));
            length += vector.length();  // Add the distance between p1 and p2
        }
        return length;  // Return the total length of the line
    }
}

// Serialize the ship route into an output stream
std::ostream& operator<<(std::ostream& os, ShipRoute& shipRoute)
{
    _ASSERT(os.good());
    if (!os.good()) {
        throw std::runtime_error("output stream error.");
    }

    ChunkHeader header;
    header.lineCount = shipRoute.getLines().size();  // Set the number of lines in the route
    os.write(reinterpret_cast<const char*>(&header), sizeof(header));  // Write the header to the stream

    // Write each line in the route to the stream
    for (const auto& line : shipRoute.getLines()) {
        const size_t count = line.size();
        os.write(reinterpret_cast<const char*>(&count), sizeof(count));  // Write the number of points in the line
        if (!line.empty()) {
            os.write(reinterpret_cast<const char*>(&line[0]), sizeof(line[0]) * line.size());  // Write the points in the line
        }
    }

    _ASSERT(os.good());
    return os;  // Return the output stream
}

// Deserialize the ship route from an input stream
std::istream& operator>>(std::istream& is, ShipRoute& shipRoute)
{
    _ASSERT(is.good());
    if (!is.good()) {
        throw std::runtime_error("input stream error.");
    }

    ChunkHeader header;
    is.read(reinterpret_cast<char*>(&header), sizeof(header));  // Read the header from the stream

    if (header.version != ChunkHeader::k_Version1) {
        throw std::runtime_error("unknown file version.");
    }

    shipRoute.setFavorite(true);
    shipRoute.setFix(true);

    // Read each line in the route from the stream
    for (size_t k = 0; k < header.lineCount; ++k) {
        size_t pointCount = 0;
        is.read(reinterpret_cast<char*>(&pointCount), sizeof(pointCount));  // Read the number of points in the line
        if (0 < pointCount) {
            ShipRoute::Line tmp(pointCount);
            is.read(reinterpret_cast<char*>(&tmp[0]), sizeof(tmp[0]) * tmp.size());  // Read the points in the line
            if (!shipRoute.getLines().empty() && !tmp.empty()) {
                auto p1 = shipRoute.getLines().back().back();
                auto p2 = tmp.front();
                shipRoute.m_length += Vector(s_denormalizedPoint(p1), s_denormalizedPoint(p2)).length();  // Add distance between last point of the last line and first point of this line
            }
            shipRoute.m_length += s_calcLineLength(tmp);  // Add the length of the current line to the total route length
            shipRoute.addLine(std::move(tmp));  // Add the line to the route
        }
    }

    _ASSERT(is.good());
    return is;  // Return the input stream
}

// Add a new route point to the current route
void ShipRoute::addRoutePoint(const NormalizedPoint& point)
{
    _ASSERT(!isFixed());  // Ensure the route is not fixed before adding new points

    // If there are no lines in the route, start a new line
    if (m_lines.empty()) {
        m_lines.push_back(Line());
    }

    Line& line = m_lines.back();  // Get the last line of the route

    // If the line is empty, just add the first point
    if (line.empty()) {
        line.push_back(point);
        return;
    }

    Vector vector(s_denormalizedPoint(line.back()), s_denormalizedPoint(point));  // Create a vector between the last point and the new point
    m_length += vector.length();  // Add the distance to the total length

    const NormalizedPoint& prevPoint = line.back();
    if (prevPoint.isEqualValue(point)) {  // If the point is equal to the previous one, don't add it
        return;
    }

    // Handle world wrapping when the x-coordinate crosses the threshold (i.e., the world loop)
    if (prevPoint.x() < point.x() && (k_worldLoopThreshold <= (point.x() - prevPoint.x()))) {
        // Wrap around the world to the left
        const NormalizedPoint leftSideSubPoint(point.x() - 1.0f, point.y());
        const NormalizedPoint rightSideSubPoint(prevPoint.x() + 1.0f, prevPoint.y());

        line.push_back(leftSideSubPoint);
        m_lines.emplace(m_lines.end(), std::move(Line{ rightSideSubPoint, point }));
    }
    else if (point.x() < prevPoint.x() && (k_worldLoopThreshold <= (prevPoint.x() - point.x()))) {
        // Wrap around the world to the right
        const NormalizedPoint rightSideSubPoint(point.x() + 1.0f, point.y());
        const NormalizedPoint leftSideSubPoint(prevPoint.x() - 1.0f, prevPoint.y());

        line.push_back(rightSideSubPoint);
        m_lines.emplace(m_lines.end(), std::move(Line{ leftSideSubPoint, point }));
    }
    else {
        line.push_back(point);  // Otherwise, just add the point to the line
    }
}

// Join the current route with another route (concatenate them)
void ShipRoute::jointPreviousLinesWithRoute(const ShipRoute& srcRoute)
{
    // If the source route is empty, there's nothing to join
    if (srcRoute.isEmptyRoute()) {
        return;
    }
    // If the current route is empty, just copy the source route's lines
    if (isEmptyRoute()) {
        m_lines = srcRoute.m_lines;
        return;
    }

    // Add the length of the source route to the current route's total length
    m_length += srcRoute.m_length;

    Lines tmp = srcRoute.m_lines;  // Get the lines from the source route

    // Get the last line from the source route and the first line from the current route
    Line& prevLine = tmp.back();
    Line& nextLine = m_lines.front();

    // If both lines are non-empty, calculate the distance between the last point of the source route and the first point of the current route
    if (!prevLine.empty() && !nextLine.empty()) {
        NormalizedPoint prevPoint = prevLine.back();
        NormalizedPoint nextPoint = nextLine.front();

        const double betweenLength = Vector(s_denormalizedPoint(prevPoint), s_denormalizedPoint(nextPoint)).length();
        m_length += betweenLength;  // Add the distance between the two lines to the total length

        // Handle world wrapping between the two lines
        if ((std::max(prevPoint.x(), nextPoint.x()) - std::min(prevPoint.x(), nextPoint.x())) < k_worldLoopThreshold) {
            prevLine.insert(prevLine.end(), nextLine.begin(), nextLine.end());  // Merge the two lines
            m_lines.erase(m_lines.begin());  // Remove the first line from the current route
        }
    }

    tmp.insert(tmp.end(), m_lines.begin(), m_lines.end());  // Add the current route's lines to the temporary lines
    m_lines.swap(tmp);  // Swap the current lines with the temporary lines

    // Set the route's favorite status based on the favorite status of the source route
    setFavorite(isFavorite() | srcRoute.isFavorite());
}
