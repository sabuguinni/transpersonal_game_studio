#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousEnvironmentSpawner.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Spawning")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Spawning")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Spawning")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Spawning")
    int32 PropDensity;

    FEnvArt_BiomeSpawnData()
    {
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        BiomeType = EBiomeType::Forest;
        PropDensity = 10;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    FVector ScaleRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    float SpawnWeight;

    FEnvArt_CretaceousPropData()
    {
        PropName = TEXT("DefaultProp");
        ScaleRange = FVector(0.8f, 1.2f, 1.0f);
        SpawnWeight = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousEnvironmentSpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousEnvironmentSpawner();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Spawning")
    TArray<FEnvArt_BiomeSpawnData> BiomeSpawnPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Spawning")
    TArray<FEnvArt_CretaceousPropData> CretaceousProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Spawning")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Spawning")
    int32 MaxPropsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    bool bEnableAtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    FLinearColor FogColor;

public:
    UFUNCTION(BlueprintCallable, Category = "Environment Spawning")
    void SpawnCretaceousPropsAtBiome(const FEnvArt_BiomeSpawnData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Environment Spawning")
    void SpawnAllBiomeProps();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void CreateAtmosphericFog(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Environment Spawning")
    AActor* SpawnPropAtLocation(const FEnvArt_CretaceousPropData& PropData, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment Spawning")
    void SpawnBiomePropsEditor();

private:
    UPROPERTY()
    TArray<AActor*> SpawnedProps;

    UPROPERTY()
    AExponentialHeightFog* AtmosphericFogActor;

    void InitializeDefaultProps();
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius) const;
    FRotator GetRandomRotation() const;
};