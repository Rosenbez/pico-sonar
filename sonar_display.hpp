#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"

class Point {
public:
    Point(int x, int y) : _x(x), _y(y) {}
    Point() : _x(0), _y(0) {}
    
    int getx() {return _x;}
    int gety() {return _y;}
    void print() {
        printf("Point: %d, %d\n", _x, _y);
    }
    
    int _x, _y;
};


// A circular type buffer to record readings on the screen.
// Holds the reading Point and the reading angle.
// Used to look up the last readings close to an angle and clear them.
class ReadingBuffer {
public:

    ReadingBuffer() {
        // TODO: add directionality and adjustable angle within

        // fill default reading angles with -1 so we don't get any
        // points by default
        clear_log();
    }

    // Clear log by filling the angle reading buffer with -1
    void clear_log() {

        for (int i = 0; i < buff_size; i++) {
            reading_angles[i] = -1;
        }
    }


    // Add a reading to the buffer
    void add_reading(Point point, float angle) {
        
        reading_points[buff_ptr] = point;
        reading_angles[buff_ptr] = angle;

        buff_ptr++;
        if (buff_ptr >= buff_size) buff_ptr = 0;

    }

    // Return the first point found within 5 deg of the given angle
    // used to get a point to clear the screen. 
    // returns a Point(999,999) if no points to clear are found.
    Point get_one_within(float angle) {
        float forward_within_degrees = 5;

        for (int i = 0; i < buff_size; i++) {
            float pt_angle = reading_angles[i];
            if ((pt_angle >= angle) && (pt_angle <= angle + forward_within_degrees)) {
                return reading_points[i];
            }
        }

        // if none found, return a Point(999,999)
        return Point(999, 999);
    }

    // Get the first 3 points found within the angle and put them in a buffer.
    // If less than 3 are found, fill remaining with Point(999,999)
    void get_x_within(float angle, Point *point_buff, int num_pts) {

        uint8_t points_found = 0;

        for (int i = 0; i < buff_size; i++) {
            float pt_angle = reading_angles[i];
            if (_should_erase(pt_angle, angle)) {
                point_buff[points_found] = reading_points[i];
                points_found++;
                if (points_found == num_pts) return;
            }
        }

        for (int i = points_found; i < num_pts; i++) {
            point_buff[i] = Point(999,999);
        }
    }

    // Determine if the stored point should be erased at the given sensor position.
    // Handle rollover over 360 deg
    bool _should_erase(float stored_point_angle, float sensor_position) {
        float max_erase_angle = sensor_position + _erase_within_deg;

        if ((max_erase_angle) < 360) {
            return ((stored_point_angle >= sensor_position) && (stored_point_angle <= max_erase_angle));
        }

        if (stored_point_angle <= 300) {
            float stored_plus_360 = stored_point_angle + 360;
            return ((stored_plus_360 >= sensor_position) && (stored_plus_360 <= max_erase_angle));
        }

        return ((stored_point_angle >= sensor_position) && (stored_point_angle <= max_erase_angle));
    }
    

    int buff_size = 300;
    int buff_ptr = 0;
    Point reading_points[300];
    int reading_angles[300];

    // Points within this number of degrees are candidates for being erased.
    int _erase_within_deg = 5;




};


class SonarDisplay {
public:
    SonarDisplay(TFTDriver &tft, int width, int height) {
        _tft = tft;
        _width = width;
        _height = height;

        center_x = (_width/2) - 1;
        center_y = (_height/2) - 1;
    }

