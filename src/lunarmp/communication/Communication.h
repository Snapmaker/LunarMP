// Copyright (c) 2018 Ultimaker B.V.
// LunarTPP is released under the terms of the AGPLv3 or higher.

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

namespace lunarmp {
// Some forward declarations to increase compilation speed.
struct LayerIndex;
struct Velocity;
enum class PrintFeatureType : unsigned char;

/*
 * An abstract class to provide a common interface for all methods of
 * communicating instructions from and to LunarTPP.
 */
class Communication {
  public:
    /*
     * \brief Close the communication channel.
     */
    virtual ~Communication() {}

    /*
     * \brief Test if there are more slices to be queued.
     */
    virtual bool hasSlice() const = 0;

    /*
     * \brief Whether the output needs to be sent from start to finish or not.
     *
     * This determines if the g-code output needs to be output from start to
     * finish in order.
     * This matters because the start g-code contains information on the
     * statistics of the print. These statistics can only be generated at the
     * end of the slice. Preferably we'd send the start g-code last, so that the
     * statistics in the start g-code can be more accurate.
     */
    virtual bool isSequential() const = 0;

    /*
     * \brief Indicate that we're beginning to send g-code.
     */
    virtual void beginGCode() = 0;

    /*
     * \brief Flush all remaining g-code to the user.
     */
    virtual void flushGCode() = 0;

    /*
     * \brief Get the next slice command from the communication and cause it to
     * slice.
     */
    virtual void sliceNext() = 0;
};

}  // namespace lunarmp

#endif  // COMMUNICATION_H
