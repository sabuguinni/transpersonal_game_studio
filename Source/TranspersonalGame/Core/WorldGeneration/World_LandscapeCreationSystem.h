#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "SharedTypes.h"
#include "World_LandscapeCreationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LandscapeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountX = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 ComponentCountY = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 QuadsPerComponent = 63;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    int32 SectionsPerComponent = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeScale = FVector(100.0f, 100.0f, 100.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FVector LandscapeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
    FRotator LandscapeRotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HeightVariation = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float NoiseScale = 0.001f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_LandscapeCreationSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_LandscapeCreationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Configuration")
    FWorld_LandscapeConfig LandscapeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    class ALandscapeProxy* GeneratedLandscape;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> BiomeMarkers;

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    bool CreateLandscapeWithBiomes();

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    bool GenerateHeightmapData(TArray<uint16>& HeightData, int32 Width, int32 Height);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateBiomeMarkers();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHeightAtBiome(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Landscape Creation")
    void ApplyBiomeHeightmaps();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugPrintLandscapeInfo() const;

private:
    void InitializeDefaultBiomes();
    float CalculateNoiseHeight(const FVector& Location, float Scale, float Amplitude) const;
    float BlendBiomeHeights(const FVector& Location) const;
};