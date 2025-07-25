#include "client.h"
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <chrono>

using namespace ECProject;

void test_single_block_repair(Client &client, int block_num)
{
  auto stripe_ids = client.list_stripes();
  int stripe_num = stripe_ids.size();
  std::vector<double> repair_times;
  std::vector<double> decoding_times;
  std::vector<double> cross_cluster_times;
  std::vector<double> meta_times;
  std::vector<int> cross_cluster_transfers;
  std::vector<int> io_cnts;
  std::cout << "Single-Block Repair:" << std::endl;
  for (int i = 0; i < stripe_num; i++)
  {
    std::cout << "[Stripe " << i << "]" << std::endl;
    double temp_repair = 0;
    double temp_decoding = 0;
    double temp_cross_cluster = 0;
    double temp_meta = 0;
    int temp_cc_transfers = 0;
    int temp_io_cnt = 0;
    for (int j = 0; j < block_num; j++)
    {
      std::vector<unsigned int> failures;
      failures.push_back((unsigned int)j);
      auto resp = client.blocks_repair(failures, stripe_ids[i]);
      temp_repair += resp.repair_time;
      temp_decoding += resp.decoding_time;
      temp_cross_cluster += resp.cross_cluster_time;
      temp_meta += resp.meta_time;
      temp_cc_transfers += resp.cross_cluster_transfers;
      temp_io_cnt += resp.io_cnt;
    }
    repair_times.push_back(temp_repair);
    decoding_times.push_back(temp_decoding);
    cross_cluster_times.push_back(temp_cross_cluster);
    meta_times.push_back(temp_meta);
    cross_cluster_transfers.push_back(temp_cc_transfers);
    io_cnts.push_back(temp_io_cnt);
    std::cout << "repair = " << temp_repair / block_num
              << "s, decoding = " << temp_decoding / block_num
              << "s, cross-cluster = " << temp_cross_cluster / block_num
              << "s, meta = " << temp_meta / block_num
              << "s, cross-cluster-count = " << (double)temp_cc_transfers / block_num
              << ", I/Os = " << temp_io_cnt / block_num
              << std::endl;
  }
  auto avg_repair = std::accumulate(repair_times.begin(),
                                    repair_times.end(), 0.0) /
                    (stripe_num * block_num);
  auto avg_decoding = std::accumulate(decoding_times.begin(),
                                      decoding_times.end(), 0.0) /
                      (stripe_num * block_num);
  auto avg_cross_cluster = std::accumulate(cross_cluster_times.begin(),
                                           cross_cluster_times.end(), 0.0) /
                           (stripe_num * block_num);
  auto avg_meta = std::accumulate(meta_times.begin(),
                                  meta_times.end(), 0.0) /
                  (stripe_num * block_num);
  auto avg_cc_transfers = (double)std::accumulate(cross_cluster_transfers.begin(),
                                                  cross_cluster_transfers.end(), 0) /
                          (stripe_num * block_num);
  auto avg_io_cnt = (double)std::accumulate(io_cnts.begin(),
                                            io_cnts.end(), 0) /
                    (stripe_num * block_num);
  std::cout << "^-^[Average]^-^" << std::endl;
  std::cout << "repair = " << avg_repair << "s, decoding = " << avg_decoding
            << "s, cross-cluster = " << avg_cross_cluster
            << "s, meta = " << avg_meta
            << "s, cross-cluster-count = " << avg_cc_transfers
            << ", I/Os = " << avg_io_cnt
            << std::endl;
}

