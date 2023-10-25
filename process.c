#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include <assert.h>

static int WINDOW_SIZE = 24 * 60 * 60 * 1000;
static int HOP_SIZE = 1000;
static char *file = "data/ETHUSDT-trades-2017-08.csv";

long deadline, cur_frame_starttime, prev_time;

double window_t_price = 0;
double n_window_item = 0;

int main()
{
    FILE *fp = fopen(file, "r");
    if (fp == NULL)
    {
        printf("Could not open file\n");
        return 1;
    }

    // initialize queue
    initializeQueue(WINDOW_SIZE / HOP_SIZE);

    char line[128];
    char *token;

    long time;
    double price, volume;

    double frame_t_price = 0.0;
    int n_frame_item = 0;

    int init_step_flag = 1;

    int queue_count = 0; // Debugging: count queue size

    fgets(line, sizeof(line), fp); // Skip header if any

    // initialize the queue
    while (fgets(line, sizeof(line), fp))
    {
        token = strtok(line, ",");
        time = atol(token);

        token = strtok(NULL, ",");
        price = atof(token);

        token = strtok(NULL, ",");
        volume = atof(token);

        // Algo to compute price
        price *= volume;

        if (init_step_flag)
        {
            // init tracking variables
            cur_frame_starttime = time;
            deadline = time + WINDOW_SIZE;
            init_step_flag = 0;
        }

        if (time <= deadline)
        {
            if (time < cur_frame_starttime + HOP_SIZE)
            {
                frame_t_price += price;
                n_frame_item++;
            }
            else
            {
                enqueue(frame_t_price);
                window_t_price += frame_t_price;
                n_window_item += n_frame_item;

                queue_count++; // Debugging

                // append zeros for missing frames
                for (long i = prev_time + HOP_SIZE; i < time; i += HOP_SIZE)
                {
                    enqueue(0);
                    queue_count++; // Debugging
                    cur_frame_starttime += HOP_SIZE;
                }

                // reset frame variables
                n_frame_item = 1;
                frame_t_price = price;
                cur_frame_starttime += HOP_SIZE;
            }

            prev_time = time;
        }
        else
        {
            break;
        }
    }

    // append zeros for missing frames
    for (long i = cur_frame_starttime + HOP_SIZE; i <= deadline; i += HOP_SIZE)
    {
        enqueue(0);
        queue_count++; // Debugging
    }
    
    assert(queue_count == WINDOW_SIZE / HOP_SIZE);
    printf("%f\n", window_t_price / n_window_item);

    fclose(fp); // Close the file

    return 0;
}
