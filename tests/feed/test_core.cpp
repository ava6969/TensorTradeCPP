////
//// Created by dewe on 7/3/21.
////
//
//#include "catch.hpp"
//#include "../../feed/core/base.h"
//#include "../../feed/core/feed.h"
//
//
//TEST_CASE("Named API init")
//{
//    ttc::Named named, named2;
//
//    REQUIRE(named.Name() == "generic");
//    named.rename("price");
//
//    REQUIRE(named.Name() == "price");
//
//    {
//        ttc::NameSpace nameSpace("price");
//        named.rename("close");
//        {
//            ttc::NameSpace p1("quote");
//            named2.rename("high");
//        }
//    }
//
//    REQUIRE(named.Name() == "price:/close");
//    REQUIRE(named2.Name() == "quote:/high"); // cant be nested namespaces
//    std::cout << named.Name() << "\n";
//    std::cout << named2.Name() << "\n";
//}
//
//
//TEST_CASE("Constant Stream ")
//{
//
//    auto f1 = ttc::constant(20);
//    REQUIRE(f1->forward_T() == 20);
//
//}
//
//TEST_CASE("Iterable Stream ")
//{
//
//    ttc::NameSpace n{"AAPL"};
//    auto f1 = ttc::source<float>({3.15f, 2.11f, 2.15f, 1.0f})
//
//    REQUIRE(f1.Name() == "iterable");
//    f1.rename("close");
//    REQUIRE(f1.Name() == "AAPL:/close");
//    REQUIRE(f1.forward_as<float>()  == 3.15f);
//
//    REQUIRE(f1.forward_as<float>()  == 2.11f);
//    REQUIRE(f1.forward_as<float>()  == 2.15f);
//    REQUIRE(f1.forward_as<float>()  == 1.0f);
//    f1.forward_as<float>();
//    REQUIRE(not f1.has_next());
//
//    f1.reset();
//    REQUIRE(f1.forward_as<float>() == 3.15f);
//    REQUIRE(f1.has_next());
//}
//
//TEST_CASE("Group Stream ")
//{
//    ttc::NameSpace n{"AAPL"};
//    ttc::IterableStream close(torch::tensor({3.15, 2.11, 2.15, 1.0}));
//
//    close.rename("close", "::");
//    ttc::IterableStream open(torch::tensor({1.15, 3.11, 12.15, 11.0}));
//    close.rename("open", "::");
//
////    ttc::IterableStream<string> ts(std::vector<string>{"12:00", "12:01", "12:02", "12:03"});
//
//    ttc::Group group;
//    std::vector<ttc::Stream*> _input{&close, &open};
//    auto* g = group(_input);
//
//    auto res = g->forward();
//
//}
//
//TEST_CASE("Data Feed Compilation ")
//{
//    ttc::NameSpace n{"AAPL"};
//    ttc::IterableStream close(torch::tensor({3.15, 2.11, 2.15, 1.0}));
//    ttc::IterableStream open(torch::tensor({1.15, 3.11, 12.15, 11.0}));
//    std::vector<ttc::Stream* > _input{&close, &open};
//    close.rename("close", "::");
//    open.rename("open", "::");
//    ttc::DataFeed df(_input);
//
//    df.compile();
//
//    for(int i = 0; i < 4; i++)
//    {
//        std::cout << df.next() << "\n";
//    }
//    df.reset();
//
//    for(int i = 0; i < 4; i++)
//    {
//        std::cout << df.next() << "\n";
//    }
//
//}
//
//TEST_CASE("Sensor Stream ")
//{
//    ttc::NameSpace n{"AAPL"};
//
//    ttc::IterableStream close(torch::tensor({3.15, 2.11, 2.15, 1.0}));
//    ttc::IterableStream open(torch::tensor({1.15, 3.11, 12.15, 11.0}));
//    ttc::Sensor s1(&close, [](ttc::Stream* s) { return std::get<torch::Tensor>(s->Value()).pow(2); });
//
//    close.rename("close");
//    open.rename("open");
//    s1.rename("close_sqr");
//
//    ttc::DataFeed df({&close, &open, &s1});
//    df.compile();
//
//    for(int i = 0; i < 4; i++)
//    {
//        std::cout << df.next() << "\n";
//    }
//
//}