void test_multiple_blocks_repair(Client &client, int block_num, const ParametersInfo &paras,
                                 int failed_num)
{
  auto stripe_ids = client.list_stripes();
  int stripe_num = stripe_ids.size();
  std::vector<double> repair_times;
  std::vector<double> decoding_times;
  std::vector<double> cross_cluster_times;
  std::vector<double> meta_times;
  std::vector<int> cross_cluster_transfers;
  std::vector<int> io_cnts;
  int run_time = 5;
  int tot_cnt = 0;
  ErasureCode *ec = ec_factory(paras.ec_type, paras.cp);
  std::cout << "Multi-Block Repair:" << std::endl;
  for (int i = 0; i < stripe_num; i++)
  {
    std::cout << "[Stripe " << i << "]" << std::endl;
    double temp_repair = 0;
    double temp_decoding = 0;
    double temp_cross_cluster = 0;
    double temp_meta = 0;
    int temp_cc_transfers = 0;
    int temp_io_cnt = 0;
    int cnt = 0;
    for (int j = 0; j < run_time; j++)
    {
      // int failed_num = random_range(2, 4);
      std::vector<int> failed_blocks;
      random_n_num(0, block_num - 1, failed_num, failed_blocks);
      std::vector<unsigned int> failures;
      for (auto &block : failed_blocks)
      {
        failures.push_back((unsigned int)block);
      }
      if (!ec->check_if_decodable(failed_blocks))
      {
        j--;
        continue;
      }
      auto resp = client.blocks_repair(failures, stripe_ids[i]);
      if (resp.success)
      {
        temp_repair += resp.repair_time;
        temp_decoding += resp.decoding_time;
        temp_cross_cluster += resp.cross_cluster_time;
        temp_meta += resp.meta_time;
        temp_cc_transfers += resp.cross_cluster_transfers;
        temp_io_cnt += resp.io_cnt;
        cnt++;
      }
    }
    repair_times.push_back(temp_repair);
    decoding_times.push_back(temp_decoding);
    cross_cluster_times.push_back(temp_cross_cluster);
    meta_times.push_back(temp_meta);
    cross_cluster_transfers.push_back(temp_cc_transfers);
    io_cnts.push_back(temp_io_cnt);
    std::cout << "repair = " << temp_repair / cnt
              << "s, decoding = " << temp_decoding / cnt
              << "s, cross-cluster = " << temp_cross_cluster / cnt
              << "s, meta = " << temp_meta / cnt
              << "s, cross-cluster-count = " << (double)temp_cc_transfers / cnt
              << ", I/Os = " << temp_io_cnt / cnt
              << std::endl;
    tot_cnt += cnt;
  }
  auto avg_repair = std::accumulate(repair_times.begin(),
                                    repair_times.end(), 0.0) /
                    tot_cnt;
  auto avg_decoding = std::accumulate(decoding_times.begin(),
                                      decoding_times.end(), 0.0) /
                      tot_cnt;
  auto avg_cross_cluster = std::accumulate(cross_cluster_times.begin(),
                                           cross_cluster_times.end(), 0.0) /
                           tot_cnt;
  auto avg_meta = std::accumulate(meta_times.begin(),
                                  meta_times.end(), 0.0) /
                  tot_cnt;
  auto avg_cc_transfers = (double)std::accumulate(cross_cluster_transfers.begin(),
                                                  cross_cluster_transfers.end(), 0) /
                          tot_cnt;
  auto avg_io_cnt = (double)std::accumulate(io_cnts.begin(),
                                            io_cnts.end(), 0) /
                    tot_cnt;
  std::cout << "^-^[Average]^-^" << std::endl;
  std::cout << "repair = " << avg_repair << "s, decoding = " << avg_decoding
            << "s, cross-cluster = " << avg_cross_cluster
            << "s, meta = " << avg_meta
            << "s, cross-cluster-count = " << avg_cc_transfers
            << ", I/Os = " << avg_io_cnt
            << std::endl;
  if (ec != nullptr)
  {
    delete ec;
    ec = nullptr;
  }
}

