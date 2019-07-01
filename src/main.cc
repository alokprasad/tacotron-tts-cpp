#include <cstdio>
#include <cstdlib>
#include <fstream>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

#include "cxxopts.hpp"
//#include "json.hpp"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "tf_synthesizer.h"

#if 0
template<typename T>
bool GetNumberArray(const nlohmann::json &j, const std::string &name,
                    std::vector<T> *value) {
  if (j.find(name) == j.end()) {
    std::cerr << "Property not found : " << name << std::endl;
    return false;
  }

  if (!j.at(name).is_array()) {
    std::cerr << "Property is not an array type : " << name << std::endl;
    return false;
  }

  std::vector<T> v;
  for (auto &element : j.at(name)) {
    if (!element.is_number()) {
      std::cerr << "An array element is not a number" << std::endl;
      return false;
    }

    v.push_back(static_cast<T>(element.get<double>()));
  }

  (*value) = v;
  return true;
}

#endif

#if 0
// Load sequence from JSON array
bool LoadSequence(const std::string &sequence_filename, std::vector<int32_t> *sequence)
{
  std::ifstream is(sequence_filename);
  if (!is) {
    std::cerr << "Failed to open/read file : " << sequence_filename << std::endl;
    return false;
  }

  nlohmann::json j;
  is >> j;

  return GetNumberArray(j, "sequence", sequence);
  
}
#endif 

static uint16_t ftous(const float x)
{
  int f = int(x);
  return std::max(uint16_t(0), std::min(std::numeric_limits<uint16_t>::max(), uint16_t(f)));
}

bool SaveWav(const std::string &filename, const std::vector<float> &samples, const int sample_rate)
{
  // We want to save audio with 32bit float format without loosing precision,
  // but librosa only supports PCM audio, so save audio data as 16bit PCM.

  drwav_data_format format;
  format.container = drwav_container_riff;     // <-- drwav_container_riff = normal WAV files, drwav_container_w64 = Sony Wave64.
  format.format = DR_WAVE_FORMAT_PCM; 
  format.channels = 1;
  format.sampleRate = sample_rate;
  format.bitsPerSample = 16;
  drwav* pWav = drwav_open_file_write(filename.c_str(), &format);

  std::vector<unsigned short> data;
  data.resize(samples.size());

  float max_value = std::fabs(samples[0]);
  for (size_t i = 0; i < samples.size(); i++) {
    max_value = std::max(max_value, std::fabs(samples[i]));
  }
  
  std::cout << "max value = " << max_value;

  float factor = 32767.0f / std::max(0.01f, max_value);

  // normalize & 16bit quantize.
  for (size_t i = 0; i < samples.size(); i++) {
    data[i] = ftous(factor * samples[i]); 
  }

  drwav_uint64 n = static_cast<drwav_uint64>(samples.size());
  drwav_uint64 samples_written = drwav_write(pWav, n, data.data());

  drwav_close(pWav);

  if (samples_written > 0) return true;

  return false;
}


int main(int argc, char **argv) {
  cxxopts::Options options("tts", "Tacotron text to speec in C++");
  options.add_options()("i,input", "Input sequence file(JSON)",
                        cxxopts::value<std::string>())(
      "g,graph", "Input freezed graph file", cxxopts::value<std::string>())
      ("h,hparams", "Hyper parameters(JSON)", cxxopts::value<std::string>());
      ("o,output", "Output WAV filename", cxxopts::value<std::string>());

  auto result = options.parse(argc, argv);

//  if (!result.count("input")) {
    //std::cerr << "Please specify input sequence file with -i or --input option."
  //            << std::endl;
//    return -1;
  //}

  if (!result.count("graph")) {
    std::cerr << "Please specify freezed graph with -g or --graph option."
              << std::endl;
    //return -1;
  }

  if (result.count("hparams")) {
  }

  std::string input_filename = result["input"].as<std::string>();
  std::string graph_filename = result["graph"].as<std::string>();
  std::string output_filename = "output.wav";

  if (result.count("output")) {
    output_filename = result["output"].as<std::string>();
  }

  std::vector<int32_t> sequence { 46, 30, 36, 32, 41, 47, 36, 46, 47, 46, 64, 28, 47, 64, 47, 35, 32, 64, 30, 32, 45, 41, 64, 39, 28, 29, 42, 45, 28, 47, 42, 45, 52, 64, 46, 28, 52, 64, 47, 35, 32, 52, 64, 35, 28, 49, 32, 64, 31, 36, 46, 30, 42, 49, 32, 45, 32, 31, 64, 28, 64, 41, 32, 50, 64, 43, 28, 45, 47, 36, 30, 39, 32, 60, 1};

#if 0 
  if (!LoadSequence(input_filename, &sequence)) {
    std::cerr << "Failed to load sequence data : " << input_filename << std::endl;
    return EXIT_FAILURE;
  }
#endif

  std::cout << "sequence = [";
  for (size_t i = 0; i < sequence.size(); i++) {
    std::cout << sequence[i];
    if (i != (sequence.size() - 1)) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;

  // Synthesize(generate wav from sequence)
  tts::TensorflowSynthesizer tf_synthesizer;
  tf_synthesizer.init(argc, argv);
  if (!tf_synthesizer.load(graph_filename, "inputs",
                    "model/griffinlim/Squeeze")) {
    std::cerr << "Failed to load/setup Tensorflow model from a frozen graph : " << graph_filename << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Synthesize..." << std::endl;

  std::vector<int32_t> input_lengths;
  input_lengths.push_back(int(sequence.size()));

  std::vector<float> output_wav;

  if (!tf_synthesizer.synthesize(sequence, input_lengths, &output_wav)) {
    std::cerr << "Failed to synthesize for a given sequence." << std::endl;
    return EXIT_FAILURE;
  }

  if (!SaveWav(output_filename, output_wav, /* sample rate */20000)) {
    std::cerr << "Failed to save wav file :" << output_filename << std::endl;

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
