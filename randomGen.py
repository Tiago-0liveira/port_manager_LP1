import sys
from random import randint, choice
from string import ascii_uppercase

menu = """
This file generates random files for testing this project.
Usage: python randomGen.py <filename> <max number of embarcacoes> <max number of piles> <max number of containers per pile> <print(1|0)>
"""


def random_contentor():
    code = choice(ascii_uppercase) + choice(ascii_uppercase) + str(randint(0, 9))
    weight = randint(500, 5000)
    return f"{code}:{weight}"


def main():
    print(sys.argv)
    if len(sys.argv) < 5 or not sys.argv[2].isdigit() or not sys.argv[3].isdigit() or not sys.argv[4].isdigit():
        print(menu)
        return
    filename = sys.argv[1]
    embarcacoes = int(sys.argv[2])
    max_piles = int(sys.argv[3])
    max_containers = int(sys.argv[4])
    print_file = len(sys.argv) > 5 and sys.argv[5] == "1"
    if 0 >= embarcacoes or embarcacoes > 10 or 0 >= max_containers:
        print("Invalid parameters, numbers cant be less than 1!")
        return
    with open(filename, "w") as f:
        for i in range(randint(1, embarcacoes)):
            s = f"d{i} {''.join(choice(ascii_uppercase) for _ in range(4))}\n"
            if print_file: print(s, end="")
            f.write(s)
            for j in range(randint(1, max_piles)):
                l = randint(1, max_containers)
                d = f"\tp{j} {l} {' '.join(random_contentor() for _ in range(l))}\n"
                if print_file: print(d, end="")
                f.write(d)


if __name__ == "__main__":
    main()