void test_multiple_blocks_repair_lrc(Client &client, const ParametersInfo &paras,
                                     int failed_num)
{
  auto stripe_ids = client.list_stripes();
  int stripe_num = stripe_ids.size();
  std::vector<double> repair_times;
  std::vector<double> decoding_times;
  std::vector<double> cross_cluster_times;
  std::vector<double> meta_times;
  std::vector<int> cross_cluster_transfers;
  std::vector<int> io_cnts;
  int run_time = 10;
  int tot_cnt = 0;
  LocallyRepairableCode *lrc = lrc_factory(paras.ec_type, paras.cp);
  std::vector<std::vector<int>> groups;
  lrc->grouping_information(groups);
  int group_num = (int)groups.size();
  std::cout << "Multi-Block Repair:" << std::endl;
  for (int i = 0; i < stripe_num; i++)
  {
    std::cout << "[Stripe " << i << "]" << std::endl;
    double temp_repair = 0;
    double temp_decoding = 0;
    double temp_cross_cluster = 0;
    double temp_meta = 0;
    int temp_cc_transfers = 0;
    int temp_io_cnt = 0;
    int cnt = 0;
    for (int j = 0; j < run_time; j++)
    {
      // int gid = random_index((size_t)group_num);
      int ran_data_idx = random_index((size_t)(lrc->k + lrc->g));
      int gid = ran_data_idx / lrc->r;
      std::vector<int> failed_blocks;
      random_n_element(2, groups[gid], failed_blocks);
      if (failed_num > 2)
      {
        int t_gid = random_index((size_t)group_num);
        int t_idx = random_index(groups[t_gid].size());
        int failed_idx = groups[t_gid][t_idx];
        while (std::find(failed_blocks.begin(), failed_blocks.end(), failed_idx) != failed_blocks.end())
        {
          t_gid = random_index((size_t)group_num);
          t_idx = random_index(groups[t_gid].size());
          failed_idx = groups[t_gid][t_idx];
        }
        failed_blocks.push_back(failed_idx);
        if (failed_num > 3)
        {
          int tt_gid = 0;
          if (gid == t_gid && paras.cp.g < 3)
          {
            tt_gid = (gid + random_index((size_t)(group_num - 1)) + 1) % group_num;
          }
          else
          {
            tt_gid = random_index((size_t)group_num);
          }
          t_idx = random_index(groups[tt_gid].size());
          failed_idx = groups[tt_gid][t_idx];
          while (std::find(failed_blocks.begin(), failed_blocks.end(), failed_idx) != failed_blocks.end())
          {
            if (gid == t_gid && paras.cp.g < 3)
            {
              tt_gid = (gid + random_index((size_t)(group_num - 1)) + 1) % group_num;
            }
            else
            {
              tt_gid = random_index((size_t)group_num);
            }
            t_idx = random_index(groups[tt_gid].size());
            failed_idx = groups[tt_gid][t_idx];
          }
          failed_blocks.push_back(failed_idx);
        }
      }
      if (!lrc->check_if_decodable(failed_blocks))
      {
        j--;
        continue;
      }
      std::vector<unsigned int> failures;
      for (auto &block : failed_blocks)
      {
        failures.push_back((unsigned int)block);
      }
      auto resp = client.blocks_repair(failures, stripe_ids[i]);
      if (resp.success)
      {
        temp_repair += resp.repair_time;
        temp_decoding += resp.decoding_time;
        temp_cross_cluster += resp.cross_cluster_time;
        temp_meta += resp.meta_time;
        temp_cc_transfers += resp.cross_cluster_transfers;
        temp_io_cnt += resp.io_cnt;
        cnt++;
      }
    }
    repair_times.push_back(temp_repair);
    decoding_times.push_back(temp_decoding);
    cross_cluster_times.push_back(temp_cross_cluster);
    meta_times.push_back(temp_meta);
    cross_cluster_transfers.push_back(temp_cc_transfers);
    io_cnts.push_back(temp_io_cnt);
    std::cout << "repair = " << temp_repair / cnt
              << "s, decoding = " << temp_decoding / cnt
              << "s, cross-cluster = " << temp_cross_cluster / cnt
              << "s, meta = " << temp_meta / cnt
              << "s, cross-cluster-count = " << (double)temp_cc_transfers / cnt
              << ", I/Os = " << temp_io_cnt / cnt
              << std::endl;
    tot_cnt += cnt;
  }
  auto avg_repair = std::accumulate(repair_times.begin(),
                                    repair_times.end(), 0.0) /
                    tot_cnt;
  auto avg_decoding = std::accumulate(decoding_times.begin(),
                                      decoding_times.end(), 0.0) /
                      tot_cnt;
  auto avg_cross_cluster = std::accumulate(cross_cluster_times.begin(),
                                           cross_cluster_times.end(), 0.0) /
                           tot_cnt;
  auto avg_meta = std::accumulate(meta_times.begin(),
                                  meta_times.end(), 0.0) /
                  tot_cnt;
  auto avg_cc_transfers = (double)std::accumulate(cross_cluster_transfers.begin(),
                                                  cross_cluster_transfers.end(), 0) /
                          tot_cnt;
  auto avg_io_cnt = (double)std::accumulate(io_cnts.begin(),
                                            io_cnts.end(), 0) /
                    tot_cnt;
  std::cout << "^-^[Average]^-^" << std::endl;
  std::cout << "repair = " << avg_repair << "s, decoding = " << avg_decoding
            << "s, cross-cluster = " << avg_cross_cluster
            << "s, meta = " << avg_meta
            << "s, cross-cluster-count = " << avg_cc_transfers
            << ", I/Os = " << avg_io_cnt
            << std::endl;
  if (lrc != nullptr)
  {
    delete lrc;
    lrc = nullptr;
  }
}

