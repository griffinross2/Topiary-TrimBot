#include "file_receiver.h"

#include "packet.h"
#include "common.h"

#include <stdio.h>
#include <string.h>

static FileReceiverStatus s_file_receiver_status = FILE_RECEIVER_STATUS_IDLE;
static char s_current_filename[64];
static uint8_t s_file_data_buf[MAX_PACKET_DATA_SIZE];
static size_t s_file_data_buf_len = 0;

static FILE *s_output_file = nullptr;

void file_receiver_task()
{
    // Big state machine
    switch (s_file_receiver_status)
    {
    case FILE_RECEIVER_STATUS_IDLE:
        break;

    case FILE_RECEIVER_STATUS_START:
    {
        // Send the start ack
        PacketID ack_id = PACKET_TYPE_FILE_START;
        ack_id.ack = 1;
        int res = packet_send(nullptr, 0, ack_id);

        if (res >= 0)
        {
            TRACE_PRINTF("Sent start ack\n");

            // Successfully sent, now wait for chunks
            s_file_receiver_status = FILE_RECEIVER_STATUS_RECEIVING;
        }

        break;
    }

    case FILE_RECEIVER_STATUS_RECEIVING:
    {
        // Do nothing for now

        break;
    }

    case FILE_RECEIVER_STATUS_END:
    {
        // Send the end ack
        PacketID ack_id = PACKET_TYPE_FILE_END;
        ack_id.ack = 1;
        int res = packet_send(nullptr, 0, ack_id);

        if (res >= 0)
        {
            TRACE_PRINTF("Sent end ack\n");

            // Successfully sent, now go back to idle
            s_file_receiver_status = FILE_RECEIVER_STATUS_IDLE;

            // Also close the output file
            if (s_output_file != nullptr)
            {
                fclose(s_output_file);
                s_output_file = nullptr;
            }
        }

        break;
    }

    default:
        break;
    }
}

void file_receiver_give_packet(PacketID id, const uint8_t *data,
                               int data_length)
{
    switch (id.type)
    {
    case PACKET_TYPE_FILE_START.type:
        if (!id.ack && (s_file_receiver_status == FILE_RECEIVER_STATUS_IDLE || s_file_receiver_status == FILE_RECEIVER_STATUS_ERROR))
        {
            // Go to start and ack
            s_file_receiver_status = FILE_RECEIVER_STATUS_START;

            // Also open the output file
            char fname[64];
            memcpy(fname, data, data_length);
            fname[data_length] = '\0';
            s_output_file = fopen(fname, "wb");
            if (s_output_file == nullptr)
            {
                TRACE_PRINTF("Failed to open output file: %s\n", fname);
                s_file_receiver_status = FILE_RECEIVER_STATUS_ERROR;
            }
        }
        else
        {
            s_file_receiver_status = FILE_RECEIVER_STATUS_ERROR;
        }
        break;

    case PACKET_TYPE_FILE_CHUNK.type:
        if (!id.ack && s_file_receiver_status == FILE_RECEIVER_STATUS_RECEIVING)
        {
            // Keep receiving chunks
            // printf("Received chunk data:");
            // for (int i = 0; i < data_length; i++)
            // {
            //     printf("%c", data[i]);
            // }
            // printf("\n");

            // Write the chunk to the file
            // int chunk_idx = *((uint32_t *)data);
            if (s_output_file != nullptr)
            {
                size_t written = fwrite(data + 4, 1, data_length - 4, s_output_file);
                if (written != (size_t)data_length - 4)
                {
                    TRACE_PRINTF("Failed to write chunk to file\n");
                    s_file_receiver_status = FILE_RECEIVER_STATUS_ERROR;
                }
            }
        }
        else
        {
            s_file_receiver_status = FILE_RECEIVER_STATUS_ERROR;
        }
        break;

    case PACKET_TYPE_FILE_RESEND.type:
        // Shouldn't receive this
        s_file_receiver_status = FILE_RECEIVER_STATUS_ERROR;
        break;

    case PACKET_TYPE_FILE_END.type:
        if (!id.ack && s_file_receiver_status == FILE_RECEIVER_STATUS_RECEIVING)
        {
            // Go to end and ack
            s_file_receiver_status = FILE_RECEIVER_STATUS_END;
        }
        else
        {
            s_file_receiver_status = FILE_RECEIVER_STATUS_ERROR;
        }
        break;

    default:
        break;
    }
}