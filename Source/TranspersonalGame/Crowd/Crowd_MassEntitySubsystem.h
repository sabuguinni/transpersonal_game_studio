#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassProcessingTypes.h"
#include "MassSpawnerTypes.h"
#include "Engine/World.h"
#include "Crowd_MassEntitySubsystem.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float WanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 EntityID;

    FCrowd_EntityData()
        : Position(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , TargetLocation(FVector::ZeroVector)
        , Speed(150.0f)
        , WanderRadius(1000.0f)
        , EntityID(-1)
    {
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    float BiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    float SpawnDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Biome")
    FString BiomeName;

    FCrowd_BiomeSettings()
        : BiomeCenter(FVector::ZeroVector)
        , BiomeRadius(5000.0f)
        , MaxCrowdSize(500)
        , SpawnDensity(0.1f)
        , BiomeName(TEXT("Unknown"))
    {
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
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Mass Entity crowd simulation interface
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdInBiome(const FCrowd_BiomeSettings& BiomeSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SetCrowdBehaviorMode(int32 BehaviorMode);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    int32 GetActiveCrowdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ClearAllCrowds();

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void RegisterBiome(const FCrowd_BiomeSettings& BiomeSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    TArray<FCrowd_BiomeSettings> GetRegisteredBiomes() const;

protected:
    // Mass Entity system reference
    UPROPERTY()
    TObjectPtr<UMassEntitySubsystem> MassEntitySubsystem;

    // Crowd entity data storage
    UPROPERTY()
    TArray<FCrowd_EntityData> CrowdEntities;

    // Biome settings storage
    UPROPERTY()
    TArray<FCrowd_BiomeSettings> RegisteredBiomes;

    // Simulation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation", meta = (AllowPrivateAccess = "true"))
    float SimulationTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation", meta = (AllowPrivateAccess = "true"))
    int32 MaxEntitiesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation", meta = (AllowPrivateAccess = "true"))
    bool bIsSimulationActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation", meta = (AllowPrivateAccess = "true"))
    int32 CurrentBehaviorMode;

private:
    // Internal crowd management
    void ProcessCrowdMovement(float DeltaTime);
    void ProcessCrowdBehavior(float DeltaTime);
    void UpdateEntityPositions(float DeltaTime);
    
    // Utility functions
    FVector CalculateFlockingForce(const FCrowd_EntityData& Entity) const;
    FVector CalculateSeparationForce(const FCrowd_EntityData& Entity) const;
    FVector CalculateWanderForce(const FCrowd_EntityData& Entity) const;
    bool IsEntityInBiome(const FCrowd_EntityData& Entity, const FCrowd_BiomeSettings& Biome) const;

    // Performance tracking
    float LastUpdateTime;
    int32 EntitiesProcessedThisFrame;
    
    // Constants
    static constexpr float DefaultTickRate = 0.016f; // ~60 FPS
    static constexpr int32 DefaultMaxEntitiesPerFrame = 1000;
};