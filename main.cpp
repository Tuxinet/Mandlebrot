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
#include "Gradient.h"

using namespace std;
using namespace boost::multiprecision;

namespace mp = boost::multiprecision;

typedef mp::number<mp::mpfr_float_backend<1000>> custom_float;

custom_float x;
custom_float y;
custom_float startingOffset_x;
custom_float endingOffset_x;
custom_float zoomFactor;

const string x_str = "-1.7685736563152709932817429153295447129341200534055498823375111352827765533646353820119779335363321986478087958745766432300344486098206084588445291690832853792608335811319613234806674959498380432536269122404488847453646628324959064543";
const string y_str = "-0.0009642968513582800001762427203738194482747761226565635652857831533070475543666558930286153827950716700828887932578932976924523447497708248894734256480183898683164582055541842171815899305250842692638349057118793296768325124255746563";
const string startingOffset_x_str = "2";
const string endingOffset_x_str = "1.3843972363826381531814689023957E-10";
//const string endingOffset_x_str = "1.3843972363826381531814689023957E-991";
const string zoomFactor_str = "0.98";

const int size_x = 256;
const int size_y = size_x / 2;
const int start_iter = 100000;
const int end_iter = 500;
const int THREAD_NUM = 2;
const int BASE = 10;
const int PRECISION = 1;
const int ITER = 500;
const float ITER_MOD = 64.5;
const float ESCAPE_RADIUS = 2;
const double tollerance = 0.00000000000001;
int num_frames = 0;

double pixels[size_x * size_y];
double smoothPixels[size_x * size_y];
int histogram[end_iter];
mutex pixel_lock;


// Color gradient stuff
Gradient::GradientColor black(0, 0, 0, 255);

Gradient::Gradient<Gradient::GradientColor> colorGradient;


bool sortfunc (double i,double j) { return (i<j); }

int comparedoubles (const void * a, const void * b)
{
    if (abs(*(double*)a - *(double*)b) < tollerance) return 0;
    else if (*(double*)a < *(double*)b) return -1;
    else if (*(double*)a > *(double*)b) return 1;
}


Gradient::GradientColor getColorForIter(double iter) {

    //return iter % 255;

    if (iter > end_iter - 1) return black;

    int index = 0;

    double * item;
    item = (double*) bsearch(&iter, smoothPixels, size_x * size_y, sizeof(double), comparedoubles);


    index = (int)(item - smoothPixels);

    float t = (float)index / (size_x * size_y);

    return colorGradient.getColorAt(t);
}

void ComputeMandlebrot(int line)
{
    clock_t tStart = clock();

    custom_float currentOffset_x = endingOffset_x;
    custom_float currentOffset_y = endingOffset_x;

    custom_float size_x_mpf(size_x);
    custom_float size_y_mpf(size_y);

    custom_float escape_radius_sqrt(ESCAPE_RADIUS);
    custom_float escape_radius(ESCAPE_RADIUS);

    escape_radius_sqrt = sqrt(escape_radius_sqrt);

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

    custom_float zRealStart(0);                               // Den reelle delen av det komplekse tallen
    custom_float zImagStart(0);                               // Den immaginære delen av det komplekse tallet

    // Declaring some stuff I'm going to need in the loop
    custom_float xp_mpf(0);
    custom_float yp_mpf(0);

    custom_float increment_step_x(0);
    custom_float increment_step_y(0);


    custom_float zRealNext(0);
    custom_float ytemp(0);
    custom_float zReal(0);
    custom_float zImag(0);
    custom_float zRealOld = custom_float(0);
    custom_float zImagOld = custom_float(0);

    custom_float two(2);
    custom_float four(4);

    size_t avg_pixel_val = 0;

    custom_float oldX;
    custom_float oldY;

    avg_pixel_val = 0;
    for (int xp = 0; xp < size_x; xp++)
    {
        zRealStart = minX + xp * increment_x;
        zImagStart = minY + line * increment_y;

        oldX = zRealStart;
        oldY = zImagStart;

        zReal = custom_float(0);
        zImag = custom_float(0);

        zRealNext = custom_float(0);

        iter = 0;

        clock_t start = clock();


        while (iter < maxIter)
        {
            zRealOld = zReal;
            zImagOld = zImag;

            zRealNext = zReal * zReal - zImag * zImag + zRealStart;
            zImag = two * zReal * zImag + zImagStart;
            zReal = zRealNext;

            iter++;

            if (abs(zImag) > escape_radius_sqrt)
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

        custom_float len = sqrt(abs(zRealOld) * abs(zRealOld) + abs(zImagOld) * abs(zImagOld));

        double smooth;


        if (iter < maxIter)
            smooth = (double)((double)iter + (double)1 - log(log(abs(len))) / log(escape_radius));
        else smooth = maxIter;

        pixel_lock.lock();

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
    double iter;

    cout << "Building image...\n";

    std::sort(smoothPixels, smoothPixels + size_x * size_y, sortfunc);

    for (int xi = 0; xi < size_x; xi++)
    {
        for (int yi = 0; yi < size_y; yi++)
        {

            iter = pixels[xi + yi * size_x];
            //uint8_t col = (uint8_t)(((float)(iter - minIt) / (float)(maxIt - minIt)) * 255);
            Gradient::GradientColor c = getColorForIter(iter);
            uint8_t r = (uint8_t)(c.r * 255);
            uint8_t g = (uint8_t)(c.g * 255);
            uint8_t bl = (uint8_t)(c.b * 255);
            b.setPixel(xi, yi, r, g, bl);
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

    colorGradient.addColorStop(0, black);
    colorGradient.addColorStop(0.16, Gradient::GradientColor(32, 107, 203, 255));
    colorGradient.addColorStop(0.42, Gradient::GradientColor(237, 255, 255, 255));
    colorGradient.addColorStop(0.6425, Gradient::GradientColor(255, 170, 0, 255));
    colorGradient.addColorStop(0.8575, Gradient::GradientColor(0, 2, 0, 255));
    colorGradient.addColorStop(1, Gradient::GradientColor(20, 68, 107, 255));

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