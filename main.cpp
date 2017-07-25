#include <iostream>
#include <string>
#include <map>
#include <time.h>
#include <chrono>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/number.hpp>
#include "Bitmap.h"
#include <thread>

using namespace std;
using namespace boost::multiprecision;

mpf_float x;
mpf_float y;
mpf_float startingOffset_x;
mpf_float endingOffset_x;
mpf_float zoomFactor;

const string x_str = "-1.74999841099374081749002483162428393452822172335808534616943930976364725846655540417646727085571962736578151132907961927190726789896685696750162524460775546580822744596887978637416593715319388030232414667046419863755743802804780843375";
const string y_str = "-0.00000000000000165712469295418692325810961981279189026504290127375760405334498110850956047368308707050735960323397389547038231194872482690340369921750514146922400928554011996123112902000856666847088788158433995358406779259404221904755";
const string startingOffset_x_str = "2";
const string endingOffset_x_str = "1E-9";
const string zoomFactor_str = "0.98";

const int size_x = 1024;
const int size_y = 1024;
const int start_iter = 50;
const int end_iter = 10000;
const int THREAD_NUM = 1;
const int BASE = 10;
const int PRECISION = 20;
const int ITER = 50;
const float ITER_MOD = 90;
int num_frames = 0;

void ComputeMandlebrot(int itteration)
{
    mpf_set_default_prec(PRECISION);
    clock_t tStart = clock();

    mpf_float currentOffset_x = startingOffset_x;
    mpf_float currentOffset_y = startingOffset_x;

    mpf_float size_x_mpf(size_x);
    mpf_float size_y_mpf(size_y);

    currentOffset_y = currentOffset_x * (size_x_mpf / size_y_mpf);

    for (int i = 0; i < itteration; i++)
    {
        currentOffset_x *= zoomFactor;
        currentOffset_y *= zoomFactor;
    }

    //int maxIter = (int)(sqrt(2 * sqrt(abs(1 - sqrt(mpf_float(5) / currentOffset_x)))) * ITER_MOD); // Takk gudene hos SO

    int maxIter = static_cast<int>((((float)itteration / num_frames)) * end_iter);
    //maxIter = ITER;

    //cout << "[" << hash<std::thread::id>()(this_thread::get_id()) << "] Doing " << maxIter << " max itterations for frame " << itteration << "\n";

    Bitmap b(size_x, size_y);
    // Her setter jeg opp grensene til området av mandlebrot fraktalen jeg skal rendre
    mpf_float minX;
    mpf_float maxX;

    mpf_float minY;
    mpf_float maxY;

    minX = x - currentOffset_x;
    maxX = x + currentOffset_x;

    minY = y - currentOffset_y;
    maxY = y + currentOffset_y;

    mpf_float increment_x(0);
    mpf_float increment_y(0);

    increment_x = (maxX - minX) / size_x_mpf;
    increment_y = (maxY - minY) / size_y_mpf;

    int iter = 0;

    mpf_float x0(0);                               // Den reelle delen av det komplekse tallen
    mpf_float y0(0);                               // Den immaginære delen av det komplekse tallet

    // Declaring some stuff I'm going to need in the loop
    mpf_float xp_mpf(0);
    mpf_float yp_mpf(0);

    mpf_float increment_step_x(0);
    mpf_float increment_step_y(0);


    mpf_float xtemp(0);
    mpf_float ytemp(0);
    mpf_float x_c(0);
    mpf_float y_c(0);

    mpf_float two(2);
    mpf_float four(4);

    for (int yp = size_y - 1; yp > 0; yp--)
    {
        b.write("/tmp/VideoFrames/" + std::to_string(itteration) + ".bmp");
        cout << "\tRendering line " << size_y - yp << "\n";
        for (int xp = 0; xp < size_x; xp++)
        {
            xp_mpf = mpf_float(xp);
            yp_mpf = mpf_float(yp);

            x0 = minX + xp_mpf * increment_x;
            y0 = minY + yp_mpf * increment_y;

            x_c = mpf_float(0);
            y_c = mpf_float(0);

            xtemp = mpf_float(0);

            iter = 0;
            while (iter < maxIter)
            {

                xtemp = x_c * x_c - y_c * y_c + x0;
                y_c = two * x_c * y_c + y0;
                x_c = xtemp;
                iter++;

                if (x_c < ((1 /(abs(y_c) - 0.5))))
                {
                    break;
                }
            }
            uint8_t col = (uint8_t)(((float)iter/maxIter) * 255);

            b.setPixel(xp, yp, col, 0, 255 - col);
        }
    }

    b.write("/tmp/VideoFrames/" + std::to_string(itteration) + ".bmp");

    printf("    %.8fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
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

    mpf_float currentOffset = startingOffset_x;

    int itterations = 0;

    while (currentOffset > endingOffset_x)
    {
        itterations++;
        currentOffset *= zoomFactor;
    }

    num_frames = itterations;

    system("rm -rf /tmp/VideoFrames");
    system("mkdir /tmp/VideoFrames");

    cout << "\nRendering deep zoom for preview...\n";

    ComputeMandlebrot(itterations);
    cout << "\nMandlebrot completed, opening preview...\n";
    system((std::string("feh /tmp/VideoFrames/") + std::to_string(itterations) + ".bmp").c_str());
    char input;
    cout << "Do you wish to proceed with mandlebrot render that will require " << itterations << " frames?(y/n): ";
    cin >> input;

    if (input != 'y')
        return;

    cout << "Mandlebrot sequence calculating, will need " << itterations << " frames!\n";

    for (int i = 0; i < itterations; i++)
    {
        ioService->post(boost::bind(ComputeMandlebrot, i));
    }

    work.reset();
    threadpool.join_all();

    cout << "\nCombining frames with ffmpeg...\n\n";
    system("ffmpeg -y -f image2 -r 30 -i /tmp/VideoFrames/%d.bmp ./out.mov");
    system("vlc out.mov");
}

int main()
{
    mpf_set_default_prec(PRECISION);

    /*if (mpf_set_str(x, "0", BASE) == -1)
    {
        cout << "[ERROR] INVALID BASE!\n";
        return -1;
    }
     */

    x = mpf_float(x_str);
    y = mpf_float(y_str);
    startingOffset_x = mpf_float(startingOffset_x_str);
    endingOffset_x = mpf_float(endingOffset_x_str);
    zoomFactor = mpf_float(zoomFactor_str);

    cout.precision(20);
    cout << "Starting mandlebrot set calculations..." << endl;
    clock_t tStart = clock();
    ComputeMandlebrotVideo();
    printf("Execute time: %.8fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    return 0;
}