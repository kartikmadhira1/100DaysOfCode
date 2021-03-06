#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace std;

class Image {
protected:
    cv::Mat image2d;
    bool loadCheck;
public:
    Image();
    Image(int _height, int _width);
    vector<int> image1d;
    int width, height;
    void loadImage(string path);
    int showImage(cv::Mat _image2d);
    void convert2D();
    cv::Mat convert1D(vector<int> image, int height, int width);
    void checkLoad();
    bool imageLoadCheck();
    int getWidth();
    int getHeight();
    cv::Mat getImage();
    int getPixelat1D(int x, int y);
    int getPixelat1D(vector<int> &_image1d, int x, int y, int width);
    void emplaceAtPixel1D(vector<int> &_image1d, int val, int x, int y, int width);
    double getPixelat2D(int x, int y);
    cv::Mat summedAreaTable(const std::vector<int> image1d, int img_height, int img_width);
    cv::Mat applyBoxFilter(const std::vector<int> &image1d, int ker_width, int ker_height);
    cv::Mat optimizedBoxFilter(const std::vector<int> image1d, int ker_height, int ker_width,
                                int img_height, int img_width);
    cv::Mat getHarrisCorners(int response_thresh);
    cv::Mat applySobel(const std::vector<int> image1d, int img_height, int img_width, bool dx);
};

Image::Image() {
    width = 0;
    height = 0;
    // image
    image1d = {};
    loadCheck = true;
}

Image::Image(int _height, int _width) {
    width = _width;
    height = _height;
    // image
    image1d.resize(width*height);
    loadCheck = true;
}

void Image::loadImage(string path) {
    image2d = cv::imread(path, 0);
    if (image2d.data == NULL) {
        loadCheck = false;
    }
}

void Image::convert2D() {
    for (int i = 0; i < image2d.rows; i++) {
        for (int j = 0; j < image2d.cols; j++) {
            image1d.emplace_back(image2d.at<uchar>(i, j));
        }
    }
    width = image2d.cols;
    height = image2d.rows;
}

// x is row index and y is column indexed
int Image::getPixelat1D(int x, int y) {
    return image1d[y + x*width];
}

int Image::getPixelat1D(vector<int> &_image1d, int x, int y, int _width) {
    return _image1d[y + x*_width];
}

void Image::emplaceAtPixel1D(vector<int> &_image1d, int val, int x, int y, int _width) {
    _image1d[y + x*_width] = val;
}

int Image::getWidth() {
    return width;
}

int Image::getHeight() {
    return height;
}

// x is row index and y is column indexed
double Image::getPixelat2D(int x, int y) {
    return image2d.at<uchar>(x, y);
}

bool Image::imageLoadCheck() {
    // bool ret_val = loadCheck;
    return loadCheck;
}

int Image::showImage(cv::Mat _image2d) {
    cv::namedWindow("Display Window", CV_WINDOW_AUTOSIZE);
    cv::imshow("Display Window", _image2d);
    cv::waitKey(0);
    return 0;
} 

cv::Mat Image::getImage() {
    return image2d;
}

cv::Mat Image::convert1D(std::vector<int> image, int _height, int _width) {
    cv::Mat ret_image(cv::Size(_width, _height), CV_8UC1);
    // cout << "comes in here\n";
    for (int i = 0; i < _height; i++) {
        for (int j = 0; j < _width; j++) {
            // cout << i << " " << j << "\n";
            ret_image.at<uchar>(i, j) = image[j + i*_width];
        }
    }
    // ret_image.convertTo(ret_image, 0);
    return ret_image;
}
cv::Mat Image::applyBoxFilter(const std::vector<int> &image1d,\
                        int ker_height, int ker_width) {
    // Create a new Image of the reduced size 
    int new_h = height - ker_height/2;
    int new_w = width - ker_width/2;
    Image ret_image(new_h, new_w);
    // First iterate over all possible I(x, y)
    for (int i = ker_height/2; i < height - ker_height/2; i++) {
        for (int j = ker_width/2; j < width - ker_width/2; j++) {
            // Now iterate over the kernel to get a sum
            int sum = 0;

            for (int m = i - ker_height/2; m < (i - ker_height/2) + ker_height; m++) {
                for (int l = j - ker_height/2; l < (j - ker_height/2) + ker_height; l++) {
                    sum += getPixelat1D(m, l);
                }
            }

            ret_image.image1d[(j - ker_width/2) + (i-ker_width/2)*width] = sum/(ker_height*ker_width);
        }
    }

    cv::Mat ret_cv_image = convert1D(ret_image.image1d, new_h, new_w);
    // showImage(ret_image1);
    return ret_cv_image;
}

cv::Mat Image::summedAreaTable(const vector<int> image1d, 
                                int img_height, int img_width) {
    // The idea is to create a summed area table with the 
    // 1d image only
    // First initialize the matrix with first column and row values
    cv::Mat ret_image(img_height, img_width, CV_64F);
    ret_image.at<float>(0, 0) = getPixelat1D(0, 0);
    for (int i = 1; i < img_width; i++) {
        ret_image.at<float>(0, i) = ret_image.at<float>(0, i - 1) 
                                    + getPixelat1D(0, i);
    }
    for (int i = 1; i < img_height; i++) {
        ret_image.at<float>(i, 0) = ret_image.at<float>(i - 1, 0) +
                                    getPixelat1D(i, 0);
    }
    // ret_image has now been initialized for dp
    for (int i = 1;i < img_height; i++) {
        for (int j = 1;j < img_width; j++) {
            ret_image.at<float>(i, j) = ret_image.at<float>(i, j-1) + ret_image.at<float>(i-1, j) 
                                        + getPixelat1D(i, j) - ret_image.at<float>(i-1, j-1);
        }
    }
    return ret_image;
}

