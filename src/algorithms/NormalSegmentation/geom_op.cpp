#include "geom_op.h"

/** VECTORS **/
geom::vectors::vector3* geom::vectors::create_vect2p(pcl::PointXYZRGB pt1, pcl::PointXYZRGB pt2)
{
    return new geom::vectors::vector3(pt1.x - pt2.x, pt1.y - pt2.y, pt1.z - pt2.z);
}

geom::vectors::vector3* geom::vectors::cross_product(geom::vectors::vector3 vect1, geom::vectors::vector3 vect2)
{
    geom::vectors::vector3 *cross_product;   // the result of the cross product between the two parameter vectors

    // calculating cross product
    cross_product = new geom::vectors::vector3();
    cross_product->set_x((vect1.get_y() * vect2.get_z()) - (vect1.get_z() * vect2.get_y()));
    cross_product->set_y((vect1.get_z() * vect2.get_x()) - (vect1.get_x() * vect2.get_z()));
    cross_product->set_z((vect1.get_x() * vect2.get_y()) - (vect1.get_y() * vect2.get_x()));

    return cross_product;
}

geom::vectors::vector3* geom::vectors::inverse(vector3 vect1)
{
    vector3 *temp_vect;

    temp_vect = new vector3(vect1.get_x(), vect1.get_y(), vect1.get_z());
    temp_vect->set_x(-vect1.get_x());
    temp_vect->set_y(-vect1.get_y());
    temp_vect->set_z(-vect1.get_z());

    return temp_vect;
}

geom::vectors::vector3* geom::vectors::translate_origin(float x1, float y1, float z1, float x2, float y2, float z2)
{
    vector3 *temp_vect;
    float x, y, z;  // translated vector's coordinates

    // calculating the coordinates of the translated vector
    x = x2 - x1;
    y = y2 - y1;
    z = z2 - z1;

    // creating and returning the new vector
    temp_vect = new vector3(x, y, z);
    return temp_vect;
}

geom::vectors::vector3* geom::vectors::scale(vector3 vect1, vector3 vect2)
{
    vector3 *temp_vect;

    temp_vect = new vector3(vect1.get_x(), vect1.get_y(), vect1.get_z());
    temp_vect->set_x(vect1.get_x() * vect2.get_x());
    temp_vect->set_y(vect1.get_y() * vect2.get_y());
    temp_vect->set_z(vect1.get_z() * vect2.get_z());

    return temp_vect;
}

float geom::vectors::dist(vector3 vect1, vector3 vect2)
{
    float x, y, z;  // coordinates

    x = vect1.get_x() - vect2.get_x();
    y = vect1.get_y() - vect2.get_y();
    z = vect1.get_z() - vect2.get_z();

    return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
}

geom::vectors::vector3*  geom::vectors::normalize_normal(vector3 normal)
{
    // norm of the normal
    float length;

    // normalized normal
    geom::vectors::vector3 *normalized_normal;

    // initializing the norm
    if (normal.get_magn() == 0.0f)
        length = 1.0f;

    else
        length = normal.get_magn();

    // normalizing normal
    normalized_normal = new geom::vectors::vector3(normal.get_x() / length, normal.get_y() / length, normal.get_z() / length);

    return normalized_normal;
}

geom::vectors::vector3* geom::vectors::vect_avg(std::vector<geom::vectors::vector3> vectors)
{
    // new vector coordinates
    float x, y, z;

    // averaged vector
    geom::vectors::vector3 *avg_vect;

    // initializing vector coordinates
    x = y = z = 0;

    // summing up
    for (auto vect : vectors)
    {
        x += vect.get_x();
        y += vect.get_y();
        z += vect.get_z();
    }

    // averaging
    x = x / vectors.size();
    y = y / vectors.size();
    z = z / vectors.size();

    // creating new vector
    avg_vect = new geom::vectors::vector3(x, y, z);

    return avg_vect;
}


