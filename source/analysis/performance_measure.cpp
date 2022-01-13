#include <jay/analysis/performance_measure.hpp>
#include <glbinding/gl/functions.h>
#include <glbinding/gl/enum.h>
#include <ratio>

namespace jay
{
  std::vector<gpu_timestamp> performance::gpu_timers;
  std::vector<gpu_query> performance::gpu_queries;
  std::vector<std::uint32_t> performance::gl_query_ids_;
  std::vector<std::chrono::steady_clock::time_point> performance::cpu_times_t1_;
  std::vector<std::chrono::steady_clock::time_point> performance::cpu_times_t2_;

  int performance::start_measure_CPU_time()
  {
    cpu_times_t1_.push_back(std::chrono::high_resolution_clock::now());
    cpu_times_t2_.resize(cpu_times_t1_.size());

    return cpu_times_t1_.size() - 1;
  }

  double performance::finish_measure_CPU_time(int id)
  {
    cpu_times_t2_[id] = std::chrono::high_resolution_clock::now();

    const auto& t1 = cpu_times_t1_[id];
    const auto& t2 = cpu_times_t2_[id];

    return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
  }

  // Minimizes overhead of GPU-Timing since queries are generated beforehand.
  // Use this method in conjunction with start_measure_GPU_time_indexed(..).
  void performance::initialize_gl_queries(unsigned int count)
  {
    auto old_ids = gl_query_ids_.size();
    gl_query_ids_.resize(old_ids + count);
    gl::glGenQueries(count, gl_query_ids_.data() + old_ids);

    for (auto i = 0; i < count; i++)
    {
      gpu_queries.push_back({ false, (int) gl_query_ids_[old_ids + i] });
    }
  }

  void performance::start_measure_GPU_time(int id)
  {
    gl::glBeginQuery(gl::GL_TIME_ELAPSED, gl_query_ids_[id]);
  }

  std::int64_t performance::finish_measure_GPU_time(int id)
  {
    gl::glEndQuery(gl::GL_TIME_ELAPSED);

    std::int64_t time;
    gl::glGetQueryObjecti64v(gl_query_ids_[id], gl::GL_QUERY_RESULT, &time);

    return time;
  }

  gpu_query performance::get_free_query()
  {
    // Find a free query
    for (auto i = 0; i < gpu_queries.size(); i++)
      if (!gpu_queries[i].in_use)
      {
        gpu_queries[i].in_use = true;
        return gpu_queries[i];
      }

    // Generate a new query if none was found
    gl_query_ids_.push_back(0);
    gl::glGenQueries(1, &gl_query_ids_.back());
    gpu_queries.push_back({ true, (int)gl_query_ids_.size() - 1 });
    return gpu_queries.back();
  }

  void performance::wait_for_GPU(int query_id)
  {
    auto done = 0;
    while (!done)
      gl::glGetQueryObjectiv(query_id, gl::GL_QUERY_RESULT_AVAILABLE, &done);
  }


  void performance::issue_GPU_timestamp(std::string description, int pass)
  {
    auto query = get_free_query();
    gl::glQueryCounter(query.id, gl::GL_TIMESTAMP);
    gpu_timers.push_back({ description, 0.0, query.id, pass });
  }

  void performance::clear_GPU_timestamps()
  {
    gpu_timers.clear();
    for (auto& q : gpu_queries)
      q.in_use = false;
  }

  gpu_timestamp performance::get_GPU_timing(std::string description)
  {
    for (auto i = 0; i < gpu_timers.size(); i++)
    {
      if (gpu_timers[i].description._Equal(description))
        for (auto j = 0; j < gpu_timers.size(); j++)
        {
          if (i >= j)
            continue;

          if (gpu_timers[i].pass != gpu_timers[j].pass)
            continue;

          if (gpu_timers[i].description._Equal(gpu_timers[j].description))
          {
            wait_for_GPU(gpu_timers[i].query_id);
            wait_for_GPU(gpu_timers[j].query_id);

            std::uint64_t t0;
            std::uint64_t t1;

            gl::glGetQueryObjectui64v(gpu_timers[i].query_id, gl::GL_QUERY_RESULT, &t0);
            gl::glGetQueryObjectui64v(gpu_timers[j].query_id, gl::GL_QUERY_RESULT, &t1);

            return { gpu_timers[i].description, std::abs((double)t1 - t0) / 1000000.0, 0, gpu_timers[i].pass };
          }
        }
    }
  }

  std::vector<gpu_timestamp> performance::get_GPU_timings()
  {
    std::vector<gpu_timestamp> timings;
    for (auto i = 0; i < gpu_timers.size(); i++)
      for (auto j = 0; j < gpu_timers.size(); j++)
      {
        if (i >= j)
          continue;

        if (gpu_timers[i].pass != gpu_timers[j].pass)
          continue;

        if (gpu_timers[i].description._Equal(gpu_timers[j].description))
        {
          wait_for_GPU(gpu_timers[i].query_id);
          wait_for_GPU(gpu_timers[j].query_id);

          std::uint64_t t0;
          std::uint64_t t1;

          gl::glGetQueryObjectui64v(gpu_timers[i].query_id, gl::GL_QUERY_RESULT, &t0);
          gl::glGetQueryObjectui64v(gpu_timers[j].query_id, gl::GL_QUERY_RESULT, &t1);

          timings.push_back({ gpu_timers[i].description, std::abs((double) t1 - t0) / 1000000.0, 0, gpu_timers[i].pass });
        }
      }

    return timings;
  }

  bool performance::timings_available()
  {
    return gpu_timers.size() > 0;
  }

}