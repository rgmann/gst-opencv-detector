import os
import sys
module_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'build', 'src', 'generated')
print('Adding module path: {}'.format(module_path))
sys.path.append(module_path) 

import socket
import argparse
from datetime import datetime
from gst_opencv_detector.DetectionList import DetectionList

HEADER_SIZE  =4
MAX_MESSAGE_SIZE = 4096

parser = argparse.ArgumentParser(
    prog='Example OpenCV detections client',
    description='This utility demonstrates how to subscribe to and receive detections.'
)
parser.add_argument('-a', '--address', type=str, required=True, help='Host address')
parser.add_argument('-p', '--port', type=int, required=True, help='Host port')

args = parser.parse_args()


def parse_message_size(raw):
    try:
        return int(raw.decode())
    except Exception as err:
        return None

def parse_detections_list(raw):
    return DetectionList.GetRootAs(raw)


def print_detections_list(detections_list : DetectionList):

    tx_ts = detections_list.Timestamp()
    tx_ts /= 1000.0
    tx_ts = datetime.fromtimestamp(tx_ts).strftime('%Y-%m-%d %H:%M:%S.%f')

    msg = [
        'Detection list\n',
        '  TX TS = {}\n'.format(tx_ts),
        '  RX TS = {}\n'.format(datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')),
        '  Image:\n',
        '    WIDTH = {}\n'.format(detections_list.ImageWidth()),
        '    HEIGHT = {}\n'.format(detections_list.ImageHeight()),
        '  Detections:\n',
    ]

    if len(detections_list.Detections()) > 0:
        for detection in detections_list.Detections():
            msg.append('    Detection:\n')
            msg.append('      ID = {}\n'.format(detection.ClassId()))
            msg.append('      NAME = {}\n'.format(detection.ClassName()))
            msg.append('      CONFIDENCE = {}\n'.format(detection.Confidence()))
            msg.append('      RECT = ({},{},{},{})\n'.format(
                detection.Box().X(), 
                detection.Box().Y(), 
                detection.Box().Width(), 
                detection.Box().Height()
            ))
    else:
        msg.append('    NONE\n')

    print(msg)

# Create a socket object
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connect to the server
    client_socket.connect((args.address, args.port))

    print('Connected to server')

    while True:
        header_data = client_socket.recv(HEADER_SIZE)

        if header_data:
            message_size = parse_message_size(header_data)

            if message_size and message_size < MAX_MESSAGE_SIZE:
                message_data = client_socket.recv(message_size)
                if message_data:
                    detections_list = parse_detections_list(message_data)

                    print_detections_list(detections_list)

                else:
                    print('Detected server disconnect. Exiting.')
                    break
            else:
                print('Invalid message size: {}'.format(message_size))
                break

        else:
            print('Detected server disconnect. Exiting.')
            break

    # Close the connection
    client_socket.close()

except Exception as e:

    print('Failed to connect to server: "{}"'.format(e))

# # Send data to the server
# message = "Hello, server!"
# client_socket.send(message.encode())

# # Receive data from the server
# data = client_socket.recv(1024)
# print("Received from server:", data.decode())