void geom::vectors::estim_normals(pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc, float range)
{
    // kd-tree used for finding neighbours
    pcl::KdTreeFLANN<pcl::PointXYZRGB> kdt;

    // auxilliary vectors for the k-tree nearest search
    std::vector<int> pointIdxRadiusSearch; // neighbours ids
    std::vector<float> pointRadiusSquaredDistance; // distances from the source to the neighbours

    // the vectors of which the cross product calculates the normal
    geom::vectors::vector3 *vect1;
    geom::vectors::vector3 *vect2;
    geom::vectors::vector3 *cross_prod;
    geom::vectors::vector3 *abs_cross_prod;

    // vectors to average
    std::vector<geom::vectors::vector3> vct_toavg;
    geom::vectors::vector3 *vct_avg;

    // the result of the cross product of the two previous vectors
    geom::vectors::vector3 *normal;

    // cloud iterator
    pcl::PointCloud<pcl::PointXYZRGB>::iterator cloud_it;

    // initializing tree
    kdt.setInputCloud(pc);

    for (cloud_it = pc->points.begin(); cloud_it < pc->points.end(); cloud_it++)
    {
        // if there are neighbours left
        if (kdt.radiusSearch(*cloud_it, range, pointIdxRadiusSearch, pointRadiusSquaredDistance, 100) > 0)
        {

            for (int pt_index = 0; pt_index < (pointIdxRadiusSearch.size() - 1); pt_index++)
            {
                // defining the first vector
                vect1 = geom::vectors::create_vect2p((*cloud_it), pc->points[pointIdxRadiusSearch[pt_index + 1]]);

                // defining the second vector; making sure there is no 'out of bounds' error
                if (pt_index == pointIdxRadiusSearch.size() - 2)
                    vect2 = geom::vectors::create_vect2p((*cloud_it), pc->points[pointIdxRadiusSearch[1]]);

                else
                    vect2 = geom::vectors::create_vect2p((*cloud_it), pc->points[pointIdxRadiusSearch[pt_index + 2]]);

                // adding the cross product of the two previous vectors to our list
                cross_prod = geom::vectors::cross_product(*vect1, *vect2);
                abs_cross_prod = geom::aux::abs_vector(*cross_prod);
                vct_toavg.push_back(*abs_cross_prod);

                // freeing memory
                delete cross_prod;
                delete abs_cross_prod;
                delete vect1;
                delete vect2;
            }

            // calculating the normalized normal
            vct_avg = geom::vectors::vect_avg(vct_toavg);
            normal = geom::vectors::normalize_normal(*vct_avg);

            // calculating point colors and adding it to the result list
            geom::aux::norm_toPtRGB(&(*(cloud_it)), *normal);

            // freeing memory
            delete vct_avg;
            delete normal;

            // clearing vectors
            vct_toavg.clear();
            pointIdxRadiusSearch.clear();
            pointRadiusSquaredDistance.clear();

            // shrinking vectors
            vct_toavg.shrink_to_fit();
            pointIdxRadiusSearch.shrink_to_fit();
            pointRadiusSquaredDistance.shrink_to_fit();
        }
    }
}

std::vector<std::pair<pcl::PointXYZRGB *, std::vector<float>>> geom::vectors::estim_normals_spherical(pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc, float range)
{
   // the dictionary to be returned
    std::vector<std::pair<pcl::PointXYZRGB *, std::vector<float>>> normal_vects;

    // kd-tree used for finding neighbours
    pcl::KdTreeFLANN<pcl::PointXYZRGB> kdt;

    // auxilliary vectors for the k-tree nearest search
    std::vector<int> pointIdxRadiusSearch; // neighbours ids
    std::vector<float> pointRadiusSquaredDistance; // distances from the source to the neighbours

    // the vectors of which the cross product calculates the normal
    geom::vectors::vector3 *vect1;
    geom::vectors::vector3 *vect2;
    geom::vectors::vector3 *cross_prod;
    geom::vectors::vector3 *abs_cross_prod;
    geom::vectors::vector3 *normal;

    // vectors to average
    std::vector<geom::vectors::vector3> vct_toavg;

    // the result of the cross product of the two previous vectors
    std::vector<float> spherical_coords;

    // cloud iterator
    pcl::PointCloud<pcl::PointXYZRGB>::iterator cloud_it;

    // initializing tree
    kdt.setInputCloud(pc);

    for (cloud_it = pc->points.begin(); cloud_it < pc->points.end(); cloud_it++)
    {
        // if there are neighbours left
        if (kdt.radiusSearch(*cloud_it, range, pointIdxRadiusSearch, pointRadiusSquaredDistance, 100) > 0)
        {

            for (int pt_index = 0; pt_index < (pointIdxRadiusSearch.size() - 1); pt_index++)
            {
                // defining the first vector
                vect1 = geom::vectors::create_vect2p((*cloud_it), pc->points[pointIdxRadiusSearch[pt_index + 1]]);

                // defining the second vector; making sure there is no 'out of bounds' error
                if (pt_index == pointIdxRadiusSearch.size() - 2)
                    vect2 = geom::vectors::create_vect2p((*cloud_it), pc->points[pointIdxRadiusSearch[1]]);


                else
                    vect2 = geom::vectors::create_vect2p((*cloud_it), pc->points[pointIdxRadiusSearch[pt_index + 2]]);

                // adding the cross product of the two previous vectors to our list
                cross_prod = geom::vectors::cross_product(*vect1, *vect2);
                abs_cross_prod = geom::aux::abs_vector(*cross_prod);
                vct_toavg.push_back(*abs_cross_prod);

                // freeing memory
                delete vect1;
                delete vect2;
                delete cross_prod;
                delete abs_cross_prod;
            }

            // calculating the normalized normal
            normal = geom::vectors::vect_avg(vct_toavg);

            // calculating spherical coords
            spherical_coords = geom::aux::calc_sphcoord(*normal);

            // adding the result to the dictionary
            normal_vects.push_back(std::pair<pcl::PointXYZRGB *, std::vector<float>>(&(*cloud_it), spherical_coords));

            // freeing memory
            delete normal;

            // clearing vectors
            vct_toavg.clear();
            pointIdxRadiusSearch.clear();
            pointRadiusSquaredDistance.clear();

            // shrinking vectors
            vct_toavg.shrink_to_fit();
            pointIdxRadiusSearch.shrink_to_fit();
            pointRadiusSquaredDistance.shrink_to_fit();
        }
    }

    return normal_vects;
}

