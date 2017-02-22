#include "ev_real.h"
#include "ev_complex.h"
#include "ev_spectralCoG.h"
#include "ev_efeatures.h"

mx_real_t get_CoG(mx_complex_t *samples, int n_samples) {
    int i;
    mx_real_t sumenergy = 0.0, sumfenergy = 0.0, rate = 1.0*SAMPLERATE/n_samples, result;
 
    for (i = 1; i < n_samples / 2; i ++) {
	mx_real_t re = mx_re(samples[i]), im = mx_im(samples[i]), energy = re * re + im * im;
	mx_real_t f = i * rate; // x1 + ... Frequenz?????
	energy = sqrt (energy);
	sumenergy += energy;
	sumfenergy += f * energy;
    }
    if (!sumenergy) 
	return 0;
    
    result = sumfenergy / sumenergy;
    return result;
}
