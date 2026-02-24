/*
 * Cycle all GPIOs on relay board to Raspberry Pi 5 
 * GPIOs using libgpiod
 * This also makes sure all are off at start up
 * and when shutting down
 * Prof. Nelson
 */
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define CHIP_PATH "/dev/gpiochip0"
#define NUM_RELAYS 24
#define DELAY_S 1

// If your relay board is active-low, set ACTIVE_LOW to 1.
// ACTIVE_LOW=0 means "GPIO high = ON" (active-high).
#define ACTIVE_LOW 0

static volatile sig_atomic_t keep_running = 1;

static void handle_sigint(int sig)
{
    (void)sig;
    keep_running = 0;
}

static enum gpiod_line_value relay_on_value(void)
{
    return ACTIVE_LOW ? GPIOD_LINE_VALUE_INACTIVE : GPIOD_LINE_VALUE_ACTIVE;
}

static enum gpiod_line_value relay_off_value(void)
{
    return ACTIVE_LOW ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE;
}

static int set_all(struct gpiod_line_request *req, const unsigned int *offsets, size_t n,
                   enum gpiod_line_value val)
{
    for (size_t i = 0; i < n; i++) {
        if (gpiod_line_request_set_value(req, offsets[i], val) < 0) {
            perror("gpiod_line_request_set_value");
            return -1;
        }
    }
    return 0;
}

int main(void)
{
    signal(SIGINT, handle_sigint);

    unsigned int offsets[NUM_RELAYS];
    for (unsigned int i = 0; i < NUM_RELAYS; i++) offsets[i] = i;

    struct gpiod_chip *chip = gpiod_chip_open(CHIP_PATH);
    if (!chip) { perror("gpiod_chip_open"); return 1; }

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!req_cfg || !line_cfg || !settings) {
        fprintf(stderr, "Failed to allocate libgpiod objects\n");
        return 1;
    }

    gpiod_request_config_set_consumer(req_cfg, "relay-cycle");
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) < 0) {
        perror("gpiod_line_settings_set_direction");
        return 1;
    }

    // Default OFF at request time (helpful, but we also force OFF explicitly below)
    gpiod_line_settings_set_output_value(settings, relay_off_value());

    if (gpiod_line_config_add_line_settings(line_cfg, offsets, NUM_RELAYS, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        return 1;
    }

    struct gpiod_line_request *req = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!req) { perror("gpiod_chip_request_lines"); return 1; }

    // ---- Safety startup reset ----
    if (set_all(req, offsets, NUM_RELAYS, relay_off_value()) < 0) return 1;
    printf("Startup reset: all relays OFF\n");
    printf("Cycling GPIO0..GPIO23, %d second(s) each. Ctrl+C to exit.\n", DELAY_S);

    while (keep_running) {
        for (unsigned int i = 0; i < NUM_RELAYS && keep_running; i++) {
            // Turn ON relay i
            if (gpiod_line_request_set_value(req, offsets[i], relay_on_value()) < 0) {
                perror("set ON");
                keep_running = 0;
                break;
            }
            printf("Relay %u ON\n", i);
            sleep(DELAY_S);

            // Turn it back OFF
            if (gpiod_line_request_set_value(req, offsets[i], relay_off_value()) < 0) {
                perror("set OFF");
                keep_running = 0;
                break;
            }
        }
    }

    // ---- Safety shutdown reset ----
    set_all(req, offsets, NUM_RELAYS, relay_off_value());
    printf("\nShutdown reset: all relays OFF\n");

    gpiod_line_request_release(req);
    gpiod_line_settings_free(settings);
    gpiod_line_config_free(line_cfg);
    gpiod_request_config_free(req_cfg);
    gpiod_chip_close(chip);

    return 0;
}
