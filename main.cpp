#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <stdint.h>
#include <string>
#include <vector>
#include <mutex>
using namespace cv;
const int MAX_USED_THREADS = 6;

void parallel_resize(std::vector<Mat> &blocks, int st, int end, double coefx, double coefy)
{
    std::cout<< "Start "<<st << "  END " << end << std::endl;
    for (; st < end; st++)
    {
        blocks.at(st);
        resize(blocks.at(st), blocks.at(st), Size(), coefx, coefy, CV_INTER_AREA);
    } 
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout<< "Please input a name of an image and a count of threads to process this image\n";
        std::cout<< "Format ./main <name> <count of threads> \n";
        return 0;
    }

    std::string im_name = argv[1];
    uint32_t thr_to_proc = atoi(argv[2]);
    uint32_t used_thr = 0;
    uint32_t hw_thr = std::thread::hardware_concurrency();
    std::cout << "Available hardware threads " << hw_thr << std::endl;
    
    if (thr_to_proc > hw_thr)
    {
        used_thr = (hw_thr < MAX_USED_THREADS ? hw_thr : MAX_USED_THREADS);
    }
    else
    {
        used_thr = thr_to_proc;
    }
    
    std::cout << used_thr <<" hardware threads will be used"  << std::endl;
    
    std::cout <<"Reading image ..."  << std::endl;
    
    Mat image = imread(im_name, IMREAD_COLOR);

    std::cout << "Width = " << image.cols << " and Height " << image.rows << " of inputed image"<<std::endl;
    uint32_t res_block_width  = image.cols / used_thr; 
    uint32_t res_block_height = image.rows / used_thr;

    std::cout << "Width of small block = " << res_block_width << " and Height of small block " << res_block_height <<std::endl;
    Size small_size(res_block_width, res_block_height);
    std::vector<Mat> blocks;   

    for (int y = 0; y < image.rows; y += small_size.height) 
    {
        for (int x = 0; x < image.cols; x += small_size.width) 
        {
            Rect rect = Rect(x, y, small_size.width, small_size.height);
            blocks.push_back(Mat(image, rect));    
        }
    }
    std::cout<<"count of blocks " << blocks.size() <<" height  " << blocks.at(0).rows <<std::endl;
    std::vector<std::thread> threads(used_thr);
    int st = 0;
    int end = used_thr;
    std::mutex mx;
    for (uint32_t i = 0; i < used_thr; i++)
    {
        threads[i] = std::thread(parallel_resize, std::ref(blocks), st, end, 0.25, 0.25);
        mx.lock();
        st += end;
        end += used_thr;
        mx.unlock();
    }
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    
    int valw = image.cols * 0.25; 
    int valh = image.rows * 0.25; 
    
    Mat combined(image.cols, image.rows, blocks[0].type());
    Size sb_size(blocks.at(0).cols, blocks.at(0).rows); 

    std::cout<< "Blocks size " << blocks.size()<<std::endl;
    std::cout << "Width of small block = " << blocks.at(0).cols << " and Height of small block " << blocks.at(0).rows <<std::endl;
    std::cout<< " New width " << valw << " New Height " << valh <<std::endl;
    
    size_t i = 0;    
    for (int y = 0; y < valh; y += sb_size.height)
    {
        for (int x = 0; x < valw; x += sb_size.width)
        {
           Mat roi = combined(Rect(x,y,sb_size.width, sb_size.height));
           blocks[i].copyTo(roi);
           i++;
        }
    }

    imwrite("outTask.jpg", combined);
    return 0;
}
