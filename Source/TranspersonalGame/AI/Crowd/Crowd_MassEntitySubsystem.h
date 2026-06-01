#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntitySubsystem.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 BiomeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    bool bIsActive;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 100.0f;
        BiomeID = 0;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FCrowd_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float EntityDensity;

    FCrowd_BiomeConfig()
    {
        BiomeName = TEXT("Default");
        CenterLocation = FVector::ZeroVector;
        Radius = 15000.0f;
        MaxEntities = 1000;
        EntityDensity = 0.1f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowd_MassEntitySubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_MassEntitySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdEntities(int32 Count, const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetBiomeConfiguration(const TArray<FCrowd_BiomeConfig>& BiomeConfigs);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllCrowdEntities();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void UpdateCrowdLOD(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd LOD")
    void SetLODDistances(float HighLOD, float MediumLOD, float LowLOD);

protected:
    // Entity Storage
    UPROPERTY(BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_EntityData> CrowdEntities;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd Data")
    TArray<FCrowd_BiomeConfig> BiomeConfigurations;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HighLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MediumLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LowLODDistance;

    // Internal Methods
    void ProcessEntityMovement(FCrowd_EntityData& Entity, float DeltaTime);
    void ApplyFlockingBehavior(FCrowd_EntityData& Entity, const TArray<FCrowd_EntityData>& NearbyEntities);
    void ConstrainToBiome(FCrowd_EntityData& Entity);
    int32 GetBiomeForLocation(const FVector& Location);
    TArray<FCrowd_EntityData> GetNearbyEntities(const FVector& Location, float Radius);

    // Timer Handle
    FTimerHandle CrowdUpdateTimer;
};