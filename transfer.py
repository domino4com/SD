import serial
import argparse
from serial.tools import list_ports
from json import loads
import pandas as pd
from time import sleep
from collections import OrderedDict


def find_port():
    target_pid = 0x7523
    target_vid = 0x1A86
    ports = list_ports.comports()
    for port in ports:
        if port.pid == target_pid and port.vid == target_vid:
            return port.device
    return None


def get_file_from_esp32(file_name, port_name):
    with serial.Serial(port_name, 115200, timeout=1) as ser:
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        command = f"GET_FILE {file_name}\n"
        ser.write(command.encode("utf-8"))
        attempts = 300
        while attempts > 0:
            start_marker = ser.readline().decode("utf-8").strip()
            if start_marker == "START_OF_FILE":
                content = ser.read_until("END_OF_FILE").decode("utf-8")
                content = "[" + content.rsplit("\n", 2)[0][:-2] + "]"
                # print(content[-200:])
                json_content = loads(content, object_pairs_hook=OrderedDict)
                print("Got the file - start saving to disk")
                return content, json_content
            attempts -= 1
        print("Start marker not found!")
        return None


def save_file_to_disk(file_name, content, json_content):
    with open(file_name, "w") as file:
        file.write(content)
        print(f"{file_name} saved successfully.")
    print("Saved the JSON file to disk, next is CSV conversion")
    csvfilename = file_name.split(".")[0] + ".csv"
    xlsfilename = file_name.split(".")[0] + ".xlsx"

    new = []

    for i, item in enumerate(json_content):
        newdict = OrderedDict()
        for j, jtem in enumerate(item):
            for k, ktem in enumerate(item[jtem]):
                heading = jtem + ": " + ktem["name"] + " [" + ktem["unit"] + "]"
                newdict[heading] = ktem["value"]
                # print(newdict)
        new.append(newdict)
    # print(new)

    df = pd.DataFrame(new)
    df.to_csv(csvfilename, index=False)
    df.to_excel(xlsfilename, sheet_name='xChip Data', index=False)
    print(f"{csvfilename} saved successfully.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="SDworks", description="SD Card manipulation via xChips"
    )
    parser.add_argument(
        "-f", "--file", type=str, help="Name of the file to retrieve from the xChips"
    )
    parser.add_argument(
        "-p", "--port", type=str, help="Serial port to connect to xChips", default=None
    )
    parser.add_argument(
        "-c",
        "--command",
        choices=["DELETEALL", "TRANSFER", "LISTDIR"],
        help="DELETEALL, TRANSFER or LISTDIR",
        default=None,
        required=True,
    )

    args = parser.parse_args()

    port = args.port
    if port is None:
        port = find_port()
        if port is None:
            print("Suitable port not found!")
            exit(1)

    command = args.command

    if command == "DELETEALL":
        with serial.Serial(port, 115200, timeout=1) as ser:
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            command = f"DELETEALL\n"
            ser.write(command.encode("utf-8"))

    elif command == "TRANSFER":
        if args.file is None:
            print("File name must be specified, use -f or --file")
            parser.parse_args(["-h"])
            exit(1)
        file_name = args.file.lower()
        file_content, json_content = get_file_from_esp32(file_name, port)
        if file_content is not None:
            save_file_to_disk(file_name, file_content, json_content)

    elif command == "LISTDIR":
        with serial.Serial(port, 115200, timeout=1) as ser:
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            command = f"LISTDIR\n"
            ser.write(command.encode("utf-8"))
            sleep(2)
            while ser.in_waiting:
                print(ser.readline().decode("utf-8").strip())

    else:
        parser.parse_args(["-h"])
        exit(1)