void test_stripe_merging(Client &client, int step_size)
{
  my_assert(step_size > 1);
  auto stripe_ids = client.list_stripes();
  int stripe_num = stripe_ids.size();
  std::cout << "Stripe Merging:" << std::endl;
  auto resp = client.merge(step_size);
  std::cout << "[Total]" << std::endl;
  std::cout << "merging = " << resp.merging_time
            << "s, computing = " << resp.computing_time
            << "s, cross-cluster = " << resp.cross_cluster_time
            << "s, meta =" << resp.meta_time
            << "s, cross-cluster-count = " << resp.cross_cluster_transfers
            << ", I/Os = " << resp.io_cnt
            << std::endl;
  std::cout << "[Average for every " << step_size << " stripes]" << std::endl;
  std::cout << "merging = " << resp.merging_time / stripe_num
            << "s, computing = " << resp.computing_time / stripe_num
            << "s, cross-cluster = " << resp.cross_cluster_time / stripe_num
            << "s, meta =" << resp.meta_time / stripe_num
            << "s, cross-cluster-count = "
            << (double)resp.cross_cluster_transfers / stripe_num
            << ", I/Os = " << resp.io_cnt / stripe_num
            << std::endl;
}

void generate_random_multi_block_failures_lrc(std::string filename,
                                              int stripe_num, const ParametersInfo &paras, int failed_num)
{
  LocallyRepairableCode *lrc = lrc_factory(paras.ec_type, paras.cp);
  std::vector<std::vector<int>> groups;
  lrc->grouping_information(groups);
  int group_num = (int)groups.size();
  std::string suf = lrc->type() + "_" + std::to_string(paras.cp.k) + "_" +
                    std::to_string(paras.cp.l) + "_" + std::to_string(paras.cp.g) + "_" +
                    std::to_string(failed_num);
  filename += suf;
  std::ofstream outFile(filename);
  if (!outFile)
  {
    std::cerr << "Error! Unable to open " << filename << std::endl;
    return;
  }
  int cases_per_stripe = 10;
  for (int i = 0; i < cases_per_stripe * stripe_num; i++)
  {
    int ran_data_idx = random_index((size_t)(lrc->k + lrc->g));
    int gid = ran_data_idx / lrc->r;
    std::vector<int> failed_blocks;
    random_n_element(2, groups[gid], failed_blocks);
    if (failed_num > 2)
    {
      int t_gid = random_index((size_t)group_num);
      int t_idx = random_index(groups[t_gid].size());
      int failed_idx = groups[t_gid][t_idx];
      while (std::find(failed_blocks.begin(), failed_blocks.end(), failed_idx) != failed_blocks.end())
      {
        t_gid = random_index((size_t)group_num);
        t_idx = random_index(groups[t_gid].size());
        failed_idx = groups[t_gid][t_idx];
      }
      failed_blocks.push_back(failed_idx);
      if (failed_num > 3)
      {
        int tt_gid = 0;
        if (gid == t_gid && paras.cp.g < 3)
        {
          tt_gid = (gid + random_index((size_t)(group_num - 1)) + 1) % group_num;
        }
        else
        {
          tt_gid = random_index((size_t)group_num);
        }
        t_idx = random_index(groups[tt_gid].size());
        failed_idx = groups[tt_gid][t_idx];
        while (std::find(failed_blocks.begin(), failed_blocks.end(), failed_idx) != failed_blocks.end())
        {
          if (gid == t_gid && paras.cp.g < 3)
          {
            tt_gid = (gid + random_index((size_t)(group_num - 1)) + 1) % group_num;
          }
          else
          {
            tt_gid = random_index((size_t)group_num);
          }
          t_idx = random_index(groups[tt_gid].size());
          failed_idx = groups[tt_gid][t_idx];
        }
        failed_blocks.push_back(failed_idx);
      }
    }
    if (!lrc->check_if_decodable(failed_blocks))
    {
      i--;
      continue;
    }
    else
    {
      for (const auto &num : failed_blocks)
      {
        outFile << num << " ";
      }
      outFile << "\n";
    }
  }
  outFile.close();
}

