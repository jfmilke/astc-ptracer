#ifndef JAY_ANALYSIS_PERFORMANCE_HPP
#define JAY_ANALYSIS_PERFORMANCE_HPP
#include <jay/export.hpp>
#include <chrono>
#include <vector>
#include <string>


namespace jay
{
  struct JAY_EXPORT gpu_timestamp
  {
    std::string description;
    double      time_ms;
    int         query_id;
    int         pass;
  };

  struct JAY_EXPORT gpu_query
  {
    bool in_use;
    int  id;
  };

  struct JAY_EXPORT performance
  {
    performance() = default;

    int    start_measure_CPU_time ();
    double finish_measure_CPU_time(int id);

    void initialize_gl_queries   (unsigned int count);
    void start_measure_GPU_time  (int id);
    std::int64_t finish_measure_GPU_time (int id);

    gpu_query get_free_query();
    // Issues an OpenGL Timestamp Query
    void issue_GPU_timestamp(std::string description, int pass = -1);
    // Returns GPU timing of the given description. Matching with other timestamps done via description equality.
    static gpu_timestamp get_GPU_timing(std::string description);
    // Returns vector holding all GPU timings. Matching with other timestamps done via description equality.
    static std::vector<gpu_timestamp> get_GPU_timings();
    // Clears all GPU timings
    static void clear_GPU_timestamps();

    static bool timings_available();


  private:
    static void wait_for_GPU(int query_id);

    static std::vector<gpu_timestamp> gpu_timers;
    static std::vector<gpu_query> gpu_queries;

    static std::vector<std::uint32_t> gl_query_ids_;
    static std::vector<std::chrono::steady_clock::time_point> cpu_times_t1_;
    static std::vector<std::chrono::steady_clock::time_point> cpu_times_t2_;
  };
}

#endif