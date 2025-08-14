import save_label

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


def check_label_by_center():
    img_name = "original_1653085977858"
    dataset_path = "../../dataset/test-dataset/"
    label_path = dataset_path + img_name + ".txt"

    spots_state = [0] * NUM_PARK_SPOTS

    labels = open(label_path, 'r')
    for line in labels:
        content = line.split()
        center_x = int(float(content[1]) * IMG_SIZE)
        center_y = int(float(content[2]) * IMG_SIZE)

        print("center => (", center_x, ", ", center_y, ")")

        for i in range(NUM_PARK_SPOTS):
            spot_dimensions = arr_spots_uncrop_small[i]
            if (center_x >= spot_dimensions[0] and center_x <= spot_dimensions[2] and center_y >= spot_dimensions[1] and center_y <= spot_dimensions[3]):
                spots_state[i] = 1

    for i in range(len(spots_state)):
        print("Spot " + str(i) + " is with state => " + str(spots_state[i]))


def check_label_by_area():
    # FALSE POSITIVES: 1 / FALSE NEGATIVES: 1 => ACCURACY: 0.9875
    # only problem related to spot number 16 in image "original_1653424120150"
    # images = ["original_1653085977858", "original_1653095607531", "original_1653435309328", "original_1653444879248", "original_1654288506194", "original_1653424120150", "original_1653427329959", "original_1653435010507", "original_1653442628326", "original_1654278637047"]

    images = ["original_1654898367208", "original_1654893898164", "original_1654890508031", "original_1654656528342", "original_1654636007070", "original_1654880727018", "original_1654646568768", "original_1654635318077", "original_1654633308801", "original_1654305576635", "original_1653439150829", "original_1653424418739", "original_1653418328639", "original_1654885735900", "original_1654636547930"]

    ans = []
    img_count = 0
    for img_name in images:
        dataset_path = "../../dataset/test-dataset-2/"
        label_path = dataset_path + img_name + ".txt"

        spots_state = [0] * NUM_PARK_SPOTS

        labels = open(label_path, 'r')
        for line in labels:
            content = line.split()

            center_x = int(float(content[1]) * IMG_SIZE)
            center_y = int(float(content[2]) * IMG_SIZE)
            width = int(float(content[3]) * IMG_SIZE)
            height = int(float(content[4]) * IMG_SIZE)

            rbb_ul = tuple(((center_x - (width / 2)), (center_y - (height / 2))))
            rbb_br = tuple(((center_x + (width / 2)), (center_y + (height / 2))))

            max_area = 0
            area = 0
            max_spot = -1
            it_count = 0
            for spot_dim in arr_spots_uncrop_small:
                ul_x = max(rbb_ul[0], spot_dim[0])
                ul_y = max(rbb_ul[1], spot_dim[1])
                br_x = min(rbb_br[0], spot_dim[2])
                br_y = min(rbb_br[1], spot_dim[3])

                if (ul_x < br_x and ul_y < br_y):
                    area = ((br_x - ul_x) * (br_y - ul_y))

                # Calculating area of the parking spot to reduce number of
                # false positives related to cars in the parking lot but not
                # in a parking spot.
                # Accuracy: before => 0.9667 && after 1 (area > 0.2) => 0.979 && after 2 (area > 0.4) => 0.9833
                area_spot = ((spot_dim[2] - spot_dim[0]) * (spot_dim[3] - spot_dim[1]))
                if area > (area_spot * 0.4) and area > max_area:
                    max_area = area
                    max_spot = it_count
                
                it_count += 1

            if max_spot != -1:
                spots_state[max_spot] = 1

        print("STATUS FOR IMAGE", images[img_count])
        for i in range(len(spots_state)):
            print("Spot " + str(i + 1) + " is with state => " + str(spots_state[i]))
        print("")

        save_label.save_labels(spots_state, img_name)

        ans.append(spots_state)
        img_count += 1

    return ans


if __name__ == "__main__":
    check_label_by_area()