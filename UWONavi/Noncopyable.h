#pragma once

/**
 * @class Noncopyable
 * @brief A utility class to prevent copying and moving of derived classes.
 *
 * This class disables copy and move semantics to ensure that objects of
 * derived classes cannot be copied or moved. This is useful for classes
 * that manage unique resources such as file handles, sockets, or device contexts.
 */
class Noncopyable {
public:
    /**
     * @brief Default constructor.
     * Allows default construction of the class or its derived classes.
     */
    Noncopyable() = default;

    /**
     * @brief Deleted copy constructor.
     * Prevents copying of the class or its derived classes.
     */
    Noncopyable(const Noncopyable&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     * Prevents assignment from another instance of the class or its derived classes.
     */
    Noncopyable& operator=(const Noncopyable&) = delete;

    /**
     * @brief Deleted move constructor.
     * Prevents moving of the class or its derived classes.
     */
    Noncopyable(const Noncopyable&&) = delete;

    /**
     * @brief Deleted move assignment operator.
     * Prevents move assignment of the class or its derived classes.
     */
    Noncopyable& operator=(const Noncopyable&&) = delete;
};
