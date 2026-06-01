#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "../SharedTypes.h"
#include "EnvArt_CretaceousRockSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_RockVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Variation")
    FString RockName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Variation")
    FVector MinScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Variation")
    FVector MaxScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Variation")
    float SpawnWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Variation")
    bool bCanCluster;

    FEnvArt_RockVariation()
    {
        RockName = TEXT("DefaultRock");
        MinScale = FVector(0.8f, 0.8f, 0.8f);
        MaxScale = FVector(1.2f, 1.2f, 1.2f);
        SpawnWeight = 1.0f;
        bCanCluster = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_RockCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Cluster")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Cluster")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Cluster")
    int32 RockCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock Cluster")
    EBiomeType BiomeType;

    FEnvArt_RockCluster()
    {
        CenterLocation = FVector::ZeroVector;
        ClusterRadius = 500.0f;
        RockCount = 5;
        BiomeType = EBiomeType::Temperate;
    }
};

UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_CretaceousRockSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_CretaceousRockSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock System")
    TArray<FEnvArt_RockVariation> RockVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock System")
    TArray<FEnvArt_RockCluster> ActiveClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock System")
    float MaxRocksPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock System")
    float MinDistanceBetweenRocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rock System")
    bool bAutoGenerateOnBeginPlay;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Rock System")
    void GenerateRocksForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Rock System")
    void CreateRockCluster(const FVector& ClusterCenter, float Radius, int32 RockCount, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Rock System")
    AStaticMeshActor* SpawnSingleRock(const FVector& Location, const FRotator& Rotation, const FEnvArt_RockVariation& Variation);

    UFUNCTION(BlueprintCallable, Category = "Rock System")
    void ClearAllRocks();

    UFUNCTION(BlueprintCallable, Category = "Rock System")
    int32 GetTotalRockCount() const;

    UFUNCTION(BlueprintCallable, Category = "Rock System")
    void ValidateRockDistribution();

private:
    UPROPERTY()
    TArray<AStaticMeshActor*> SpawnedRocks;

    void InitializeRockVariations();
    FEnvArt_RockVariation SelectRandomRockVariation() const;
    bool IsValidRockLocation(const FVector& Location, float MinDistance) const;
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius) const;
    void ApplyBiomeSpecificMaterial(AStaticMeshActor* RockActor, EBiomeType BiomeType);
};