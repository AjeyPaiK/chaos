# Configuration Guide

**Copyright (c) 2024 Ajey Pai Karkala**

This guide explains how to customize the Lorenz Attractor Watch Face to suit your preferences.

## Quick Configuration

All settings are located in `chaos/settings.h`. Edit this file to customize behavior:

```cpp
// Wake up settings
#define WAKE_UP_INTERVAL_MINUTES 0.05  // 3 seconds

// Lorenz attractor settings
#define LORENZ_POINTS_PER_UPDATE 50    // Points per refresh
#define LORENZ_MAX_POINTS 300          // Trajectory buffer size
#define LORENZ_ROTATION_SPEED 0.5      // Rotation speed
```

## Wake-up Settings

### Wake-up Frequency

Control how often the watch updates:

```cpp
#define WAKE_UP_INTERVAL_MINUTES 0.05  // 3 seconds (fast)
#define WAKE_UP_INTERVAL_MINUTES 0.1   // 6 seconds (medium)
#define WAKE_UP_INTERVAL_MINUTES 0.5   // 30 seconds (slow)
#define WAKE_UP_INTERVAL_MINUTES 1.0   // 1 minute (battery saving)
```

**Recommendations**:
- **3 seconds**: Best for watching evolution, higher battery usage
- **6 seconds**: Good balance of evolution and battery life
- **30 seconds**: Battery saving mode, slower evolution
- **1 minute**: Maximum battery life, minimal evolution

## Lorenz Attractor Settings

### Points Per Update

Control how many new points are added each refresh:

```cpp
#define LORENZ_POINTS_PER_UPDATE 20    // Slow evolution
#define LORENZ_POINTS_PER_UPDATE 50    // Default (balanced)
#define LORENZ_POINTS_PER_UPDATE 100   // Fast evolution
#define LORENZ_POINTS_PER_UPDATE 200   // Very fast evolution
```

**Effects**:
- **Higher values**: Faster pattern evolution, more battery usage
- **Lower values**: Slower evolution, better battery life

### Maximum Points

Control trajectory buffer size:

```cpp
#define LORENZ_MAX_POINTS 200   // Shorter trails
#define LORENZ_MAX_POINTS 300   // Default (balanced)
#define LORENZ_MAX_POINTS 500   // Longer trails
#define LORENZ_MAX_POINTS 1000  // Very long trails
```

**Effects**:
- **Higher values**: Longer, more detailed trails, more memory usage
- **Lower values**: Shorter trails, less memory usage

### Rotation Speed

Control how fast the 3D view rotates:

```cpp
#define LORENZ_ROTATION_SPEED 0.1   // Slow rotation
#define LORENZ_ROTATION_SPEED 0.5   // Default (balanced)
#define LORENZ_ROTATION_SPEED 1.0   // Fast rotation
#define LORENZ_ROTATION_SPEED 2.0   // Very fast rotation
```

**Effects**:
- **Higher values**: Faster rotation, more dynamic view
- **Lower values**: Slower rotation, more stable view

## Advanced Configuration

### Lorenz System Parameters

For advanced users, you can modify the Lorenz system parameters in `chaos/chaos.ino`:

```cpp
static constexpr float LORENZ_SIGMA = 10.0;    // Prandtl number
static constexpr float LORENZ_RHO = 28.0;      // Rayleigh number
static constexpr float LORENZ_BETA = 8.0/3.0;  // Geometric factor
static constexpr float DT = 0.05;              // Time step
```

**Parameter Effects**:
- **Sigma (σ)**: Controls the rate of change in x direction
- **Rho (ρ)**: Controls the transition to chaos (critical value ~24.74)
- **Beta (β)**: Controls the rate of change in z direction
- **DT**: Integration time step (smaller = more accurate, slower)

### Display Settings

Modify the display window in `drawTrajectory()`:

```cpp
// Current bounds (190x190 pixels)
if (screenX[i] < 5) screenX[i] = 5;
if (screenX[i] > 195) screenX[i] = 195;
if (screenY[i] < 5) screenY[i] = 5;
if (screenY[i] > 195) screenY[i] = 195;

// Larger window (195x195 pixels)
if (screenX[i] < 2) screenX[i] = 2;
if (screenX[i] > 198) screenX[i] = 198;
if (screenY[i] < 2) screenY[i] = 2;
if (screenY[i] > 198) screenY[i] = 198;
```

## Example Configurations

### Battery Saver Mode

For maximum battery life:

```cpp
#define WAKE_UP_INTERVAL_MINUTES 1.0   // 1 minute updates
#define LORENZ_POINTS_PER_UPDATE 20    // Fewer points
#define LORENZ_MAX_POINTS 200          // Shorter trails
#define LORENZ_ROTATION_SPEED 0.2      // Slow rotation
```

**Expected battery life**: 10-14 days

### Performance Mode

For maximum visual impact:

```cpp
#define WAKE_UP_INTERVAL_MINUTES 0.05  // 3 second updates
#define LORENZ_POINTS_PER_UPDATE 100   // Many points
#define LORENZ_MAX_POINTS 500          // Long trails
#define LORENZ_ROTATION_SPEED 1.0      // Fast rotation
```

**Expected battery life**: 3-5 days

### Balanced Mode (Default)

Good balance of performance and battery:

```cpp
#define WAKE_UP_INTERVAL_MINUTES 0.05  // 3 second updates
#define LORENZ_POINTS_PER_UPDATE 50    // Moderate points
#define LORENZ_MAX_POINTS 300          // Medium trails
#define LORENZ_ROTATION_SPEED 0.5      // Moderate rotation
```

**Expected battery life**: 7-10 days

## Custom Configurations

### Creating Your Own Config

1. Copy `chaos/settings.h` to `examples/custom_configs/my_config.h`
2. Modify the values as desired
3. Replace the include in `chaos/chaos.ino`:
   ```cpp
   #include "examples/custom_configs/my_config.h"
   ```
4. Upload the modified code

### Sharing Configurations

To share your configuration:

1. Create a new file in `examples/custom_configs/`
2. Name it descriptively (e.g., `battery_saver.h`)
3. Add a comment explaining the purpose
4. Submit a pull request

## Performance Impact

### Battery Life Estimates

| Update Interval | Points/Update | Max Points | Estimated Battery Life |
|----------------|---------------|------------|----------------------|
| 3 seconds      | 50           | 300        | 7-10 days            |
| 6 seconds      | 50           | 300        | 10-14 days           |
| 30 seconds     | 50           | 300        | 14-20 days           |
| 1 minute       | 50           | 300        | 20-30 days           |

### Memory Usage

- **RTC Memory**: ~2KB for state variables
- **Flash Memory**: ~15KB for program code
- **RAM**: Minimal during deep sleep

## Troubleshooting Configuration

### Common Issues

**Pattern too fast/slow**:
- Adjust `LORENZ_POINTS_PER_UPDATE`
- Modify `WAKE_UP_INTERVAL_MINUTES`

**Battery drains quickly**:
- Increase `WAKE_UP_INTERVAL_MINUTES`
- Decrease `LORENZ_POINTS_PER_UPDATE`

**Pattern not visible**:
- Check display bounds in `drawTrajectory()`
- Verify scaling parameters

**Compilation errors**:
- Check syntax in `settings.h`
- Ensure all values are valid numbers

## Best Practices

1. **Start with defaults**: Use the provided balanced configuration
2. **Test changes**: Make one change at a time
3. **Monitor battery**: Check battery life with new settings
4. **Backup configs**: Save working configurations
5. **Document changes**: Note what each setting does

---

**Happy customizing!** ⚙️✨
