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
    for (int i = 0; i < 13; i++) rotations.push_back(1 << i); // summation rotations
    rotations.push_back(-1);                                  // sigmoid rotations
    for (int i = 0; i < NUM_PARK_SPOTS; i++) {
        rotations.push_back(-multipacking_idxs[i][0]);        // multipacking rotations
        rotations.push_back(BATCH_SIZE - multipacking_idxs[i][1]); // split multipacking rotations
        rotations.push_back(multipacking_idxs[i][0] - BATCH_SIZE);
    }
    fhe_config->get_context()->EvalRotateKeyGen(fhe_config->get_key_pair().secretKey, rotations);

    // --- PRINTING MODULI CHAIN ---
    // const std::vector<DCRTPoly>& ckks_pk = fhe_config->get_key_pair().publicKey->GetPublicElements();
    // std::cout << "moduli chain of pk" << std::endl;
    // print_moduli_chain(ckks_pk[0]);
    
    // --- GENERATING MASKING VECTORS ---
    std::vector<double> clear_mask_vec(BATCH_SIZE, 0.0);
    clear_mask_vec[0] = 1.0;
    std::vector<double> pre_sig_vec(BATCH_SIZE, 0.0);
    Ciphertext<DCRTPoly> ctxt_mask_vec = fhe_config->encrypt(clear_mask_vec);
    Ciphertext<DCRTPoly> ctxt_pre_sig_vec = fhe_config->encrypt(pre_sig_vec);

    // --- VARS FOR MULT PACKING ---
    std::vector<Ciphertext<DCRTPoly>> multipack_vector;
    multipack_vector.push_back(fhe_config->encrypt(std::vector<double>(BATCH_SIZE, 0.0)));
    std::vector<Ciphertext<DCRTPoly>> unpacking_vector(NUM_PARK_SPOTS);
    int cryptogram_idx = 0;

    for (int i = 0; i < NUM_PARK_SPOTS; i++) {
        std::cout << std::endl;
        std::cout << "--- Beginning encryption of " << i << "th image ---" << std::endl;

        std::string dataset_path = "../../dataset/all-spots-one-image/";
        std::string img_test_path = dataset_path + "original_1653085977858_resize_park_spot_" + std::to_string(i) + ".jpg";
        std::vector<double> img_data = read_image(img_test_path.c_str());
        std::cout << "--- Image reading complete ---" << std::endl;

        // --- IMAGE ENCRYPTION ---
        Ciphertext<DCRTPoly> ctxt_img = fhe_config->encrypt(img_data);
        std::cout << "--- Image encryption complete ---" << std::endl;

        // --- MULTIPACKING IMAGE ---
        multipacking(i, ctxt_img, multipack_vector, cryptogram_idx, fhe_config);
        std::cout << "--- Image multipacking complete ---" << std::endl;
    }
    unpacking(multipack_vector, unpacking_vector, fhe_config);
    std::cout << "--- Unpacking complete ---" << std::endl;

    std::cout << std::endl << "--- Beginning Neural Network evaluation ---\n" << std::endl;
    // --- READING IMAGE ---
    for (int i = 0; i < NUM_PARK_SPOTS; i++) {
        // --- NETWORK EVALUATION ---
        Plaintext ptxt_weights = fhe_config->get_context()->MakeCKKSPackedPlaintext(weights);
        Ciphertext<DCRTPoly> ctxt_mult = fhe_config->get_context()->EvalMult(unpacking_vector[i], ptxt_weights);
        // Ciphertext<DCRTPoly> ctxt_mult = fhe_config->get_context()->EvalMult(ctxt_img, ptxt_weights);
        std::cout << "--- Image * Weights complete ---" << std::endl;
        
        Ciphertext<DCRTPoly> rotation;
        Ciphertext<DCRTPoly> summation_layer = ctxt_mult;
        for (int i = 0; i < 13; i++) {
            rotation = fhe_config->get_context()->EvalRotate(summation_layer, 1 << i);
            summation_layer = fhe_config->get_context()->EvalAdd(summation_layer, rotation);
        }
        std::cout << "--- Summation complete ---" << std::endl;

        Ciphertext<DCRTPoly> ctxt_ith_pre_sig = fhe_config->get_context()->EvalAdd(summation_layer, bias);
        std::cout << "--- Bias addition complete ---" << std::endl;

        Ciphertext<DCRTPoly> ctxt_temp = fhe_config->get_context()->EvalMult(ctxt_mask_vec, ctxt_ith_pre_sig);

        ctxt_pre_sig_vec = fhe_config->get_context()->EvalAdd(ctxt_pre_sig_vec, ctxt_temp);
        ctxt_mask_vec = fhe_config->get_context()->EvalRotate(ctxt_mask_vec, -1);
        std::cout << std::endl;
    }

    // --- EVALUATING SIGMOID THROUGH CHEBYSHEV APPROXIMATION ---
    const double lower_bound = -70;
    const double upper_bound = 70;
    Ciphertext<DCRTPoly> ctxt_prediction = fhe_config->get_context()->EvalLogistic(ctxt_pre_sig_vec, lower_bound, upper_bound, 70);
    std::cout << "--- Sigmoid evaluation complete ---" << std::endl;

    std::vector<double> prediction = fhe_config->decrypt(ctxt_prediction);
    std::cout << "--- Prediction decryption complete ---" << std::endl;

    for (int i = 0; i < NUM_PARK_SPOTS; i++) {
        std::cout << "PREDICTION FOR " << i << "th PARKING SPOT = " << prediction[i] << std::endl;
    }
    return 0;
}