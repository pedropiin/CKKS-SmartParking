def save_labels(arr_spots_state, img_name):
    labels_path = "../../dataset/labels.csv"

    with open(labels_path, 'a') as fd:
        for i in range(len(arr_spots_state)):
            spot_name = img_name + "_resize_park_spot_" + str(i) + ".jpg"
            new_row = spot_name + "," + str(arr_spots_state[i]) + "\n"
            fd.write(new_row)

    print("Wrote value for img: " + img_name)

if __name__ == "__main__":
    save_labels()