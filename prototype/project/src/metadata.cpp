#include "metadata.h"
#include <fstream>
#include <sstream>

namespace ECProject
{
  static const std::unordered_map<std::string, MultiStripePR> mspr_map = {
    {"RAND", RAND},
    {"DISPERSED", DISPERSED},
    {"AGGREGATED", AGGREGATED},
    {"HORIZONTAL", HORIZONTAL},
    {"VERTICAL", VERTICAL}
  };

  static const std::unordered_map<std::string, PlacementRule> pr_map = {
    {"FLAT", FLAT},
    {"RANDOM", RANDOM},
    {"OPTIMAL", OPTIMAL}
  };

  static const std::unordered_map<std::string, ECTYPE> ec_map = {
    {"RS", RS},
    {"AZURE_LRC", AZURE_LRC},
    {"AZURE_LRC_1", AZURE_LRC_1},
    {"OPTIMAL_LRC", OPTIMAL_LRC},
    {"OPTIMAL_CAUCHY_LRC", OPTIMAL_CAUCHY_LRC},
    {"UNIFORM_CAUCHY_LRC", UNIFORM_CAUCHY_LRC},
    {"NON_UNIFORM_LRC", NON_UNIFORM_LRC},
    {"PC", PC},
    {"HV_PC", HV_PC}
  };

  ECFAMILY check_ec_family(ECTYPE ec_type)
  {
    if (ec_type == AZURE_LRC || ec_type == AZURE_LRC_1 ||
        ec_type == OPTIMAL_LRC || ec_type == OPTIMAL_CAUCHY_LRC ||
        ec_type == UNIFORM_CAUCHY_LRC || ec_type == NON_UNIFORM_LRC) {
      return LRCs;
    } else if (ec_type == PC || ec_type == HV_PC) {
      return PCs;
    } else {
      return RSCodes;
    }
  }

  ErasureCode* ec_factory(ECTYPE ec_type, CodingParameters cp)
  {
    ErasureCode *ec;
    if (ec_type == RS) {
      ec = new RSCode(cp.k, cp.m);
    } else if (ec_type == AZURE_LRC) {
      ec = new Azu_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == AZURE_LRC_1) {
      ec = new Azu_LRC_1(cp.k, cp.l, cp.g);
    } else if (ec_type == OPTIMAL_LRC) {
      ec = new Opt_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == OPTIMAL_CAUCHY_LRC) {
      ec = new Opt_Cau_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == UNIFORM_CAUCHY_LRC) {
      ec = new Uni_Cau_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == NON_UNIFORM_LRC) {
      ec = new Non_Uni_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == PC) {
      ec = new ProductCode(cp.k1, cp.m1, cp.k2, cp.m2);
    } else if (ec_type == HV_PC) {
      ec = new HVPC(cp.k1, cp.m1, cp.k2, cp.m2);
    } else {
      ec = nullptr;
    }
    return ec;
  }

  RSCode* rs_factory(ECTYPE ec_type, CodingParameters cp)
  {
    RSCode *rs;
    if (ec_type == RS) {
      rs = new RSCode(cp.k, cp.m);
    } else {
      rs = nullptr;
    }
    return rs;
  }

  LocallyRepairableCode* lrc_factory(ECTYPE ec_type, CodingParameters cp)
  {
    LocallyRepairableCode *lrc;
    if (ec_type == AZURE_LRC) {
      lrc = new Azu_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == AZURE_LRC_1) {
      lrc = new Azu_LRC_1(cp.k, cp.l, cp.g);
    } else if (ec_type == OPTIMAL_LRC) {
      lrc = new Opt_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == OPTIMAL_CAUCHY_LRC) {
      lrc = new Opt_Cau_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == UNIFORM_CAUCHY_LRC) {
      lrc = new Uni_Cau_LRC(cp.k, cp.l, cp.g);
    } else if (ec_type == NON_UNIFORM_LRC) {
      lrc = new Non_Uni_LRC(cp.k, cp.l, cp.g);
    } else {
      lrc = nullptr;
    }
    return lrc;
  }

