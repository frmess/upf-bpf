#include "Controller.h"
#include <UserPlaneComponent.h>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <utils/LogDefines.h>
#include <string>     // std::string, std::stoul, std::getline

using json = nlohmann::json;

int main(int argc, char *argv[])
{
  using namespace httplib;

  Server svr;

  // Check the number of parameters
  if(argc < 3) {
    // Tell the user how to run the program
    std::cerr << "Usage: " << argv[0] << " <ENDPOINT> <PORT>" << std::endl;
    return 1;
  }

  char* pAddress = argv[1];
  uint32_t port = std::stoul(argv[2]);

  LOG_INF("Running server on endpoint {}:{}", pAddress, port);

  svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
  res.set_content("Hello World!", "text/plain");
  });

  svr.Post("/configure", [](const Request& req, Response& res) {
    json jRequest = json::parse(req.body);
    json jResponse;
    res.status = Controller::setup(jRequest, jResponse);
    res.set_content("{\"teste\": \"Hello World!\"s}", "application/json");
    //res.set_content("{\"ok 200\": \"Interfaces have been configured!\"s}", "application/json");
  });

  svr.Post("/createSession", [](const Request &req, Response &res) {
    json jRequest = json::parse(req.body);
    json jResponse;
    res.status = Controller::createSesssion(jRequest, jResponse);
    res.set_content("{\"teste\": \"Hello World!\"s}", "application/json");
    //res.set_content("{\"ok 200\": \"PDU Session has been created!\"s}", "application/json");
  });

  svr.listen(pAddress, port);
}