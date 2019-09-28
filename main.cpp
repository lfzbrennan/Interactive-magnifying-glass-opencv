//
//  main.cpp
//  cv2_testing_stuff
//
//  Created by Liam Brennan


#include <iostream>
#include "opencv2/core/cvdef.h"
#include "opencv2/core/version.hpp"
#include "opencv2/core/base.hpp"
#include "opencv2/core/cvstd.hpp"
#include "opencv2/core/traits.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/videoio/videoio.hpp"
#include <stdint.h>
#include <vector>
#include <dirent.h>

using namespace std;
using namespace cv;

int zoomNum = 2;

void detectAndDisplay(Mat& frame);

Mat overlayImage(Mat &background, Mat &forground, Point2i p);

Size findSize();

Point drawEllipse(Mat &m);

vector<vector<int>> buildMask(int x, int y);

// for haar cascade application (not yet implemented)

CascadeClassifier c_eyes;
CascadeClassifier c_face;

double o_eye_size_1;
double o_eye_size_2;

int width_glass = 880;
int height_glass = 793;


const int start_glass_w = 460;
const int start_glass_h = 375;

int curr_glass_w;
int curr_glass_h;

int mouse_pos_x;
int mouse_pos_y;

Point glassCenter;
int centerRad;

Mat drawZoom(Mat &f, Mat &f_f, Point p, Mat &m, vector<vector<int>> tMask);

Mat zoom;

// controls placement of magnifying glass based on mouse

void mousePos(int event, int x, int y, int flags, void* userdata) {
    mouse_pos_x = x - curr_glass_w / 2.56;
    mouse_pos_y = y - curr_glass_h / 2.0;
    
    
    int maxX = 1280 - (curr_glass_w - ( curr_glass_w / 4));
    int maxY = 720 - (curr_glass_h - (curr_glass_h / 9));
    int minX = -(curr_glass_w / 11);
    int minY = -(curr_glass_h / 9);
    
    if (mouse_pos_x > maxX) {
        mouse_pos_x = maxX;
        //cout << "left stop" << endl;
    } else if (mouse_pos_x < minX) {
        mouse_pos_x = minX;
        //cout << "right stop" << endl;
    }
    
    if (mouse_pos_y > maxY) {
        mouse_pos_y = maxY;
        //cout << "top stop" << endl;
    } else if (mouse_pos_y < minY) {
        mouse_pos_y = minY;
        //cout << "bottom stop" << endl;
    }
    
}

// returns transparency based on euclidean distance

int giveTransparency(int x, int y, int rad) {
    
    x -= rad/2;
    y -= rad/2;
    
    int euclidD = sqrt((x*x) + (y*y));
    
    //euclidD = 100;
    
    if (euclidD > (rad / 2)) {
        return 0;
    }
    
    return 255;
}

// create circular transparency mask

vector<vector<int>> buildMask(int h, int w) {
    
    cout << h << endl;
    cout << w << endl;
    
    vector<vector<int>> temp;
    
    for (int i = 0; i < h; i++) {
        
        vector<int> temp2;
        for (int j = 0; j < w; j++) {
            temp2.push_back(giveTransparency(i, j, 134));
        }
        temp.push_back(temp2);
    }
    
    return temp;
}

// find inverse color for maximum visibility

Scalar getInverseColor(Point p, Mat &f) {
    
    int half_way_ish = p.x + 20;
    int length_ish = 400;
    int offset = 40;
    
    int times = 10;
    int b = 0;
    int g = 0;
    int r = 0;
    
    for (int i = offset; i < (length_ish + offset); i += (length_ish / times)) {
        
        circle(f, Point(i, half_way_ish), 3, Scalar(0, 255, 255), 4);
        
        Vec3b in = f.at<Vec3b>(i, half_way_ish);
        b += float(in[0]);
        g += float(in[1]);
        r += float(in[2]);
    }
    
    b /= (times);
    g /= (times);
    r /= (times);
    
    
    Scalar s = Scalar(255 - b, 255 - g, 255 - r);
    
    //cout << s << endl;
    
    return s;
    
}

