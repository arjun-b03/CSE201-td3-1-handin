#include <iostream>
#include "td3.hpp"
#include "support.hpp"
#include <stdlib.h>
#include <math.h>       // sin, cos
#include <assert.h>

using namespace std;

using namespace support;

double* extend_array(double* array, int length, int new_size) {
  // IMPLEMENT YOUR FUNCTION HERE
    double* new_arr = new double[new_size];
    for (int i = 0; i < length; i++) {
           new_arr[i] = array[i];
    }

    for (int i = length; i < new_size; i++) {
        new_arr[i] = 0;
    }
    delete [] array;
    return new_arr;
}

double* shrink_array(double* array, int length, int new_size) {
  // IMPLEMENT YOUR FUNCTION HERE

    double* n_arr = new double[new_size];

    for (int i = 0; i < new_size; i++) {
        n_arr[i] = array[i];
    }

    delete [] array;
    return n_arr;



}

double* append_to_array(double element,
                        double* array,
                        int &current_size,
                        int &max_size) {
  // IMPLEMENT YOUR FUNCTION HERE
    if (current_size+1 > max_size) {
        max_size += 5;
    }
    double* new_arr = new double[max_size];
    for (int i = 0; i < current_size; i++) {
        new_arr[i] = array[i];
    }
    new_arr[current_size] = element;
    current_size += 1;
    delete [] array;
    return new_arr;
}

double* remove_from_array(double* array,
                          int &current_size,
                          int &max_size) {
  // IMPLEMENT YOUR FUNCTION
    if (max_size - current_size + 1 >= 5) {
        max_size -= 5;
    }

    double* new_arr = new double[max_size];

    for (int i = 0; i < current_size; i++) {
        new_arr[i] = array[i];
    }
    current_size -= 1;
    delete [] array;
    return new_arr;

  return NULL; // YOU CAN CHANGE THIS
}

bool simulate_projectile(const double magnitude, const double angle,
                         const double simulation_interval,
                         double *targets, int &tot_targets,
                         int *obstacles, int tot_obstacles,
                         double* &telemetry,
                         int &telemetry_current_size,
                         int &telemetry_max_size) {
  // YOU CAN MODIFY THIS FUNCTION TO RECORD THE TELEMETRY

  bool hit_target, hit_obstacle;
  double v0_x, v0_y, x, y, t;
  double PI = 3.14159265;
  double g = 9.8;

  v0_x = magnitude * cos(angle * PI / 180);
  v0_y = magnitude * sin(angle * PI / 180);

  t = 0;
  x = 0;
  y = 0;

  hit_target = false;
  hit_obstacle = false;
  while (y >= 0 && (! hit_target) && (! hit_obstacle)) {
    double * target_coordinates = find_collision(x, y, targets, tot_targets);
    if (target_coordinates != NULL) {
      remove_target(targets, tot_targets, target_coordinates);
      hit_target = true;
    } else if (find_collision(x, y, obstacles, tot_obstacles) != NULL) {
      hit_obstacle = true;
    } else {
      t = t + simulation_interval;
      y = v0_y * t  - 0.5 * g * t * t;
      x = v0_x * t;


    }
    telemetry = append_to_array(t, telemetry, telemetry_current_size, telemetry_max_size);
    telemetry = append_to_array(x, telemetry, telemetry_current_size, telemetry_max_size);
    telemetry = append_to_array(y, telemetry, telemetry_current_size, telemetry_max_size);
  }

  return hit_target;
}

void merge(double* telemetry, int lower, int split, int upper) {
  double left_arr[(split - lower)*3];
  double right_arr[(upper - split)*3];

  for (int k = lower, i = 0; k < split; k++, i++) {
      int k_r = k*3;
      int i_r = i*3;
      left_arr[i_r] = telemetry[k_r];
      left_arr[i_r+1] = telemetry[k_r+1];
      left_arr[i_r+2] = telemetry[k_r+2];

  }

  for (int k = split, i = 0; k < upper; k++, i++) {
    int k_r = k*3;
    int i_r = i*3;
    right_arr[i_r] = telemetry[k_r];
    right_arr[i_r+1] = telemetry[k_r+1];
    right_arr[i_r+2] = telemetry[k_r+2];
  }


  int l = 0;
  int r = 0;
  for (int k = lower; k < upper; k++) {
    if (l < (split - lower) && r < (upper - split)) {
      if (left_arr[l*3] <= right_arr[r*3]) {
        int k_r = k*3;
        int l_r = l*3;
        telemetry[k_r] = left_arr[l_r];
        telemetry[k_r+1] = left_arr[l_r+1];
        telemetry[k_r+2] = left_arr[l_r+2];
        l++;
      }
      else {
        int k_r = k*3;
        int r_r = r*3;
        telemetry[k_r] = right_arr[r_r];
        telemetry[k_r+1] = right_arr[r_r+1];
        telemetry[k_r+2] = right_arr[r_r+2];
        r++;
      }
    } else if (l < (split - lower)) {
        int k_r = k*3;
        int l_r = l*3;
        telemetry[k_r] = left_arr[l_r];
        telemetry[k_r+1] = left_arr[l_r+1];
        telemetry[k_r+2] = left_arr[l_r+2];
        l++;
    } else if (r < (upper - split)) {\
        int k_r = k*3;
        int r_r = r*3;
        telemetry[k_r] = right_arr[r_r];
        telemetry[k_r+1] = right_arr[r_r+1];
        telemetry[k_r+2] = right_arr[r_r+2];
        r++;
    }
  }
}

void merge_sort(double* telemetry, int lower, int upper) {
  if (lower + 1 < upper) {
    int split = (lower + upper) / 2;

    merge_sort(telemetry, lower, split);
    merge_sort(telemetry, split, upper);

    merge(telemetry, lower, split, upper);
  }
}



void merge_telemetry(double **telemetries,
                     int tot_telemetries,
                     int *telemetries_sizes,
                     double* &global_telemetry,
                     int &global_telemetry_current_size,
                     int &global_telemetry_max_size) {
  // IMPLEMENT YOUR FUNCTION HERE


    global_telemetry_max_size = 0;
      for (int i = 0; i < tot_telemetries; ++i) {
        global_telemetry_max_size += telemetries_sizes[i];
      }
      global_telemetry = new double[global_telemetry_max_size];

      global_telemetry_current_size = 0;
      for (int i = 0; i < tot_telemetries; ++i) {
        for (int j = 0; j < telemetries_sizes[i]; ++j) {
          global_telemetry = append_to_array(telemetries[i][j],
                                      global_telemetry,
                                      global_telemetry_current_size,
                                      global_telemetry_max_size);
      merge_sort(global_telemetry, 0,  global_telemetry_current_size/3);
        }
      }
    }

