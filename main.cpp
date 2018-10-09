#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>


//OPENCV
 #include <opencv2/core/utility.hpp>
 #include <opencv2/tracking.hpp>
 #include <opencv2/videoio.hpp>
 #include <opencv2/highgui.hpp>

//YOLO V3 INCLUDES - https://github.com/AlexeyAB/darknet
//compiled for SO/Lib and OPENCV using GPU(Cuda/CuDNN) and GPU Tracking.
#define OPENCV
//#define GPU
#include <darknet/src/yolo_v2_class.hpp>

//check if file exists..
bool fexists(const std::string& filename) 
{
  std::ifstream ifile(filename.c_str());
  return (bool)ifile;
}

//generete next log file name in numbered sequence
std::string generate_filename(std::string file_name, std::string file_ext)
{
    int file_seq = 1;
    int max_tries = 1000;
    std::string default_file = file_name + "." + file_ext;
    std::string tmp_file = default_file;
    
    do 
    {
        std::stringstream strstrm;
        strstrm << file_name << "_" << file_seq << "." << file_ext;
        tmp_file = strstrm.str();
        
        file_seq++;
        max_tries --;

        //max file sequence generation reached, use default and overwrite.
        if (max_tries <= 0)
        {
            tmp_file = default_file;
            break;
        }
        
    } while (fexists(tmp_file));
    
    return tmp_file;
}

std::vector<std::string> read_names(std::string txtfilepath)
{
    std::vector<std::string> names_list;
    
    //If names file exist, load them into vector..
    if (fexists(txtfilepath.c_str()))
    {
        // bounding boxes
        std::ifstream infile(txtfilepath.c_str());
        
        std::string class_name;
        //read all names from file 
        while (std::getline(infile, class_name))
        {
            names_list.push_back(class_name);
        }
    }//endif names file..

    return names_list;
}

int main(int argc, char** argv) 
{
    //Neural network settings
    float net_confidence = 0.01f;
    float net_threshold = 0.20f;
    std::string net_config = "/home/koos/ml/darknet/cfg/yolov3-spp.cfg";
    std::string net_weights = "/home/koos/ml/darknet/weights/yolov3-spp.weights";
    std::string net_names = "/home/koos/ml/darknet/data/coco.names";

    //Create Darknet Detector
    Detector darknet(net_config, net_weights);

    //vector for class names.
     std::vector<std::string> names_list = read_names(net_names);
     
    //detection area bounding box color
    cv::Scalar obj_box_color = cv::Scalar(0x00, 0xff, 0x00);

    //opencv image
    cv::Mat src1;

    //capture video stream from cam 0 at specific camera res.. 
    cv::VideoCapture stream1(1); //0 is the id of video device.0 if you have only one camera.
//    stream1.set(3,1920); // resolution X
//    stream1.set(4,1080); //resolution Y
//    stream1.set(3,1280); // resolution X
//    stream1.set(4,960); //resolution Y
    stream1.set(3,1920); // resolution X
    stream1.set(4,1080); //resolution Y
    
    //make sure video stream is open/active
    if (!stream1.isOpened())
    { //check if video device has been initialised
        std::cout << "Error loading video" << std::endl;
        return -1;
    }    
    
    //unconditional loop while vide is feeding..
    while (true)
    {
        stream1.read(src1);
        if (!src1.data)
        {
            std::cout << "No image data" << std::endl;
            break;
        }
        else
        {
            //detect objects using yolo detector & return vector of bounding boxes!
            std::vector<bbox_t> bboxes = darknet.detect(src1, net_threshold, true);
            
            //check objects location relative to areas of interrest..
            for (auto obj_box : bboxes)
            {
                //only mark higher than specified confidence
                if (obj_box.prob > net_confidence)
                {
                    cv::Point obj_box_tl = cv::Point(obj_box.x, obj_box.y);
                    cv::Point obj_box_br = cv::Point(obj_box.x + obj_box.w, obj_box.y + obj_box.h);
                    cv::rectangle(src1, obj_box_tl, obj_box_br, obj_box_color, 2, 8, 0 );

                    std::stringstream strstrm;
                    strstrm << names_list.at(obj_box.obj_id) << " " << (int)(obj_box.prob * 100) << "%";
                    cv::putText(src1, strstrm.str().c_str(), obj_box_tl, cv:: FONT_HERSHEY_PLAIN, 2,  obj_box_color, 2);
                }

            }
            
            cv::imshow("cam", src1);
        }
        
        //33ms = roughly 30 fps
        int x = cv::waitKey(33);
        
        //ESC exits
        if(x == 27) //ESC = 27
        {
            break;       
        }
        else if (x == 115) //s = save
        {
            std::string jpgname = generate_filename("outfile", "jpg");
            std::cout << "saving: " << jpgname  << std::endl;
            cv::imwrite(jpgname, src1);
        }
    }
    
    return 0;
}


