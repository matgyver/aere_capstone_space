/*
 * Simple WASD Example using RPI5 
 * GPIOs using libgpiod
 * Toggles the relay using keyboard commands
 * W - GPIO0, A - GPIO1, S - GPIO2, D - GPIO3
 * Prof. Nelson
 */
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>

#define CHIP_PATH "/dev/gpiochip0"
#define NUM_RELAYS 24

// If your relay board is active-low, set ACTIVE_LOW to 1.
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

    // Define all 24 relay GPIOs (GPIO0..GPIO23)
    unsigned int offsets[NUM_RELAYS];
    for (unsigned int i = 0; i < NUM_RELAYS; i++) offsets[i] = i;

    // Track state for all relays (we’ll only toggle first four for the demo)
    int state[NUM_RELAYS] = {0};

    struct gpiod_chip *chip = gpiod_chip_open(CHIP_PATH);
    if (!chip) { perror("gpiod_chip_open"); return 1; }

    struct gpiod_request_config *req_cfg = gpiod_request_config_new();
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!req_cfg || !line_cfg || !settings) {
        fprintf(stderr, "Failed to allocate libgpiod objects\n");
        return 1;
    }

    gpiod_request_config_set_consumer(req_cfg, "relay-wasd");
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) < 0) {
        perror("gpiod_line_settings_set_direction");
        return 1;
    }

    // Default OFF at request time
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
    printf("WASD toggles GPIO0..GPIO3 (W=0, A=1, S=2, D=3). Ctrl+C to exit.\n");

    // ---- Terminal raw mode + non-blocking stdin ----
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        perror("tcgetattr");
        return 1;
    }
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) {
        perror("tcsetattr");
        return 1;
    }

    int oldflags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (oldflags < 0) oldflags = 0;
    fcntl(STDIN_FILENO, F_SETFL, oldflags | O_NONBLOCK);

    while (keep_running) {
        char ch;
        int n = (int)read(STDIN_FILENO, &ch, 1);

        if (n > 0) {
            int idx = -1;
            switch (ch) {
                case 'w': case 'W': idx = 0; break;
                case 'a': case 'A': idx = 1; break;
                case 's': case 'S': idx = 2; break;
                case 'd': case 'D': idx = 3; break;
                default: break;
            }

            if (idx >= 0) {
                state[idx] = !state[idx];

                enum gpiod_line_value v = state[idx] ? relay_on_value() : relay_off_value();
                if (gpiod_line_request_set_value(req, offsets[idx], v) < 0) {
                    perror("gpiod_line_request_set_value");
                    keep_running = 0;
                    break;
                }

                printf("Relay %d %s\n", idx, state[idx] ? "ON" : "OFF");
                fflush(stdout);
            }
        }

        usleep(5000); // reduce CPU usage
    }

    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

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