void test_multiple_blocks_repair_lrc_with_testcases(std::string filename,
                                                    Client &client, const ParametersInfo &paras, int failed_num)
{
  auto stripe_ids = client.list_stripes();
  int stripe_num = stripe_ids.size();
  std::vector<double> repair_times;
  std::vector<double> decoding_times;
  std::vector<double> cross_cluster_times;
  std::vector<double> meta_times;
  std::vector<int> cross_cluster_transfers;
  std::vector<int> io_cnts;
  int run_time = 10;
  int tot_cnt = 0;
  LocallyRepairableCode *lrc = lrc_factory(paras.ec_type, paras.cp);
  std::vector<std::vector<int>> groups;
  lrc->grouping_information(groups);
  int group_num = (int)groups.size();
  std::string suf = lrc->type() + "_" + std::to_string(paras.cp.k) + "_" +
                    std::to_string(paras.cp.l) + "_" + std::to_string(paras.cp.g) + "_" +
                    std::to_string(failed_num);
  filename += suf;
  std::ifstream inFile(filename);
  if (!inFile)
  {
    std::cerr << "Error! Unable to open " << filename << std::endl;
    return;
  }
  std::string line;
  std::cout << "Multi-Block Repair:" << std::endl;
  int ii = 0;
  int test_stripe_num = 5;
  for (int i = 0; i < test_stripe_num; i++)
  {
    std::cout << "[Stripe " << i << "]" << std::endl;
    double temp_repair = 0;
    double temp_decoding = 0;
    double temp_cross_cluster = 0;
    double temp_meta = 0;
    int temp_cc_transfers = 0;
    int temp_io_cnt = 0;
    int cnt = 0;
    for (int j = 0; j < run_time; j++)
    {
      std::vector<int> failed_blocks;
      std::getline(inFile, line);
      std::istringstream lineStream(line);
      int num;
      while (lineStream >> num)
      {
        failed_blocks.push_back(num);
      }
      if (i < test_stripe_num - stripe_num)
      {
        continue;
      }
      std::vector<unsigned int> failures;
      for (auto &block : failed_blocks)
      {
        failures.push_back((unsigned int)block);
      }
      auto resp = client.blocks_repair(failures, stripe_ids[ii]);
      if (resp.success)
      {
        temp_repair += resp.repair_time;
        temp_decoding += resp.decoding_time;
        temp_cross_cluster += resp.cross_cluster_time;
        temp_meta += resp.meta_time;
        temp_cc_transfers += resp.cross_cluster_transfers;
        temp_io_cnt += resp.io_cnt;
        cnt++;
      }
    }
    if (i < test_stripe_num - stripe_num)
    {
      continue;
    }
    ii++;
    repair_times.push_back(temp_repair);
    decoding_times.push_back(temp_decoding);
    cross_cluster_times.push_back(temp_cross_cluster);
    meta_times.push_back(temp_meta);
    cross_cluster_transfers.push_back(temp_cc_transfers);
    io_cnts.push_back(temp_io_cnt);
    std::cout << "repair = " << temp_repair / cnt
              << "s, decoding = " << temp_decoding / cnt
              << "s, cross-cluster = " << temp_cross_cluster / cnt
              << "s, meta = " << temp_meta / cnt
              << "s, cross-cluster-count = " << (double)temp_cc_transfers / cnt
              << ", I/Os = " << temp_io_cnt / cnt
              << std::endl;
    tot_cnt += cnt;
  }
  inFile.close();
  auto avg_repair = std::accumulate(repair_times.begin(),
                                    repair_times.end(), 0.0) /
                    tot_cnt;
  auto avg_decoding = std::accumulate(decoding_times.begin(),
                                      decoding_times.end(), 0.0) /
                      tot_cnt;
  auto avg_cross_cluster = std::accumulate(cross_cluster_times.begin(),
                                           cross_cluster_times.end(), 0.0) /
                           tot_cnt;
  auto avg_meta = std::accumulate(meta_times.begin(),
                                  meta_times.end(), 0.0) /
                  tot_cnt;
  auto avg_cc_transfers = (double)std::accumulate(cross_cluster_transfers.begin(),
                                                  cross_cluster_transfers.end(), 0) /
                          tot_cnt;
  auto avg_io_cnt = (double)std::accumulate(io_cnts.begin(),
                                            io_cnts.end(), 0) /
                    tot_cnt;
  std::cout << "^-^[Average]^-^" << std::endl;
  std::cout << "repair = " << avg_repair << "s, decoding = " << avg_decoding
            << "s, cross-cluster = " << avg_cross_cluster
            << "s, meta = " << avg_meta
            << "s, cross-cluster-count = " << avg_cc_transfers
            << ", I/Os = " << avg_io_cnt
            << std::endl;
  if (lrc != nullptr)
  {
    delete lrc;
    lrc = nullptr;
  }
}

