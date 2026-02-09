import random

rows, cols = 176, 4
with open("random_rgba_values.txt", "w") as file:
    for _ in range(rows):
        # r = random.randint(0, 255)
        # g = random.randint(0, 255)
        # b = random.randint(0, 255)
        r = 255
        g = 255
        b = 255
        a = 255
        rgba_values = f"{r} {g} {b} {a}"
        file.write(rgba_values + "\n")

print("File saved as random_rgba_values.txt")
