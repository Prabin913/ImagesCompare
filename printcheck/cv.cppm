module;

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "..\utils.h"
#include <filesystem>
#include <tuple>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

export module printcheck.cv;

using namespace cv;
using namespace std;
using std::filesystem::path;

// Helper function to convert UTF-8 to wide string
std::wstring utf8_to_wide(const std::string& utf8_str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wstrTo[0], size_needed);
    return wstrTo;
}

// Helper function to convert wide string to UTF-8
std::string wide_to_utf8(const std::wstring& wide_str)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

export namespace printcheck
{
    /*!
     *  @details
     *    Read image with error check..
     *  @note
     *    windows specific UTF-8 -> WCHAR conversion required for multibytes
     *  @warning
     *    WINDOWS specific implementation
     *    imdecode crashed if called with empty data, so we must check before calling it
     *  @param  img_path
     *    path to the image to be read
     */
    Mat read(const path& img_path)
    {
        std::wstring wide_path = utf8_to_wide(img_path.string());
        WriteLogFile(L"Reading image: %S", wide_path.c_str());

        FILE* file = _wfopen(wide_path.c_str(), L"rb");
        if (!file)
        {
            WriteLogFile(L"Failed to open file!");
            return {};
        }
        ifstream ifs(file);
        vector<char> data(istreambuf_iterator<char>{ifs}, {});
        fclose(file);
        if (data.empty())
        {
            WriteLogFile(L"Failed to read image!");
            return {};
        }
        auto img = imdecode(data, IMREAD_COLOR);
        if (img.empty())
        {
            WriteLogFile(L"Failed to decode image!");
        }
        return img;
    }

    void write(const path& img_path, const Mat& img)
    {
        WriteLogFile(L"Writing image: %S",img_path.c_str());
        imwrite(img_path.string(), img);
    }

    /*!
     *  @details
     *    Waits until a key is pressed..
     *  @param  name
     *    Name of the window to be displayed
     *  @param  img
     *    Image to be displayed
     */
    void display(const string& name, const Mat& img)
    {
        const float scale = min(1280.0f / img.size().width, 720.0f / img.size().height);
        WriteLogFile(L"fit to screen scale calculated: %f",scale);
        Mat disp;
        resize(img, disp, Size(), scale, scale);
        imshow(name, disp);
        waitKey(1);
    }

    /*!
     *  @details
     *    Uses jet map to blend the error surface above the original image.
     *    Blending is only applied where the mask requests it!
     *  @note
     *    Original ref image is overwritten as reloaded for each calc call!
     *  @param  ref
     *    reference image, deep copy is made, the original input kept.
     *  @param  err
     *    Contains the error values to be visualized
     *  @param  mask
     *    area to be ignored for visualization
     *  @return
     *    tuple containing the blended error map and the generated jet visualization
     */
    tuple<Mat, Mat> blend_error(const Mat& ref, const Mat& err, const Mat& mask = {})
    {
        Mat jet;
        normalize(err, jet, 0, 255, NORM_MINMAX, CV_8U);
        applyColorMap(err, jet, COLORMAP_INFERNO);
        if (jet.size() != ref.size())
        {
            WriteLogFile(L"Error map has different size than reference!");
            resize(jet, jet, ref.size());
        }
        Mat blend = ref.clone();
        jet.copyTo(blend, mask);
        return { blend, jet };
    }
}
