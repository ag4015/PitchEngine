
#include "audioUtils.h"
#include "audioData.h"
#include <iostream>
#include <vector>
#include <limits>

void fill_test_values(buffer_data* bf);
my_float timePropagation(buffer_data* bf, uint32_t idx);
my_float freqPropagation(buffer_data* bf, uint32_t idx, my_float b_s, const std::string& direction);
void propagate_phase_test(buffer_data* bf, my_float b_s, my_float* expectedPhase);
bool nearly_equal(double a, double b, int factor /* a factor of epsilon */);

int main()
{
	my_float steps = 12;
    constexpr uint32_t buflen = 4;
    uint32_t numSamp = 0;
    my_float* in_audio = nullptr;
	buffer_data_t bf, testing_bf;
	audio_data_t audat, testing_audat;
    my_float expectedPhase[buflen];
    uint32_t idx;

    init_variables(&bf, &audat, numSamp, in_audio, steps, buflen);
    init_variables(&testing_bf, &testing_audat, numSamp, in_audio, steps, buflen);

    my_float b_a = 1;
    my_float b_s = bf.shift * b_a;
    my_float abstol = 1e-6;
    fill_test_values(&bf);
    fill_test_values(&testing_bf);

	propagate_phase(&bf, &audat, b_s, abstol);

    propagate_phase_test(&testing_bf, b_s, expectedPhase);
    bool failed = false;

    for (uint32_t i = 0; i < buflen; i++)
    {
        int factor = 100;
        std::cout << "Index: " << i << ", returned phase: " << bf.phi_s[i]
            << ", expected phase: " << expectedPhase[i] << std::endl;
        if (!nearly_equal(bf.phi_s[i], expectedPhase[i], factor))
        {
            failed = true;
            std::cout << "Failed, not nearly equal by a factor of " << factor << " epsilons." << std::endl;
        }
    }
    if (!failed)
    {
        std::cout << "Test successful" << std::endl;
    }

	free_audio_data(&audat);
	free_audio_data(&testing_audat);
	free_buffer_data(&bf);
	free_buffer_data(&testing_bf);

    return 0;
}

void fill_test_values(buffer_data* bf)
{
    std::vector<my_float*> v = { bf->magPrev, bf->mag, bf->phi_s, bf->phi_sPrev, bf->delta_t, bf->delta_tPrev, bf->delta_f };
    std::vector<std::vector<my_float> > list =
    {
        // Higher index = Higher frequency bin
        {10, 25, 30, 10}, // magPrev
        { 1, 10, 20, 30}, // mag
        { 0,  0,  0,  0}, // phi_s
        { 5,  6,  7,  8}, // phi_sPrev
        { 9,  8,  7,  6}, // delta_t
        { 5,  4,  3,  2}, // delta_tPrev
        { 4,  5,  6,  7}  // delta_f
    };
    for (int i = 0; i < v.size(); i++)
    {
        for (int k = 0; k < bf->buflen; k++)
        {
            v[i][k] = list[i][k];
        }
    }
}

my_float timePropagation(buffer_data* bf, uint32_t idx)
{
	bf->phi_s[idx] = bf->phi_sPrev[idx] + (bf->hopS/2)*(bf->delta_tPrev[idx] + bf->delta_t[idx]); // STEP 10
    return bf->phi_s[idx];
}
my_float freqPropagation(buffer_data* bf, uint32_t idx, my_float b_s, const std::string& direction)
{
    if (direction == "up")
    {
        bf->phi_s[idx] = bf->phi_s[idx - 1] + (b_s / 2) * (bf->delta_f[idx - 1] + bf->delta_f[idx]); // STEP 17
    }
    else if (direction == "down")
    {
		bf->phi_s[idx] = bf->phi_s[idx + 1] - (b_s/2) * (bf->delta_f[idx + 1] + bf->delta_f[idx]); // STEP 22
    }
    else
    {
        std::cout << "Direction not valid" << std::endl;
        exit(EXIT_FAILURE);
    }
    return bf->phi_s[idx];

}

void propagate_phase_test(buffer_data* bf, my_float b_s, my_float* expectedPhase)
{
    uint32_t idx = 2;
    expectedPhase[idx] = timePropagation(bf, idx);
    idx = 1;
    expectedPhase[idx] = timePropagation(bf, idx);
    idx = 3;
    expectedPhase[idx] = freqPropagation(bf, idx, b_s, "up");
    idx = 0;
    expectedPhase[idx] = freqPropagation(bf, idx, b_s, "down");
}

bool nearly_equal(double a, double b, int factor /* a factor of epsilon */)
{
    double min_a = a - (a - std::nextafter(a, std::numeric_limits<double>::lowest())) * factor;
    double max_a = a + (std::nextafter(a, std::numeric_limits<double>::max()) - a) * factor;

    return min_a <= b && max_a >= b;
}

