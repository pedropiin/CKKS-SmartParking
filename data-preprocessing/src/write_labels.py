from os import listdir

IMG_SIZE = 640
NUM_PARK_SPOTS = 16

"""
Array of parking spot positions in the uncropped resized images
"""
arr_spots_uncrop_small = [(263, 183, 303, 223),     # 1th parking spot  (40x40)
                        (297, 187, 337, 227),       # 2th parking spot  (40x40)
                        (331, 192, 371, 232),       # 3th parking spot  (40x40)
                        (366, 200, 406, 240),       # 4th parking spot  (40x40)
                        (400, 207, 440, 247),       # 5th parking spot  (40x40)
                        (427, 217, 467, 257),       # 6th parking spot  (40x40)
                        (447, 223, 487, 263),       # 7th parking spot  (40x40)
                        (472, 226, 512, 266),       # 8th parking spot  (40x40)
                        (84, 263, 164, 373),        # 9th parking spot  (80x110)
                        (131, 278, 211, 388),       # 10th parking spot (80x110)
                        (334, 320, 414, 430),       # 11th parking spot (80x110)
                        (407, 330, 487, 440),       # 12th parking spot (80x110)
                        (472, 333, 532, 433),       # 13th parking spot (60x100)
                        (501, 339, 551, 429),       # 14th parking spot (50x90)
                        (529, 340, 579, 430),       # 15th parking spot (50x90)
                        (554, 340, 594, 420)]       # 16th parking spot (40x80)


def write_labels():
    dataset_path = "../../dataset/spots-uncropped/"
    labels_path = "../../dataset/labels-spots-uncropped/"
    save_path = "../../dataset/labels.csv"

    label_list = listdir(labels_path)
    for label_num, img_name in enumerate(label_list):
        spots_state = [0] * NUM_PARK_SPOTS

        labels_it = open((labels_path + img_name), 'r') 
        for line in labels_it:
            content = line.split()

            center_x_bb = int(float(content[1]) * IMG_SIZE)
            center_y_bb = int(float(content[2]) * IMG_SIZE)
            width_bb = int(float(content[3]) * IMG_SIZE)
            height_bb = int(float(content[4]) * IMG_SIZE)

            bb_up_left = tuple(((center_x_bb - (width_bb / 2)), (center_y_bb - (height_bb / 2))))
            bb_bot_right = tuple(((center_x_bb + (width_bb / 2)), (center_y_bb + (height_bb / 2))))

            max_area = 0 
            area = 0
            max_spot = -1
            spot_idx = 0

            for spot_dim in arr_spots_uncrop_small:
                up_left_x = max(bb_up_left[0], spot_dim[0])
                up_left_y = max(bb_up_left[1], spot_dim[1])
                bot_right_x = min(bb_bot_right[0], spot_dim[2])
                bot_right_y = min(bb_bot_right[1], spot_dim[3])

                if (up_left_x < bot_right_x and up_left_y < bot_right_y):
                    area = ((bot_right_x - up_left_x) * (bot_right_y - up_left_y))

                area_spot = ((spot_dim[2] - spot_dim[0]) * (spot_dim[3] - spot_dim[1]))
                if (area > (area_spot * 0.4) and area > max_area):
                    max_area = area
                    max_spot = spot_idx

                spot_idx += 1
            
            if max_spot != -1:
                spots_state[max_spot] = 1

        with open(save_path, 'a') as fd:
            for i in range(NUM_PARK_SPOTS):
                spot_name = img_name[:-4] + "_resize_park_spot_" + str(i) + ".jpg"
                new_row = spot_name + "," + str(spots_state[i]) + '\n'
                fd.write(new_row)

        print("Saved labels for the " + str(label_num) + "th image: '" + img_name[:-4] + "'")

if __name__ == "__main__":
    write_labels()