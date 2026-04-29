from packet import *
from create_slices import get_slices, get_plant_slice, get_model_slice, get_slicer_bound, show_segments
import struct
from usb import USBDev
from enum import Enum

class SlicerStatus(Enum):
    SLICER_STATUS_IDLE = 0
    SLICER_STATUS_SENDING = 1

plant_mesh = None
model_mesh = None
slicer_bound = 0
waiting_ack = False
segment_i = 0
slicer_status = SlicerStatus.SLICER_STATUS_IDLE
plant_slice = None
model_slice = None
slice_idx = 0
on_model = False

def slicer_view_init():
    global plant_mesh
    global model_mesh
    global slicer_bound
    global waiting_ack
    global segment_i
    global slicer_status
    global plant_slice
    global model_slice
    global slice_idx
    global on_model

    plant_mesh = None
    model_mesh = None
    slicer_bound = 0
    waiting_ack = False
    segment_i = 0
    slicer_status = SlicerStatus.SLICER_STATUS_IDLE
    plant_slice = None
    model_slice = None
    slice_idx = 0
    on_model = False

def slicer_view_task(usb_dev: USBDev):
    global slicer_status
    global waiting_ack
    global segment_i
    global plant_slice
    global model_slice
    global on_model

    match slicer_status:
        case SlicerStatus.SLICER_STATUS_IDLE:
            pass

        case SlicerStatus.SLICER_STATUS_SENDING:
            if not waiting_ack:
                if not on_model:
                    segment_i = slicer_view_send_slice(plant_slice, slice_idx, False, usb_dev, segment_i)
                    # print(f"len(plant_slice): {len(plant_slice)} segment_i after plant send: {segment_i}")
                else:
                    segment_i = slicer_view_send_slice(model_slice, slice_idx, True, usb_dev, segment_i)
                    # print(f"len(model_slice): {len(model_slice)} segment_i after model send: {segment_i}")
                waiting_ack = True

def slicer_view_give_meshes(new_plant_mesh, new_model_mesh):
    global plant_mesh
    global model_mesh

    plant_mesh = new_plant_mesh
    model_mesh = new_model_mesh

def slicer_view_send_slice(slice, slice_idx, is_model, usb_dev, segment_i):
    # First byte: [7-1] slice_idx, [0] is_model_slice
    # Next four bytes: float representing the slicer bound
    data_size = 5
    header_data = bytes([((slice_idx << 1) & 0xFE) + (1 if is_model else 0)]) + struct.pack('f', slicer_bound)
    msg_data = header_data

    # Now send segments in full messages
    while segment_i < len(slice):
        segment = slice[segment_i]
        segment_i += 1

        # Check if we can still add to this message
        if data_size + 16 <= max_packet_data_size():
            x1 = segment[0][0]
            y1 = segment[0][1]
            x2 = segment[1][0]
            y2 = segment[1][1]
            print(f"Line segment: ({x1}, {y1}) to ({x2}, {y2})")
            msg_data += struct.pack('f', x1)
            msg_data += struct.pack('f', y1)
            msg_data += struct.pack('f', x2)
            msg_data += struct.pack('f', y2)
            data_size += 16
        else:
            # Send off this message
            packet_send(usb_dev, msg_data, PACKET_TYPE_CROSS_SECTION)
            data_size = 5
            msg_data = header_data
            return segment_i

    # Send off the last one
    if data_size > 5:
        packet_send(usb_dev, msg_data, PACKET_TYPE_CROSS_SECTION)

    return segment_i

def slicer_view_give_packet(id: PacketID, data: bytes):
    global plant_mesh
    global model_mesh
    global slicer_bound
    global plant_slice
    global model_slice
    global slicer_status
    global segment_i
    global slice_idx
    global waiting_ack
    global on_model

    match id.packet_type:
        case PACKET_TYPE_CROSS_SECTION.packet_type:
            # Should be an ack
            if id.ack:
                waiting_ack = False
                if not on_model and segment_i == len(plant_slice):
                    # print("going to models")
                    on_model = True
                    segment_i = 0

                    # If no model segments, just end
                    if len(model_slice) < 1:
                        on_model = False
                        slicer_status = SlicerStatus.SLICER_STATUS_IDLE

                elif segment_i == len(model_slice):
                    # print("models done")
                    on_model = False
                    segment_i = 0
                    slicer_status = SlicerStatus.SLICER_STATUS_IDLE

        case PACKET_TYPE_CREATE_CROSS_SECTIONS.packet_type:
            # If we have the meshes (we should) generate the slices
            if plant_mesh is None or model_mesh is None:
                print("Meshes not present in slicer view")
                return

            if len(data) < 1:
                print("Expected longer packet CREATE_CROSS_SECTIONS")
                return
            
            # Extract number of slices requested
            num_slices = int(data[0])

            # Generate the slices and remember the size of them
            get_slices(plant_mesh, model_mesh, num_slices)
            slicer_bound = get_slicer_bound(plant_mesh)

        case PACKET_TYPE_GET_CROSS_SECTION.packet_type:
            # Send the requested cross section
            if len(data) < 1:
                print("Expected longer packet CREATE_CROSS_SECTIONS")
                return
            
            on_model = False
            
            # Extract slice index
            slice_idx = int(data[0])

            # Get the plant slice
            plant_slice = get_plant_slice(slice_idx)

            # Now the model slice
            model_slice = get_model_slice(slice_idx)

            segment_i = 0

            # If no plant segments, go to models
            if len(plant_slice) < 1:
                on_model = True

                # If no model segments, just dont send anything
                if len(model_slice) < 1:
                    on_model = False
                    slicer_status = SlicerStatus.SLICER_STATUS_IDLE
                    return

            # show_segments(plant_slice + model_slice, 'k-', slicer_bound)
            
            slicer_status = SlicerStatus.SLICER_STATUS_SENDING

        case _:
            pass