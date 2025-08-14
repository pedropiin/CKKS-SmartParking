#pragma once

#include <vector>
#include <string>

#include "openfhe.h"
#include "weights.hpp"

#define IMG_W 40
#define IMG_H 40
#define NUM_CHANNELS 3
#define NUM_PARK_SPOTS 16

using namespace lbcrypto;

class FHEConfig {
    private:
        CCParams<CryptoContextCKKSRNS> parameters;
        CryptoContext<DCRTPoly> context;
        KeyPair<DCRTPoly> key_pair;

    public:
        void config_params() {
            ScalingTechnique scale_tech = FLEXIBLEAUTO;     // recommended value by OpenFHE
            this->parameters.SetScalingTechnique(scale_tech);

            uint32_t scale_mod_size = 32;       // recommended value by OpenFHE
            this->parameters.SetScalingModSize(scale_mod_size);

            uint32_t first_mod_size = 33;       // recommended value by OpenFHE
            this->parameters.SetFirstModSize(first_mod_size);

            // Batch size
            // We have 40x40x3 input images => 4800 values. This way, 
            // 1 << 12 = 4096 would not be sufficient for a single image.
            uint32_t num_slots = 1 << 13;
            this->parameters.SetBatchSize(num_slots);

            //
            uint32_t ring_dim = 1 << 14;
            this->parameters.SetRingDim(ring_dim);

            // Secret key distribuition
            // Recommended value = UNIFORM_TERNARY  
            SecretKeyDist secret_key_dist = UNIFORM_TERNARY;
            this->parameters.SetSecretKeyDist(secret_key_dist);

            // Security level
            // "NotSet" useful for non-production environments, but needs to be changed later on
            SecurityLevel sec_level = HEStd_NotSet;
            // SecurityLevel sec_level = HEStd_128_classic;
            this->parameters.SetSecurityLevel(sec_level);

            uint32_t multDepth = 12;  // Read https://github.com/openfheorg/openfhe-development/blob/main/src/pke/examples/FUNCTION_EVALUATION.md for more info
            this->parameters.SetMultiplicativeDepth(multDepth);
        }


        void generate_context() {
            CryptoContext<DCRTPoly> context = GenCryptoContext(this->parameters);
            this->context = context;
        }


        void generate_key_pair() {
            KeyPair<DCRTPoly> key_pair = this->context->KeyGen();
            this->context->EvalMultKeyGen(key_pair.secretKey);
            this->key_pair = key_pair;
        }

        Ciphertext<DCRTPoly> encrypt(const std::vector<double> &img) {
            Plaintext ptxt = this->context->MakeCKKSPackedPlaintext(img);
            Ciphertext<DCRTPoly> ctxt = this->context->Encrypt(ptxt, this->key_pair.publicKey);
            return ctxt;
        }

        std::vector<double> decrypt(const Ciphertext<DCRTPoly> &ctxt) {
            Plaintext ptxt;
            this->context->Decrypt(this->key_pair.secretKey, ctxt, &ptxt);
            std::vector<double> prediction = ptxt->GetRealPackedValue();
            return prediction;
        }


        CCParams<CryptoContextCKKSRNS> get_parameters(void) {
            return this->parameters;
        }

        CryptoContext<DCRTPoly> get_context(void) {
            return this->context;
        }

        KeyPair<DCRTPoly> get_key_pair(void) {
            return this->key_pair;
        }
};