void test_repair_performance(
    std::string path_prefix, int stripe_num, const ParametersInfo &paras, int failed_num, bool optimized)
{
  int block_num = paras.cp.k + paras.cp.m;

  Client client("10.0.0.4", CLIENT_PORT, "10.0.0.4", COORDINATOR_PORT);

  // set erasure coding parameters
  client.set_ec_parameters(paras);

  struct timeval start_time, end_time;
  // generate key-value pair
  std::vector<size_t> value_lengths(stripe_num, 0);
  std::vector<std::vector<std::string>> ms_object_keys;
  std::vector<std::vector<size_t>> ms_object_sizes;
  std::vector<std::vector<unsigned int>> ms_object_accessrates;
  // std::vector<std::vector<std::string>> ms_object_values;
  int obj_id = 0;
  for (int i = 0; i < stripe_num; i++)
  {
    std::vector<std::string> object_keys;
    for (int j = 0; j < paras.cp.k; j++)
    {
      value_lengths[i] += paras.block_size;
      object_keys.emplace_back("obj" + std::to_string(obj_id++));
    }
    ms_object_keys.emplace_back(object_keys);
    ms_object_sizes.emplace_back(std::vector<size_t>(paras.cp.k, paras.block_size));
    ms_object_accessrates.emplace_back(std::vector<unsigned int>(paras.cp.k, 1));
  }
#ifdef IN_MEMORY
  std::unordered_map<std::string, std::string> key_value;
  generate_unique_random_strings_difflen(5, stripe_num, value_lengths, key_value);
#endif

  // set
  double set_time = 0;
#ifdef IN_MEMORY
  int i = 0;
  for (auto &kv : key_value)
  {
    gettimeofday(&start_time, NULL);
    double encoding_time = client.set(kv.second, ms_object_keys[i], ms_object_sizes[i],
                                      ms_object_accessrates[i]);
    gettimeofday(&end_time, NULL);
    double temp_time = end_time.tv_sec - start_time.tv_sec +
                       (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    set_time += temp_time;
    std::cout << "[SET] set time: " << temp_time << ", encoding time: "
              << encoding_time << std::endl;
    ++i;
  }
  std::cout << "Total set time: " << set_time << ", average set time:"
            << set_time / stripe_num << std::endl;
  std::cout << "Write Throughput: " << paras.block_size * paras.cp.k * stripe_num / (set_time * 1024)
            << " KB/s" << std::endl;
#else
  for (int i = 0; i < stripe_num; i++)
  {
    std::string readpath = path_prefix + "/../../data/Object";
    double encoding_time = 0;
    gettimeofday(&start_time, NULL);
    if (access(readpath.c_str(), 0) == -1)
    {
      std::cout << "[Client] file does not exist!" << std::endl;
      exit(-1);
    }
    else
    {
      char *buf = new char[value_lengths[i]];
      std::ifstream ifs(readpath);
      ifs.read(buf, value_lengths[i]);
      encoding_time = client.set(std::string(buf, value_lengths[i]), ms_object_keys[i],
                                 ms_object_sizes[i], ms_object_accessrates[i]);
      ifs.close();
      // std::vector<std::string> object_values;
      // for (int j = 0; j < paras.cp.k; j++) {
      //   object_values.emplace_back(std::string(buf + j * paras.block_size, paras.block_size));
      // }
      // ms_object_values.emplace_back(object_values);
      delete buf;
    }
    gettimeofday(&end_time, NULL);
    double temp_time = end_time.tv_sec - start_time.tv_sec +
                       (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    set_time += temp_time;
    std::cout << "[SET] set time: " << temp_time << ", encoding time: "
              << encoding_time << std::endl;
  }
  std::cout << "Total set time: " << set_time << ", average set time:"
            << set_time / stripe_num << std::endl;
  std::cout << "Write Throughput: " << paras.block_size * paras.cp.k * stripe_num / (set_time * 1024)
            << " KB/s" << std::endl;
#endif

  /*
    // get
    double get_time = 0.0;
    auto start = std::chrono::steady_clock::now();
    for (auto &object_keys : ms_object_keys)
    {
      for (auto &key : object_keys)
      {
        auto value = client.get(key);
        // my_assert(value == ms_object_values[i][j]);
      }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    get_time = elapsed.count();
    std::cout << "Total get time: " << get_time << " , average get time:"
              << double(get_time) / double(stripe_num * paras.cp.k) << std::endl;
    std::cout << "Read Throughput: " << double(paras.block_size * paras.cp.k * stripe_num) / double(get_time * 1024)
              << " MB/s" << std::endl;
  */

  bool is_ec = paras.is_ec_now;

  // redundancy transitioning, from replicas to ec
  if (!is_ec)
  {
    auto resp = client.redundancy_transition(optimized);
    if (resp.iftransed)
    {
      std::cout << "[Rep->EC] transition time: " << resp.transition_time
                << ", encoding time: " << resp.encoding_time
                << ", cross-cluster time: " << resp.cross_cluster_time
                << ", meta time: " << resp.meta_time
                << ", cross-cluster-count: " << resp.cross_cluster_transfers
                << "(data=" << resp.data_reloc_cnt
                << ", parity=" << resp.parity_reloc_cnt
                << "), I/Os = " << resp.io_cnt
                << std::endl;
      std::cout << "[Average for every stripes]" << std::endl;
      std::cout << "[Rep->EC] transition time: " << resp.transition_time / stripe_num
                << ", encoding time: " << resp.encoding_time / stripe_num
                << ", cross-cluster time: " << resp.cross_cluster_time / stripe_num
                << ", meta time: " << resp.meta_time / stripe_num
                << ", cross-cluster-count: " << resp.cross_cluster_transfers / stripe_num
                << "(data=" << resp.data_reloc_cnt / stripe_num
                << ", parity=" << resp.parity_reloc_cnt / stripe_num
                << "), I/Os = " << resp.io_cnt / stripe_num
                << std::endl;
      std::cout << "[Max] transition time: " << resp.max_trans_time
                << ", [Min] transition time: " << resp.min_trans_time << std::endl;
    }
    else
    {
      std::cout << "[Rep->EC] failed!" << std::endl;
    }
    is_ec = true;
  }

  // test repair
  if (is_ec)
  {
    if (failed_num == 1)
    {
      test_single_block_repair(client, block_num);
    }
    else if (failed_num == 2)
    {
      test_multiple_blocks_repair(client, block_num, paras, failed_num);
    }
  }

  // delete
  client.delete_all_stripes();
}

int main(int argc, char **argv)
{
  if (argc != 4 && argc != 5)
  {
    std::cout << "./run_client config_file stripe_num failed_num" << std::endl;
    exit(0);
  }

  char buff[256];
  getcwd(buff, 256);
  std::string cwf = std::string(argv[0]);
  std::string path_prefix = std::string(buff) + cwf.substr(1, cwf.rfind('/') - 1);

  ParametersInfo paras;
  parse_args(nullptr, paras, path_prefix + "/../" + std::string(argv[1]));
  int stripe_num = std::stoi(argv[2]);

  int failed_num = std::stoi(argv[3]);
  my_assert(0 <= failed_num && failed_num <= 2);
  bool optimized = true;
  if (argc == 5)
  {
    optimized = (std::string(argv[4]) == "true");
  }

  test_repair_performance(path_prefix, stripe_num, paras, failed_num, optimized);

  return 0;
}