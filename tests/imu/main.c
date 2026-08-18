#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "pico/time.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// I2C pins for Raspberry Pi Pico
#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

// MPU6050 registers
#define MPU6050_ADDR 0x68
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_CONFIG 0x1C
#define REG_GYRO_CONFIG 0x1B
#define REG_ACCEL_XOUT_H 0x3B

// Scale factors
#define ACCEL_SCALE 16384.0f   // ±2g
#define GYRO_SCALE 131.0f      // ±250 dps
#define TEMP_SCALE 340.0f
#define TEMP_OFFSET 36.53f

#define GRAVITY 9.80665f
#define ALPHA 0.98f   // complementary filter coefficient

static void mpu6050_write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, buf, 2, false);
}

static void mpu6050_read_regs(uint8_t reg, uint8_t *buf, size_t len) {
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true); // keep control for restart
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, buf, len, false);
}

int main() {
    stdio_init_all();

    // Wait for USB serial terminal to connect
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Initialize I2C at 400 kHz
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Wake up MPU6050
    mpu6050_write_reg(REG_PWR_MGMT_1, 0x00);
    sleep_ms(100);

    // Set accelerometer full scale: ±2g
    mpu6050_write_reg(REG_ACCEL_CONFIG, 0x00);
    // Set gyro full scale: ±250 dps
    mpu6050_write_reg(REG_GYRO_CONFIG, 0x00);

    uint64_t last_time = time_us_64();

    float roll = 0.0f;   // radians
    float pitch = 0.0f;  // radians

    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;

    printf("MPU6050 ready.\n");


    while (true) {
        uint64_t now = time_us_64();
        float dt = (now - last_time) / 1000000.0f;
        last_time = now;

        if (dt <= 0.0f || dt > 0.5f) {
            dt = 0.01f;  // clamp to avoid spikes
        }

        // Read 14 bytes starting from ACCEL_XOUT_H
        uint8_t buf[14];
        mpu6050_read_regs(REG_ACCEL_XOUT_H, buf, 14);

        int16_t ax_raw = (int16_t)((buf[0] << 8) | buf[1]);
        int16_t ay_raw = (int16_t)((buf[2] << 8) | buf[3]);
        int16_t az_raw = (int16_t)((buf[4] << 8) | buf[5]);
        int16_t temp_raw = (int16_t)((buf[6] << 8) | buf[7]);
        int16_t gx_raw = (int16_t)((buf[8] << 8) | buf[9]);
        int16_t gy_raw = (int16_t)((buf[10] << 8) | buf[11]);
        int16_t gz_raw = (int16_t)((buf[12] << 8) | buf[13]);

        // Convert to physical units
        float ax_g = ax_raw / ACCEL_SCALE;
        float ay_g = ay_raw / ACCEL_SCALE;
        float az_g = az_raw / ACCEL_SCALE;

        float gx_dps = gx_raw / GYRO_SCALE;
        float gy_dps = gy_raw / GYRO_SCALE;
        float gz_dps = gz_raw / GYRO_SCALE;

        float temp_c = temp_raw / TEMP_SCALE + TEMP_OFFSET;

        // Accelerometer-based roll/pitch (in radians)
        float accel_roll = atan2f(ay_g, az_g);
        float accel_pitch = atan2f(-ax_g, sqrtf(ay_g * ay_g + az_g * az_g));

        // Gyro rates in rad/s
        float gx_rad = gx_dps * (float)M_PI / 180.0f;
        float gy_rad = gy_dps * (float)M_PI / 180.0f;

        // Complementary filter: combine gyro and accel
        roll = ALPHA * (roll + gx_rad * dt) + (1.0f - ALPHA) * accel_roll;
        pitch = ALPHA * (pitch + gy_rad * dt) + (1.0f - ALPHA) * accel_pitch;

        // Rotate body acceleration to world frame (yaw ignored)
        float cos_r = cosf(roll);
        float sin_r = sinf(roll);
        float cos_p = cosf(pitch);
        float sin_p = sinf(pitch);

        float ax_world = cos_p * ax_g + sin_r * sin_p * ay_g + cos_r * sin_p * az_g;
        float ay_world = cos_r * ay_g - sin_r * az_g;
        float az_world = -sin_p * ax_g + sin_r * cos_p * ay_g + cos_r * cos_p * az_g;

        // Remove gravity from world Z
        float lin_ax = ax_world;
        float lin_ay = ay_world;
        float lin_az = az_world - 1.0f;

        // Integrate linear acceleration to get velocity
        vx += lin_ax * GRAVITY * dt;
        vy += lin_ay * GRAVITY * dt;
        vz += lin_az * GRAVITY * dt;

        printf("ax=%.3f ay=%.3f az=%.3f g | "
               "gx=%.2f gy=%.2f gz=%.2f dps | "
               "temp=%.2f C | "
               "roll=%.1f pitch=%.1f deg | "
               "vx=%.2f vy=%.2f vz=%.2f m/s\n",
               ax_g, ay_g, az_g,
               gx_dps, gy_dps, gz_dps,
               temp_c,
               roll * 180.0f / (float)M_PI,
               pitch * 180.0f / (float)M_PI,
               vx, vy, vz);

        sleep_ms(50);
    }

    return 0;
}