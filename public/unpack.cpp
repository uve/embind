#include <iostream>
#include <sstream>
#include <cassert>
#include <msgpack.hpp>

#include <random>

#include <sys/resource.h>



/*

 Compile:
./native 
or
./wasm 
*/


const long ARRAY_SIZE = 2*1000;


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



struct MyResult {
    MyType data[ARRAY_SIZE];
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

    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(25, 63); // define the range
    std::uniform_real_distribution<> distr_real(-1, 1); // define the range

    time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

//std::array<MyType,size> msgs_unpack()
MyResult msg_unpack()
{

    // packing
    std::stringstream ss;

 
    std::array<MyType, ARRAY_SIZE> m;


    std::cout << "ARRAY_SIZE = " << ARRAY_SIZE << std::endl;



    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Generation time = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;


    /****************************************/
    begin = std::chrono::steady_clock::now();


    for (int i = 0; i < ARRAY_SIZE; i++) {
        
        m[i] = MyType{ distr(gen), now_time, distr(gen), now_time, distr(gen), distr(gen), 1,
                    gen_random(distr(gen)), gen_random(distr(gen)),
                    distr_real(gen), distr_real(gen)};
                    

        if (i < 1) {
            
            std::cout << "****** Generated data ******" << std::endl;
            std::cout << "Column1 = " << m[i].Column1 << std::endl;
            std::cout << "str1 = " << m[i].str1 << std::endl;
            std::cout << "start_station_code = " << m[i].start_station_code << std::endl;
            std::cout << "****************************" << std::endl;
            
        }

         //pt.put (item);
    }


    msgpack::pack(ss, m);

    std::cout << "object lenght: " << convertSize(ss.str().size()) << std::endl;

    std::cout << "Pack time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << "[ms]" << std::endl;

    //std::cout << ss.str() << std::endl;

    /***************************************/
    
    begin = std::chrono::steady_clock::now();

    auto oh = msgpack::unpack(ss.str().data(), ss.str().size());

    std::cout << "Unpack time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << "[ms]" << std::endl;

    /****************************************/
    begin = std::chrono::steady_clock::now();
    
    // converting
    msgpack::object const &obj = oh.get();

    std::array<MyType, ARRAY_SIZE> v2 = obj.as<std::array<MyType, ARRAY_SIZE > >();

    std::cout << "Cast time time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << "[ms]" << std::endl;


    /****************************************/
    begin = std::chrono::steady_clock::now();
    

    MyResult d = MyResult{};

    for (int i=0; i < ARRAY_SIZE; i++) {
        d.data[i] = v2[i];
    }

    std::cout << "Copy array time = " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count() << "[ms]" << std::endl;


    return d;
    //return v2;
   
}


int main() {

    

#ifdef EMSCRIPTEN
#else
    msg_unpack();
    #endif
}


#ifdef EMSCRIPTEN
#include <emscripten/bind.h>
using namespace emscripten;


template<typename ArrayT, size_t N>
struct ArrayInitializer : public ArrayInitializer<ArrayT, N-1>
{
    explicit ArrayInitializer(emscripten::value_array<ArrayT>& arr) : ArrayInitializer<ArrayT, N-1>{arr}
    {
        arr.element(emscripten::index<N-1>());
    }
};

template<typename ArrayT>
struct ArrayInitializer<ArrayT, 0>
{
    explicit ArrayInitializer(emscripten::value_array<ArrayT>& arr)
    {
    }
};


EMSCRIPTEN_BINDINGS(module) {

    function("msg_unpack", &msg_unpack);


    value_object<MyResult>("MyResult")
        .field("data", &MyResult::data)
        ;

  
    // Register std::array<int, 2> because ArrayInStruct::field is interpreted as such
    value_array<std::array<MyType, 2>>("array_MyType_2")
        .element(emscripten::index<0>())
        .element(emscripten::index<1>())
        ;
        


    // Register array of Pixels as a Scanline
    value_array<std::array<MyType, ARRAY_SIZE>> MyType_value_array("MyType");
    ArrayInitializer<std::array<MyType, ARRAY_SIZE>, ARRAY_SIZE>{MyType_value_array};


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


   
}



#endif