    long map(long x, long in_min, long in_max, long out_min, long out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    long map_mm_distance_to_px_distance(int dist) {
        return map(dist, 30, _max_distance, 10, 120);
    }

    // Change the maximum distance and scaling of the display.
    void set_max_distance(int max_distance_mm) {
        _max_distance = max_distance_mm;
    }

    float radian(float degrees) {
        return (3.14159/180) * degrees;
    }

    // Create a point in the global coords from a distance and angle
    // reading. Distance should be in pixels, not mm.
    Point reading_to_point(int screen_distance, float angle) {

        float radians = radian(angle);
        int sin_d = sin(radians) * screen_distance;
        int cos_d = cos(radians) * screen_distance;
        int x, y;
        
        if (angle < 45) {
            x = sin_d;
            y = cos_d;
        } else if (angle < 90) {
            radians = radian(90 - angle);
            sin_d = sin(radians) * screen_distance;
            cos_d = cos(radians) * screen_distance;
            x = cos_d;
            y = sin_d;
        } else if (angle < 135) {
            radians = radian(angle - 90);
            sin_d = sin(radians) * screen_distance;
            cos_d = cos(radians) * screen_distance;
            x = cos_d;
            y = - sin_d;
        } else if (angle < 180) {
            radians = radian(180 - angle);
            sin_d = sin(radians) * screen_distance;
            cos_d = cos(radians) * screen_distance;
            x = sin_d;
            y = -cos_d;
        } else if (angle < 225) {
            radians = radian(angle - 180);
            sin_d = sin(radians) * screen_distance;
            cos_d = cos(radians) * screen_distance;
            x = -sin_d;
            y = - cos_d;
        } else if (angle < 270) {
            radians = radian(270 - angle);
            sin_d = sin(radians) * screen_distance;
            cos_d = cos(radians) * screen_distance;
            x = -cos_d;
            y = -sin_d;
        } else if (angle < 295) {
            radians = radian(angle - 270);
            sin_d = sin(radians) * screen_distance;
            cos_d = cos(radians) * screen_distance;
            x = -cos_d;
            y = sin_d;
        } else if (angle < 360) {
            radians = radian(360 - angle);
            sin_d = sin(radians) * screen_distance;
            cos_d = cos(radians) * screen_distance;
            x = -sin_d;
            y = cos_d;
        }
        
        return Point(center_x + x, center_y - y);
    
    }

    void plot_reading(int distance, float angle ) {
        int px_dist = map_mm_distance_to_px_distance(distance);

        Point p = reading_to_point(px_dist, angle);

        p.print();
        uint8_t px_green[3] = {0, 63 << 2, 0};

        _write_point(px_green, p.getx(), p.gety());
        point_log.add_reading(p, angle);
    }

    // Plot a black circle at the given distance in mm. Useful for showing screen scale.
    // Will not be erased during operation, only if the screen is cleared.
    void plot_circle_at(int distance_mm) {
        int circle_px_dist = map_mm_distance_to_px_distance(distance_mm);
        int circle_deg = 2;
        uint8_t col[] = {0, 0, 0};
        Point p;

        for (int i = 0; i < 360; i += circle_deg) {
            p = reading_to_point(circle_px_dist, i);
            _tft.write_pixel(col, p.getx(), p.gety(), 1);
        }

    }

    // write a small multi-pixel point centered on x/y
    void _write_point(uint8_t *color, int x, int y) {

        _tft.write_pixel(color, x - 1, y - 1, 3);
    }

    // Erase a standard sized point by writing the background color to it's location
    void _erase_point(int x, int y) {
        _write_point(_bg_color, x, y);
    }

    // clear a point on the screen to make way for the next reading
    void clear_point_within(float angle) {
        Point point_to_erase = point_log.get_one_within(angle);

        // If we get a no-point result
        if (point_to_erase.getx() == 999) {
            printf("no point found to erase within %f deg\n", angle);
            return;
        }

        puts("erasing point");
        point_to_erase.print();

        _erase_point(point_to_erase.getx(), point_to_erase.gety());

    }


    void clear_3_within(float angle) {
        Point pts_to_erase[5];
        uint8_t erased = 0;

        point_log.get_x_within(angle, pts_to_erase, 5);

        for (int i = 0; i < 3; i++) {
            if (pts_to_erase[i].getx() != 999) {
                _erase_point(pts_to_erase[i].getx(), pts_to_erase[i].gety());
                erased++;
            }
        }
        printf("erased %d points within %f degrees! \n", erased, angle);
    }

    


    int _width, _height, center_x, center_y;

    // The maximum distance the display will show in mm. Used for scaling the display readings.
    int _max_distance = 3000;

    TFTDriver _tft;
    uint8_t _bg_color[3] = {60 << 2, 60 << 2, 60 << 2};
    ReadingBuffer point_log = ReadingBuffer();
};