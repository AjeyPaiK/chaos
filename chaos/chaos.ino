/*
 * Chaos - Lorenz Attractor Watch Face for Watchy
 * 
 * Copyright (c) 2024 Ajey Pai Karkala
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * A mesmerizing watch face for the Watchy ESP32 smartwatch featuring a real-time
 * evolving Lorenz attractor pattern. Watch the beautiful chaos unfold on your wrist
 * with a continuously updating 3D trajectory that rotates and evolves every 3 seconds.
 */

#define ARDUINO_WATCHY_V20
#include <Watchy.h>
#include "settings.h"
#include "esp_sleep.h"

// Global RTC variables for persistence across deep sleep
RTC_DATA_ATTR float g_pos[3] = {1.0, 1.0, 1.0};
RTC_DATA_ATTR float g_trajectory[300][3];
RTC_DATA_ATTR int g_pointCount = 0;
RTC_DATA_ATTR int g_trajectoryIndex = 0;
RTC_DATA_ATTR float g_rotationAngle = 0.0;
RTC_DATA_ATTR bool g_initialized = false;

class WatchFace : public Watchy { //inherit and extend Watchy class
  using Watchy::Watchy;
  public:
    void drawWatchFace() { //override this method to customize how the watch face looks
      
      int16_t  x1, y1;
      uint16_t w, h;
      String textstring;
      bool light = true; // left this here if someone wanted to tweak for dark

      //resets step counter at midnight everyday
      if(currentTime.Hour == 00 && currentTime.Minute == 00) {
        sensor.resetStepCounter();
      }

      // draw background
      display.fillScreen(light ? GxEPD_WHITE : GxEPD_BLACK);
      display.setTextColor(light ? GxEPD_BLACK : GxEPD_WHITE);
      display.setTextWrap(false);

      // Draw Lorenz attractor
      drawLorenzAttractor();
      
      // Draw time
      display.setFont(&FreeMonoBold9pt7b);
      
      // Format with leading zeros for 24-hour format
      if (currentTime.Hour < 10) {
        textstring = "0";
      } else {
        textstring = "";
      }
      textstring += currentTime.Hour;
      textstring += ":";
      if (currentTime.Minute < 10) {
        textstring += "0";
      }
      textstring += currentTime.Minute;
      
      display.setCursor(5, 19);
      display.print(textstring);
      //END time

      // Draw Date
      display.setFont(&FreeMonoBold9pt7b);
      textstring = dayShortStr(currentTime.Wday);
      textstring += " ";
      textstring += currentTime.Day;
      textstring += "/";
      textstring += currentTime.Month;
      
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(200-w-5, 19);
      display.print(textstring);
      //END date

      //  Draw Battery Icon
      float VBAT = getBatteryVoltage();
      bool isBatteryFull = (VBAT >= 4.2);
      
      // Battery icon dimensions
      int batteryX = 200 - 25; // Right side with margin
      int batteryY = 200 - 15; // Bottom with margin
      int batteryWidth = 20;
      int batteryHeight = 10;
      
      // Draw battery outline
      display.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, GxEPD_BLACK);
      
      // Draw battery terminal (right side)
      display.drawRect(batteryX + batteryWidth, batteryY + 2, 2, 6, GxEPD_BLACK);
      
      // Fill battery if full
      if (isBatteryFull) {
        display.fillRect(batteryX + 1, batteryY + 1, batteryWidth - 2, batteryHeight - 2, GxEPD_BLACK);
      }
      //  END OF Draw Battery

      // Draw Steps
      textstring = String(sensor.getCounter());
      display.getTextBounds(textstring, 0, 0, &x1, &y1, &w, &h);
      display.setCursor(5, 200-15 + 5); // Align with battery icon center
      display.print(textstring);
      
