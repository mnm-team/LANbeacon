#ifndef EVALUATELANBEACON_H
#define EVALUATELANBEACON_H

char ** evaluateLANbeacon (unsigned char *LLDPreceivedPayload, ssize_t payloadSize);
void bananaPIprint (char **parsedBeaconContents);

#endif