void geom::vectors::pcl_estim_normals(pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc)
{
      // Create the normal estimation class, and pass the input dataset to it
      pcl::NormalEstimation<pcl::PointXYZRGB, pcl::Normal> ne;
      ne.setInputCloud(pc);

      // Create an empty kdtree representation, and pass it to the normal estimation object.
      // Its content will be filled inside the object, based on the given input dataset (as no other search surface is given).
      pcl::search::KdTree<pcl::PointXYZRGB>::Ptr tree (new pcl::search::KdTree<pcl::PointXYZRGB> ());
      ne.setSearchMethod (tree);

      // Output datasets
      pcl::PointCloud<pcl::Normal>::Ptr cloud_normals (new pcl::PointCloud<pcl::Normal>);

      // Use all neighbors in a sphere of radius 3cm
      ne.setRadiusSearch (0.03);

      // Compute the features
      ne.compute (*cloud_normals);
}


/** AUX **/
float geom::aux::calc_avg(std::vector<float> floats)
{
    float sum = 0;  // sum
    float avg = 0;  // average

    for (auto f : floats)
    {
        sum += f;
    }

    avg = sum / floats.size();

    return avg;
}

float geom::aux::calc_angle3p(float x1, float y1, float x2, float y2, float x3, float y3)
{
    float dist12;   // distance from point 1 to point 2
    float dist13;   // distance from point 1 to point 3
    float dist23;   // distance from point 2 to point 3
    float val_ang;  // the value of the angle calculated using arccos

    // establishing the distances between the different points
    dist12 = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    dist13 = sqrt(pow(x3 - x1, 2) + pow(y3 - y1, 2));
    dist23 = sqrt(pow(x3 - x2, 2) + pow(y3 - y2, 2));

    // calculating angle value
    val_ang = std::acos((pow(dist12, 2) + pow(dist23, 2) - pow(dist13,2)) / (2 * dist12 * dist13)) * 180/PI;
    return val_ang;
}

bool geom::aux::cmp_norm(vectors::vector3 vect1, vectors::vector3 vect2)
{
    bool similar = true;

    if ((std::abs(vect1.get_x()) > std::abs(vect2.get_x() + 35))
            || (std::abs(vect1.get_y()) > std::abs(vect2.get_y() + 35))
            || (std::abs(vect1.get_z()) > std::abs(vect2.get_z() + 35)))
        similar = false;

    return similar;
}

void geom::aux::norm_toPtRGB(pcl::PointXYZRGB *pt, geom::vectors::vector3 normal)
{
    pt->r = normal.get_x() * 255;
    pt->g = normal.get_y() * 255;
    pt->b = normal.get_z() * 255;
}

std::vector<float> geom::aux::calc_sphcoord(vectors::vector3 vect)
{
    // cartesian coordinates
    float x, y, z;

    // spherical coordinates
    float r;    // radius; the distance from the origin to the point in the YZ plane
    float theta;    // polar angle; the angle that the radius forms with the Z axis
    float phi;  // azimuth; the angle that the projection of the radius onto the XY axis forms with the X axis
    std::vector<float> sph_coord;   // regroups the 3 other variables

    // initializing cartesian coordinates
    x = vect.get_x();
    y = vect.get_y();
    z = vect.get_z();

    // calculating r, phi and theta with a precision of 2 digits after the decimal point
    r = std::sqrt(pow(x, 2) + pow(y, 2) +pow(z, 2));
    theta = std::acos(z / r);
    phi = std::atan(y / z);

    // adding values to the vector
    sph_coord.push_back(r);
    sph_coord.push_back(theta);
    sph_coord.push_back(phi);

    return sph_coord;
}

bool geom::aux::cmp_angles(std::vector<float> vect1, std::vector<float> vect2, float epsilon)
{
    if ((std::abs(vect1[1]) < (std::abs(vect2[1])- epsilon)) || (std::abs(vect1[1]) > (std::abs(vect2[1]) + epsilon)))
        return false;

    if ((std::abs(vect1[2]) < (std::abs(vect2[2])- epsilon)) || (std::abs(vect1[2]) > (std::abs(vect2[2]) + epsilon)))
        return false;

    return true;
}

geom::vectors::vector3* geom::aux::abs_vector(geom::vectors::vector3 vect)
{
    return new geom::vectors::vector3(std::abs(vect.get_x()), std::abs(vect.get_y()), std::abs(vect.get_z()));
}
