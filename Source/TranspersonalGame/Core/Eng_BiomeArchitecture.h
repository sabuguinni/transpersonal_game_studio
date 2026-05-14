#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalFramework.h"
#include "Eng_BiomeArchitecture.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 002 - BIOME ARCHITECTURE SYSTEM
 * 
 * Defines the architectural framework for biome management that Agent #5 (World Generator)
 * must follow. This establishes the rules for biome creation, boundaries, transitions,
 * and performance optimization across the 10km² world.
 * 
 * MANDATORY BIOME LAYOUT (as per Hugo's criteria):
 * - Pantano (Swamp) SW: Coordinates (-5000, -5000) to (-1000, -1000)
 * - Floresta (Forest) NW: Coordinates (-5000, 1000) to (-1000, 5000)  
 * - Savana (Savanna) Center: Coordinates (-1000, -1000) to (1000, 1000)
 * - Deserto (Desert) E: Coordinates (1000, -5000) to (5000, 5000)
 * - Montanha (Mountain) NE: Coordinates (1000, 1000) to (5000, 5000)
 */

// Biome Types (matches SharedTypes.h)
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Swamp           UMETA(DisplayName = "Swamp"),
    Forest          UMETA(DisplayName = "Forest"), 
    Savanna         UMETA(DisplayName = "Savanna"),
    Desert          UMETA(DisplayName = "Desert"),
    Mountain        UMETA(DisplayName = "Mountain"),
    Transition      UMETA(DisplayName = "Transition")
};

// Biome Performance Levels
UENUM(BlueprintType)
enum class EEng_BiomePerformance : uint8
{
    High            UMETA(DisplayName = "High Detail"),
    Medium          UMETA(DisplayName = "Medium Detail"),
    Low             UMETA(DisplayName = "Low Detail"),
    Culled          UMETA(DisplayName = "Culled")
};

// Biome Validation Status
UENUM(BlueprintType)
enum class EEng_BiomeValidation : uint8
{
    NotValidated    UMETA(DisplayName = "Not Validated"),
    Valid           UMETA(DisplayName = "Valid"),
    Invalid         UMETA(DisplayName = "Invalid"),
    NeedsUpdate     UMETA(DisplayName = "Needs Update")
};

// Biome Definition Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D MinBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D MaxBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TransitionZoneWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomePerformance PerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeValidation ValidationStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxVegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float LODDistanceMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FDateTime LastValidation;

    FEng_BiomeDefinition()
    {
        BiomeType = EEng_BiomeType::Savanna;
        MinBounds = FVector2D::ZeroVector;
        MaxBounds = FVector2D(1000.0f, 1000.0f);
        TransitionZoneWidth = 500.0f;
        PerformanceLevel = EEng_BiomePerformance::Medium;
        ValidationStatus = EEng_BiomeValidation::NotValidated;
        MaxVegetationDensity = 100;
        MaxActorCount = 1000;
        LODDistanceMultiplier = 1.0f;
        LastValidation = FDateTime::Now();
    }
};

// Biome Performance Metrics
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CurrentActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    int32 CurrentVegetationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float AverageFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    bool bIsPlayerInBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metrics")
    float DistanceToPlayer;

    FEng_BiomeMetrics()
    {
        BiomeType = EEng_BiomeType::Savanna;
        CurrentActorCount = 0;
        CurrentVegetationCount = 0;
        AverageFrameTime = 16.67f;
        MemoryUsageMB = 0.0f;
        bIsPlayerInBiome = false;
        DistanceToPlayer = 0.0f;
    }
};

/**
 * BIOME ARCHITECTURE SUBSYSTEM
 * 
 * This subsystem enforces architectural rules for biome management.
 * Agent #5 (World Generator) MUST use this system to register biomes
 * and validate their implementation meets performance requirements.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_BiomeArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;

    // Biome Registration (MANDATORY for Agent #5)
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool RegisterBiome(const FEng_BiomeDefinition& BiomeDefinition);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool UnregisterBiome(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    TArray<FEng_BiomeDefinition> GetAllBiomes();

    // Biome Validation (ENFORCED by Architecture)
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool ValidateBiomeLayout();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool ValidateBiomePerformance();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    EEng_BiomeValidation GetBiomeValidationStatus(EEng_BiomeType BiomeType);

    // Biome Queries
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool IsLocationInTransitionZone(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    float GetTransitionBlendFactor(const FVector& WorldLocation, EEng_BiomeType BiomeA, EEng_BiomeType BiomeB);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeMetrics GetBiomeMetrics(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool IsBiomePerformanceAcceptable(EEng_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void UpdateBiomePerformanceLevel(EEng_BiomeType BiomeType, EEng_BiomePerformance NewLevel);

    // World Partition Integration
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool ValidateWorldPartitionSetup();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    TArray<FString> GetWorldPartitionErrors();

protected:
    // Biome Registry
    UPROPERTY()
    TMap<EEng_BiomeType, FEng_BiomeDefinition> RegisteredBiomes;

    // Performance Tracking
    UPROPERTY()
    TMap<EEng_BiomeType, FEng_BiomeMetrics> BiomeMetrics;

    // Validation Settings
    UPROPERTY(EditAnywhere, Category = "Architecture")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, Category = "Architecture")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, Category = "Architecture")
    float MaxAllowedFrameTime;

    UPROPERTY(EditAnywhere, Category = "Architecture")
    int32 MaxActorsPerBiome;

    // Internal Methods
    void InitializeDefaultBiomes();
    void UpdateBiomeMetrics();
    void ValidateIndividualBiome(EEng_BiomeType BiomeType);
    bool CheckBiomeBoundaries();
    void LogBiomeEvent(const FString& Event, EEng_BiomeType BiomeType, const FString& Details);

private:
    // Validation timer
    FTimerHandle ValidationTimer;
    
    // Performance tracking
    float LastValidationTime;
    TArray<FString> ValidationErrors;
    
    // Reference to architectural framework
    UPROPERTY()
    UEng_ArchitecturalFramework* ArchFramework;
};