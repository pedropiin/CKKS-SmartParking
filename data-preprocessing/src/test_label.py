import check_label

def test_labels():
    """
    FALSE POSITIVES: 1 / FALSE NEGATIVES: 1 => ACCURACY: 0.9875
    only problem related to spot number 16 in image "original_1653424120150"
    images = ["original_1653085977858", "original_1653095607531", "original_1653435309328", "original_1653444879248", "original_1654288506194", "original_1653424120150", "original_1653427329959", "original_1653435010507", "original_1653442628326", "original_1654278637047"]

    true_values = [[0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
                    [0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1],
                    [1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0],
                    [0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0],
                    [0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0],
                    [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0],
                    [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1],]
    """

    """
    FALSE POSITIVES: 6 / FALSE NEGATIVES: 2 => ACCURACY: 0.9667
    problems per spot:
    [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 4]
    1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16

    Most problems are related to spot number 16, probably because of
    the horrible camera angle to it, specially when spot number 15 is
    occupied.
    There is also a problem with images that have cars in the parking 
    lot, but they are not parked (in the process of parking or leaving),
    because if they are near a parking spot, the intersection area will be
    minimal, but > 0.
    """

    images = ["original_1654898367208", "original_1654893898164", "original_1654890508031", "original_1654656528342", "original_1654636007070", "original_1654880727018", "original_1654646568768", "original_1654635318077", "original_1654633308801", "original_1654305576635", "original_1653439150829", "original_1653424418739", "original_1653418328639", "original_1654885735900", "original_1654636547930"]

    true_values = [[0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0],
                    [1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0],
                    [1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0],
                    [1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0],
                    [1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
                    [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0],
                    [1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0],
                    [1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0],
                    [1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0],
                    [1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0]]
    
    label_pred = check_label.check_label_by_area()

    total_correct = 0
    total_false_positive = 0
    total_false_negative = 0

    it_count = 0
    for ans in label_pred:
        print("--- Checking image " + images[it_count] + " ---")

        false_positives = 0
        false_negatives = 0
        for i in range(len(ans)):
            if ((ans[i] == 0 and true_values[it_count][i] == 0) or (ans[i] == 1 and true_values[it_count][i] == 1)):
                continue
            elif (ans[i] == 0 and true_values[it_count][i] == 1):
                false_negatives += 1
            elif (ans[i] == 1 and true_values[it_count][i] == 0):
                false_positives += 1

        correct_checks = 16 - (false_positives + false_negatives)
        print("Number of correct checks:", correct_checks)
        print("Number of false positives:", false_positives)
        print("Number of false negatives:", false_negatives)
        print("Accuracy for this image:", correct_checks / 16)

        print("")

        total_correct += correct_checks
        total_false_positive += false_positives
        total_false_negative += false_negatives

        it_count += 1

    print("Total number of false positives:", total_false_positive)
    print("Total number of false negatives:", total_false_negative)
    print("Final accuracy:", total_correct / 240)

if __name__ == "__main__":
    test_labels()