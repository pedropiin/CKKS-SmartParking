from PIL import Image
from os import listdir

def resize_uncropped():
    dataset_path = "../../dataset/uncropped/"
    save_folder_path = "../../dataset/uncropped-resize/"
    img_list = listdir(dataset_path)
    for img_num, img_name in enumerate(img_list):
        img_path = dataset_path + img_name
        try:
            img_it = Image.open(img_path)
        except FileNotFoundError:
            print("Could not open file '" + img_name + "'.")
            break
            
        new_path = save_folder_path + img_name[:-4] + "_resize.jpg"
        img_rsz = img_it.resize((640, 640))
        img_rsz.save(new_path)


def resize_spots():
    dataset_path = "../../dataset/external-dataset/"
    save_path = "../../dataset/external-resized/"

    img_list = listdir(dataset_path)
    for img_num, img_name in enumerate(img_list):
        img_path = dataset_path + img_name
        try:
            img_it = Image.open(img_path)
        except FileNotFoundError:
            print("Could not open file '" + img_name + "'.")
            break
    
        new_path = save_path + img_name
        img_rsz= img_it.resize((40, 40))
        img_rsz.save(new_path)

        print("Resized and saved " + str(img_num) + "th image: " + img_name)

if __name__ == "__main__":
    resize_spots()  