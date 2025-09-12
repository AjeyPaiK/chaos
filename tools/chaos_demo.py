#!/usr/bin/env python3
"""
Lorenz Attractor Watchy E-ink GIF Generator (Simple Version)

Copyright (c) 2024 Ajey Pai Karkala

This script generates a GIF animation showing how the Lorenz attractor
would appear on the Watchy e-ink display, simulating the actual watch face.
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.animation import FuncAnimation
import math
from PIL import Image, ImageDraw, ImageFont
import io

class LorenzSimulator:
    def __init__(self):
        # Lorenz system parameters (matching Arduino code)
        self.sigma = 10.0
        self.rho = 28.0
        self.beta = 8.0/3.0
        self.dt = 0.05
        
        # Initial position
        self.pos = np.array([1.0, 1.0, 1.0])
        
        # Trajectory storage
        self.max_points = 300
        self.trajectory = []
        self.rotation_angle = 0.0
        
        # Watchy display parameters
        self.display_width = 200
        self.display_height = 200
        
    def lorenz_derivative(self, p):
        """Calculate Lorenz system derivatives"""
        x, y, z = p
        dx = self.sigma * (y - x)
        dy = x * (self.rho - z) - y
        dz = x * y - self.beta * z
        return np.array([dx, dy, dz])
    
    def update_lorenz(self):
        """Runge-Kutta 4th order integration"""
        k1 = self.lorenz_derivative(self.pos)
        k2 = self.lorenz_derivative(self.pos + k1 * self.dt / 2)
        k3 = self.lorenz_derivative(self.pos + k2 * self.dt / 2)
        k4 = self.lorenz_derivative(self.pos + k3 * self.dt)
        
        self.pos += (k1 + 2*k2 + 2*k3 + k4) * self.dt / 6
    
    def project_3d_to_2d(self, point_3d):
        """Project 3D point to 2D with rotation"""
        x, y, z = point_3d
        
        # Apply Y-axis rotation
        cos_rot = math.cos(self.rotation_angle)
        sin_rot = math.sin(self.rotation_angle)
        
        x_rot = x * cos_rot - z * sin_rot
        y_rot = y
        
        return x_rot, y_rot
    
    def add_trajectory_point(self):
        """Add current position to trajectory"""
        if len(self.trajectory) < self.max_points:
            self.trajectory.append(self.pos.copy())
        else:
            # Circular buffer
            self.trajectory.pop(0)
            self.trajectory.append(self.pos.copy())
    
    def get_screen_coordinates(self):
        """Convert trajectory to screen coordinates"""
        if not self.trajectory:
            return [], []
        
        # Project all points to 2D
        screen_points = []
        for point in self.trajectory:
            x_2d, y_2d = self.project_3d_to_2d(point)
            screen_points.append([x_2d, y_2d])
        
        screen_points = np.array(screen_points)
        
        if len(screen_points) == 0:
            return [], []
        
        # Find bounds
        min_x, max_x = screen_points[:, 0].min(), screen_points[:, 0].max()
        min_y, max_y = screen_points[:, 1].min(), screen_points[:, 1].max()
        
        # Calculate scale factor
        range_x = max_x - min_x
        range_y = max_y - min_y
        
        if range_x < 0.1:
            range_x = 0.1
        if range_y < 0.1:
            range_y = 0.1
        
        scale_x = 80.0 / range_x
        scale_y = 80.0 / range_y
        scale = min(scale_x, scale_y)
        
        # Center and scale
        center_x = (min_x + max_x) / 2
        center_y = (min_y + max_y) / 2
        
        screen_x = (screen_points[:, 0] - center_x) * scale + 100
        screen_y = (screen_points[:, 1] - center_y) * scale + 100
        
        # Clamp to screen bounds
        screen_x = np.clip(screen_x, 5, 195)
        screen_y = np.clip(screen_y, 5, 195)
        
        return screen_x, screen_y
    
    def simulate_frame(self):
        """Simulate one frame of the animation"""
        # Add 50 points per frame (matching Arduino code)
        for _ in range(50):
            self.update_lorenz()
            self.add_trajectory_point()
        
        # Rotate view
        self.rotation_angle += 0.2
        if self.rotation_angle > 2 * math.pi:
            self.rotation_angle -= 2 * math.pi

def create_watchy_frame_pil(simulator, frame_num):
    """Create a single frame using PIL for better e-ink simulation"""
    # Simulate the frame
    simulator.simulate_frame()
    
    # Get screen coordinates
    screen_x, screen_y = simulator.get_screen_coordinates()
    
    # Create image (200x200, white background)
    img = Image.new('L', (200, 200), 255)  # 'L' = grayscale, 255 = white
    draw = ImageDraw.Draw(img)
    
    # Draw trajectory points (3x3 squares)
    for x, y in zip(screen_x, screen_y):
        # Draw 3x3 square for each point
        for dx in range(-1, 2):
            for dy in range(-1, 2):
                if 0 <= x + dx < 200 and 0 <= y + dy < 200:
                    draw.point((int(x + dx), int(y + dy)), fill=0)  # 0 = black
    
    # Draw current position (5x5 square) - last point
    if len(screen_x) > 0:
        current_x = int(screen_x[-1])
        current_y = int(screen_y[-1])
        for dx in range(-2, 3):
            for dy in range(-2, 3):
                if 0 <= current_x + dx < 200 and 0 <= current_y + dy < 200:
                    draw.point((current_x + dx, current_y + dy), fill=0)
    
    # Draw watch elements (FIXED - no animation)
    # Time (fixed)
    time_str = "14:23"
    try:
        # Try to use a monospace font
        font = ImageFont.truetype("/System/Library/Fonts/Monaco.ttf", 12)
    except:
        font = ImageFont.load_default()
    
    draw.text((5, 5), time_str, fill=0, font=font)
    
    # Date (fixed) - shorter format to fit
    date_str = "15/03"
    # Get text width to position it properly
    bbox = draw.textbbox((0, 0), date_str, font=font)
    text_width = bbox[2] - bbox[0]
    draw.text((200 - text_width - 5, 5), date_str, fill=0, font=font)
    
    # Battery icon (FULL - solid black)
    # Battery outline
    draw.rectangle([175, 185, 195, 195], outline=0, width=1)
    # Battery terminal
    draw.rectangle([195, 187, 197, 193], fill=0)
    # Fill battery (full)
    draw.rectangle([176, 186, 194, 194], fill=0)
    
    # Steps (fixed)
    steps = "2847"
    draw.text((5, 185), steps, fill=0, font=font)
    
    return img

def generate_gif():
    """Generate the complete GIF animation"""
    print("Generating Lorenz Attractor Watchy GIF...")
    
    # Create simulator
    simulator = LorenzSimulator()
    
    # Generate frames
    num_frames = 30  # 30 frames for 3-second cycle
    frames = []
    
    for frame in range(num_frames):
        print(f"Generating frame {frame + 1}/{num_frames}")
        img = create_watchy_frame_pil(simulator, frame)
        frames.append(img)
    
    # Save as GIF
    output_file = "chaos_watchy_demo.gif"
    frames[0].save(
        output_file,
        save_all=True,
        append_images=frames[1:],
        duration=100,  # 100ms per frame (10 FPS)
        loop=0
    )
    
    print(f"GIF saved as: {output_file}")
    print(f"Animation shows {num_frames} frames at 10 FPS")
    print("This simulates the 3-second update cycle of the actual Watchy")
    print("The GIF shows how the Lorenz attractor would appear on the e-ink display")

if __name__ == "__main__":
    generate_gif()
