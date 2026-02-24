/*
 * Simple GPIO call to Raspberry Pi 5 
 * GPIOs using libgpiod
 * This simply calls GPIO0, which is the first
 * switch on the board and toggles it.
 * Prof. Nelson
 */
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    const char *chip_path = "/dev/gpiochip0";
    const unsigned int sw1 = 0;  // GPIO0
    const unsigned int sw2 = 1;  // GPIO1
    const unsigned int sw3 = 2;  // GPIO2
    const unsigned int sw4 = 3;  // GPIO3
    
    const char *consumer = "relay-demo";

    struct gpiod_chip *chip = NULL;
    struct gpiod_request_config *req_cfg = NULL;
    struct gpiod_line_config *line_cfg = NULL;
    struct gpiod_line_settings *settings = NULL;
    struct gpiod_line_request *request = NULL;

    chip = gpiod_chip_open(chip_path);
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    req_cfg = gpiod_request_config_new();
    line_cfg = gpiod_line_config_new();
    settings = gpiod_line_settings_new();
    if (!req_cfg || !line_cfg || !settings) {
        fprintf(stderr, "Failed to allocate libgpiod objects\n");
        goto cleanup;
    }

    gpiod_request_config_set_consumer(req_cfg, consumer);

    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) < 0) {
        perror("gpiod_line_settings_set_direction");
        goto cleanup;
    }

    // Optional: default output low
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    if (gpiod_line_config_add_line_settings(line_cfg, &sw1, 1, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        goto cleanup;
    }
    if (gpiod_line_config_add_line_settings(line_cfg, &sw2, 1, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        goto cleanup;
    }
    if (gpiod_line_config_add_line_settings(line_cfg, &sw3, 1, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        goto cleanup;
    }
    if (gpiod_line_config_add_line_settings(line_cfg, &sw4, 1, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        goto cleanup;
    }

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!request) {
        perror("gpiod_chip_request_lines");
        goto cleanup;
    }

    while (1) {
        if (gpiod_line_request_set_value(request, sw1, GPIOD_LINE_VALUE_ACTIVE) < 0) {
            perror("gpiod_line_request_set_value (ON)");
            break;
        }
        if (gpiod_line_request_set_value(request, sw2, GPIOD_LINE_VALUE_ACTIVE) < 0) {
            perror("gpiod_line_request_set_value (ON)");
            break;
        }
        if (gpiod_line_request_set_value(request, sw3, GPIOD_LINE_VALUE_ACTIVE) < 0) {
            perror("gpiod_line_request_set_value (ON)");
            break;
        }
        if (gpiod_line_request_set_value(request, sw4, GPIOD_LINE_VALUE_ACTIVE) < 0) {
            perror("gpiod_line_request_set_value (ON)");
            break;
        }
        printf("ON\n");
        sleep(1);

        if (gpiod_line_request_set_value(request, sw1, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("gpiod_line_request_set_value (OFF)");
            break;
        }
        if (gpiod_line_request_set_value(request, sw2, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("gpiod_line_request_set_value (OFF)");
            break;
        }
        if (gpiod_line_request_set_value(request, sw3, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("gpiod_line_request_set_value (OFF)");
            break;
        }
        if (gpiod_line_request_set_value(request, sw4, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("gpiod_line_request_set_value (OFF)");
            break;
        }
        printf("OFF\n");
        sleep(1);
    }
    
cleanup:
    if (request) gpiod_line_request_release(request);
    if (settings) gpiod_line_settings_free(settings);
    if (line_cfg) gpiod_line_config_free(line_cfg);
    if (req_cfg) gpiod_request_config_free(req_cfg);
    if (chip) gpiod_chip_close(chip);

    return 0;
}