int main(int argc, const char * argv[]) {
    
    // create reference mask to create circular zoom area
    
    vector<vector<int>> transparencyMask(centerRad, vector<int>(centerRad));
    
    transparencyMask = buildMask(134, 134);
    
    Mat glass;
    glass = imread("MUST_GLASS.png", IMREAD_UNCHANGED);
    
    Mat ROI(glass, Rect(width_glass - 500, 30, start_glass_w, start_glass_h));
    
    curr_glass_w = start_glass_w;
    curr_glass_h = start_glass_h;
    
    ROI.copyTo(glass);
    
    Size s = findSize();
    
    resize(glass, glass, s);
    //glass.convertTo(glass, CV_32FC3, 1.0/255);
    
    srand(0);
    
    cout << "TYPE IS: " << glass.type() << endl;
    
    
    for (int i = 0; i < 100; i++) {
        
        //cout << glass.at<double>();
        //cout << rand() % 10 << endl;
    }
    
    VideoCapture cap;
    
    cap.open(0);
    
    bool c = true;
    
    // wait for permission go open webcam
    
    if (!cap.isOpened()) {
        cout << "Waiting to instantialize" << endl;
        cin.get();
        cap.open(0);
    }
    
    
    
    if (!cap.isOpened()) {
        cout << "Error opening Webcam" << endl;
        return -1;
    }
    string winName = "Webcam";
    namedWindow(winName, WINDOW_AUTOSIZE);
    
    setMouseCallback(winName, mousePos, NULL);
    
    int cap_height = cap.get(CAP_PROP_FRAME_HEIGHT);
    int cap_width = cap.get(CAP_PROP_FRAME_WIDTH);
    
    cout << "Webcam Height: " << cap_height << endl;
    cout << "Webcam Width: " << cap_width << endl;
    
    double fps = cap.get(CAP_PROP_FPS);
    
    
    // to record video if needed >>
    //VideoWriter video("first_demo.wmv", VideoWriter::fourcc('W','M','V','2'), fps, Size(cap_width, cap_height));
    
    /*
    if (!video.isOpened()) {
        cout << "VIDEO BROKE" << endl;
    }
     */
    
    int esc_t_h;
    int esc_t_w;
    
    int number_size = 3;
    int text_size = 1;
    
    // referenced colors
    
    Scalar black = Scalar(0, 0, 0);
    Scalar white = Scalar(255, 255, 255);
    Scalar red = Scalar(0, 0, 255);
    
    int count = 0;
    
    int startCountDown = 6;
    
    Size textSize;
    
    
    while(true) {
        
        Mat frame;
        
        Mat frame_instance;
        
        int fps = cap.get(CAP_PROP_FPS);
        
        bool success = cap.read(frame);
        
        esc_t_w = frame.cols - (frame.cols - (frame.cols / 20.0));
        esc_t_h = frame.rows - (frame.rows - (frame.rows / 20.0));
        
        if (!success) {
            cout << "Camera has been disconnected" << endl;
            break;
        }
        
        //detectAndDisplay(frame);
        
        
        frame_instance = frame;
        Scalar x = getInverseColor(Point(esc_t_h, esc_t_w), frame_instance);
        putText(frame, "Press ESC key to exit", Point(esc_t_h, esc_t_w), FONT_HERSHEY_COMPLEX, text_size, x, 3);
        
        // haar cascade countdown (not fully used yet)
        
        if (c) {
            if (count % fps == 0) {
                if ((startCountDown - 1) != 0) {
                    cout << "Countdown changed" << endl;
                    startCountDown -= 1 ;
                }
                else {
                    c = false;
                }
            }
            textSize = cv::getTextSize(to_string(startCountDown), FONT_HERSHEY_COMPLEX, text_size, 3, 0);
            putText(frame, to_string(startCountDown), Point((frame.cols/2.0 - textSize.height), frame.rows/2.0 - textSize.width), FONT_HERSHEY_COMPLEX, number_size, white, 3);
        }
        
        
        glassCenter = drawEllipse(glass);
        
        // create the overlays
        
        frame = overlayImage(frame, glass, Point2i(mouse_pos_x, mouse_pos_y));
        frame = drawZoom(frame_instance, frame, glassCenter, glass, transparencyMask);
        
        //video.write(frame);
        
        
        //line(frame, Point(0, frame.rows/2.0), Point(frame.cols, frame.rows/2.0), red, 2);
        //line(frame, Point(frame.cols/2.0, 0), Point(frame.cols/2.0, frame.rows), red, 2);
        
        //circle(frame, glassCenter, 66, Scalar(255, 0, 0));

        imshow(winName, frame);
        
        if (waitKey(10) == 27) {
            cout << "Escape key has been pressed" << endl;
            break;
        }
        
        count++;
    }
    
    //video.release();
    cap.release();
    
    destroyAllWindows();
    
    return 0;
}

