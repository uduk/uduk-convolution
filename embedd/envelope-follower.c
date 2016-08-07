double *
envelopeFollower (double *originalSignal, long originalLen) 
{
  int sampleRate = 44100;
  double attackTime =  0.0002;
  double releaseTime =  0.0004;

  float gainAttack = (float) exp(-1.0 / (sampleRate*attackTime));
  float gainRelease = (float) exp(-1.0 / (sampleRate*releaseTime));
  float envelopeOut = 0.0f;

  double *envelopeBuffer = (double *) calloc (originalLen, sizeof (double));

  #pragma omp for schedule(dynamic, CHUNKSIZE)
  for (int i = 0; i < originalLen; i++) {

    float envelopeIn = fabs(originalSignal[i]);

    if (envelopeOut < envelopeIn) {
      envelopeOut = envelopeIn + gainAttack * (envelopeOut - envelopeIn);
    } else {
      envelopeOut = envelopeIn + gainRelease * (envelopeOut - envelopeIn);
    }   

    envelopeBuffer[i] = envelopeOut;
  }

  return envelopeBuffer;

}
