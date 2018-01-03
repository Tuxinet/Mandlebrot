#include <iostream>
#include <string>
#include <map>
#include <time.h>
#include <chrono>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/number.hpp>
#include "Bitmap.h"
#include <thread>
#include <stdlib.h>
#include <mutex>

using namespace std;
using namespace boost::multiprecision;

namespace mp = boost::multiprecision;

typedef mp::number<mp::mpfr_float_backend<1000>> custom_float;

custom_float x;
custom_float y;
custom_float startingOffset_x;
custom_float endingOffset_x;
custom_float zoomFactor;

const string x_str = "-1.99999448488549867428401441643683795102899538102902580324795342945604162962465026042603051902985842646249953510033412012870163506133527611316456992016236456776519655813990262063581327180660478125086264132681575110267264042604739834139915976095802882600062026461852230249626693722284748450711764388299183881389086081031315206905563519792902262181330947585192715453649151051779006232917721787959720694534229460433476498589098901844934596776620772627101970902762768835049777093575563159176047446122779786657620643652452659847712504583823630051458931326564147303538244150941484718807287471951353256718899707308282003779841446265147058161973704889532775078291061997199756543440639882852257856735592312514444936688337427366930467714251631082785568754403933698374918743560899948699557180802691110414289421888325009239671123407561148275175154779372502194822643235505761875407404451392480455";
const string y_str = "0";
const string startingOffset_x_str = "2";
const string endingOffset_x_str = "1.3843972363826381531814689023957E-100";
//const string endingOffset_x_str = "1.3843972363826381531814689023957E-991";
const string zoomFactor_str = "0.98";

const int size_x = 64;
const int size_y = size_x / 2;
const int start_iter = 100000;
const int end_iter = 15000;
const int THREAD_NUM = 2;
const int BASE = 10;
const int PRECISION = 1;
const int ITER = 500;
const float ITER_MOD = 64.5;
const float ESCAPE_RADIUS = 2;
int num_frames = 0;

int pixels[size_x * size_y];
int smoothPixels[size_x * size_y];
int histogram[end_iter];
mutex pixel_lock;



bool sortfunc (int i,int j) { return (i<j); }

int compareints (const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}


int getColorForIter(int iter) {

    //return iter % 255;

    auto pItemPot = (int)((int*) bsearch(&iter, smoothPixels, size_x * size_y, sizeof(int), compareints) - smoothPixels);

    int index = 0;
    for (int i = 0; i<size_x * size_y; i++)
    {
        if (smoothPixels[i] == iter) {
            index = i;
            break;
        }
    }

    float scaled = (float)pItemPot / (size_x * size_y);
    auto scaledIter = (int)(scaled * end_iter);

    //return (int)(((float)smoothPixels[histogram[scaledIter]] / end_iter) * 255) ;


    int lSum = 0;
    for (int j = 0; j < iter; j++)
    {
        lSum += histogram[j];
    }

    return (int)((((float)lSum)/(size_x * size_y)) * 255);

    return smoothPixels[(int)((((float)lSum)/(1)))];
}

