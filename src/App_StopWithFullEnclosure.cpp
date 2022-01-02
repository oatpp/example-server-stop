#include "./controller/MyController.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>

void myBackendLogicDummy() {
  std::cout << "Press enter to shut down" << std::endl;
  std::cin.ignore();
}

/**
 * This example shows how to start the server without and stop it with a call to server::stop();
 * Some may encounter the situation where the whole Oat++ runtime AND environment should be enclosed in a single thread
 * and its stack. A "full enclosure".
 * WARNING: This also encapsulates the Oat++ environment in the thread. Thus other Oat++ mechanisms like logging is only
 * available while the thread is running. However, you should NOT rely on using Oat++ environment mechanisms outside
 * of the Oat++ part of your project. If you still want to use Oat++ Logging and other Oat++ functions, call the
 * environments init and destroy in main or their respective scope.
 */
void run() {
  /* In this example, we keep the server in a shared pointer and pass a weak pointer to the thread where
   * it should place a reference to the server. */
  std::weak_ptr<oatpp::network::Server> weakServerPtr;

  /* Optional race-condition prevention, see big comment further down */
  std::condition_variable race_guard;
  std::mutex race_guard_mutex;
  std::unique_lock<std::mutex> race_guard_lock(race_guard_mutex);
  bool ready = false;

  /**
   * You can't run two of those threads in one application concurrently in this setup. Especially with the AppComponents inside the
   * Threads scope. If you want to run multiple threads with multiple servers you either have to manage the components
   * by yourself and do not rely on the OATPP_COMPONENT mechanism or have one process-global AppComponent.
   * Further you have to make sure you don't have multiple ServerConnectionProvider listening to the same port.
   */
  std::thread oatppThread([&weakServerPtr, &race_guard, &ready] {

    /* Init Oat++ Environment in the scope of the thread */
    oatpp::base::Environment::init();

    /* Have the thread logic in a sub-scope so every Oat++ object is destroyed when we destroy the environment on thread close */
    {
      /* Register Components in scope of run() method */
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
      auto serverPtr = oatpp::network::Server::createShared(connectionProvider, connectionHandler);

      weakServerPtr = serverPtr;

      /* Unlock the race-guard */
      ready = true;
      race_guard.notify_one();

      /* Print info about server port */
      OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

      /* Run server */
      serverPtr->run();

      /* Server has shut down, so we dont want to connect any new connections */
      connectionProvider->stop();

      /* Now stop the connection handler and wait until all running connections are served */
      connectionHandler->stop();
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

  /*
   * Warning:
   * Keep in mind we can have a race-condition here. If myBackendLogicDummy() exits before the weak pointer is assigned
   * the pointer appears expired and stop() will never be called. Therefore it is advised to have i.E. a condition
   * variable to prevent this kind of situation. In C++20, a std::binary_semaphore could be used for less lines of code.
   * This is optional if you are 100% positive that your logic will never return this quickly.
   * Also, if you need to have Oat++ up and running before your logic starts, you can move this
   */
  race_guard.wait(race_guard_lock, [&ready]{return ready;});

  /* Check if the weak pointer to the server instance still valid */
  if (!weakServerPtr.expired()) {
    /* Pointer still valid, lock the pointer send the stop-command */
    weakServerPtr.lock()->stop();
  }

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
