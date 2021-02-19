#include "./controller/MyController.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>

std::atomic_bool server_should_continue(true);

void myBackendLogicDummy() {
  std::cout << "Press enter to shut down" << std::endl;
  std::cin.ignore();
}

/**
 * This example shows how to keep Oat++ entirely in its own thread and let it stop by a check condition. This might be
 * a more straight forward solution compared to calling `server->stop()`.
 * Some may encounter the situation where the whole Oat++ runtime AND environment should be enclosed in a single thread
 * and its stack. A "full enclosure".
 * WARNING: This also encapsulates the Oat++ environment in the thread. Thus other Oat++ mechanisms like logging is only
 * available while the thread is running. However, you should NOT rely on using Oat++ environment mechanisms outside
 * of the Oat++ part of your project. If you still want to use Oat++ Logging and other Oat++ functions, call the
 * environments init and destroy in main or their respective scope.
 */
void run() {

  std::thread oatppThread([] {

    /* Init Oat++ Environment in the scope of the thread */
    oatpp::base::Environment::init();

    /* Have the thread logic in a sub-scope so every Oat++ object is destroyed when we destroy the environment on thread close */
    {
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
      auto serverPtr = oatpp::network::Server::createShared(connectionProvider, connectionHandler);

      /* Print info about server port */
      OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

      /* Run server, let it check a lambda-function if it should continue to run
       * Return true to keep the server up, return false to stop it.
       * Treat this function like a ISR: Don't do anything heavy in it! Just check some flags or at max some very
       * lightweight logic.
       * The performance of your REST-API depends on this function returning as fast as possible! */
      std::function<bool()> condition = [](){
        return server_should_continue.load();
      };

      /* Run server */
      serverPtr->run(condition);
    }

    /* Print how much objects were created during app running, and what have left-probably leaked */
    /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
    std::cout << "\nEnvironment:\n";
    std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
    std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";

    oatpp::base::Environment::destroy();
  });

  /* ToDo: Call your logic here! We are just calling some blocking dummy logic here */
  myBackendLogicDummy();

  /* Signal the stop condition */
  server_should_continue.store(false);

  /* Check if we have already stopped or if we need to wait for the server to stop */
  if(oatppThread.joinable()) {

    /* We need to wait until the thread is done */
    oatppThread.join();
  }
  
}

/**
 *  main
 */
int main(int argc, const char * argv[]) {

  run();

  return 0;
}
