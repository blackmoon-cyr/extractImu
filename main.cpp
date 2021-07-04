#include <stdio.h>
#include <stdlib.h>
#include <k4a/k4a.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc.hpp>
#include <k4arecord/types.h>
#include <k4arecord/playback.h>
#include <k4arecord/record.h>
#include <fstream>
#include <opencv2/core/mat.hpp>
#include <turbojpeg.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <inttypes.h>
#include <time.h>




using namespace std;
using namespace cv;




int main()
{
    k4a_image_t uncompressed_color_image = NULL;
    k4a_image_t depth_image = NULL;
    k4a_image_t color_image = NULL;
    k4a_calibration_t calibration;
    k4a_playback_t playback_handle = NULL;
    if (k4a_playback_open("stable.mkv", &playback_handle) != K4A_RESULT_SUCCEEDED)
    {
        printf("Failed to open recording\n");
        return 1;
    }
    else {
        printf("Successful to open recording\n");
    }

   
   
    k4a_capture_t capture = NULL;
    k4a_stream_result_t result = K4A_STREAM_RESULT_SUCCEEDED;
    k4a_imu_sample_t imu_sample;
    k4a_playback_get_next_imu_sample(playback_handle, &imu_sample);
    
    
    double i = 66721461;
    
    

    if (K4A_RESULT_SUCCEEDED != k4a_playback_get_calibration(playback_handle, &calibration))
    {
        printf("Failed to get calibration\n");
        
    }

    printf("Seeking to timestamp: %d (ms)\n",
        k4a_playback_get_recording_length_usec(playback_handle));
    
    while (result == K4A_STREAM_RESULT_SUCCEEDED)
    {
        result = k4a_playback_get_next_capture(playback_handle, &capture);
        if (result == K4A_STREAM_RESULT_SUCCEEDED)
        {
            
            
            // Process capture here
            
            
            color_image = k4a_capture_get_color_image(capture);
            if (color_image == 0)
            {
                printf("Failed to get color image from capture\n");
                break;
                
            }
            k4a_image_format_t format1;
            format1 = k4a_image_get_format(color_image);
            double time = 0;
            time= k4a_image_get_device_timestamp_usec(color_image);
            //cout<<"\n" <<time;
            if (format1 != K4A_IMAGE_FORMAT_COLOR_MJPG)
            {
                printf("Color format not supported. Please use MJPEG\n");
                
            }
            //k4a_image_format_t format = k4a_image_get_format(color_image);
            int color_width, color_height;
            color_width = k4a_image_get_width_pixels(color_image);
            color_height = k4a_image_get_height_pixels(color_image);
            if (K4A_RESULT_SUCCEEDED != k4a_image_create(K4A_IMAGE_FORMAT_COLOR_BGRA32,
                color_width,
                color_height,
                color_width * 4 * (int)sizeof(uint8_t),
                &uncompressed_color_image))
            {
                printf("Failed to create image buffer\n");
                
            }
            tjhandle tjHandle;
            tjHandle = tjInitDecompress();
            if (tjDecompress2(tjInitDecompress(),
                k4a_image_get_buffer(color_image),
                static_cast<unsigned long>(k4a_image_get_size(color_image)),
                k4a_image_get_buffer(uncompressed_color_image),
                color_width,
                0, // pitch
                color_height,
                TJPF_BGRA,
                TJFLAG_FASTDCT | TJFLAG_FASTUPSAMPLE) != 0)
            {
                printf("Failed to decompress color frame\n");
                
               
            }
            if (tjDestroy(tjHandle))
            {
                printf("Failed to destroy turboJPEG handle\n");
            }

            
            
            uint8_t* color_buffer = k4a_image_get_buffer(uncompressed_color_image);
            
         
           
     
          
           

            cv::Mat color(color_height,color_width, CV_8UC4, (void*)color_buffer, cv::Mat::AUTO_STEP);
            //namedWindow("Image", WINDOW_AUTOSIZE);
            //imshow("Image", color);
           
            cv::String filename = format("%lld.png",(long long)time);
            

            //cout << cols;

            cv::Mat greyMat;
            cv::cvtColor(color, greyMat, cv::COLOR_BGR2GRAY);
            cv::Mat resize;
            cv::resize(greyMat, resize, Size(752, 480));
            
            imwrite(filename, resize);
            
            k4a_capture_release(capture);
            //i=i+ 66721461;
            
            k4a_image_release(color_image);
            
            k4a_image_release(uncompressed_color_image);

           
        }
        
        else if (result == K4A_STREAM_RESULT_EOF)
        {
            // End of file reached
            printf("not open");
            break;
        }
    }
    
    i = 0;

    result = K4A_STREAM_RESULT_SUCCEEDED;
   
    //double **imu= double[i][7];
    //std:ofstream ofs("imu.csv", std::ofstream::out | std::ofstream::trunc);
    FILE *fp = fopen("imu0.csv", "w");

    while (result == K4A_STREAM_RESULT_SUCCEEDED)
    {
        result = k4a_playback_get_next_imu_sample(playback_handle, &imu_sample);
        if (result == K4A_STREAM_RESULT_SUCCEEDED)
        {
            // Process imu here
            
            //printf("%lld\n", (long long)i);
            /*ofs << imu_sample.acc_timestamp_usec+i<<
                "," << imu_sample.gyro_sample.xyz.x <<
                "," << imu_sample.gyro_sample.xyz.y <<
                "," << imu_sample.gyro_sample.xyz.z <<
                "," << imu_sample.acc_sample.xyz.x <<
                "," << imu_sample.acc_sample.xyz.y <<
                "," << imu_sample.acc_sample.xyz.z << "\n";*/
            i = 0;
            
            i = i + imu_sample.acc_timestamp_usec;
            fprintf(fp, "%lld,%f,%f,%f,%f,%f,%f\n", (long long)i,
                imu_sample.acc_sample.xyz.x,
                imu_sample.acc_sample.xyz.y,
                imu_sample.acc_sample.xyz.z,
                imu_sample.gyro_sample.xyz.x,
                imu_sample.gyro_sample.xyz.y,
                imu_sample.gyro_sample.xyz.z
            );

               /* cout<<"Time:"<<imu_sample.acc_timestamp_usec<<
                "Time_Gyr:" << imu_sample.acc_timestamp_usec << "Gyro_Sample" <<
                "GyroX:"<<imu_sample.gyro_sample.xyz.x<<
                "GyroY:"<< imu_sample.gyro_sample.xyz.y <<
                "GyroZ:"<< imu_sample.gyro_sample.xyz.z <<
                "Acc_X"<<imu_sample.acc_sample.xyz.x<<
                "Acc_Y" << imu_sample.acc_sample.xyz.y<<
                "Acc_Z" << imu_sample.acc_sample.xyz.z<<"\n";*/

        }
        else if (result == K4A_STREAM_RESULT_EOF)
        {
            // End of file reached
            printf("not open");
            break;
        }
    }
    if (result == K4A_STREAM_RESULT_FAILED)
    {
        printf("Failed to read entire recording\n");
        return 1;
    }
    //ofs.close();
    fclose(fp);
    k4a_playback_close(playback_handle);


    

    

    return 0;

    
}

