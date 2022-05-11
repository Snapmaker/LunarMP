// Copyright (c) 2018 Ultimaker B.V.
// LunarTPP is released under the terms of the AGPLv3 or higher.

#ifndef SMPROGRESS_H
#define SMPROGRESS_H

#include <string>
#include <vector>

namespace lunarmp {
class TimeKeeper;

#define SM_N_PROGRESS_STAGES 5

/*!
 * Class for handling the progress bar and the progress logging.
 *
 * The progress bar is based on a single slicing of a rather large model which
 * needs some complex support; the relative timing of each stage is currently
 * based on that of the slicing of dragon_65_tilted_large.stl
 */
class SMProgress {
  public:
    /*!
     * The stage in the whole slicing process
     */
    enum class Stage : unsigned int { START = 0, MODEL = 1, TOOLPATH = 2, EXPORT = 3, FINISH = 4 };

  private:
    static std::vector<double> sm_times;                       //!< Time estimates per stage
    static std::vector<double> sm_accumulated_times;           //!< Time past before each stage
    static std::string sm_names[SM_N_PROGRESS_STAGES];         //!< name of each stage
    static float stage_progress_starts[SM_N_PROGRESS_STAGES];  //!< name of each stage
    static float stage_progress_ends[SM_N_PROGRESS_STAGES];    //!< name of each stage
    static double sm_last_progress;                            //!< An estimate of the total time
    static double sm_total_timing;                             //!< An estimate of the total time
    /*!
     * Give an estimate between 0 and 1 of how far the process is.
     *
     * \param stage The current stage of processing
     * \param stage_process How far we currently are in the \p stage
     * \return An estimate of the overall progress.
     */
    static float calcOverallProgress(Stage stage, float stage_progress);

  public:
    static void init();  //!< Initialize some values needed in a fast computation
                         //!< of the progress
    /*!
     * Message progress over the CommandSocket and to the terminal (if the
     * command line arg '-p' is provided).
     *
     * \param stage The current stage of processing
     * \param progress_in_stage Any number giving the progress within the stage
     * \param progress_in_stage_max The maximal value of \p progress_in_stage
     */
    //  static void messageProgress(Stage stage, int progress_in_stage, int
    //  progress_in_stage_max);

    static void messageProgress(Stage stage, int progress_in_stage, int progress_in_stage_max);
    /*!
     * Message the progress stage over the command socket.
     *
     * \param stage The current stage
     * \param timeKeeper The stapwatch keeping track of the timings for each
     * stage (optional)
     */
    //  static void messageProgressStage(Stage stage, TimeKeeper *timeKeeper,
    //  float time);

    static void messageProgressStage(Stage stage, TimeKeeper* timeKeeper, float stage_progress_start = 0, float stage_progress_end = 0);
};

}  // namespace lunarmp
#endif  // SMPROGRESS_H
