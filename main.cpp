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

const string x_str = "-1.7687069531872665107195026392814306235258139295002977194556256258178403449259311946492525705135341365783015626091746124127633553564998154567954573665585325117119952433499858145623972454257441977707920519017714749037394368328598741933787973344329826735585131527420664060570517455598996272289702838148013780249146657159250536145050424325634364117494878708215622708608029486758359093373124868767128017856030188237184457363335619981487897715226284818824501748791852848168945501411819383098612269482764188298430857648872365348609596382751025743832470915600069716006302634890654151885480377223228088663246587578622618912359329149703020416622966572595604285854841815292147128466961063480174518758783556074946982657022173899469187218776757166741467940104096912337128831674480670949056596997443171241257034256321533936609890433562686875886764738220321788554803506347411761965898780647740077719716289104556218693463703676245706590057653334644526507718088121464145068313357515060563588867275351913354259897273";
const string y_str = "0.0018686866862279881761306473079545815087381675832414495947164115594424058231219070805026760427478575088649834823179731306911976608370719809744633868290234509568164190354727292419681471360386554166495138079188264761841410646230443457612589349079231888102187459730260171860689777921861392670692659482196233928380740947582835082307883291013399788576542907631044256929318441886285791857702439222352470350976810008786212443061966628876668722537436013643113171876926769271934858701607290881838741479284870389058450255205035322778607972467865024568990055897951559466191499150928809590681727659963230610105407015930150993085869024936254296478480690929268677595326609497059510789509669398383439528128606345550393203323058221227141166550822251291135269159413497232199835774904713436261899881943913451284627119728428379720351199063354436496535433039375892005669047504286121327416180942130549008520116952461826560688134132523497397576298135134946882431111090921646140854528417535270191424755556737149996929864";
const string startingOffset_x_str = "2";
const string endingOffset_x_str = "1.3843972363826381531814689023957E-991";
const string zoomFactor_str = "0.98";

const int size_x = 8;
const int size_y = size_x;
const int start_iter = 100000;
const int end_iter = 2000000;
const int THREAD_NUM = 4;
const int BASE = 10;
const int PRECISION = 1;
const int ITER = 50;
const float ITER_MOD = 64.5;
const float ESCAPE_RADIUS = 2;
int num_frames = 0;

int pixels[size_x * size_y];
mutex pixel_lock;


void ComputeMandlebrot(int line)
{
    clock_t tStart = clock();

    custom_float currentOffset_x = endingOffset_x;
    custom_float currentOffset_y = endingOffset_x;

    custom_float size_x_mpf(size_x);
    custom_float size_y_mpf(size_y);

    custom_float escape_radius(ESCAPE_RADIUS);

    escape_radius = sqrt(escape_radius);

    currentOffset_y = currentOffset_x * (size_x_mpf / size_y_mpf);

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

        //cout << (int)col << "\n";

        // Storing result in the pixel array
        pixel_lock.lock();
        pixels[xp + line * size_x] = iter;
        pixel_lock.unlock();
    }

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
            uint8_t col = (uint8_t)(((float)(iter - minIt) / (float)(maxIt - minIt)) * 255);
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
    printf("Execute time: %.8fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);

    return 0;
}