#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "EngineArchitectCore.generated.h"

// Forward declarations
class UEngineArchitectSubsystem;
class AEngineValidationActor;

/**
 * Engine Architecture Configuration
 * Defines core technical rules and constraints for all game systems
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureConfig
{
    GENERATED_BODY()

    // Performance budgets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxActorsPerBiome = 500;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxDinosaurCount = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate = 60.0f;

    // World constraints
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    float WorldSizeKm = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    int32 BiomeCount = 5;

    FEng_ArchitectureConfig()
    {
        MaxActorsPerBiome = 500;
        MaxDinosaurCount = 50;
        TargetFrameRate = 60.0f;
        WorldSizeKm = 16.0f;
        bUseWorldPartition = true;
        BiomeCount = 5;
    }
};

/**
 * Engine validation states
 */
UENUM(BlueprintType)
enum class EEng_ValidationState : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Validating  UMETA(DisplayName = "Validating"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Critical    UMETA(DisplayName = "Critical Error")
};

/**
 * Module dependency tracking
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDependency
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    bool bIsRequired = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    bool bIsLoaded = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
    FString Version;

    FEng_ModuleDependency()
    {
        ModuleName = TEXT("");
        bIsRequired = true;
        bIsLoaded = false;
        Version = TEXT("1.0.0");
    }
};

/**
 * Engine Architect Subsystem
 * Manages technical architecture validation and enforcement
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEngineArchitectSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    bool ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    EEng_ValidationState GetValidationState() const { return ValidationState; }

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    FEng_ArchitectureConfig GetArchitectureConfig() const { return Config; }

    UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
    void SetArchitectureConfig(const FEng_ArchitectureConfig& NewConfig);

    // Module management
    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool RegisterModule(const FString& ModuleName, const FString& Version);

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    bool IsModuleLoaded(const FString& ModuleName) const;

    UFUNCTION(BlueprintCallable, Category = "Module Management")
    TArray<FEng_ModuleDependency> GetModuleDependencies() const;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinBudget() const;

protected:
    UPROPERTY()
    FEng_ArchitectureConfig Config;

    UPROPERTY()
    EEng_ValidationState ValidationState = EEng_ValidationState::Unknown;

    UPROPERTY()
    TArray<FEng_ModuleDependency> ModuleDependencies;

    UPROPERTY()
    float LastFrameRate = 0.0f;

private:
    void InitializeModuleDependencies();
    bool ValidateModules();
    bool ValidatePerformance();
    bool ValidateWorldStructure();
};

/**
 * Engine Validation Actor
 * Placed in levels to perform runtime architecture validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEngineValidationActor : public AActor
{
    GENERATED_BODY()

public:
    AEngineValidationActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bPerformContinuousValidation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationInterval = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EEng_ValidationState CurrentValidationState = EEng_ValidationState::Unknown;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void PerformValidation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Validation")
    void OnValidationStateChanged(EEng_ValidationState NewState);

private:
    float LastValidationTime = 0.0f;
    UEngineArchitectSubsystem* ArchitectSubsystem = nullptr;
};

/**
 * Engine Performance Monitor Component
 * Tracks performance metrics for individual actors
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnginePerformanceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnginePerformanceComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bTrackPerformance = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TickCount = 0;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPerformanceScore() const;

private:
    float TotalTickTime = 0.0f;
    double LastTickStartTime = 0.0;
};