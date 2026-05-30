#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetBiomeCenter(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FVector> GetSpawnPointsInBiome(EBiomeType BiomeType, int32 NumPoints) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void DistributeActorsAcrossBiomes(TArray<AActor*> ActorsToDistribute);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(FVector Location, EBiomeType BiomeType) const;

    // Biome Configuration
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeRadius() const { return BiomeRadius; }

    // Statistics and Validation
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    int32 GetActorCountInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ValidateBiomeDistribution();

protected:
    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System", meta = (AllowPrivateAccess = "true"))
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FVector> BiomeCenters;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, float> BiomeWeights;

private:
    void InitializeBiomeCenters();
    FVector GetRandomPointInBiome(EBiomeType BiomeType) const;
    bool IsValidSpawnLocation(FVector Location) const;
};