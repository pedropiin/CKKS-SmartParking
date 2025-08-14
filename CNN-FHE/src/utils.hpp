#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "openfhe.h"
#include "weights.hpp"
#include "fhe_config.hpp"

#define STBI_FAILURE_USERMSG

#define IMG_W 40
#define IMG_H 40
#define NUM_CHANNELS 3
#define NUM_PARK_SPOTS 16
#define BATCH_SIZE 8192

using namespace lbcrypto;
namespace fs = std::filesystem;

// --- PROTOTYPES ---

void print_moduli_chain(const DCRTPoly& poly);

std::vector<double> read_image(const char *filename);

Ciphertext<DCRTPoly> masking_for_multipacking(int len, Ciphertext<DCRTPoly> &ctxt_image, FHEConfig *fhe_config);

Ciphertext<DCRTPoly> masking_middle(int start, int end, Ciphertext<DCRTPoly> &ctxt_image, FHEConfig *fhe_config);

void multipacking(int idx_img, Ciphertext<DCRTPoly> &ctxt_img, std::vector<Ciphertext<DCRTPoly>> &multipack_vector, int &cryptogram_idx, FHEConfig *fhe_config);

void unpacking(std::vector<Ciphertext<DCRTPoly>> &multipack_vector, std::vector<Ciphertext<DCRTPoly>> &images, FHEConfig *fhe_config);


// --- METHOD DEFINITIONS ---

void print_moduli_chain(const DCRTPoly& poly){
    int num_primes = poly.GetNumOfElements();
    double total_bit_len = 0.0;
    for (int i = 0; i < num_primes; i++) {
        auto qi = poly.GetParams()->GetParams()[i]->GetModulus();
        std::cout << "q_" << i << ": " 
                    << qi
                    << ",  log q_" << i <<": " << log(qi.ConvertToDouble()) / log(2)
                    << std::endl;
        total_bit_len += log(qi.ConvertToDouble()) / log(2);
    }   
    std::cout << "Total bit length: " << total_bit_len << std::endl;
}

std::vector<double> read_image(const char *filename) {
    int img_width = IMG_W;
    int img_height = IMG_H;
    int img_channels = 3;

    unsigned char *img_data = stbi_load(filename, &img_width, &img_height, &img_channels, 3);

    if (!img_data) {
        std::cerr << "Could not read image '" << filename << "'." << std::endl;
        stbi_failure_reason();
        return std::vector<double>();
    }

    size_t img_size = img_width * img_height;
    std::vector<double> img_values(img_size * img_channels);
    for (size_t i = 0; i < img_size; i++) {
        img_values[3 * i] = static_cast<double>(img_data[3 * i]) / 255.0f;
        img_values[(3 * i) + 1] = static_cast<double>(img_data[(3 * i) + 1]) / 255.0f;
        img_values[(3 * i) + 2] = static_cast<double>(img_data[(3 * i) + 2]) / 255.0f;
    }

    stbi_image_free(img_data);

    return img_values;
}

Ciphertext<DCRTPoly> masking_for_multipacking(
    int len, 
    Ciphertext<DCRTPoly> &ctxt_image,
    FHEConfig *fhe_config
) {
    std::vector<double> mask_vector(BATCH_SIZE, 0.0);
    for (int i = 0; i < len; i++) mask_vector[i] = 1.0;
    Ciphertext<DCRTPoly> ctxt_mask = fhe_config->encrypt(mask_vector);
    Ciphertext<DCRTPoly> result = fhe_config->get_context()->EvalMult(ctxt_image, ctxt_mask);
    return result;
}

Ciphertext<DCRTPoly> masking_middle(
    int start,
    int end,
    Ciphertext<DCRTPoly> &ctxt_image,
    FHEConfig *fhe_config
) {
    std::vector<double> mask_vector(BATCH_SIZE, 0.0);
    for (int i = start; i <= end; i++) mask_vector[i] = 1.0;
    Ciphertext<DCRTPoly> ctxt_mask = fhe_config->encrypt(mask_vector);
    Ciphertext<DCRTPoly> result = fhe_config->get_context()->EvalMult(ctxt_image, ctxt_mask);
    return result;
}

void multipacking(
    int idx_img,
    Ciphertext<DCRTPoly> &ctxt_img, 
    std::vector<Ciphertext<DCRTPoly>> &multipack_vector,
    int &cryptogram_idx,
    FHEConfig *fhe_config
) {
    if (multipacking_idxs[idx_img][1] < multipacking_idxs[idx_img][0]) {
        // first part
        size_t len_part1 = BATCH_SIZE - multipacking_idxs[idx_img][0];
        Ciphertext<DCRTPoly> part1 = masking_for_multipacking(len_part1, ctxt_img, fhe_config);
        part1 = fhe_config->get_context()->EvalRotate(part1, -multipacking_idxs[idx_img][0]);
        multipack_vector[cryptogram_idx] = fhe_config->get_context()->EvalAdd(multipack_vector[cryptogram_idx], part1);

        // second part
        size_t len_part2 = 1 + multipacking_idxs[idx_img][1];
        Ciphertext<DCRTPoly> rotated_img = fhe_config->get_context()->EvalRotate(ctxt_img, len_part1);
        Ciphertext<DCRTPoly> part2 = masking_for_multipacking(len_part2, rotated_img, fhe_config);
        multipack_vector.push_back(part2);
        cryptogram_idx++;
    } else {
        Ciphertext<DCRTPoly> rotated_img = fhe_config->get_context()->EvalRotate(ctxt_img, -multipacking_idxs[idx_img][0]);
        multipack_vector[cryptogram_idx] = fhe_config->get_context()->EvalAdd(multipack_vector[cryptogram_idx], rotated_img);
    }
}

void unpacking(
    std::vector<Ciphertext<DCRTPoly>> &multipack_vector,
    std::vector<Ciphertext<DCRTPoly>> &images,
    FHEConfig *fhe_config
) {
    int cryptogram_idx = 0;
    for (int idx_img = 0; idx_img < NUM_PARK_SPOTS; idx_img++) {
        if (multipacking_idxs[idx_img][1] < multipacking_idxs[idx_img][0]) { 
            Ciphertext<DCRTPoly> part1 = fhe_config->get_context()->EvalRotate(multipack_vector[cryptogram_idx], multipacking_idxs[idx_img][0]);
            part1 = masking_for_multipacking(BATCH_SIZE - multipacking_idxs[idx_img][0], part1, fhe_config);
            images[idx_img] = part1;
            cryptogram_idx++;

            Ciphertext<DCRTPoly> part2 = masking_for_multipacking(multipacking_idxs[idx_img][1] + 1, multipack_vector[cryptogram_idx], fhe_config);
            part2 = fhe_config->get_context()->EvalRotate(part2, multipacking_idxs[idx_img][0] - BATCH_SIZE);
            images[idx_img] = fhe_config->get_context()->EvalAdd(images[idx_img], part2);
        } else {
            Ciphertext<DCRTPoly> img = masking_middle(multipacking_idxs[idx_img][0], multipacking_idxs[idx_img][1], multipack_vector[cryptogram_idx], fhe_config);
            img = fhe_config->get_context()->EvalRotate(img, multipacking_idxs[idx_img][0]);
            images[idx_img] = img;
        }
    }
}