
#include "../filters/aes_encryption.hpp"
#include <rlib/stdio.hpp>

int main() {
	Filters::AESFilter f;
	f.loadConfig("aes@hello world");


    std::string plain = "Hi, I'm asoinvdowaieviosandoiv dasf sda ifsdh ofisdaf oisdfoisanoids nfoisdoafnsdoif nsdo fisdnio |";
    auto r = f.convertForward(plain);
    rlib::println(plain);
    rlib::println(r);
    auto p = f.convertBackward(r);
    rlib::println(p);
	

}


