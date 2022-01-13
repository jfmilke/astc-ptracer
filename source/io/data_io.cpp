#include <jay/io/data_io.hpp>
#include <fstream>

namespace jay
{
  std::string data_io::read_shader_file(
    const std::string filepath
  )
  {
    std::ifstream sourceFile(filepath, std::fstream::in);
    std::string   sourceCode;
    if (sourceFile.is_open())
      sourceCode = std::string(std::istreambuf_iterator<char>(sourceFile),
        std::istreambuf_iterator<char>());
    return sourceCode;
  }

  char* data_io::read_binary(
    std::string filepath
  )
  {
    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file)
    {
      printf("Error: File open failed '%s'\n", filepath);
      return nullptr;
    }
    std::size_t filesize = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[filesize];

    file.read(buffer, filesize);
    file.close();

    return buffer;
  }

  std::size_t data_io::get_filesize(
    std::string  filepath
  )
  {
    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file)
    {
      printf("Error: File open failed '%s'\n", filepath);
      return 0;
    }
    std::size_t filesize = file.tellg();
    file.close();

    return filesize;
  }

  long int data_io::get_max_string_length(std::vector<std::string> string_vector)
  {
    long int max_length = -1;

    for (const auto& element : string_vector)
    {
      long int l = element.length();
      if (l > max_length)
        max_length = l;
    }

    return max_length;
  }

  int data_io::store_binary(
    const char* header,
    std::size_t header_size,
    const char* data,
    std::size_t data_size,
    std::string filepath,
    bool        newFile
  )
  {
    std::ofstream file = (newFile) ? std::ofstream(filepath, std::ios::out | std::ios::binary) : std::ofstream(filepath, std::ios::out | std::ios::binary | std::ios::app);

    if (!file)
    {
      printf("Error: File open failed '%s'\n", filepath);
      return -1;
    }

    file.write(header, header_size);
    file.write(data, data_size);
    file.close();

    return 1;
  }

  // Store data
  int data_io::store_binary(
    const char* data,
    std::size_t data_size,
    std::string filepath,
    bool        newFile
  )
  {
    std::ofstream file = (newFile) ? std::ofstream(filepath, std::ios::out | std::ios::binary) : std::ofstream(filepath, std::ios::out | std::ios::binary | std::ios::app);

    if (!file)
    {
      printf("Error: File open failed '%s'\n", filepath);
      return -1;
    }

    file.write(data, data_size);
    file.close();

    return 1;
  }
}