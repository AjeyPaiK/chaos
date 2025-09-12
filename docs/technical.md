# Technical Documentation

**Copyright (c) 2024 Ajey Pai Karkala**

This document provides detailed technical information about the Lorenz Attractor Watch Face implementation.

## Architecture Overview

The watch face is built on the Watchy ESP32 platform and implements a real-time Lorenz attractor simulation with persistent state across deep sleep cycles.

### Core Components

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Watchy Core   │    │  Lorenz System   │    │  Display Layer  │
│                 │    │                  │    │                 │
│ • RTC Management│◄──►│ • Runge-Kutta    │◄──►│ • E-ink Display │
│ • Deep Sleep    │    │ • 3D Projection  │    │ • Watch Elements│
│ • Wake-up Timer │    │ • State Storage  │    │ • Pattern Render│
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

## Lorenz System Implementation

### Mathematical Model

The Lorenz system is defined by three coupled differential equations:

```
dx/dt = σ(y - x)
dy/dt = x(ρ - z) - y  
dz/dt = xy - βz
```

Where:
- **σ (Sigma)**: 10.0 - Prandtl number
- **ρ (Rho)**: 28.0 - Rayleigh number
- **β (Beta)**: 8/3 - Geometric factor

### Numerical Integration

**Runge-Kutta 4th Order Method**:

```cpp
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

// Update position: y = y + (k1 + 2*k2 + 2*k3 + k4) * dt/6
for (int i = 0; i < 3; i++) {
  g_pos[i] += (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * DT / 6;
}
```

**Integration Parameters**:
- **Time Step (DT)**: 0.05
- **Steps per Update**: 50
- **Total Time per Update**: 2.5 time units

## State Persistence

### RTC Memory Storage

Critical state variables are stored in RTC memory to survive deep sleep:

```cpp
RTC_DATA_ATTR float g_pos[3] = {1.0, 1.0, 1.0};           // Current position
RTC_DATA_ATTR float g_trajectory[300][3];                  // Trajectory points
RTC_DATA_ATTR int g_pointCount = 0;                        // Point counter
RTC_DATA_ATTR int g_trajectoryIndex = 0;                   // Circular buffer index
RTC_DATA_ATTR float g_rotationAngle = 0.0;                 // View rotation
RTC_DATA_ATTR bool g_initialized = false;                  // Initialization flag
```

### Memory Layout

| Variable | Type | Size | Purpose |
|----------|------|------|---------|
| `g_pos` | float[3] | 12 bytes | Current Lorenz position |
| `g_trajectory` | float[300][3] | 3,600 bytes | Trajectory history |
| `g_pointCount` | int | 4 bytes | Number of stored points |
| `g_trajectoryIndex` | int | 4 bytes | Circular buffer position |
| `g_rotationAngle` | float | 4 bytes | 3D view rotation |
| `g_initialized` | bool | 1 byte | Initialization state |

**Total RTC Memory Usage**: ~3,625 bytes

## 3D to 2D Projection

### Rotation Matrix

The 3D Lorenz points are rotated around the Y-axis for dynamic viewing:

```cpp
void project3DTo2D(float point3D[3], float& x2D, float& y2D) {
  float cos_rot = cos(g_rotationAngle);
  float sin_rot = sin(g_rotationAngle);
  
  // Apply Y-axis rotation
  float x_rot = point3D[0] * cos_rot - point3D[2] * sin_rot;
  float y_rot = point3D[1];
  float z_rot = point3D[0] * sin_rot + point3D[2] * cos_rot;
  
  // Project to 2D (orthographic)
  x2D = x_rot;
  y2D = y_rot;
}
```

### Scaling and Centering

Dynamic scaling ensures the pattern fits the display:

```cpp
// Find bounds
float minX = 999, maxX = -999, minY = 999, maxY = -999;
for (int i = 0; i < numPoints; i++) {
  if (screenX[i] < minX) minX = screenX[i];
  if (screenX[i] > maxX) maxX = screenX[i];
  // ... similar for Y
}

// Calculate scale factor
float rangeX = maxX - minX;
float rangeY = maxY - minY;
float scaleX = 80.0 / rangeX;
float scaleY = 80.0 / rangeY;
float scale = (scaleX < scaleY) ? scaleX : scaleY;

// Center and scale
float centerX = (minX + maxX) / 2;
float centerY = (minY + maxY) / 2;
screenX[i] = (screenX[i] - centerX) * scale + 100;
screenY[i] = (screenY[i] - centerY) * scale + 100;
```

## Wake-up and Sleep Management

### ESP-IDF Sleep API

The watch uses ESP-IDF sleep APIs for precise wake-up control:

```cpp
const uint64_t WAKE_US = 3ULL * 1000000ULL;  // 3 seconds

void setup() {
  m.init("");
  m.showWatchFace(false);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  esp_sleep_enable_timer_wakeup(WAKE_US);
  esp_deep_sleep_start();
}
```

### Sleep Cycle

