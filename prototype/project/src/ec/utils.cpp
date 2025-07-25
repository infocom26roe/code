#include "utils.h"

const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// generate random index in range [0, len - 1]
int ECProject::random_index(size_t len)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, len - 1);
  return dist(gen);
}

// generate random numbers in range [min, max]
int ECProject::random_range(int min, int max)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(min, max);
  return dist(gen);
}

// generate n random numbers in range [min, max]
void ECProject::random_n_num(int min, int max, int n, std::vector<int> &random_numbers)
{
  my_assert(n <= max - min + 1);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(min, max);

  int cnt = 0;
  int num = dis(gen);
  random_numbers.push_back(num);
  cnt++;
  while (cnt < n) {
    while (std::find(random_numbers.begin(), random_numbers.end(), num) != random_numbers.end()) {
      num = dis(gen);
    }
    random_numbers.push_back(num);
    cnt++;
  }
}

void ECProject::random_n_element(int n, std::vector<int> array,
                                 std::vector<int>& selected_num)
{
  my_assert(n <= int(array.size()));
  int cnt = 0;
  while (cnt < n) {
    int ran_idx = random_index(array.size());
    selected_num.push_back(array[ran_idx]);
    array.erase(array.begin() + ran_idx);
    cnt++;
  }
}

// generate random strings
std::string ECProject::generate_random_string(int length)
{
  std::string result;

  std::mt19937 rng(std::time(0)); // take current time as random seed
  std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

  for (int i = 0; i < length; ++i) {
    int random_index = distribution(rng);
    result += charset[random_index];
  }

  return result;
}

// generate n key-value pairs with distinct keys
void ECProject::generate_unique_random_strings(
        int key_length, int value_length, int n,
        std::unordered_map<std::string, std::string> &key_value)
{

  std::mt19937 rng(std::time(0)); // take current time as random seed
  std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

  for (int i = 0; i < n; i++) {
    std::string key;

    do {
      for (int i = 0; i < key_length; ++i) {
        int random_index = distribution(rng);
        key += charset[random_index];
      }
    } while (key_value.find(key) != key_value.end());

    std::string value(value_length, key[0]);

    key_value[key] = value;
  }
}

void ECProject::generate_unique_random_strings_difflen(int key_length, int n,
          const std::vector<size_t>& value_lengths,
          std::unordered_map<std::string, std::string> &key_value)
{
  std::mt19937 rng(std::time(0)); // take current time as random seed
  std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

  for (int i = 0; i < n; i++) {
    std::string key;

    do {
      for (int i = 0; i < key_length; ++i) {
        int random_index = distribution(rng);
        key += charset[random_index];
      }
    } while (key_value.find(key) != key_value.end());

    std::string value(value_lengths[i], key[0]);

    key_value[key] = value;
  }
}

void ECProject::generate_unique_random_keys(int key_length, int n, std::unordered_set<std::string> &keys)
{
  std::mt19937 rng(std::time(0)); // take current time as random seed
  std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

  for (int i = 0; i < n; i++) {
    std::string key;
    do {
      for (int i = 0; i < key_length; ++i) {
        int random_index = distribution(rng);
        key += charset[random_index];
      }
    } while (keys.find(key) != keys.end());

    keys.insert(key);
  }
}

void ECProject::exit_when(bool condition, const std::source_location &location)
{
  if (!condition) {
    std::cerr << "Condition failed at " << location.file_name() << ":" << location.line()
              << " - " << location.function_name() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

int ECProject::bytes_to_int(std::vector<unsigned char> &bytes)
{
  int integer;
  unsigned char *p = (unsigned char *)(&integer);
  for (int i = 0; i < int(bytes.size()); i++) {
    memcpy(p + i, &bytes[i], 1);
  }
  return integer;
}

std::vector<unsigned char> ECProject::int_to_bytes(int integer)
{
  std::vector<unsigned char> bytes(sizeof(int));
  unsigned char *p = (unsigned char *)(&integer);
  for (int i = 0; i < int(bytes.size()); i++) {
    memcpy(&bytes[i], p + i, 1);
  }
  return bytes;
}

double ECProject::bytes_to_double(std::vector<unsigned char> &bytes)
{
  double doubler;
  memcpy(&doubler, bytes.data(), sizeof(double));
  return doubler;
}

std::vector<unsigned char> ECProject::double_to_bytes(double doubler)
{
  std::vector<unsigned char> bytes(sizeof(double));
  memcpy(bytes.data(), &doubler, sizeof(double));
  return bytes;
}

bool ECProject::cmp_descending(std::pair<int, int> &a,std::pair<int, int> &b)
{
  return a.second > b.second;
}