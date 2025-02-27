import sys
import random
import string

def get_random_string(len):
    return ''.join(random.choice(string.ascii_letters + string.digits) for _ in range(len))

if __name__ == '__main__':
    path = sys.argv[1]
    lines_count = int(sys.argv[2])
    line_avg_size = int(sys.argv[3])

    with open(path, "w") as file:
        for i in range(lines_count):
            line_length = random.randrange(1, 2 * line_avg_size)
            str = get_random_string(line_length)
            file.write(str)
            file.write("\n")