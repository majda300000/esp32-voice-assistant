
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "driver/i2s.h"
#include "ringbuf.h"
#include "recsrc.h"
#include "esp_aec.h"
#include "esp_mase.h"
#include "esp_ns.h"
#include "esp_agc.h"

extern struct RingBuf *rec_rb;
extern struct RingBuf *ns_rb;
extern struct RingBuf *agc_rb;

#define AEC_FRAME_BYTES 512
#define MASE_FRAME_BYTES 512
#define NS_FRAME_BYTES 960
#define AGC_FRAME_BYTES 320

#define I2S_CHANNEL_NUM 2

void recsrcTask(void *arg)
{
    size_t bytes_read;
    int16_t *rsp_in = malloc(AEC_FRAME_BYTES * I2S_CHANNEL_NUM);
    int16_t *aec_ref = malloc(AEC_FRAME_BYTES);
    int nch = 2;
    int16_t *aec_rec = malloc(AEC_FRAME_BYTES * nch);
    int16_t *aec_out = malloc(AEC_FRAME_BYTES * nch);
    int16_t *mase_out = malloc(MASE_FRAME_BYTES);
    void *aec_handle = aec_create_multimic(16000, AEC_FRAME_LENGTH_MS, AEC_FILTER_LENGTH, nch);
    void *mase_handle = mase_create(16000, MASE_FRAME_SIZE, TWO_MIC_LINE, 65, WAKE_UP_ENHANCEMENT_MODE, 0);

    while (1)
    {
        i2s_read(I2S_NUM_0, rsp_in, 2 * AEC_FRAME_BYTES, &bytes_read, portMAX_DELAY);
        for (int i = 0; i < AEC_FRAME_BYTES / 2; i++)
        {
            aec_out[i] = (rsp_in[2 * i] + rsp_in[2 * i + 1]) / 2;
        }
        rb_write(rec_rb, aec_out, AEC_FRAME_BYTES, portMAX_DELAY);
    }
}

void nsTask(void *arg)
{
    int16_t *ns_in = malloc(NS_FRAME_BYTES);
    int16_t *ns_out = malloc(NS_FRAME_BYTES);
    ns_handle_t ns_inst = ns_create(30);
    while (1)
    {
        rb_read(rec_rb, (uint8_t *)ns_in, NS_FRAME_BYTES, portMAX_DELAY);
        ns_process(ns_inst, ns_in, ns_out);
        rb_write(ns_rb, (uint8_t *)ns_out, NS_FRAME_BYTES, portMAX_DELAY);
    }
}

void agcTask(void *arg)
{
    int16_t *agc_in = malloc(AGC_FRAME_BYTES);
    int16_t *agc_out = malloc(AGC_FRAME_BYTES);

    void *agc_handle = esp_agc_open(3, 16000);
    set_agc_config(agc_handle, 15, 1, 3);

    int _err_step = 1;
    if (0 == (agc_in && _err_step++ && agc_out && _err_step++ && agc_handle && _err_step++))
    {
        printf("Failed to apply for memory, err_step = %d", _err_step);
        goto _agc_init_fail;
    }
    while (1)
    {
        rb_read(ns_rb, (uint8_t *)agc_in, AGC_FRAME_BYTES, portMAX_DELAY);
        esp_agc_process(agc_handle, agc_in, agc_out, AGC_FRAME_BYTES / 2, 16000);
        rb_write(agc_rb, (uint8_t *)agc_out, AGC_FRAME_BYTES, portMAX_DELAY);
    }
_agc_init_fail:
    if (agc_in)
    {
        free(agc_in);
        agc_in = NULL;
    }
    if (agc_out)
    {
        free(agc_out);
        agc_out = NULL;
    }
    if (agc_handle)
    {
        free(agc_handle);
        agc_handle = NULL;
    }
    vTaskDelete(NULL);
}