  ProductCode* pc_factory(ECTYPE ec_type, CodingParameters cp)
  {
    ProductCode *pc;
    if (ec_type == PC) {
      pc = new ProductCode(cp.k1, cp.m1, cp.k2, cp.m2);
    } else if (ec_type == HV_PC) {
      pc = new HVPC(cp.k1, cp.m1, cp.k2, cp.m2);
    } else {
      pc = nullptr;
    }
    return pc;
  }

  ErasureCode* clone_ec(ECTYPE ec_type, ErasureCode* ec)
  {
    CodingParameters cp;
    ec->get_coding_parameters(cp);
    return ec_factory(ec_type, cp);
  }

  void parse_args(Logger* logger, ParametersInfo& paras, std::string config_file)
  {
    std::unordered_map<std::string, std::string> config;
    std::ifstream file(config_file);
    if (file.is_open()) {
      std::string line;
      while (std::getline(file, line)) {
        if (line[0] == '[' || line[0] == '#' || line.empty())
          continue;

        std::stringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
          // remove space around '='
          auto new_key_end = std::remove_if(key.begin(), key.end(),
                  [](unsigned char ch) { return std::isspace(ch); });
          key.erase(new_key_end, key.end());
          auto new_val_end = std::remove_if(value.begin(), value.end(),
              [](unsigned char ch) { return std::isspace(ch); });
          value.erase(new_val_end, value.end());
          config[key] = value;
        }
      }
      file.close();
    } else {
      std::cerr << "Unable to open " << config_file << std::endl;
      return;
    }

    paras.partial_decoding = (config["partial_decoding"] == "true");
    paras.partial_scheme = (config["partial_scheme"] == "true");
    paras.repair_priority = (config["repair_priority"] == "true");
    paras.repair_method = (config["repair_method"] == "true");
    paras.from_two_replica = (config["from_two_replica"] == "true");
    paras.is_ec_now = (config["is_ec_now"] == "true");
    std::string temp = config["ec_type"];
    if (ec_map.find(temp) == ec_map.end()) {
      std::cerr << "Unknown ec_type: " << temp << std::endl;
      exit(0);
    } else {
      paras.ec_type = ec_map.at(temp);
    }
    temp = config["placement_rule"];
    if (pr_map.find(temp) == pr_map.end()) {
      std::cerr << "Unknown placement_rule: " << temp << std::endl;
      exit(0);
    } else {
      paras.placement_rule = pr_map.at(temp);
    }
    temp = config["placement_rule_for_trans"];
    if (pr_map.find(temp) == pr_map.end()) {
      std::cerr << "Unknown placement_rule: " << temp << std::endl;
      exit(0);
    } else {
      paras.placement_rule_for_trans = pr_map.at(temp);
    }
    temp = config["multistripe_placement_rule"];
    if (mspr_map.find(temp) == mspr_map.end()) {
      std::cerr << "Unknown multistripe_placement_rule: " << temp << std::endl;
      exit(0);
    } else {
      paras.multistripe_placement_rule = mspr_map.at(temp);
    }
    paras.block_size = std::stoul(config["block_size"]) * 1024;
    paras.cp.x = std::stoi(config["x"]);
    paras.cp.k = std::stoi(config["k"]);
    paras.cp.m = std::stoi(config["m"]);
    paras.cp.l = std::stoi(config["l"]);
    paras.cp.g = std::stoi(config["g"]);
    paras.cp.k1 = std::stoi(config["k1"]);
    paras.cp.m1 = std::stoi(config["m1"]);
    paras.cp.k2 = std::stoi(config["k2"]);
    paras.cp.m2 = std::stoi(config["m2"]);
    paras.cp.storage_overhead = float(paras.cp.k + paras.cp.m) / float(paras.cp.k);

