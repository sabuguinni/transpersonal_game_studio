#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_VolcanicBiomeGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    EWorld_BiomeType BiomeType;

    FWorld_VolcanicFeature()
    {
        Location = FVector::ZeroVector;
        Intensity = 1.0f;
        Radius = 500.0f;
        BiomeType = EWorld_BiomeType::Volcanic;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_VolcanicBiomeGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_VolcanicBiomeGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Generation")
    TArray<FWorld_VolcanicFeature> VolcanicFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Generation")
    int32 MaxVolcanicRocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Generation")
    float LavaFlowIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Generation")
    float AshDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Generation")
    float TemperatureMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Generation")
    bool bGenerateOnBeginPlay;

public:
    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void GenerateVolcanicTerrain();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void SpawnVolcanicRocks(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void CreateLavaFlows();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void ApplyVolcanicEffects();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    FWorld_VolcanicFeature CreateVolcanicFeature(FVector Location, float Intensity, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void ClearVolcanicTerrain();

    UFUNCTION(BlueprintPure, Category = "Volcanic Generation")
    float GetTemperatureAtLocation(FVector Location) const;

    UFUNCTION(BlueprintPure, Category = "Volcanic Generation")
    bool IsLocationInVolcanicZone(FVector Location) const;

private:
    UPROPERTY()
    TArray<AActor*> SpawnedVolcanicActors;

    void SpawnVolcanicRockAtLocation(FVector Location);
    void ApplyVolcanicMaterial(AActor* Actor);
    float CalculateVolcanicIntensity(FVector Location) const;
};