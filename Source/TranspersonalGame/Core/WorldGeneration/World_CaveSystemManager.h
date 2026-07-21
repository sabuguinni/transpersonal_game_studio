#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "World_CaveSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_CaveEntrance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    float EntranceSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    EWorld_BiomeType ConnectedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    bool bHasCrystalFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    bool bHasUndergroundWater;

    FWorld_CaveEntrance()
    {
        Location = FVector::ZeroVector;
        EntranceSize = 400.0f;
        ConnectedBiome = EWorld_BiomeType::Forest;
        bHasCrystalFormations = true;
        bHasUndergroundWater = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_CrystalFormation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Crystals")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Crystals")
    FLinearColor LightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Crystals")
    float LightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Crystals")
    float CrystalScale;

    FWorld_CrystalFormation()
    {
        Location = FVector::ZeroVector;
        LightColor = FLinearColor(0.7f, 0.9f, 1.0f, 1.0f);
        LightIntensity = 1200.0f;
        CrystalScale = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CaveSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CaveSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Cave system configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<FWorld_CaveEntrance> CaveEntrances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<FWorld_CrystalFormation> CrystalFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    int32 MaxCaveEntrances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    float MinDistanceBetweenCaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    float CaveDepthRange;

    // Lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float AmbientLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor AmbientLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float CrystalLightRadius;

    // Atmospheric effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    float FogHeightFalloff;

    // Runtime cave actors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Runtime")
    TArray<AActor*> SpawnedCaveEntrances;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Runtime")
    TArray<APointLight*> SpawnedCaveLights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Runtime")
    TArray<AExponentialHeightFog*> SpawnedCaveFog;

    // Cave system methods
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void GenerateCaveSystem();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void CreateCaveEntrance(const FWorld_CaveEntrance& CaveData);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void CreateCrystalFormation(const FWorld_CrystalFormation& CrystalData);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void CreateUndergroundWater(const FVector& Location, float WaterSize);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void CreateCaveTunnel(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void AddCaveAtmosphere(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void ClearCaveSystem();

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool IsValidCaveLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    FVector GetNearestCaveEntrance(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    float GetCaveAmbientLightLevel(const FVector& Location) const;

private:
    void InitializeCaveSystem();
    void GenerateDefaultCaveLayout();
    AActor* SpawnCaveEntranceActor(const FVector& Location, float Size);
    APointLight* SpawnCaveLight(const FVector& Location, const FLinearColor& Color, float Intensity);
    AExponentialHeightFog* SpawnCaveFog(const FVector& Location);
};