      //END steps

    }

  private:
    // Lorenz attractor parameters
    static constexpr float LORENZ_SIGMA = 10.0;
    static constexpr float LORENZ_RHO = 28.0;
    static constexpr float LORENZ_BETA = 8.0/3.0;
    static constexpr float DT = 0.05;
    static constexpr int MAX_POINTS = LORENZ_MAX_POINTS;
    static constexpr float SCALE_FACTOR = 12.0;
    
    // State variables
    float pos[3] = {1.0, 1.0, 1.0};
    float trajectory[MAX_POINTS][3];
    int pointCount = 0;
    int trajectoryIndex = 0;
    float rotationAngle = 0.0;
    bool initialized = false;

    void drawLorenzAttractor() {
      // Simple, guaranteed-to-work approach
      // Always add points and evolve the system
      for (int i = 0; i < 50; i++) {
        updateLorenz();
        addTrajectoryPoint();
      }
      
      // Rotate view
      g_rotationAngle += 0.2;
      if (g_rotationAngle > 2 * PI) {
        g_rotationAngle -= 2 * PI;
      }
      
      // Draw trajectory
      drawTrajectory();
    }
    
    
    void drawSimplePattern() {
      // Draw a simple spiral pattern that's definitely visible
      for (int i = 0; i < 100; i++) {
        float angle = i * 0.1 + g_rotationAngle;
        int x = 100 + (int)(30 * cos(angle));
        int y = 100 + (int)(30 * sin(angle));
        
        // Clamp to screen bounds
        if (x >= 20 && x <= 180 && y >= 20 && y <= 180) {
          display.drawPixel(x, y, GxEPD_BLACK);
          display.drawPixel(x+1, y, GxEPD_BLACK);
          display.drawPixel(x, y+1, GxEPD_BLACK);
          display.drawPixel(x+1, y+1, GxEPD_BLACK);
        }
      }
    }

    void updateLorenz() {
      // Runge-Kutta 4th order integration
      float k1[3], k2[3], k3[3], k4[3];
      float temp[3];
      
      // k1 = f(t, y)
      lorenzDerivative(g_pos, k1);
      
      // k2 = f(t + dt/2, y + k1*dt/2)
      for (int i = 0; i < 3; i++) {
        temp[i] = g_pos[i] + k1[i] * DT / 2;
      }
      lorenzDerivative(temp, k2);
      
      // k3 = f(t + dt/2, y + k2*dt/2)
      for (int i = 0; i < 3; i++) {
        temp[i] = g_pos[i] + k2[i] * DT / 2;
      }
      lorenzDerivative(temp, k3);
      
      // k4 = f(t + dt, y + k3*dt)
      for (int i = 0; i < 3; i++) {
        temp[i] = g_pos[i] + k3[i] * DT;
      }
      lorenzDerivative(temp, k4);
      
      // Update position
      for (int i = 0; i < 3; i++) {
        g_pos[i] += (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * DT / 6;
      }
    }

    void lorenzDerivative(float p[3], float result[3]) {
      result[0] = LORENZ_SIGMA * (p[1] - p[0]);
      result[1] = p[0] * (LORENZ_RHO - p[2]) - p[1];
      result[2] = p[0] * p[1] - LORENZ_BETA * p[2];
    }

    void addTrajectoryPoint() {
      if (g_pointCount < MAX_POINTS) {
        // Still growing - just increment index and count
        g_trajectory[g_trajectoryIndex][0] = g_pos[0];
        g_trajectory[g_trajectoryIndex][1] = g_pos[1];
        g_trajectory[g_trajectoryIndex][2] = g_pos[2];
        g_trajectoryIndex++;
        g_pointCount++;
      } else {
        // Buffer is full - use circular buffer
        g_trajectory[g_trajectoryIndex][0] = g_pos[0];
        g_trajectory[g_trajectoryIndex][1] = g_pos[1];
        g_trajectory[g_trajectoryIndex][2] = g_pos[2];
        g_trajectoryIndex = (g_trajectoryIndex + 1) % MAX_POINTS;
        // Keep pointCount at MAX_POINTS to show we're using circular buffer
        g_pointCount = MAX_POINTS;
      }
    }

    void project3DTo2D(float point3D[3], float& x2D, float& y2D) {
      // Rotate around Y axis
      float cos_rot = cos(g_rotationAngle);
      float sin_rot = sin(g_rotationAngle);
      
      // Apply rotation
      float x_rot = point3D[0] * cos_rot - point3D[2] * sin_rot;
      float y_rot = point3D[1];
      float z_rot = point3D[0] * sin_rot + point3D[2] * cos_rot;
      
      // Project to 2D (ignore z for now)
      x2D = x_rot;
      y2D = y_rot;
    }

    void drawTrajectory() {
      if (g_pointCount == 0) return;
      
      // Get all valid points
      int numPoints = (g_pointCount < MAX_POINTS) ? g_pointCount : MAX_POINTS;
      
      // Project to 2D and find bounds
      float screenX[300];
      float screenY[300];
      float minX = 999, maxX = -999, minY = 999, maxY = -999;
      
      for (int i = 0; i < numPoints; i++) {
        int idx = (g_pointCount < MAX_POINTS) ? i : (g_trajectoryIndex + i) % MAX_POINTS;
        project3DTo2D(g_trajectory[idx], screenX[i], screenY[i]);
        
        if (screenX[i] < minX) minX = screenX[i];
        if (screenX[i] > maxX) maxX = screenX[i];
        if (screenY[i] < minY) minY = screenY[i];
        if (screenY[i] > maxY) maxY = screenY[i];
      }
      
      // Scale to fit screen with larger, more visible scaling
      float rangeX = maxX - minX;
      float rangeY = maxY - minY;
      
      // Ensure minimum range to avoid division by zero
      if (rangeX < 0.1) rangeX = 0.1;
      if (rangeY < 0.1) rangeY = 0.1;
      
      // Use larger scale factor for better visibility
      float scaleX = 80.0 / rangeX;
      float scaleY = 80.0 / rangeY;
      float scale = (scaleX < scaleY) ? scaleX : scaleY;
      
      // Center and scale
      float centerX = (minX + maxX) / 2;
      float centerY = (minY + maxY) / 2;
      
      for (int i = 0; i < numPoints; i++) {
        screenX[i] = (screenX[i] - centerX) * scale + 100; // Center at 100,100
        screenY[i] = (screenY[i] - centerY) * scale + 100;
        
        // Clamp to screen bounds - slightly larger window
        if (screenX[i] < 5) screenX[i] = 5;
        if (screenX[i] > 195) screenX[i] = 195;
        if (screenY[i] < 5) screenY[i] = 5;
        if (screenY[i] > 195) screenY[i] = 195;
      }
      
      // Draw trajectory points and lines - LARGER POINTS
      for (int i = 0; i < numPoints; i++) {
        int x = (int)screenX[i];
        int y = (int)screenY[i];
        
        // Draw larger 3x3 squares for better visibility
        if (x >= 0 && x < 200 && y >= 0 && y < 200) {
          // Draw 3x3 square for each point
          for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
              if (x + dx >= 0 && x + dx < 200 && y + dy >= 0 && y + dy < 200) {
                display.drawPixel(x + dx, y + dy, GxEPD_BLACK);
              }
            }
          }
        }
        
        // Draw line to next point
        if (i < numPoints - 1) {
          int nextI = i + 1;
          drawLine((int)screenX[i], (int)screenY[i], (int)screenX[nextI], (int)screenY[nextI]);
        }
      }
      
      // Draw current position (MUCH LARGER dot - 5x5)
      if (numPoints > 0) {
        int lastIdx = (g_pointCount < MAX_POINTS) ? numPoints - 1 : (g_trajectoryIndex - 1 + MAX_POINTS) % MAX_POINTS;
        int currentX = (int)screenX[lastIdx];
        int currentY = (int)screenY[lastIdx];
        
        // Draw large 5x5 square for current position
        if (currentX >= 0 && currentX < 200 && currentY >= 0 && currentY < 200) {
          for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
              if (currentX + dx >= 0 && currentX + dx < 200 && currentY + dy >= 0 && currentY + dy < 200) {
                display.drawPixel(currentX + dx, currentY + dy, GxEPD_BLACK);
              }
            }
          }
        }
      }
      
    }

    void drawLine(int x0, int y0, int x1, int y1) {
      // Bresenham's line algorithm
      int dx = abs(x1 - x0);
      int dy = abs(y1 - y0);
      int sx = (x0 < x1) ? 1 : -1;
      int sy = (y0 < y1) ? 1 : -1;
      int err = dx - dy;
      
      while (true) {
        display.drawPixel(x0, y0, GxEPD_BLACK);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
          err -= dy;
          x0 += sx;
        }
        if (e2 < dx) {
          err += dx;
          y0 += sy;
        }
      }
    }

    void drawCircle(int x0, int y0, int radius) {
      // Simple circle drawing
      for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
          if (x*x + y*y <= radius*radius) {
            display.drawPixel(x0 + x, y0 + y, GxEPD_BLACK);
          }
        }
      }
    }
};

// Fast wake-up configuration
const uint64_t WAKE_US = 3ULL * 1000000ULL;  // wake every 3 seconds

// Create settings object
watchySettings settings;
WatchFace m(settings); //instantiate your watchface

void setup() {
  m.init("");                   // empty string: don't update NTP/weather every wake
  m.showWatchFace(false);       // draw once (no partial update)
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_sleep_enable_timer_wakeup(WAKE_US);
  esp_deep_sleep_start();
}

void loop() {
  // this should never run, Watchy deep sleeps after init();
}
