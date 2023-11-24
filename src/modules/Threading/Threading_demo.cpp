// #include "modules/Network/NetConf.hpp"
// #include "libs/buffers.hpp"
// #include "libs/p2b/core.hpp"
// #include "libs/p2b/bitmap.hpp"

// #include "CaptureThread.hpp"

// #include <boost/interprocess/creation_tags.hpp>
// #include <cstdlib>
// #include <cstring>
// #include <opencv4/opencv2/highgui.hpp>
// #include <opencv4/opencv2/core/mat.hpp>
// #include <sys/types.h>
// #include <thread>

// #include <boost/interprocess/shared_memory_object.hpp>
// #include <boost/interprocess/mapped_region.hpp>

// using namespace std;
// using namespace boost::interprocess;

// //? Multithread experiment with FIFOBuffer

// bool isShmemEmpty(cv::Mat* ptr, const long shmem_size){
//     char* tmp_cast = (char*) ptr;
//     for (long i=0; i<shmem_size; ++i){
//         if (*tmp_cast++ != 0) return false;
//     }
//     return true;
// }


// //? Test thread class to experiment with a bitmap
// class BitmapThread {

//     private:
//         p2b::Bitmap frame_bitmap;

//         const char* shmem_name;
//         long shmem_size;
//         int shmem_frames;
//         shared_memory_object shmem;
//         mapped_region mem_region;
//         cv::Mat* shmem_ptr;

//     public:

//         BitmapThread(
//             const char* shmem_name,
//             const long shmem_size,
//             const int shmem_frames
//         ){
//             this->frame_bitmap = p2b::Bitmap(
//                 800,
//                 200,
//                 2,
//                 {85, 170, 255}
//             );
            
//             this->shmem_name = shmem_name;
//             this->shmem_size = shmem_size;
//             this->shmem_frames = shmem_frames;
//             this->shmem = shared_memory_object(
//                 open_only,
//                 this->shmem_name,
//                 read_write
//             );
//             this->mem_region = mapped_region(this->shmem, read_write);
//             this->shmem_ptr = (cv::Mat*) mem_region.get_address();

//         }
        
//         ~BitmapThread(){
//             shared_memory_object::remove(this->shmem_name);
//         }
        

//         // Thread start method
//         void start(){

//             cout << "---- BITMAP THREAD STARTED ----" << endl;

//             cv::namedWindow(
//                 "Bitmap visualization",
//                 cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO | cv::WINDOW_GUI_EXPANDED
//             );
//             cv::resizeWindow("Bitmap visualization", 800, 800);
//             cv::Mat outImg;

//             this->frame_bitmap.toGrayscaleImage_parallel(
//                 &outImg, 
//                 {85,170,255}
//             );
//             cout << "BMP_TH: Bitmap object created" << endl;

//             // Main loop        
//             while(1){

//                 cv::imshow("Bitmap visualization", outImg);
                
//                 if (!isShmemEmpty(this->shmem_ptr, this->shmem_size)){

//                     cout << "BMP_TH: SHMEM FULL! ADDING FRAMES" << endl;

//                     for(int i=0; i<this->shmem_frames; ++i){
//                         this->frame_bitmap.addImage(
//                             &this->shmem_ptr[i], 
//                             p2b::DIR_RIGHT, 
//                             true
//                         );
//                     }

//                     this->frame_bitmap.toGrayscaleImage_parallel(
//                         &outImg, 
//                         {85,170,255}
//                     );

//                     // Emptying shmem
//                     memset((char*) this->shmem_ptr, 0, this->shmem_size);
//                 }

//             }

//             cv::destroyAllWindows();
//             cout << "BMP_TH: Stopped" << endl;

//         }

// };



// int Threading_demo(){
    
//     // Experimental parameters
//     const long initial_buffer_capacity = 16;
//     const char* shmem_name = "frame_shmem";
//     const long shmem_size = 10'000'000;
//     const int shmem_frames = 3;

//     //! SHMEM REMOVER, REALLY IMPORTANT
//     struct shm_remove
//     {   //! Can't use shmem_name var here, hard coded name
//         shm_remove() { shared_memory_object::remove("frame_shmem"); }
//         ~shm_remove(){ shared_memory_object::remove("frame_shmem"); }
//     } remover;

//     //Shmem init
//     shared_memory_object shmem = shared_memory_object(
//         create_only, shmem_name, read_write
//     );
//     shmem.truncate(shmem_size);
//     mapped_region mem_region = mapped_region(
//         shmem, read_write
//     );
//     memset(mem_region.get_address(), 0, shmem_size);

//     // Thread classes init
//     CaptureThread capThread = CaptureThread(
//         initial_buffer_capacity,
//         shmem_name,
//         shmem_size,
//         shmem_frames
//     );

//     BitmapThread bmpThread = BitmapThread(
//         shmem_name,
//         shmem_size,
//         shmem_frames
//     );

//     // Thread starts
//     thread capture_thread = thread(&CaptureThread::start, &capThread);
//     thread bitmap_thread = thread(&BitmapThread::start, &bmpThread);

//     // Thread join
//     capture_thread.join();
//     bitmap_thread.join();

//     return 0;
// }