void ComputeMandlebrot(int line)
{
    clock_t tStart = clock();

    custom_float currentOffset_x = endingOffset_x;
    custom_float currentOffset_y = endingOffset_x;

    custom_float size_x_mpf(size_x);
    custom_float size_y_mpf(size_y);

    custom_float escape_radius(ESCAPE_RADIUS);

    escape_radius = sqrt(escape_radius);

    currentOffset_y = currentOffset_x * (size_y_mpf / size_x_mpf);

    int maxIter = (int)(sqrt(2 * sqrt(abs(1 - sqrt(custom_float(5) / currentOffset_x)))) * ITER_MOD); // Takk gudene hos SO

    //int maxIter = static_cast<int>((((float)itteration / num_frames)) * end_iter);
    //maxIter = ITER;

    maxIter = end_iter;

    //cout << "[" << hash<std::thread::id>()(this_thread::get_id()) << "] Doing " << maxIter << " max itterations for frame " << itteration << "\n";

    // Her setter jeg opp grensene til området av mandlebrot fraktalen jeg skal rendre
    custom_float minX;
    custom_float maxX;

    custom_float minY;
    custom_float maxY;

    minX = x - currentOffset_x;
    maxX = x + currentOffset_x;

    minY = y - currentOffset_y;
    maxY = y + currentOffset_y;

    custom_float increment_x(0);
    custom_float increment_y(0);

    increment_x = (maxX - minX) / size_x;
    increment_y = (maxY - minY) / size_y;

    int iter = 0;

    custom_float x0(0);                               // Den reelle delen av det komplekse tallen
    custom_float y0(0);                               // Den immaginære delen av det komplekse tallet

    // Declaring some stuff I'm going to need in the loop
    custom_float xp_mpf(0);
    custom_float yp_mpf(0);

    custom_float increment_step_x(0);
    custom_float increment_step_y(0);


    custom_float xtemp(0);
    custom_float ytemp(0);
    custom_float x_c(0);
    custom_float y_c(0);

    custom_float two(2);
    custom_float four(4);

    size_t avg_pixel_val = 0;

    custom_float oldX;
    custom_float oldY;

    avg_pixel_val = 0;
    for (int xp = 0; xp < size_x; xp++)
    {
        x0 = minX + xp * increment_x;
        y0 = minY + line * increment_y;

        oldX = x0;
        oldY = y0;

        x_c = custom_float(0);
        y_c = custom_float(0);

        xtemp = custom_float(0);

        iter = 0;

        clock_t start = clock();

        while (iter < maxIter)
        {
            xtemp = x_c * x_c - y_c * y_c + x0;
            y_c = two * x_c * y_c + y0;
            x_c = xtemp;

            iter++;

            if (abs(y_c) > escape_radius)
            {
                break;
            }
        }

        if ((clock() - start) / CLOCKS_PER_SEC / THREAD_NUM > 1)   // If rendering the pixel is taking longer than 1 second
        {                                            // display a notification
            printf("    Finished rendering (%d,%d)\n", xp, line);
        }

        //cout << iter << '\n';

        // Storing result in the pixel array

        float len = (float)sqrt(x_c * x_c + y_c * y_c);
        int smooth;


        if (len >= 1)
        {
            smooth = iter - (int)(log((log(len)) / log(2)));
        }
        else {
            smooth = iter;
            cout << "ASDASD\n";
        }


        if (smooth > end_iter) smooth = end_iter;

        pixel_lock.lock();

        histogram[smooth] += 1;
        pixels[xp + line * size_x] = smooth;
        smoothPixels[xp + line * size_x] = smooth;

        pixel_lock.unlock();

        //cout << iter << '\n';
    }

    printf("    Rendered line %d %.2fs\n", line, (double)(clock() - tStart)/CLOCKS_PER_SEC / THREAD_NUM);
}

void WorkerThread( boost::shared_ptr< boost::asio::io_service > io_service )
{
    io_service->run();
}

void ComputeMandlebrotVideo()
{
    boost::shared_ptr< boost::asio::io_service > ioService(
            new boost::asio::io_service
    );
    boost::shared_ptr< boost::asio::io_service::work > work(
            new boost::asio::io_service::work( *ioService )
    );
    boost::thread_group threadpool;

    for (int i = 0; i < THREAD_NUM; i++)
    {
        threadpool.create_thread(
                boost::bind(&WorkerThread, ioService)
        );
    }

    custom_float currentOffset = startingOffset_x;

    system("rm -rf /tmp/VideoFrames");
    system("mkdir /tmp/VideoFrames");

    for (int i = 0; i < size_y; i++)
    {
        ioService->post(boost::bind(ComputeMandlebrot, i));
    }

    work.reset();
    threadpool.join_all();

    Bitmap b(size_x, size_y);
    int iter, minIt, maxIt;

    minIt = 40000000;
    maxIt = 0;

    cout << "Building image...\n";

    std::sort(smoothPixels, smoothPixels + size_x * size_y, sortfunc);

    for (int i = 0; i < size_x * size_y; i++)
    {
        iter = pixels[i];
        if (iter > maxIt) maxIt = iter;
        if (iter < minIt) minIt = iter;
    }
    for (int xi = 0; xi < size_x; xi++)
    {
        for (int yi = 0; yi < size_y; yi++)
        {

            iter = pixels[xi + yi * size_x];
            //uint8_t col = (uint8_t)(((float)(iter - minIt) / (float)(maxIt - minIt)) * 255);
            uint8_t  col = (uint8_t)getColorForIter(iter);
            b.setPixel(xi, yi, col, col, col);
        }
    }

    b.write("/tmp/VideoFrames/render.bmp");

    system((std::string("eog /tmp/VideoFrames/render.bmp").c_str()));
}

int main()
{

    /*if (mpf_set_str(x, "0", BASE) == -1)
    {
        cout << "[ERROR] INVALID BASE!\n";
        return -1;
    }
     */

    x = custom_float(x_str);
    y = custom_float(y_str);
    startingOffset_x = custom_float(startingOffset_x_str);
    endingOffset_x = custom_float(endingOffset_x_str);
    zoomFactor = custom_float(zoomFactor_str);

    cout << "Starting mandlebrot set calculations..." << endl;
    clock_t tStart = clock();
    ComputeMandlebrotVideo();
    printf("Average time per pixel: %.8fms\n", (double)(clock() - tStart) / CLOCKS_PER_SEC / (size_x * size_y) * 1000);
    printf("CPU-time: %.8fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    return 0;
}