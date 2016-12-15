#include "bilinear_sampler.h"
#include "matrix.h"
#include "image.h"
#include "image_saver.h"
#include <memory>

using namespace img_processing;
using namespace std;

float degree_to_radians(const float angle) {
	const auto PI = 3.14159265358979323846f /180;
	return angle * PI;
}

int main() {

	ComInitialize com;
	
	//! load a image file
	auto src_image_path = LR"(n:\ab.jpg)";
	auto img = imager{};
	
	BYTE* input_img_data;
	auto const img_info = img.load_image(src_image_path, &input_img_data);
	auto src_img_obj = image_t<byte_t>(img_info.width, img_info.height, img_info.channel_count);
	src_img_obj.reference_from(input_img_data);

	//! create transformation matrix
	auto mat = matrix3x2<float>::rotation(degree_to_radians(45)) * matrix3x2<float>::scale(0.5, 0.5);
	
	//! transform the image
	auto dest_img_obj = image_t<byte_t>{};
	transform_pixels(src_img_obj, dest_img_obj, mat);

	//! save the image
	auto dest_image_path = LR"(n:\)";
	img.save_image(dest_image_path, dest_img_obj.get(), dest_img_obj.get_width(), dest_img_obj.get_height());

	
	delete input_img_data;
	
}