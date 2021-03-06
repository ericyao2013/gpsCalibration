/**
* Copyright (C) 2017-2018 Zhaorui Zhang (iMorpheusAI)
* For more information see <https://github.com/iMorpheusAI/gpsCalibration>
*
* gpsCalibration is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/

#ifndef CELLINFO_H
#define CELLINFO_H

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/common/transforms.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/registration/ndt.h>
#include <pcl/filters/radius_outlier_removal.h>
#include <pcl/filters/conditional_removal.h>
#include <pcl/registration/icp.h>
#include <pcl/common/transforms.h>
#include <pcl/console/parse.h>
#include <pcl/console/time.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/ransac.h>
#include <pcl/sample_consensus/sac_model_registration.h>
#include <pcl/kdtree/kdtree.h>

#include <opencv2/opencv.hpp>

#define CELLSIZE (5.0)
#define CELLPOINTCNTTH (5)
#define CELLCVROUND //采用四舍五入

struct CellsENUServerData
{
    CellsENUServerData()
    {
        ResetData();
    }
    CellsENUServerData(const CellsENUServerData& data)
    {
        m_N = data.m_N;
        m_Mean = data.m_Mean;
        m_Covariance = data.m_Covariance;
        m_NumInRepeatCells = data.m_NumInRepeatCells;
        m_Confidence = data.m_Confidence;
        m_Cell = data.m_Cell;
    }
    CellsENUServerData(size_t N, Eigen::Vector3d mean, Eigen::Matrix3d cov, pcl::PointXYZ cell, float conf = 0)
    {
        m_N = N;
        m_Mean = mean;
        m_Covariance = cov;
        m_Confidence = conf;
        m_Cell = cell;
    }
    ~CellsENUServerData() {}
    void ResetData()
    {
        m_N = 0;
        m_Mean(0) = 0; m_Mean(1) = 0; m_Mean(2) = 0;
        m_Covariance(0,0) = 0; m_Covariance(0,1) = 0; m_Covariance(0,2) = 0;
        m_Covariance(1,0) = 0; m_Covariance(1,1) = 0; m_Covariance(1,2) = 0;
        m_Covariance(2,0) = 0; m_Covariance(2,1) = 0; m_Covariance(2,2) = 0;
        m_NumInRepeatCells = 0;
        m_Confidence = 0;
        m_Cell.x = 0; m_Cell.y = 0; m_Cell.z = 0;
    }
    void operator=(const CellsENUServerData& data)
    {
        this->m_N = data.m_N;
        this->m_Mean = data.m_Mean;
        this->m_Covariance = data.m_Covariance;
        this->m_NumInRepeatCells = data.m_NumInRepeatCells;
        this->m_Confidence = data.m_Confidence;
        this->m_Cell = data.m_Cell;
    }

    size_t m_N;
    Eigen::Vector3d m_Mean;
    Eigen::Matrix3d m_Covariance;
    size_t m_NumInRepeatCells;
    float m_Confidence;
    pcl::PointXYZ m_Cell;
};

struct CellsNum
{
    CellsNum()
    {
        m_X = 0; m_Y = 0; m_Z = 0;
        m_Num = 0;
    }
    CellsNum(float _x, float _y, float _z, size_t _num):
        m_X(_x), m_Y(_y), m_Z(_z), m_Num(_num) {}
    ~CellsNum() {}

    float m_X;
    float m_Y;
    float m_Z;
    size_t m_Num;
};

class CellsENU
{
public:
    CellsENU();
    ~CellsENU();

    CellsENU(const pcl::PointCloud<pcl::PointXYZ>::Ptr inputCloud);

    CellsENU(const pcl::PointCloud<pcl::PointXYZ>::Ptr inputCloud, const float cellSize);

    CellsENU(const CellsENU& inputCellsENU);

    float GetCellSize() const;

    void SetCellSize(float cellSize);

    void AddCellData(pcl::PointXYZ cell, CellsENUServerData data);//不更改KDTree

    void UpdateCellData(size_t num, CellsENUServerData data);

    //创建Kd-tree用来查找点对应的cell
    void CreateKDTree();

    pcl::PointCloud<pcl::PointXYZ>::Ptr GetCellsCloud() const;

    std::vector<CellsENUServerData> GetCellsData() const;

    void CreateCells(const pcl::PointCloud<pcl::PointXYZ>::Ptr inputCloud);

    pcl::KdTreeFLANN<pcl::PointXYZ> GetCellsKDTree() const;

    void AddVerticalVec_Point(std::pair<Eigen::Vector3d, pcl::PointXYZ> verticalVec_Point);

    std::vector<std::pair<Eigen::Vector3d, pcl::PointXYZ> > GetVerticalVec_Points() const;

    void ResetData();

    void operator=(const CellsENU& cellsData);

    //File format:
    //#base
    //cellSize Xr Yr
    //#vertical vector
    //Xc Yc Zc Xv Yv Zv
    //#data
    //...
    bool ReadCellsDataFromFile(std::string path);

    bool WriteCellsDataToFile(std::string filePath);

private:
    pcl::PointCloud<pcl::PointXYZ>::Ptr ResampleCloud(const pcl::PointCloud<pcl::PointXYZ>::Ptr input,
                                                      const std::vector<float> leafsize);

    void InsertCellsENUServerData(CellsENUServerData data, pcl::PointXYZ cell);

    //结构体排序的sort算法
    static bool SortComp(const CellsNum &a, const CellsNum &b);

    std::vector<CellsNum> m_RepeatCellsNum;
    pcl::PointCloud<pcl::PointXYZ>::Ptr m_Cells;
    pcl::KdTreeFLANN<pcl::PointXYZ> m_CellsKDTree;

    //base
    float m_CellSize;
    std::vector<size_t> m_CoordRef;
    //vertical vector
    std::vector<std::pair<Eigen::Vector3d, pcl::PointXYZ>> m_VerticalVec_Point;
    //data
    std::vector<CellsENUServerData> m_CellsENUServerData;

};

#endif //CELLINFO_H
