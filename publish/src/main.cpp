#include <iostream>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

#include <wampcc/wampcc.h>

int main(int argc, char* argv[])
{
    CLI::App app{"Utility to send fast publications to benchmark wamplv"};

    std::string ip;
    std::string realm;
    std::string uri;
    uint count;
    unsigned short port;

    app.add_option("--ip", ip, "WAMP router address")->required();
    app.add_option("--port", port, "WAMP router port")->default_val(55555);
    app.add_option("--count", count, "Number of publications")->default_val(1E5);
    app.add_option("--realm", realm, "WAMP router realm")->default_val("default_realm");
    app.add_option("--uri", uri, "URI to publish to ")->default_val("com.wamplv_benchmark.publish");

    CLI11_PARSE(app, argc, argv);

    // WAPCC kernel
    std::unique_ptr<wampcc::kernel> the_kernel( new wampcc::kernel({}, wampcc::logger::nolog() ));

    /* Create the TCP socket and attempt to connect. */
    std::unique_ptr<wampcc::tcp_socket> sock(new wampcc::tcp_socket(the_kernel.get()));
    auto fut = sock->connect(ip, port);

    if (fut.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
      throw std::runtime_error("timeout during connect");

    if (wampcc::uverr ec = fut.get())
      throw std::runtime_error("connect failed: " + std::to_string(ec.os_value()) + ", " + ec.message());

    /* Using the connected socket, now create the wamp session object. */
    std::promise<void> ready_to_exit;
    std::shared_ptr<wampcc::wamp_session> session = wampcc::wamp_session::create<wampcc::websocket_protocol>(
      the_kernel.get(),
      std::move(sock),
      [&ready_to_exit](wampcc::wamp_session&, bool is_open){
        if (!is_open)
          try {
            ready_to_exit.set_value();
          } catch (...) {/* ignore promise already set error */}
      },
      {});

    session->hello(realm).wait_for(std::chrono::seconds(3));

    if (!session->is_open())
      throw std::runtime_error("realm logon failed");

    wampcc::json_object opt({{"acknowledge", wampcc::json_value(false)}});

    auto exit_fut = ready_to_exit.get_future();
    for(uint i =0; i < count; i++)        
        if( exit_fut.wait_for(std::chrono::microseconds(0)) != std::future_status::ready )
        {
            wampcc::wamp_args args;
            args.args_list.push_back(i);
            session->publish(uri, opt, std::move(args));
        }
    session->close();

    return 0;
}
