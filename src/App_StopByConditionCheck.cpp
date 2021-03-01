#include "./controller/MyController.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>

std::atomic_bool server_should_continue(true);

void myBackendLogicDummy() {
    OATPP_LOGI("MyBackend", "Press enter to continue the loop");
    std::cin.ignore();
}

/**
 * This example shows how to start the server and stop it gracefully with a condition function.
 * You are free to have the components, server and controller in your main stack and just run the server in its own
 * thread like in the StopSimple example and still use the condition function.
 */
void run() {

  /* Register components in scope of run() */
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

    /* Run server, let it check a lambda-function if it should continue to run
     * Return true to keep the server up, return false to stop it.
     * Treat this function like a ISR: Don't do anything heavy in it! Just check some flags or at max some very
     * lightweight logic.
     * The performance of your REST-API depends on this function returning as fast as possible! */
    std::function<bool()> condition = [](){
      return server_should_continue.load();
    };

    server.run(condition);
  });

  /* Print info about server port */
  OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

  /* ToDo: Call your logic here! We are just calling some blocking dummy logic here */
  myBackendLogicDummy();

  /* First, stop the ServerConnectionProvider so we don't accept any new connections */
  connectionProvider->stop();

  /* Signal the stop condition */
  server_should_continue.store(false);

  /* Finally, stop the ConnectionHandler and wait until all running connections are closed */
  connectionHandler->stop();

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
