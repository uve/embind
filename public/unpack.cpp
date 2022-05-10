#include <iostream>
#include <sstream>
#include <cassert>
#include <msgpack.hpp>

#include <random>




/*

 Compile:

emcc -lembind -o unpack.js unpack.cpp  -I /usr/local/include/ -I  /opt/homebrew/Cellar/boost/1.78.0_1/include  -Wall -Wextra -std=gnu++17  -sALLOW_MEMORY_GROWTH -O3

g++ unpack.cpp -I /usr/local/include/ -I  /opt/homebrew/Cellar/boost/1.78.0_1/include  -Wall -Wextra -std=gnu++17 -O3 && ./a.out          

*/
//using namespace std;


struct buffer {
  unsigned int pointer;
  unsigned int size;
};

struct MyType {
    int Column1;
    time_t start_date;
    int start_station_code;
    time_t end_date;
    int end_station_code;
    int duration_sec;
    int is_member;
    std::string str1;
    std::string str2;
    double float1;
    double float2;
    // You can choose any order. It is represented to the msgpack array elements order.
    MSGPACK_DEFINE_MAP(Column1, start_date,start_station_code,  end_date,
                    end_station_code, duration_sec, is_member,
                    str1, str2,
                    float1, float2);
};


// Utility functions:

std::string convertToString(double num) {
    std::ostringstream convert;
    convert << num;
    return convert.str();
}

double roundOff(double n) {
    double d = n * 100.0;
    int i = d + 0.5;
    d = (float)i / 100.0;
    return d;
}

std::string convertSize(size_t size) {              
    static const char *SIZES[] = { "B", "KB", "MB", "GB" };
    unsigned long div = 0;
    size_t rem = 0;

    while (size >= 1024 && div < (sizeof SIZES / sizeof *SIZES)) {
        rem = (size % 1024);
        div++;
        size /= 1024;
    }

    double size_d = (float)size + (float)rem / 1024.0;
    std::string result = convertToString(roundOff(size_d)) + " " + SIZES[div];
    return result;
}

std::string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

//std::vector<MyType > msg_unpack(int size)

buffer msg_unpack(int size)
{

    buffer myBuffer;
   

    time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // packing
    std::stringstream ss;
    std::vector<MyType > m {
    };


    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(25, 63); // define the range
    std::uniform_real_distribution<> distr_real(-1, 1); // define the range

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Generation time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;


    /****************************************/
    begin = std::chrono::steady_clock::now();


    for (int i = 0; i < size; i++) {
        m.push_back(
            { distr(gen), now_time, distr(gen), now_time, distr(gen), distr(gen), 1,
                    gen_random(distr(gen)), gen_random(distr(gen)),
                    distr_real(gen), distr_real(gen)}
        );

        if (i == 0) {
            std::cout << "****** Generated data ******" << std::endl;
            std::cout << "Column1 = " << m.front().Column1 << std::endl;
            std::cout << "str1 = " << m.front().str1 << std::endl;
            std::cout << "start_station_code = " << m.front().start_station_code << std::endl;
            std::cout << "****************************" << std::endl;
        }

         //pt.put (item);
    }

    msgpack::pack(ss, m);

    std::cout << "object lenght: " << convertSize(ss.str().size()) << std::endl;

    std::cout << "Pack time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << "[ms]" << std::endl;

    /****************************************/
    begin = std::chrono::steady_clock::now();

    auto oh = msgpack::unpack(ss.str().data(), ss.str().size());

    std::cout << "Unpack time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << "[ms]" << std::endl;

    /****************************************/
    begin = std::chrono::steady_clock::now();
    
    // converting
    msgpack::object const &obj = oh.get();
    auto v2 = obj.as<std::vector<MyType > >();

    std::cout << "Cast time time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << "[ms]" << std::endl;

    //return v2;

    MyType * myTab = (MyType*)malloc(size * sizeof(MyType));

    for (int i = 0; i < size; i++) {
      myTab[i] = v2.front();
      //printf(" Native side index: %d value: %f address: %p\n", i, myTab[i], &myTab[i]);
    }

    myBuffer.pointer = (unsigned int) myTab; 
    myBuffer.size = size;

    printf(" Native side pointer: %p size: %d\n", myTab, size);
    return myBuffer;
}
int main() {


#ifdef EMSCRIPTEN
#else
    msg_unpack(500*1000);
    #endif
}


#ifdef EMSCRIPTEN
#include <emscripten/bind.h>
using namespace emscripten;



EMSCRIPTEN_BINDINGS(module) {

        value_array<buffer>("buffer")
        .element(&buffer::pointer)
        .element(&buffer::size)
        ;



    function("msg_unpack", &msg_unpack, allow_raw_pointers());


    value_object<MyType>("MyType")
        .field("Column1", &MyType::Column1)
        .field("start_date", &MyType::start_date)
        .field("start_station_code", &MyType::start_station_code)
        .field("end_date", &MyType::end_date)
        .field("end_station_code", &MyType::end_station_code)
        .field("duration_sec", &MyType::duration_sec)
        .field("is_member", &MyType::is_member)
        .field("str1", &MyType::str1)
        .field("str2", &MyType::str2)
        .field("float1", &MyType::float1)
        .field("float2", &MyType::float2)
        ;



    register_vector<MyType>("vector<MyType>");
}



#endif


