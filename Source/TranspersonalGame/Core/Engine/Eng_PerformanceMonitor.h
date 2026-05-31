#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "../../SharedTypes.h"
#include "Eng_PerformanceMonitor.generated.h"

/**
 * Engine Architect's Performance Monitoring System
 * Enforces critical performance limits and prevents system overload
 * Based on memory constraints from production cycles
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Critical performance limits (from memory)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxTotalActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxActorsPerBiome = 4000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxTotalDinosaurs = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxPropsPerBiome = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Limits")
    int32 MaxTotalProps = 5000;

    // Species-specific limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Limits")
    int32 MaxTRex = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Limits")
    int32 MaxVelociraptor = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Limits")
    int32 MaxTriceratops = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Limits")
    int32 MaxBrachiosaurus = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Limits")
    int32 MaxAnkylosaurus = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Limits")
    int32 MaxParasaurolophus = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Limits")
    int32 MaxOtherSpecies = 10;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinGlobalLimits();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CanSpawnActor(EBiomeType TargetBiome);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CanSpawnDinosaur(EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool CanSpawnProp(EBiomeType TargetBiome);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentActorCount();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentDinosaurCount();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetSpeciesCount(EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActorCountInBiome(EBiomeType BiomeType);

    // Cleanup operations
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupExcessDinosaurs();

    // Performance reporting
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceCritical();

protected:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    
    // Cleanup helpers
    void RemoveOldestActorsInBiome(EBiomeType BiomeType, int32 CountToRemove);
    void RemoveExcessSpecies(EDinosaurSpecies Species, int32 MaxAllowed);

    // Biome center coordinates for distance calculations
    FVector GetBiomeCenter(EBiomeType BiomeType);

private:
    // Performance tracking
    int32 LastActorCount = 0;
    int32 LastDinosaurCount = 0;
    float LastUpdateTime = 0.0f;
    
    // Update frequency
    float UpdateInterval = 5.0f;
    
    // Critical thresholds
    float CriticalMemoryThreshold = 0.85f;
    float CriticalActorThreshold = 0.9f;
};