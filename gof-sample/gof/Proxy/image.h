#ifndef __Proxy_Image_Inc_H__
#define __Proxy_Image_Inc_H__

#include "../stdafx.h"

namespace Proxy {

class Image {
public:
	virtual ~Image() { }
    CLASS_PTR(Image);

	virtual void display() = 0;
};

class RealImage : public Image {
public:
	RealImage(std::string& file) {
		filename_ = file;
		loadFromDisk();
	}

	void display() override {
		std::cout << "display real image." << std::endl;
	}

private:
	void loadFromDisk() {
		std::cout << "load " << filename_ << " from disk." << std::endl;
	}

private:
	std::string filename_;
};

class ProxyImage : public Image {
public:
	ProxyImage(std::string filename) {
        filename_ = filename;
	}

	void display() override {
        // 延迟实例化
        if (!real_img_) {
            real_img_ = std::make_shared<RealImage>(filename_);
        }
        real_img_->display();
	}

private:
    std::string filename_;
	Image::Ptr real_img_;
};

void test() {
    std::cout << "\n\n proxy pattern." << std::endl;

    auto image = std::make_shared<ProxyImage>("beautiful.jpg");
    image->display();
}

}

#endif //  __Proxy_Image_Inc_H__
