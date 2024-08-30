#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

// 定义一个 Image 类来存储图片数据
class Image {
public:
    int width, height; // 图片的宽度和高度
    vector<uint8_t> data; // 用于存储图片像素的数组，每个像素值为一个字节（8位，范围0-255）

    // 构造函数，用于初始化图片的宽度、高度和像素数组
    Image(int w, int h) : width(w), height(h), data(w* h) {}

    // 返回指定位置 (x, y) 的像素值，允许修改该像素
    uint8_t& at(int x, int y) {
        return data[y * width + x]; // 根据行列计算一维数组的索引
    }

    // 返回指定位置 (x, y) 的像素值，只读
    const uint8_t& at(int x, int y) const {
        return data[y * width + x]; // 根据行列计算一维数组的索引
    }
};

// 从 PGM 文件中加载灰度图片
Image load_pgm(const string& filename) {
    ifstream file(filename, ios::binary); // 以二进制方式打开文件
    if (!file) { // 如果文件无法打开
        cerr << "Cannot open file: " << filename << endl;
        exit(1); // 输出错误信息并退出程序
    }

    string header;
    int width, height, max_val;

    // 读取 PGM 文件的头信息：类型、宽度、高度和最大灰度值
    file >> header >> width >> height >> max_val;
    file.get();  // 读取并跳过一个换行符

    if (header != "P5") { // 检查文件类型是否为 P5，即二进制格式的灰度图像
        cerr << "Invalid PGM file: " << filename << endl;
        exit(1); // 如果文件类型不对，输出错误信息并退出程序
    }

    // 创建一个 Image 对象，宽度和高度分别是从文件头信息中读取的值
    Image image(width, height);

    // 逐像素读取图片数据
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            image.at(x, y) = file.get(); // 从文件中读取一个字节并存储到图片数据中
        }
    }

    return image; // 返回加载的图片对象
}

// 裁剪图片的上下部分
Image crop_image(const Image& image, double top_crop = 0.2, double bottom_crop = 0.2) {
    int top = static_cast<int>(image.height * top_crop); // 计算裁剪的上部分高度
    int bottom = static_cast<int>(image.height * (1 - bottom_crop)); // 计算裁剪的下部分高度
    int new_height = bottom - top; // 新的图片高度

    // 创建一个新的 Image 对象，用于存储裁剪后的图片
    Image cropped(image.width, new_height);

    // 将裁剪后的图片数据复制到新图片中
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            cropped.at(x, y) = image.at(x, y + top); // 复制裁剪后的像素
        }
    }

    return cropped; // 返回裁剪后的图片
}

// 缩放图片的尺寸
Image resize_image(const Image& image, double scale = 0.25) {
    int new_width = static_cast<int>(image.width * scale); // 计算缩放后的宽度
    int new_height = static_cast<int>(image.height * scale); // 计算缩放后的高度

    // 创建一个新的 Image 对象，用于存储缩放后的图片
    Image resized(new_width, new_height);

    // 将缩放后的图片数据复制到新图片中
    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            int src_x = static_cast<int>(x / scale); // 计算源图片中的 x 坐标
            int src_y = static_cast<int>(y / scale); // 计算源图片中的 y 坐标
            resized.at(x, y) = image.at(src_x, src_y); // 复制对应的像素
        }
    }

    return resized; // 返回缩放后的图片
}

// 使用大津法计算图片的二值化阈值
int otsu_threshold(const Image& image) {
    vector<int> histogram(256, 0); // 创建一个大小为256的数组，用于存储像素值的直方图

    // 统计图片中每个灰度级的像素数量
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            histogram[image.at(x, y)]++; // 增加对应灰度值的计数
        }
    }

    int total = image.width * image.height; // 计算图片总像素数
    double sum = 0;
    for (int i = 0; i < 256; ++i) {
        sum += i * histogram[i]; // 计算所有像素的加权和
    }

    double sum_b = 0, max_variance = 0;
    int weight_b = 0, weight_f = 0, threshold = 0;

    // 遍历所有可能的阈值，计算类间方差，找到最佳阈值
    for (int t = 0; t < 256; ++t) {
        weight_b += histogram[t]; // 计算前景像素数
        if (weight_b == 0) continue;

        weight_f = total - weight_b; // 计算背景像素数
        if (weight_f == 0) break;

        sum_b += t * histogram[t]; // 计算前景像素的加权和
        double mean_b = sum_b / weight_b; // 计算前景像素的平均灰度
        double mean_f = (sum - sum_b) / weight_f; // 计算背景像素的平均灰度

        // 计算类间方差
        double variance = weight_b * weight_f * (mean_b - mean_f) * (mean_b - mean_f);
        if (variance > max_variance) {
            max_variance = variance; // 更新最大方差
            threshold = t; // 更新最佳阈值
        }
    }

    return threshold; // 返回计算得到的最佳阈值
}

// 根据阈值将图片进行二值化处理
Image binarize_image(const Image& image, int threshold) {
    Image binarized(image.width, image.height); // 创建一个新的 Image 对象，用于存储二值化后的图片

    // 遍历图片的每一个像素，将其与阈值比较，进行二值化
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            binarized.at(x, y) = image.at(x, y) > threshold ? 255 : 0; // 超过阈值的像素设为255（白色），否则设为0（黑色）
        }
    }

    return binarized; // 返回二值化后的图片
}

// 将图片保存为 PGM 格式文件
void save_pgm(const Image& image, const string& filename) {
    ofstream file(filename, ios::binary); // 以二进制方式打开文件
    if (!file) { // 如果文件无法打开
        cerr << "Cannot open file: " << filename << endl;
        exit(1); // 输出错误信息并退出程序
    }

    file << "P5\n" << image.width << " " << image.height << "\n255\n"; // 写入 PGM 文件头信息

    // 逐像素写入图片数据
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            file.put(image.at(x, y)); // 写入一个字节的像素值
        }
    }
}

// 主函数，程序的入口点
int main() {
    // 从文件加载图片
    Image image = load_pgm("1.pgm");

    // 步骤 1: 裁剪图片
    Image cropped_image = crop_image(image);

    // 步骤 2: 缩放图片
    Image resized_image = resize_image(cropped_image);

    // 步骤 3: 计算大津法阈值
    int threshold = otsu_threshold(resized_image);

    // 步骤 4: 二值化图片
    Image binarized_image = binarize_image(resized_image, threshold);

    // 保存处理后的图片
    save_pgm(binarized_image, "binarized_image.pgm");

    // 输出处理完成的消息
    cout << "Image processing completed and saved as binarized_image.pgm" << endl;

    return 0; // 程序正常结束
}
