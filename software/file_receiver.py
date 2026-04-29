from packet import *
from usb import USBDev
from enum import Enum

class FileReceiverStatus(Enum):
    FILE_RECEIVER_STATUS_IDLE = 0
    FILE_RECEIVER_STATUS_START = 1
    FILE_RECEIVER_STATUS_RECEIVING = 2
    FILE_RECEIVER_STATUS_RESENDING = 3
    FILE_RECEIVER_STATUS_END = 4
    FILE_RECEIVER_STATUS_ERROR = 5

file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_IDLE
file_receiver_out_file = None
file_receiver_file_name = ""

def file_receiver_init():
    global file_receiver_status
    global file_receiver_out_file
    global file_receiver_file_name

    file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_IDLE
    file_receiver_out_file = None
    file_receiver_file_name = ""

def file_receiver_task(dev: USBDev):
    global file_receiver_status
    global file_receiver_out_file

    # State machine
    match file_receiver_status:
        case FileReceiverStatus.FILE_RECEIVER_STATUS_IDLE:
            pass

        case FileReceiverStatus.FILE_RECEIVER_STATUS_START:
            # Send the start ack
            ack_id = PACKET_TYPE_FILE_START
            ack_id.ack = True

            res = packet_send(dev, bytes(), ack_id)

            if res >= 0:
                print("Sent start ack")
                file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_RECEIVING
            
        case FileReceiverStatus.FILE_RECEIVER_STATUS_RECEIVING:
            pass

        case FileReceiverStatus.FILE_RECEIVER_STATUS_END:
            # Send the end ack
            ack_id = PACKET_TYPE_FILE_END
            ack_id.ack = True

            res = packet_send(dev, bytes(), ack_id)

            if res >= 0:
                # Go back to idle and close the file
                print("Sent end ack")
                file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_IDLE

                if file_receiver_out_file:
                    file_receiver_out_file.close()
                    file_receiver_out_file = None

        case _:
            pass

def file_receiver_give_packet(id: PacketID, data: bytes):
    global file_receiver_status
    global file_receiver_out_file
    global file_receiver_file_name

    match id.packet_type:
        case PACKET_TYPE_FILE_START.packet_type:
            if not id.ack and (file_receiver_status == FileReceiverStatus.FILE_RECEIVER_STATUS_IDLE or FileReceiverStatus.FILE_RECEIVER_STATUS_ERROR):
                # Go to start
                file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_START

                # Open output file
                fname = "meshes/" + data.decode('utf-8')
                file_receiver_file_name = fname
                file_receiver_out_file = open(fname, 'wb')
                if file_receiver_out_file is None:
                    print(f"Failed to open output file: {fname}")
                    file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_ERROR

            else:
                file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_ERROR
        
        case PACKET_TYPE_FILE_CHUNK.packet_type:
            if not id.ack and file_receiver_status == FileReceiverStatus.FILE_RECEIVER_STATUS_RECEIVING:
                # Write chunk to file
                if file_receiver_out_file:
                    written = file_receiver_out_file.write(data[4:])
                    if written != len(data) - 4:
                        print("Failed to write chunk to file")
                        file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_ERROR
                else:
                    print("Failed to write chunk to file")
                    file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_ERROR
            else:
                file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_ERROR

        case PACKET_TYPE_FILE_RESEND.packet_type:
            file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_ERROR

        case PACKET_TYPE_FILE_END.packet_type:
            if not id.ack and file_receiver_status == FileReceiverStatus.FILE_RECEIVER_STATUS_RECEIVING:
                file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_END
            else:
                file_receiver_status = FileReceiverStatus.FILE_RECEIVER_STATUS_ERROR

        case _:
            pass

def file_receiver_get_model_file_name():
    global file_receiver_file_name
    return file_receiver_file_name