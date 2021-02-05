
#include "audioUtils.h"
#include <iostream>

#define NUMPOINTS 4


int main()
{
	float steps = 12;
	float shift = pow(2, steps/12);
    float hopA = 256;
    float magPrev[NUMPOINTS]     = {-30, 30, 25,-30};
    float mag[NUMPOINTS]         = {30, 20, 10,-30};
    float phi_s[NUMPOINTS]       = {0, 0, 0, 0};
    float phi_sPrev[NUMPOINTS]   = {5, 6, 7, 8};
    float delta_t[NUMPOINTS]     = {9, 8, 7, 6};
    float delta_tPrev[NUMPOINTS] = {5, 4, 3, 2};
    float delta_f[NUMPOINTS]     = {4, 5, 6, 7};
    propagate_phase(delta_t, delta_tPrev, delta_f, mag, magPrev, phi_s, phi_sPrev, hopA, shift, NUMPOINTS);
    int failed = 0;
    failed += !(phi_s[1] == phi_sPrev[1] + (hopA/2)*(delta_tPrev[1] + delta_t[1]));
    failed += !(phi_s[2] == phi_sPrev[2] + (hopA/2)*(delta_tPrev[2] + delta_t[2]));
    failed += !(phi_s[0] == phi_s[1] - (shift/2) * (delta_f[1] + delta_f[0]));
    std::cout << phi_s[0] << std::endl;
    std::cout << phi_s[1] - (shift/2) * (delta_f[1] + delta_f[0]) << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    
    return 0;
}
