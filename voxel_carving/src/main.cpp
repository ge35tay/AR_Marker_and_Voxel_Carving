#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkPointData.h>
#include <vtkPLYWriter.h>
#include <vtkFloatArray.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkMarchingCubes.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);


const int IMG_WIDTH = 1280;
const int IMG_HEIGHT = 960;
const int VOXEL_DIM = 128;
const int VOXEL_SIZE = VOXEL_DIM * VOXEL_DIM * VOXEL_DIM;
const int VOXEL_SLICE = VOXEL_DIM * VOXEL_DIM;
const int OUTSIDE = 0;


struct coord
{
    int x;
    int y;
};

struct voxel
{
    float xpos;
    float ypos;
    float zpos;
    float res;
    float value;
};

struct startParams
{
    float startX;
    float startY;
    float startZ;
    float voxelWidth;
    float voxelHeight;
    float voxelDepth;
};

struct camera
{
    cv::Mat Image;
    cv::Mat P;
    cv::Mat K;
    cv::Mat R;
    cv::Mat t;
    cv::Mat Silhouette;
};

coord project(camera cam, voxel v)
{   
    // Project voxel into camera image coords
    coord img;

    float z = cam.P.at<float>(2, 0) * v.xpos +
            cam.P.at<float>(2, 1) * v.ypos +
            cam.P.at<float>(2, 2) * v.zpos +
            cam.P.at<float>(2, 3);

    img.x = (cam.P.at<float>(0, 0) * v.xpos +
            cam.P.at<float>(0, 1) * v.ypos +
            cam.P.at<float>(0, 2) * v.zpos +
            cam.P.at<float>(0, 3)) / z;
    
    img.y = (cam.P.at<float>(1, 0) * v.xpos +
            cam.P.at<float>(1, 1) * v.ypos +
            cam.P.at<float>(1, 2) * v.zpos +
            cam.P.at<float>(1, 3)) / z;

    return img;
}

void exportModel(char *filename, vtkPolyData *poly_data)
{
    // Export 3D model in ply format
    vtkSmartPointer<vtkPLYWriter> plyExporter = vtkSmartPointer<vtkPLYWriter>::New();
    plyExporter->SetFileName(filename);
    plyExporter->SetInputData(poly_data);
    plyExporter->Update();
    plyExporter->Write();
}

void render_model(float fArray[], startParams params)
{
    // Create vtk visualization pipeline from voxel grid (float array)
    vtkSmartPointer<vtkStructuredPoints> sPoints = vtkSmartPointer<vtkStructuredPoints>::New();
    sPoints->SetDimensions(VOXEL_DIM, VOXEL_DIM, VOXEL_DIM);
    sPoints->SetSpacing(params.voxelDepth, params.voxelHeight, params.voxelWidth);
    sPoints->SetOrigin(params.startZ, params.startY, params.startX);
    // todo: here may need to be converted to float
    // sPoints->AllocateScalars(VTK_FLOAT, VOXEL_DIM * VOXEL_DIM * VOXEL_DIM);

    vtkSmartPointer<vtkFloatArray> vtkFArray = vtkSmartPointer<vtkFloatArray>::New();
    vtkFArray->SetNumberOfValues(VOXEL_SIZE);
    vtkFArray->SetArray(fArray, VOXEL_SIZE, 1);

    sPoints->GetPointData() ->SetScalars(vtkFArray);
    sPoints-> GetPointData() -> Update();   // ? 

    // Create iso surface with marching cubes algorithm
    vtkSmartPointer<vtkMarchingCubes> mcSource = vtkSmartPointer<vtkMarchingCubes>::New();
    mcSource->SetInputData(sPoints);
    mcSource->SetNumberOfContours(1);
    mcSource->SetValue(0, 0.5);
    mcSource->Update();


    // Recreate mesh with topology and merge vertices
    vtkSmartPointer<vtkCleanPolyData> cleanPolyData = vtkSmartPointer<vtkCleanPolyData>::New();
    cleanPolyData->SetInputConnection(mcSource->GetOutputPort());
    cleanPolyData->Update();


    /* usual render stuff */
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(.45, .45, .9);
    renderer->SetBackground2(.0, .0, .0);
    renderer->GradientBackgroundOn();


    vtkSmartPointer<vtkRenderWindow> render_window = vtkSmartPointer<vtkRenderWindow>::New();
    render_window->AddRenderer(renderer);
    vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(render_window);


    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper -> SetInputConnection(cleanPolyData -> GetOutputPort());


    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor -> SetMapper(mapper);



    /* visible light properties */
    actor->GetProperty()->SetSpecular(0.15);
    actor->GetProperty()->SetInterpolationToPhong();
    renderer->AddActor(actor);
    
    render_window->Render();
    interactor->Start();
}

