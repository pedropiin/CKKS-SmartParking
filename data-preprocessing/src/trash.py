import pandas as pd

if __name__ == "__main__":
    true = []
    with open("../../dataset/labels-external.txt") as file:
        for x in file:
            true.append(int(x[0]))

    pred = []
    with open("../../dataset/output_encrypted_external.txt") as file:
        for x in file:
            pred.append(int(x[0]))

    total = len(pred)
    hit = 0
    miss = 0
    for i in range(total):
        if pred[i] == true[i]:
            hit += 1

    print(f"number of hits = {hit}")
    print(f"accuracy = {hit / total}")