1. **Wake-up**: ESP32 exits deep sleep
2. **Initialize**: RTC memory variables restored
3. **Update Lorenz**: 50 integration steps
4. **Add Points**: Store new trajectory points
5. **Rotate View**: Update 3D rotation angle
6. **Render**: Draw pattern and watch elements
7. **Sleep**: Enter deep sleep for 3 seconds

## Display Management

### E-ink Display

The Watchy uses a 200x200 pixel e-ink display:

- **Resolution**: 200x200 pixels
- **Drawing Area**: 190x190 pixels (5px margins)
- **Color Depth**: 1-bit (black/white)
- **Refresh Type**: Full refresh every update

### Drawing Pipeline

1. **Clear Display**: `display.fillScreen(GxEPD_WHITE)`
2. **Draw Watch Elements**: Time, date, battery, steps
3. **Calculate Trajectory**: Project 3D points to 2D
4. **Scale and Center**: Fit pattern to display
5. **Draw Points**: Render trajectory as 2x2 squares
6. **Draw Lines**: Connect consecutive points
7. **Update Display**: `display.display()`

## Performance Analysis

### Computational Complexity

**Per Update Cycle**:
- **Integration Steps**: 50 × 4 = 200 derivative calculations
- **Point Storage**: 50 array operations
- **3D Projection**: 300 × 3 = 900 trigonometric operations
- **Scaling**: 300 × 4 = 1,200 arithmetic operations
- **Drawing**: 300 × 4 = 1,200 pixel operations

**Total Operations per Update**: ~3,500 operations

### Memory Usage

| Component | Size | Type | Purpose |
|-----------|------|------|---------|
| Program Code | ~15KB | Flash | Main program |
| RTC Variables | ~3.6KB | RTC RAM | State persistence |
| Local Variables | ~2KB | RAM | Runtime calculations |
| Display Buffer | ~5KB | RAM | E-ink display |

### Power Consumption

**Active Mode** (during update):
- **CPU**: 240MHz
- **Current**: ~80mA
- **Duration**: ~200ms
- **Energy**: ~16mJ per update

**Sleep Mode**:
- **Current**: ~10μA
- **Duration**: 2.8s
- **Energy**: ~28μJ per cycle

**Total Energy per Update**: ~16mJ (99.8% in sleep)

## Error Handling

### Numerical Stability

**Overflow Protection**:
```cpp
// Clamp to screen bounds
if (screenX[i] < 5) screenX[i] = 5;
if (screenX[i] > 195) screenX[i] = 195;
if (screenY[i] < 5) screenY[i] = 5;
if (screenY[i] > 195) screenY[i] = 195;
```

**Division by Zero**:
```cpp
// Ensure minimum range
if (rangeX < 0.1) rangeX = 0.1;
if (rangeY < 0.1) rangeY = 0.1;
```

### State Recovery

**Initialization Check**:
```cpp
if (!g_initialized) {
  // Reset to known good state
  g_pos[0] = 1.0; g_pos[1] = 1.0; g_pos[2] = 1.0;
  g_pointCount = 0;
  g_trajectoryIndex = 0;
  g_rotationAngle = 0.0;
  g_initialized = true;
}
```

## Optimization Techniques

### Circular Buffer

Efficient trajectory storage using modulo arithmetic:

```cpp
if (g_pointCount < MAX_POINTS) {
  // Linear growth phase
  g_trajectory[g_trajectoryIndex][0] = g_pos[0];
  g_trajectoryIndex++;
  g_pointCount++;
} else {
  // Circular buffer phase
  g_trajectory[g_trajectoryIndex][0] = g_pos[0];
  g_trajectoryIndex = (g_trajectoryIndex + 1) % MAX_POINTS;
  g_pointCount = MAX_POINTS;
}
```

### Trigonometric Optimization

Pre-calculate rotation values:

```cpp
float cos_rot = cos(g_rotationAngle);
float sin_rot = sin(g_rotationAngle);
// Reuse for all points in current frame
```

### Memory Alignment

RTC variables aligned for optimal access:

```cpp
RTC_DATA_ATTR float g_pos[3] __attribute__((aligned(4)));
```

## Debugging and Diagnostics

### Debug Output

Enable debug information by uncommenting:

```cpp
// Point count display
display.print("P:"); display.print(g_pointCount);

// Position display  
display.print("X:"); display.print(g_pos[0]);
display.print("Y:"); display.print(g_pos[1]);
display.print("Z:"); display.print(g_pos[2]);
```

### Performance Monitoring

Track update timing:

```cpp
unsigned long startTime = millis();
// ... update code ...
unsigned long updateTime = millis() - startTime;
display.print("T:"); display.print(updateTime);
```

## Future Enhancements

### Potential Improvements

1. **Adaptive Scaling**: Dynamic scale factor based on pattern size
2. **Color Depth**: Multi-level grayscale display
3. **Pattern Modes**: Different attractor systems
4. **User Interaction**: Button controls for settings
5. **Data Logging**: Save pattern evolution to flash

### Hardware Considerations

- **Memory**: Larger trajectory buffers
- **Processing**: Faster wake-up times
- **Display**: Higher resolution e-ink
- **Sensors**: Accelerometer for rotation control

---

**Technical excellence through mathematical precision** 🔬⚡
