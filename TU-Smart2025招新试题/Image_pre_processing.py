import numpy as np
from PIL import Image
import matplotlib.pyplot as plt

def crop_image(image, top_crop=0.2, bottom_crop=0.2):
    """
    裁剪图片的上下部分
    :param image: 输入灰度图片 (PIL Image)
    :param top_crop: 上部裁剪比例
    :param bottom_crop: 下部裁剪比例
    :return: 裁剪后的图片
    """
    width, height = image.size  # 获取图片的宽度和高度
    top = int(height * top_crop)  # 计算裁剪的上部分高度
    bottom = int(height * (1 - bottom_crop))  # 计算裁剪的下部分高度
    cropped_image = image.crop((0, top, width, bottom))  # 裁剪图片，保留中间部分
    return cropped_image  # 返回裁剪后的图片

def resize_image(image, scale=0.25):
    """
    缩放图片
    :param image: 输入图片 (PIL Image)
    :param scale: 缩放比例
    :return: 缩放后的图片
    """
    new_size = (int(image.width * scale), int(image.height * scale))  # 计算缩放后的尺寸
    resized_image = image.resize(new_size, Image.ANTIALIAS)  # 使用抗锯齿算法缩放图片
    return resized_image  # 返回缩放后的图片

def otsu_threshold(gray_array):
    """
    大津法计算二值化阈值
    :param gray_array: 输入灰度值数组
    :return: 二值化阈值
    """
    pixel_counts = np.bincount(gray_array.flatten(), minlength=256)  # 计算每个灰度级别的像素数量
    total_pixels = gray_array.size  # 获取总像素数
    sum_all = np.dot(np.arange(256), pixel_counts)  # 计算所有像素的加权和
    
    sum_b = 0  # 初始化前景像素的加权和
    weight_b = 0  # 初始化前景像素的数量
    max_variance = 0  # 初始化最大方差
    threshold = 0  # 初始化阈值
    
    # 遍历所有可能的阈值
    for t in range(256):
        weight_b += pixel_counts[t]  # 更新前景像素的数量
        if weight_b == 0:
            continue
        
        weight_f = total_pixels - weight_b  # 计算背景像素的数量
        if weight_f == 0:
            break
        
        sum_b += t * pixel_counts[t]  # 更新前景像素的加权和
        mean_b = sum_b / weight_b  # 计算前景的平均灰度
        mean_f = (sum_all - sum_b) / weight_f  # 计算背景的平均灰度
        
        # 计算类间方差
        variance = weight_b * weight_f * (mean_b - mean_f) ** 2
        
        # 如果当前方差大于之前的最大方差，则更新最大方差和最佳阈值
        if variance > max_variance:
            max_variance = variance
            threshold = t
    
    return threshold  # 返回计算出的最佳阈值

def binarize_image(image, threshold):
    """
    二值化图片
    :param image: 输入灰度图片 (PIL Image)
    :param threshold: 二值化阈值
    :return: 二值化图片
    """
    # 将图片中大于阈值的像素设置为255（白色），小于等于阈值的设置为0（黑色）
    binarized_image = image.point(lambda p: 255 if p > threshold else 0)
    return binarized_image  # 返回二值化后的图片

# 主函数
def process_image(image_path):
    # 读取图片并转换为灰度图
    image = Image.open(image_path).convert('L')
    
    # 步骤 1: 裁剪图片
    cropped_image = crop_image(image)
    
    # 步骤 2: 缩放图片
    resized_image = resize_image(cropped_image)
    
    # 将图片转化为二维灰度值数组
    gray_array = np.array(resized_image)
    
    # 步骤 3: 计算大津法阈值
    threshold = otsu_threshold(gray_array)
    
    # 步骤 4: 二值化图片
    binarized_image = binarize_image(resized_image, threshold)
    
    # 显示结果
    plt.figure(figsize=(12, 6))  # 创建一个图像窗口，设置大小
    
    plt.subplot(1, 3, 1)  # 创建第一个子图
    plt.title("Cropped Image")  # 设置子图的标题
    plt.imshow(cropped_image, cmap='gray')  # 显示裁剪后的图片
    
    plt.subplot(1, 3, 2)  # 创建第二个子图
    plt.title("Resized Image")  # 设置子图的标题
    plt.imshow(resized_image, cmap='gray')  # 显示缩放后的图片
    
    plt.subplot(1, 3, 3)  # 创建第三个子图
    plt.title("Binarized Image")  # 设置子图的标题
    plt.imshow(binarized_image, cmap='gray')  # 显示二值化后的图片
    
    plt.show()  # 显示图像窗口

# 调用主函数，处理并显示 "1.png" 图片
process_image("1.png")

