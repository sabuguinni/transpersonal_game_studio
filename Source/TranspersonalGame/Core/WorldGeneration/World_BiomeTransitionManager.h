#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "World_BiomeTransitionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTransitionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    EBiomeType FromBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    EBiomeType ToBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    float TransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    float BlendStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Transition")
    TArray<FVector> TransitionPoints;

    FWorld_BiomeTransitionData()
    {
        FromBiome = EBiomeType::Forest;
        ToBiome = EBiomeType::Swamp;
        TransitionDistance = 500.0f;
        BlendStrength = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeInfluenceZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Influence")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Influence")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Influence")
    float InfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Influence")
    float InfluenceStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Influence")
    bool bIsActive;

    FWorld_BiomeInfluenceZone()
    {
        BiomeType = EBiomeType::Forest;
        CenterLocation = FVector::ZeroVector;
        InfluenceRadius = 1000.0f;
        InfluenceStrength = 1.0f;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeTransitionManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeTransitionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Biome transition management
    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    void InitializeBiomeTransitions();

    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    float GetBiomeInfluenceAtLocation(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    TArray<EBiomeType> GetNearbyBiomes(const FVector& Location, float SearchRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    bool IsInTransitionZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Transition")
    FWorld_BiomeTransitionData GetTransitionData(const FVector& Location) const;

    // Biome influence zone management
    UFUNCTION(BlueprintCallable, Category = "Biome Influence")
    void AddBiomeInfluenceZone(const FWorld_BiomeInfluenceZone& InfluenceZone);

    UFUNCTION(BlueprintCallable, Category = "Biome Influence")
    void RemoveBiomeInfluenceZone(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Biome Influence")
    void UpdateBiomeInfluenceZone(int32 ZoneIndex, const FWorld_BiomeInfluenceZone& NewData);

    UFUNCTION(BlueprintCallable, Category = "Biome Influence")
    TArray<FWorld_BiomeInfluenceZone> GetInfluenceZonesAtLocation(const FVector& Location) const;

    // Environmental transition effects
    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void ApplyTransitionEffects(const FVector& Location, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void UpdateVegetationTransition(const FVector& Location, EBiomeType FromBiome, EBiomeType ToBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void UpdateWeatherTransition(const FVector& Location, EBiomeType FromBiome, EBiomeType ToBiome);

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void UpdateAudioTransition(const FVector& Location, EBiomeType FromBiome, EBiomeType ToBiome);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Data")
    TArray<FWorld_BiomeTransitionData> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Data")
    TArray<FWorld_BiomeInfluenceZone> BiomeInfluenceZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Settings")
    float TransitionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Settings")
    float MaxTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition Settings")
    bool bEnableRealTimeTransitions;

private:
    float LastTransitionUpdateTime;

    // Helper functions
    float CalculateDistanceToBiome(const FVector& Location, EBiomeType BiomeType) const;
    FVector GetBiomeCenterLocation(EBiomeType BiomeType) const;
    float CalculateTransitionBlend(const FVector& Location, const FWorld_BiomeTransitionData& TransitionData) const;
    void ProcessTransitionEffects(const FVector& Location, const FWorld_BiomeTransitionData& TransitionData, float DeltaTime);
};