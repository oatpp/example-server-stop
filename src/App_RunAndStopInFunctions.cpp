#include "./controller/MyController.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>

/* Could be implemented as class but must be kept singleton if no parallelization preparations were done */
namespace MyOatppFunctions {

bool server_running = false;
std::mutex server_op_mutex;
std::atomic_bool server_should_continue;
std::thread oatppThread;

/**
 * You can't run two of those threads in one application concurrently in this setup. Especially with the AppComponents inside the
 * Threads scope. If you want to run multiple threads with multiple servers you either have to manage the components
 * by yourself and do not rely on the OATPP_COMPONENT mechanism or have one process-global AppComponent.
 * Further you have to make sure you don't have multiple ServerConnectionProvider listening to the same port.
 */
void StartOatppServer() {
  std::lock_guard<std::mutex> lock(server_op_mutex);

  /* Check if server is already running, if so, do nothing. */
  if (server_running) {
    return;
  }

  /* Signal that the server is running */
  server_running = true;

  /* Tell the server it should run */
  server_should_continue.store(true);

  oatppThread = std::thread([] {
    /* Register components in scope of thread WARNING: COMPONENTS ONLY VALID WHILE THREAD IS RUNNING! */
    AppComponent components;

    /* Get router component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

    /* Create MyController and add all of its endpoints to router */
    router->addController(std::make_shared<MyController>());

    /* Get connection handler component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler);

    /* Get connection provider component */
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

    /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
    oatpp::network::Server server(connectionProvider, connectionHandler);

    /* Run server, let it check a lambda-function if it should continue to run
     * Return true to keep the server up, return false to stop it.
     * Treat this function like a ISR: Don't do anything heavy in it! Just check some flags or at max some very
     * lightweight logic.
     * The performance of your REST-API depends on this function returning as fast as possible! */
    std::function<bool()> condition = [](){
      return server_should_continue.load();
    };

    /* Print info about server port */
    OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

    server.run(condition);

    /* Server has shut down, so we dont want to connect any new connections */
    connectionProvider->stop();

    /* Now stop the connection handler and wait until all running connections are served */
    connectionHandler->stop();
  });
}

void StopOatppServer() {
  std::lock_guard<std::mutex> lock(server_op_mutex);

  /* Tell server to stop */
  server_should_continue.store(false);

  /* Wait for the server to stop */
  if (oatppThread.joinable()) {
    oatppThread.join();
  }
}

}


void myBackendLogicDummy() {
  OATPP_LOGI("MyBackend", "Press enter to continue the loop");
  std::cin.ignore();
}

/**
 * This example shows how to start the server and stop it gracefully with a call to stop().
 * You are free to have the components, server and controller in your main stack and just run the server in its own
 * thread like in the StopSimple example and still use the condition function.
 */
void run() {

 MyOatppFunctions::StartOatppServer();

 myBackendLogicDummy();

 MyOatppFunctions::StopOatppServer();

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
