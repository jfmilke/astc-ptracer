#ifndef JAY_DATA_IO_HPP
#define JAY_DATA_IO_HPP

#include <string>

#include <jay/export.hpp>
#include <filesystem>
#include <fstream>

namespace jay
{

struct JAY_EXPORT data_io
{
  data_io() = default;

  static std::string data_io::read_shader_file(const std::string filepath);

  // Read data
  static char* read_binary(std::string filepath);

  // Read data into a pointer (check if it fits!)
  template <typename T>
  static void read_binary_into(T* data_ptr, std::string filepath)
  {
    std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file)
    {
      printf("Error: File open failed '%s'\n", filepath);
      return;
    }
    std::size_t filesize = file.tellg();
    file.seekg(0, std::ios::beg);

    file.read(reinterpret_cast<char*>(data_ptr), filesize);
    file.close();
  }

  // Read data
  static std::size_t get_filesize(std::string  filepath);
  template <typename T>
  static long int get_max_predecimal_length(std::vector<T> data_vector)
  {
    long int max_length = -1;

    for (const auto& element : data_vector)
    {
      long int l = std::to_string((long int)element).length();
      if (l > max_length)
        max_length = l;
    }

    return max_length;
  }

  static long int get_max_string_length(std::vector<std::string> string_vector);
  static bool file_exists(std::string filepath)
  {
    if (std::ifstream(filepath))
      return true;
    else
      return false;
  }

  // Store header + data
  static int store_binary(
    const char*       header,
          std::size_t header_size,
    const char*       data,
          std::size_t data_size,
          std::string filepath,
          bool        newFile
  );

  // Store data
  static int store_binary(
    const char*       data,
          std::size_t data_size,
          std::string filepath,
          bool        newFile
  );

  template <typename T>
  static int store_descriptive_text(
    std::string descriptive_header,
    std::vector<std::string>& descriptions,
    std::vector<T>& data,
    std::string filepath,
    unsigned int digits_in_decimal,
    bool         overwrite
  )
  {
    std::string desc_seperator = ": ";
    int decimal_seperator = (digits_in_decimal) ? 1 : 0; // to account for the decimal point if decimals are printed
    //int decimal_seperator = 1; // to account for the decimal point if decimals are printed
    auto desc_max_length = get_max_string_length(descriptions) + desc_seperator.length();
    auto data_max_predecimal_length = get_max_predecimal_length(data);

    if (!overwrite)
      if (file_exists(filepath))
        return -1;

    std::ofstream output_file = std::ofstream(filepath, std::ios::out);
    if (!output_file)
      return -1;

    output_file << std::fixed << std::setprecision(digits_in_decimal);

    output_file << descriptive_header << "\n\n";

    long int i = 0;
    for (const auto& desc : descriptions)
    {
      const auto& value = data[i];
      int desc_indents = desc_max_length - desc.length() - desc_seperator.length();
      int data_indents = data_max_predecimal_length - std::to_string((int)value).length();
      // Description part
      output_file << desc + desc_seperator + std::string(desc_indents, ' ');
      // Value part
      output_file << std::string(data_indents, ' ');
      output_file << data[i] << "\n";

      i++;
    }
    return 1;
  }

  template <typename T>
  static int store_text(
    std::string descriptive_header,
    T& data,
    std::string filepath,
    unsigned int digits_in_decimal,
    bool         overwrite
  )
  {
    std::vector<T> t{ data };
    return store_text(descriptive_header, t, filepath, digits_in_decimal, overwrite);
  }

  template <typename T>
  static int store_text(
    std::string descriptive_header,
    std::vector<T>& data,
    std::string filepath,
    unsigned int digits_in_decimal,
    bool         overwrite
  )
  {
    int decimal_seperator = (digits_in_decimal) ? 1 : 0; // to account for the decimal point if decimals are printed
    auto data_max_predecimal_length = get_max_predecimal_length(data);

    if (!overwrite)
      if (file_exists(filepath))
        return -1;

    std::ofstream output_file = std::ofstream(filepath, std::ios::out);
    if (!output_file)
      return -1;

    output_file << std::fixed << std::setprecision(digits_in_decimal);
    output_file << descriptive_header << "\n\n";

    for (const auto& value : data)
    {
      int data_indents = data_max_predecimal_length - std::to_string((int)value).length();

      output_file << std::string(data_indents, ' ');
      output_file << value << "\n";
    }
    return 1;
  }
};

}
#endif