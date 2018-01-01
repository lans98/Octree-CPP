#include <pcl/visualization/cloud_viewer.h>
#include <pcl/visualization/point_picking_event.h>
#include <iostream>
#include <cstring>
#include "Tools.hpp"
#include "PointOctree.hpp"

namespace global {
  int    argc;
  char** argv;
}

void show_help() {
  std::cout << "|- Octree Demo -|\n";
  std::cout << " Usage: " << global::argv[0] << '\n';
  std::cout << " Example: " << global::argv[0] << " cube.off\n";
  std::cout << "\t-h    Muestra esta ayuda\n";
  std::cout << "\t-v    Muestra version\n";
}

void show_version() {
  std::cout << "Octree demo\n";
  std::cout << "version: 0.0.1\n";
}

void point_pick_func(const pcl::visualization::PointPickingEvent& e, void* p) {//es bucle. p es la dir del octree que contiene los puntos de la nube
  float x, y, z;
  e.getPoint(x,y,z);
  qoaed::Point3D<float> point(x,y,z);
  auto octree = *(std::shared_ptr<qoaed::PointOctree<pcl::PointXYZRGB*, float>>*) p;
  auto nvis = octree->find(point);

  std::cout << "Clicked " << point.x << ' ' << point.y << ' ' << point.z << '\n';

  if (!nvis) std::cerr << "Seems like the point isn't in the octree" << '\n';
  
  //ranged_query
//  octree->ranged_query(qoaed::PointOctree<pcl::PointXYZRGB*, float>::Cube
//    (1, 1, 1, 110, 110, 110),
//    [](auto& p) { (*p)->r = 255; (*p)->g = 0; (*p)->b = 0; }//como p contiene los puntos de la nube los cambios son a ellos
//  );

  //spheric_query
    double radio = 100;
    octree->spheric_query(point, radio,[](auto& p) { (*p)->r = 255; (*p)->g = 0; (*p)->b = 0;});

}

int main(int argc, char** argv) {
  global::argc = argc;
  global::argv = argv;

  if (argc < 2) {
    show_help();
    return 1;
  }

  if (std::strcmp(argv[1], "-h") == 0) {
    show_help();
    return 0;
  } 

  if (std::strcmp(argv[1], "-v") == 0) {
    show_version();
    return 0;
  }

  try {
    auto points = qoaed::tools::read_off<float>(argv[1]);
    auto octree = std::make_shared<qoaed::PointOctree<pcl::PointXYZRGB*, float>>();

    pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);
    pcl::visualization::CloudViewer cloud_viewer("Viewer");

    cloud->width  = points.size();
    cloud->height = 1;
    cloud->reserve(points.size());

    pcl::PointXYZRGB tmp_point;
    for (size_t ii = 0; ii < points.size(); ++ii) {
      tmp_point.x = points[ii].x;
      tmp_point.y = points[ii].y;
      tmp_point.z = points[ii].z;
      tmp_point.r = 255;
      tmp_point.g = 255;
      tmp_point.b = 255;
      cloud->push_back(tmp_point);
      octree->insert(points[ii].x, points[ii].y, points[ii].z, &cloud->back());
    }

    octree->visit_bfs([](auto& p) { 
      std::cout << p.get_x() << ' ' << p.get_y() << ' ' << p.get_z() << '\n'; 
    });


    cloud_viewer.registerPointPickingCallback(point_pick_func, &octree);
    cloud_viewer.showCloud(cloud);
    while (!cloud_viewer.wasStopped(100)) 
      cloud_viewer.showCloud(cloud);

  } catch (const std::exception& e) {
    std::cout << e.what() << '\n';
    return 1;
  }

  return 0;
}
