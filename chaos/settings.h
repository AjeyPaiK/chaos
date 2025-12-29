/*
 * settings.h
 * Configuration file for Lorenz Attractor Watch Face
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
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#define WAKE_UP_INTERVAL_MINUTES (0.5/60.0)

#define LORENZ_POINTS_PER_UPDATE 5
#define LORENZ_MAX_POINTS 500
#define LORENZ_ROTATION_SPEED 0.0523598776

// Timezone offset for moon phase calculation (in hours from UTC)
// Examples:
//   Amsterdam (CET): 1  (winter) or 2 (summer/CEST)
//   New York (EST): -5  (winter) or -4 (summer/EDT)
//   Tokyo (JST): 9
//   London (GMT): 0    (winter) or 1 (summer/BST)
//   Los Angeles (PST): -8 (winter) or -7 (summer/PDT)
#define TIMEZONE_OFFSET_HOURS 1  // Amsterdam (CET) - change to 2 for CEST (summer time)

// Location coordinates for sunrise/sunset calculation
// Amsterdam coordinates: 52.3676° N, 4.9041° E
#define LATITUDE 52.3676   // Latitude in degrees (positive = North, negative = South)
#define LONGITUDE 4.9041   // Longitude in degrees (positive = East, negative = West)

#endif