cv::Mat Image::optimizedBoxFilter(const vector<int> image1d, int ker_height, int ker_width, 
                                    int img_height, int img_width) {
    cv::Mat summed_image = summedAreaTable(image1d, img_height, img_width);
    cv::Mat ret_image(img_height, img_width, CV_8U);
    for (int i = ker_height/2; i < img_height - ker_height/2; i++) {
        // int ker_sum = 0;
        for (int j = ker_height/2; j < img_width - ker_width/2; j++) {
            int ker_sum = summed_image.at<float>(i + ker_height/2, j + ker_width/2) +
                            summed_image.at<float>(i - ker_height/2, j - ker_width/2) - 
                            summed_image.at<float>(i - ker_height/2, j + ker_width/2) -
                            summed_image.at<float>(i + ker_height/2, j - ker_width/2);
            ret_image.at<uchar>(i, j) = ker_sum/(ker_height*ker_width);
            // cout << summed_image.at<float>(i + img_height/2, j + img_width/2) << "\n";
        }
    }
    return ret_image;
}

cv::Mat Image::applySobel(const vector<int> image1d, int img_height, int img_width, bool dx) {
    vector<int> Gxy = {1, 2, 1};
    vector<int> Gxx = {1, 0, -1};

    if (!dx) {
        Gxx = {1, 2, 1};
        Gxy = {1, 0, -1};
    }
    vector<int> image1Dvec(img_height*img_width, 0);
    for (int i = Gxx.size()/2; i < img_height - Gxx.size()/2; i++) {
        for (int j = Gxy.size()/2; j < img_width - Gxy.size()/2; j++) {
            // for (int k = 0; k < Gxx.size(); k++) {
            emplaceAtPixel1D(image1Dvec, getPixelat1D(i, j-1)*Gxx[0] + getPixelat1D(i, j)*Gxx[1] +
                                            getPixelat1D(i, j+1)*Gxx[2], i, j, img_width);
        }
    }
    int sum = 0;
    for (int i = Gxx.size()/2; i < img_height - Gxx.size()/2; i++) {
        for (int j = Gxy.size()/2; j < img_width - Gxy.size()/2; j++) {
            // for (int k = 0; k < Gxx.size(); k++) {
            sum = getPixelat1D(image1Dvec , i-1, j, img_width)*Gxy[0] + getPixelat1D(image1Dvec , i, j, img_width)*Gxy[1] +
                                            getPixelat1D(image1Dvec , i+1, j, img_width)*Gxy[2];
            emplaceAtPixel1D(image1Dvec, sum/9 , i, j, img_width);
        }
    }

    
    // for (int i = Gxx.size()/2; i < img_height - Gxx.size()/2; i++) {
    //     for (int j = Gxy.size()/2; j < img_width - Gxy.size()/2; j++) {
    //         ret_imageX.at<float>(i, j) = std::sqrt(std::pow(ret_imageX.at<float>(i, j)/9, 2) + std::pow(ret_imageY.at<float>(i, j)/9, 2)); 
    //     }
    // }
    cv::Mat new_img;
    cv::Mat ret_img = convert1D(image1Dvec, img_height, img_width);
    ret_img.convertTo(new_img, CV_8UC1);
    return ret_img;
}



cv::Mat Image::getHarrisCorners(int response_thresh) {
    // 1. Get the X and Y gradients of the image.
    // 2. Construct the weight matrix
    //    | Ix*Ix , Ix*Iy |
    //    | Ix*Iy , Iy*Iy |
    // cv::Mat Ix = applySobel(image1d, height, width, 1);
    // cv::Mat Iy = applySobel(image1d, height, width, 0);
    cv::Mat Ix;
    cv::Mat Iy;

    cv::Sobel(getImage(), Ix, -1, 1, 0);
    cv::Sobel(getImage(), Iy, -1, 0, 1);

    cv::Mat ret_image = getImage();
    cout << Ix.cols << " " << Iy.rows << "\n";
    // Get the corresponding hessian matrix for each of the pixels
    for (int i = 1; i < Ix.rows - 1; i++) {
        for (int j = 1; j < Iy.cols - 1; j++) {
            cv::Mat weight_matrix(2, 2, CV_64FC1);
            weight_matrix.at<float>(0, 0) = Ix.at<uchar>(i, j)*Ix.at<uchar>(i, j); weight_matrix.at<float>(0, 1) = Ix.at<uchar>(i, j)*Iy.at<uchar>(i, j);
            weight_matrix.at<float>(1, 0) = Ix.at<uchar>(i, j)*Iy.at<uchar>(i, j); weight_matrix.at<float>(1, 1) = Iy.at<uchar>(i, j)*Iy.at<uchar>(i, j);
            cv::Vec2b eig_values;
            cv::eigen(weight_matrix, eig_values);
            // Calculate the harris corner score
            float l1 = eig_values[0];
            float l2 = eig_values[1];

            float score = l1*l2 - 0.04*pow(l1 + l2, 2);
            if (response_thresh < score) {
                cv::circle(ret_image, cv::Point(i, j), 1, cv::Scalar(255, 0, 0));
            }
        }
    }
    return ret_image;
}