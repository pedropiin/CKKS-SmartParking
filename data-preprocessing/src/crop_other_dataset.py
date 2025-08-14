from PIL import Image

arr_crop_image_1 = [(1501, 637, 1650, 727),
(1451, 1138, 1731, 1524),
(966, 940, 1172, 1210),
(2444, 852, 2613, 1081)]

def crop_image(img_path, save_path):
    try:
        img_it = Image.open(img_path)
    except FileNotFoundError:
        print("Could not open image at '" + img_path + "'.")
        return
    
    for i, box in enumerate(arr_crop_image_1):
        cropped_section = img_it.crop(box)
        new_path = save_path + "crop" + str(i + 58) + ".jpg"
        cropped_section.save(new_path)

    print("Finished cropping")

if __name__ == "__main__":
    img_path = "../../dataset/external-tobe-cropped/GOPR6631.JPG"
    save_folder = "../../dataset/external-dataset/"
    
    crop_image(img_path, save_folder)


