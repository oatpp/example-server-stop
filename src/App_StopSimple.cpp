#include "./controller/MyController.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>

void myBackendLogicDummy() {
    OATPP_LOGI("MyBackend", "Press enter to continue the loop");
    std::cin.ignore();
}

/**
 * This example shows how to start the server and stop it gracefully with a call to stop().
 */
void run() {

  /* Register Components in scope of run() method */
  AppComponent components;

  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  /* Create MyController and add all of its endpoints to router */
  auto myController = std::make_shared<MyController>();
  myController->addEndpointsToRouter(router);

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::Server server(connectionProvider, connectionHandler);

  std::thread oatppThread([&server] {
    /* Run server */
    server.run();
  });

  /* Print info about server port */
  OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

  /* ToDo: Call your logic here! We are just calling some blocking dummy logic here */
  myBackendLogicDummy();

  /* Check if server is still running and stop it if needed */
  if (server.getStatus() == oatpp::network::Server::STATUS_RUNNING) {
    server.stop();
  }

  /* Check if the thread has already stopped or if we need to wait for the server to stop */
  if(oatppThread.joinable()) {

    /* We need to wait until the thread is done */
    oatppThread.join();
  }

}

/**
 *  main
 */
int main(int argc, const char * argv[]) {

  oatpp::base::Environment::init();

  run();
  
  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";
  
  oatpp::base::Environment::destroy();
  
  return 0;
}
