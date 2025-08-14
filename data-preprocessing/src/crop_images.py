from PIL import Image
from os import listdir

# """
# Array of parking spot positions in the cropped original images
# """
# arr_spots_crop_big = [(326, 249, 381, 321),     # 1th parking spot
#                     (376, 270, 436, 325),       # 2th parking spot
#                     (432, 267, 489, 327),       # 3th parking spot
#                     (490, 273, 540, 337),       # 4th parking spot
#                     (535, 289, 586, 349),       # 5th parking spot
#                     (588, 299, 638, 358),       # 6th parking spot
#                     (620, 310, 670, 370),       # 7th parking spot
#                     (660, 331, 708, 380),       # 8th parking spot
#                     (17, 360, 123, 512),        # 9th parking spot
#                     (129, 370, 236, 545),       # 10th parking spot
#                     (445, 429, 562, 602),       # 11th parking spot
#                     (550, 443, 655, 599),       # 12th parking spot
#                     (637, 458, 739, 596),       # 13th parking spot
#                     (703, 479, 775, 587),       # 14th parking spot
#                     (750, 492, 829, 583),       # 15th parking spot
#                     (779, 480, 857, 575)]       # 16th parking spot

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


if __name__ == "__main__":
    dataset_path = "../../dataset/uncropped-resize/"
    save_folder_path = "../../dataset/spots-uncropped/"
    img_list = listdir(dataset_path)
    for img_num, img_name in enumerate(img_list):
        img_path = dataset_path + img_name
        try:
            img_it = Image.open(img_path)
        except FileNotFoundError:
            print("Could not open file '" + img_name + "'.")
            break
        for i, box in enumerate(arr_spots_uncrop_small):
            cropped_section = img_it.crop(box)
            new_path = save_folder_path + img_name[:-4] + "_park_spot_" + str(i) + ".jpg"
            cropped_section.save(new_path)
        print("Finished cropping image ", img_num)