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
    std::string net_config = "/home/koos/ml/darknet/cfg/yolov3-spp.cfg";
    std::string net_weights = "/home/koos/ml/darknet/weights/yolov3-spp.weights";
    std::string net_names = "/home/koos/ml/darknet/data/coco.names";
            
    float net_confidence = 0.10f;
    float net_threshold = 0.20f;

    //Darknet Detector
    Detector* darknet = new Detector(net_config, net_weights);

    //load names.
     std::vector<std::string> names_list = read_names(net_names);
     
    //detection area bounding box color
    cv::Scalar obj_box_color = cv::Scalar(0x00, 0xff, 0x00);

    //rc image
    cv::Mat src1;
    
    cv::VideoCapture stream1(1); //0 is the id of video device.0 if you have only one camera.
    stream1.set(3,1280); // resolution X
    stream1.set(4,960); //resolution Y
    
            
    if (!stream1.isOpened())
    { //check if video device has been initialised
        std::cout << "Error loading video" << std::endl;
        return -1;
    }    
    
 //unconditional loop
    while (true)
    {
        stream1.read(src1);
        if (!src1.data)
            std::cout << "Image not loaded";
        else
        {
            //detect objects!
            std::vector<bbox_t> bboxes = darknet->detect(src1, 0.1, true);
            
            
            if (bboxes.size() > 0)
            {
                //check objects location relative to areas of interrest..
                for (auto obj_box : bboxes)
                {
                    //only mark higher than 30% probability
                    if (obj_box.prob > 0.15)
                    {
                        cv::Point obj_box_tl = cv::Point(obj_box.x, obj_box.y);
                        cv::Point obj_box_br = cv::Point(obj_box.x + obj_box.w, obj_box.y + obj_box.h);
                        cv::rectangle(src1, obj_box_tl, obj_box_br, obj_box_color, 2, 8, 0 );
                        
                        std::stringstream strstrm;
                        strstrm << names_list.at(obj_box.obj_id) << " " << (int)(obj_box.prob * 100) << "%";
                        cv::putText(src1, strstrm.str().c_str(), obj_box_tl, cv::FONT_HERSHEY_PLAIN, 2,  obj_box_color, 2);
                    }
                    
                    //cout << obj_box.prob << endl;
                 
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
        else if (x == 112) //s = save
        {
            cv::imwrite("outfile.jpg", src1);
        }
    }
    
    //close yolo..
    delete darknet;
    
    return 0;
}


