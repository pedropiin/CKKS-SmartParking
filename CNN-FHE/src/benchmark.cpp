#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "openfhe.h"
#include "weights.hpp"
#include "fhe_config.hpp"
#include "utils.hpp"

int main(int argc, char *argv[]) {
    FHEConfig *fhe_config = new FHEConfig();

    // --- SETTING PARAMETERS ---
    fhe_config->config_params();
    std::cout << "--- Parameter configuration complete ---" << std::endl;


    // --- GENERATING CONTEXT ---
    fhe_config->generate_context();
    std::cout << "--- Context generation complete ---" << std::endl;

    // --- ENABLING FEATURES ---
    fhe_config->get_context()->Enable(PKE);
    fhe_config->get_context()->Enable(KEYSWITCH);
    fhe_config->get_context()->Enable(LEVELEDSHE);
    fhe_config->get_context()->Enable(ADVANCEDSHE);
    fhe_config->get_context()->Enable(FHE);
    std::cout << "--- Feature enabling complete ---" << std::endl;

    // --- GENERATING KEYS ---
    fhe_config->generate_key_pair();
    std::cout << "--- Multiplication key generation complete ---" << std::endl;

    std::cout << "--- Using Ring Dimension == " << fhe_config->get_context()->GetRingDimension() << " ---" << std::endl;

    // --- GENERATING ROTATION KEYS ---
    std::vector<int> rotations;
    for (int i = 0; i < 13; i++) rotations.push_back(1 << i);
    rotations.push_back(-1);
    fhe_config->get_context()->EvalRotateKeyGen(fhe_config->get_key_pair().secretKey, rotations);
    
    // --- GENERATING MASKING VECTORS ---
    std::vector<double> clear_mask_vec(8192, 0.0);
    clear_mask_vec[0] = 1.0;
    std::vector<double> pre_sig_vec(8192, 0.0);
    Ciphertext<DCRTPoly> ctxt_mask_vec = fhe_config->encrypt(clear_mask_vec);
    Ciphertext<DCRTPoly> ctxt_pre_sig_vec = fhe_config->encrypt(pre_sig_vec);

    // TESTING ACCURACY 
    std::string dataset_path = "../../dataset/external-resized/";
    std::vector<fs::directory_entry> entries;
    for (const auto & entry : fs::directory_iterator(dataset_path)) {
        if (entry.is_regular_file()) {
            entries.push_back(entry);
        }
    }
    std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
        return a.path().filename() < b.path().filename();
    });

    int count = 0;
    for (const auto & entry : entries) {
        if (count > 500) break;
        std::string img_path = entry.path();
        std::vector<double> img_data = read_image(img_path.c_str());

        Ciphertext<DCRTPoly> ctxt_img = fhe_config->encrypt(img_data);
        Plaintext ptxt_weights = fhe_config->get_context()->MakeCKKSPackedPlaintext(weights);
        Ciphertext<DCRTPoly> ctxt_mult = fhe_config->get_context()->EvalMult(ctxt_img, ptxt_weights);
        Ciphertext<DCRTPoly> rotation;
        Ciphertext<DCRTPoly> summation_layer = ctxt_mult;
        for (int i = 0; i < 13; i++) {
            rotation = fhe_config->get_context()->EvalRotate(summation_layer, 1 << i);
            summation_layer = fhe_config->get_context()->EvalAdd(summation_layer, rotation);
        }
        Ciphertext<DCRTPoly> ctxt_ith_pre_sig = fhe_config->get_context()->EvalAdd(summation_layer, bias);
        const double lower_bound = -70;
        const double upper_bound = 70;
        Ciphertext<DCRTPoly> ctxt_prediction = fhe_config->get_context()->EvalLogistic(ctxt_ith_pre_sig, lower_bound, upper_bound, 1);
        std::vector<double> prediction = fhe_config->decrypt(ctxt_prediction);
        std::string ans1 = "PREDICTION FOR IMAGE " + img_path + " => ";
        if (prediction[0] > 0.5) {
            ans1 += "1";
        } else {
            ans1 += "0";
        }
        std::cout << ans1 << std::endl;
        count++;
    }

    return 0;
}