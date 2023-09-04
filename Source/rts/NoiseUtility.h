#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NoiseUtility.generated.h"

/**
 * Utility class for generating noise
 */
UCLASS()
class RTS_API UNoiseUtility : public UObject
{
    GENERATED_BODY()

public:
    // Generate Perlin noise based on X and Y coordinates
    UFUNCTION(BlueprintCallable, Category = "Noise")
    static float PerlinNoise(float x, float y);

private:
    // Helper functions and variables for Perlin noise
    static int* GeneratePermutation(); // You can also pre-calculate and hardcode this
};