// overlay semi-transparent png onto webcam output

Mat overlayImage(Mat &bg, Mat &fg, Point p) {
    
    Mat result;
    
    bg.copyTo(result);
    
    for (int y = max(p.y, 0); y < bg.rows; y++) {
        int fy = y - p.y;
        
        if (fy > fg.rows) {
            break;
        }
        
        for (int x = max(p.x, 0); x < bg.cols; x++)  {
            int fx = x - p.x;
            if (fx > fg.cols) {
                break;
            }
            
            double opacity = ((double)fg.data[fy * fg.step + fx * fg.channels() + 3]) / 255.;
            
            for (int c = 0; opacity > 0 && c < result.channels(); c++) {
                
                unsigned char fgPx = fg.data[fy * fg.step + fx * fg.channels() + c];
                unsigned char bgPx = bg.data[y * bg.step + x * bg.channels() + c];
                
                result.data[y * result.step + x * result.channels() + c] = bgPx * (1 - opacity) + fgPx * opacity;
            }
            
        }
    }
    
    return result;
}

// create the "zoom" effect

Mat drawZoom(Mat &f, Mat &f_f, Point p, Mat &m, vector<vector<int>> tMask) {
    
    Mat zoom = Mat::zeros(centerRad, centerRad, CV_8UC3);

    int x_offset = mouse_pos_x + (centerRad / (2/zoomNum));
    int y_offset = mouse_pos_y + (centerRad / (2/zoomNum));
    
    for (int r = 0; r < zoom.rows; r++) {
        for (int c = 0; c < zoom.cols; c++) {
            
            int f_y = r + y_offset;
            int f_x = c + x_offset;
            
            zoom.at<cv::Vec3b>(r, c) = f.at<cv::Vec3b>(f_y, f_x);
        }
    }
    
    resize(zoom, zoom, Size(zoomNum * zoom.rows, zoomNum * zoom.cols));
    
    Point centerShift = glassCenter;
    centerShift.x -= centerRad / zoomNum;
    centerShift.y -= centerRad /zoomNum;
    
    cvtColor(zoom, zoom, COLOR_BGR2BGRA);
    
    for (int r = 0; r < zoom.rows; r++) {
        //tMask[r] = vector<int>(r);
        for (int c = 0; c < zoom.cols; c++) {
            auto& pix = zoom.at<Vec4b>(r, c);
            pix[3] = tMask[r][c];
        }
    }
    
    zoom = overlayImage(f_f, zoom, Point(mouse_pos_x + centerRad / (1.5 * zoomNum), mouse_pos_y + centerRad / (1.2 * zoomNum)));
    
    return zoom;
    
}

// new size of magnifying glass

Size findSize() {
    
    double div = 2;
    
    curr_glass_w /= div;
    curr_glass_h /= div;
    
    Size s = Point(curr_glass_w, curr_glass_h);
    
    return s;
}

// locate zoomed area

Point drawEllipse(Mat &m) {
    centerRad = m.cols / 3.42;
    
    glassCenter = Point(m.rows / 2.1, m.cols / 2.45);
    //circle (m, glassCenter, centerRad, Scalar(0, 0, 255, 255), 2);
    
    return glassCenter;
}

// applying haar cascades

void detectAndDisplay(Mat& frame) {
    
    vector<Rect> faces;
    
    
}