void carve(float fArray[], startParams params, camera cam)
{
    cv::Mat silhouette, distImage;
    cv::Canny(cam.Silhouette, silhouette, 0, 255);
    cv::bitwise_not(silhouette, silhouette);
    cv::distanceTransform(silhouette, distImage, cv::DIST_L2, 3);


    for (int i=0; i < VOXEL_DIM; i++)
    {
        for (int j = 0; j < VOXEL_DIM; j++)
        {
            for (int k = 0; k < VOXEL_DIM; k++)
            {

                // calc voxel position inside view frustum
                voxel v;
                v.xpos = params.startX + i * params.voxelWidth;
                v.ypos = params.startY + j * params.voxelHeight;
                v.zpos = params.startZ + k * params.voxelDepth;
                v.value = 1.0f;

                coord im = project(cam, v);
                float dist = -1.0f;

                // test if projected voxel is within image coords
                if (im.x > 0 && im.y > 0 && im.x < IMG_WIDTH && im.y <IMG_HEIGHT)
                {
                    dist = distImage.at<float>(im.y, im.x);
                    if (cam.Silhouette.at<uchar>(im.y, im.x) == OUTSIDE)
                    {
                        dist *= -1.0f;
                    }
                }

                if (dist < fArray[i * VOXEL_SLICE + j * VOXEL_DIM+k])
                {
                    fArray[i * VOXEL_SLICE + j * VOXEL_DIM+k] = dist;
                }
            }
        }
    }
}


int main(int argc, char* argv[])
{
    /* acquire camera images, silhouettes and camera matrix */
    std::vector<camera> cameras;

    cv::FileStorage fs("../source/viff.xml", cv::FileStorage::READ);
    for (int i=0; i<36; i++) {
        
        /* camera image */
        std::stringstream simg;
        simg << "../source/image_" << i << ".jpg";
        cv::Mat img = cv::imread(simg.str());
        
        /* silhouette */
        cv::Mat silhouette;
        cv::cvtColor(img, silhouette, cv::COLOR_BGR2HSV);
        cv::inRange(silhouette, cv::Scalar(0, 0, 30), cv::Scalar(255,255,255), silhouette);
        
        /* camera matrix */
        std::stringstream smat;
        smat << "viff" << std::setfill('0') << std::setw(3) << i << "_matrix";
        cv::Mat P;
        fs[smat.str()] >> P;
        
        /* decompose proj matrix to cam- and rot matrix and trans vect */
        cv::Mat K, R, t;
        cv::decomposeProjectionMatrix(P, K, R, t);
        K = cv::Mat::eye(3, 3, CV_32FC1);
        K.at<float>(0,0) = 1680.2631413061415; /* fx */
        K.at<float>(1,1) = 1676.1202869984309; /* fy */
        K.at<float>(0,2) = 621.59194200994375; /* cx */
        K.at<float>(1,2) = 467.7223229477861; /* cy */
        
        camera c;
        c.Image = img;
        c.P = P;
        c.K = K;
        c.R = R;
        c.t = t;
        c.Silhouette = silhouette;
                
        cameras.push_back(c);
    }
    
    /* bounding box dimensions of squirrel */
    float xmin = -6.21639, ymin = -10.2796, zmin = -14.0349;
    float xmax = 7.62138, ymax = 12.1731, zmax = 12.5358;
            
    float bbwidth = std::abs(xmax-xmin)*1.15;
    float bbheight = std::abs(ymax-ymin)*1.15;
    float bbdepth = std::abs(zmax-zmin)*1.05;
    
    startParams params;
    params.startX = xmin-std::abs(xmax-xmin)*0.15;
    params.startY = ymin-std::abs(ymax-ymin)*0.15;
    params.startZ = 0.0f;
    params.voxelWidth = bbwidth/VOXEL_DIM;
    params.voxelHeight = bbheight/VOXEL_DIM;
    params.voxelDepth = bbdepth/VOXEL_DIM;
    
    /* 3 dimensional voxel grid */
    float *fArray = new float[VOXEL_SIZE];
    std::fill_n(fArray, VOXEL_SIZE, 1000.0f);
    
    /* carving model for every given camera image */
    for (int i=0; i<36; i++) {
        carve(fArray, params, cameras.at(i));
    }

    /* show example of segmented image */
    cv::Mat original, segmented;
    cv::resize(cameras.at(1).Image, original, cv::Size(640, 480));
    cv::resize(cameras.at(1).Silhouette, segmented, cv::Size(640, 480));
    cv::imshow("Squirrel" , original);
    cv::imshow("Squirrel Silhouette", segmented);
    
    render_model(fArray, params);
    
    return 0;
}