    if (check_ec_family(paras.ec_type) == LRCs) {
      paras.cp.m = paras.cp.l + paras.cp.g;
    } else if (check_ec_family(paras.ec_type) == PCs) {
      if (paras.ec_type == HV_PC) {
        paras.cp.m = paras.cp.k1 * paras.cp.m2 + paras.cp.k2 * paras.cp.m1;
      } else {
        paras.cp.m = paras.cp.k1 * paras.cp.m2 + paras.cp.k2 * paras.cp.m1
            + paras.cp.m1 * paras.cp.m2;
      }
    }

    std::string str = "*-*-*-*-*-*-*-EC-Info-*-*-*-*-*-*-*\n";
    str += "Partial?:" + std::to_string(paras.partial_decoding)
              + " P-S:" + std::to_string(paras.partial_scheme)
              + " R-P:" + std::to_string(paras.repair_priority)
              + " R-M:" + std::to_string(paras.repair_method) + "\n";
    str += config["ec_type"] + " " + config["placement_rule"] + "(s) "
              + config["multistripe_placement_rule"] + "(m)\n";
    if (check_ec_family(paras.ec_type) == LRCs) {
      str += "(" + std::to_string(paras.cp.k) + "," + std::to_string(paras.cp.l)
                + "," + std::to_string(paras.cp.g) + ") ";
    } else if (check_ec_family(paras.ec_type) == PCs) {
      str += "(" + std::to_string(paras.cp.k1) + "," + std::to_string(paras.cp.m1)
             + "," + std::to_string(paras.cp.k2) + "," + std::to_string(paras.cp.m2) + ") ";
    } else {
      str += "(" + std::to_string(paras.cp.k) + "," + std::to_string(paras.cp.m) + ") ";
    }
    str += std::to_string(paras.cp.x) + "(x) "
              + std::to_string(paras.block_size) + "(bytes)\n";
    str += "*-*-*-*-*-*-*-EC-Info-*-*-*-*-*-*-*-*\n";
    if (logger == nullptr || !IF_LOG_TO_FILE) {
      std::cout << str << std::endl;
    } else {
      logger->log(Logger::LogLevel::DEBUG, str);
    }
  }

  int stripe_wide_after_merge(ParametersInfo paras, int step_size)
  {
     ECFAMILY ec_family = check_ec_family(paras.ec_type);
    if (ec_family == RSCodes) {
      paras.cp.k *= step_size;
      return paras.cp.k + paras.cp.m;
    } else if (ec_family == LRCs) {
      paras.cp.k *= step_size;
      paras.cp.l *= step_size;
      return paras.cp.k + paras.cp.l + paras.cp.g;
    } else {
      if (paras.multistripe_placement_rule == VERTICAL) {
        paras.cp.k2 *= step_size;
      } else {
        paras.cp.k1 *= step_size;
      }
      if (paras.ec_type == HV_PC) {
        return (paras.cp.k1 + paras.cp.m1) * (paras.cp.k2 + paras.cp.m2)
            - paras.cp.m1 * paras.cp.m2;
      }
      return (paras.cp.k1 + paras.cp.m1) * (paras.cp.k2 * paras.cp.m2);
    }
  }

  std::string getStartTime() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M", std::localtime(&now));
    return std::string(buf);
  }

  void deepcopy_codingparameters(const CodingParameters& src_cp, CodingParameters& des_cp) {
    des_cp.k = src_cp.k;
    des_cp.m = src_cp.m;
    des_cp.l = src_cp.l;
    des_cp.g = src_cp.g;
    des_cp.k1 = src_cp.k1;
    des_cp.m1 = src_cp.m1;
    des_cp.k2 = src_cp.k2;
    des_cp.m2 = src_cp.m2;
    des_cp.x = src_cp.x;
    des_cp.seri_num = src_cp.seri_num;
    des_cp.storage_overhead = src_cp.storage_overhead;
    des_cp.local_or_column = src_cp.local_or_column;
    des_cp.krs.clear();
    for (auto pair : src_cp.krs) {
      des_cp.krs.emplace_back(pair);
    }
  }
}
