import os

reverse_hex_nibble = {
    '0': '0', # 0b0000 -> 0b0000
    '1': '8', # 0b0001 -> 0b1000
    '2': '4', # 0b0010 -> 0b0100
    '3': 'C', # 0b0011 -> 0b1100
    '4': '2', # 0b0100 -> 0b0010
    '5': 'A', # 0b0101 -> 0b1010
    '6': '6', # 0b0110 -> 0b0110
    '7': 'E', # 0b0111 -> 0b1110
    '8': '1', # 0b1000 -> 0b0001
    '9': '9', # 0b1001 -> 0b1001
    'A': '5', # 0b1010 -> 0b0101
    'B': 'D', # 0b1011 -> 0b1101
    'C': '3', # 0b1100 -> 0b0011
    'D': 'B', # 0b1101 -> 0b1011
    'E': '7', # 0b1110 -> 0b0111
    'F': 'F', # 0b1111 -> 0b1111
}

def reverse_hex_file(input_file, output_file):
    with open(input_file, 'r') as i_f:
        print(f'    Reading from {input_file}...')
        with open(output_file, 'w') as o_f:
            print(f'    Writing to {output_file}...')
            for l in i_f:
                hex_byte = l.strip()
                reversed_byte = hex_byte[::-1]
                new_byte = ''
                for nibble in reversed_byte:
                    new_byte += reverse_hex_nibble[nibble.upper()]
                print(f'    {hex_byte} -> {reversed_byte} -> {new_byte}')
                o_f.write(new_byte + '\n')
    print(f'{os.path.basename(input_file)} -> {os.path.basename(output_file)} complete.')

if __name__ == "__main__":
    directory = os.path.dirname(os.path.realpath(__file__))
    input_dir = os.path.join(directory, 'inputs')
    output_dir = os.path.join(directory, 'outputs')
    print(f'Inputs: {input_dir}')
    print(f'Outputs: {output_dir}')
    for f in os.listdir(input_dir):
        print(f'Reversing {f}...')
        reverse_hex_file(os.path.join(input_dir, f), os.path.join(output_dir, f.split('.')[0] + '_reversed.txt'))