/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include <string>
#include <vector>
#include <appbase/application.hpp>
#include <eosio/launcher_service_plugin/httpc.hpp>

#include <fc/static_variant.hpp>
#include <fc/crypto/private_key.hpp>
#include <fc/crypto/public_key.hpp>

using public_key_type = fc::crypto::public_key;
using private_key_type = fc::crypto::private_key;

namespace eosio {

using namespace appbase;

namespace launcher_service {

   struct node_def {
      static const int base_port = 1600;
      static const int cluster_span = 1000;
      static const int node_span = 10;
      static const int max_nodes_per_cluster = 100;
      static const int max_clusters = 30;

      int cluster_id = 0;
      int node_id = 0;
      std::vector<string> producers;
      std::vector<private_key_type> producing_keys;
      std::vector<string> extra_configs;
      bool dont_start = false;

      int http_port() const {
         return base_port + cluster_id * cluster_span + node_id * node_span;
      }
      int p2p_port() const {
         return http_port() + 1;
      }
      bool is_bios() const {
         for (auto &p: producers) {
            if (p == "eosio") return true;
         }
         return false;
      }
   };

   struct cluster_def {
      std::string shape = "mesh";
      int cluster_id = 0;
      int node_count = 0;
      std::vector<node_def> nodes;
      std::vector<string> extra_configs;
      std::string extra_args;

      node_def get_node_def(int id) const {
         for (auto &n: nodes) {
            if (n.node_id == id) return n;
         }
         node_def n;
         n.node_id = id;
         n.cluster_id = cluster_id;
         return n;
      }
   };

   struct launcher_config {
      std::string data_dir = "data-dir";
      std::string host_name = "127.0.0.1";
      std::string listen_addr = "0.0.0.0";
      std::string nodeos_cmd = "./programs/nodeos/nodeos";
      int         log_file_rotate_max = 8;
      private_key_type default_key;

      launcher_config() : default_key(std::string("5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3")) { }
   };

}

/**
 *  This is a template plugin, intended to serve as a starting point for making new plugins
 */
class launcher_service_plugin : public appbase::plugin<launcher_service_plugin> {
public:
   launcher_service_plugin();
   virtual ~launcher_service_plugin();
 
   APPBASE_PLUGIN_REQUIRES()
   virtual void set_program_options(options_description&, options_description& cfg) override;
 
   void plugin_initialize(const variables_map& options);
   void plugin_startup();
   void plugin_shutdown();

   fc::variant get_info(std::string url);
   fc::variant get_cluster_info(int cluster_id);
   fc::variant launch_cluster(launcher_service::cluster_def cluster_def);
   fc::variant stop_all_clusters();

private:
   std::unique_ptr<class launcher_service_plugin_impl>  _my;
};

}

FC_REFLECT(eosio::launcher_service::node_def, (cluster_id)(node_id)(producers)(producing_keys)(extra_configs)(dont_start) )
FC_REFLECT(eosio::launcher_service::cluster_def, (shape)(cluster_id)(node_count)(nodes)(extra_configs)(extra_args) )
