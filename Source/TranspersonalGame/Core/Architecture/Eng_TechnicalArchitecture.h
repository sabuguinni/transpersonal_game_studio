#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_TechnicalArchitecture.generated.h"

// Architecture validation result
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitectureValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bIsValid = false;

    UPROPERTY(BlueprintReadOnly)
    FString ValidationMessage;

    UPROPERTY(BlueprintReadOnly)
    float PerformanceScore = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 DinosaurCount = 0;
};

// Module dependency info
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ModuleDependency
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly)
    TArray<FString> Dependencies;

    UPROPERTY(BlueprintReadOnly)
    int32 Priority = 0;

    UPROPERTY(BlueprintReadOnly)
    bool bIsInitialized = false;
};

// Performance targets
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceTargets
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float TargetFPS = 60.0f;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxActors = 8000;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxDinosaurs = 150;

    UPROPERTY(BlueprintReadWrite)
    float MaxMemoryGB = 4.0f;

    UPROPERTY(BlueprintReadWrite)
    float MaxDrawCalls = 2000.0f;
};

/**
 * Technical Architecture Manager - Defines and enforces coding standards, performance targets,
 * and module organization across the entire project
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_TechnicalArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Architecture validation
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FEng_ArchitectureValidation ValidateArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool EnforcePerformanceTargets();

    // Module management
    UFUNCTION(BlueprintCallable, Category = "Modules")
    void RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies, int32 Priority);

    UFUNCTION(BlueprintCallable, Category = "Modules")
    TArray<FEng_ModuleDependency> GetModuleLoadOrder();

    UFUNCTION(BlueprintCallable, Category = "Modules")
    bool InitializeModule(const FString& ModuleName);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceTargets GetPerformanceTargets() const { return PerformanceTargets; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTargets(const FEng_PerformanceTargets& NewTargets);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetCurrentActorCount();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentMemoryUsageGB();

    // Coding standards enforcement
    UFUNCTION(BlueprintCallable, Category = "Standards")
    bool ValidateNamingConventions(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "Standards")
    bool ValidateIncludeStructure(const FString& HeaderPath);

    UFUNCTION(BlueprintCallable, Category = "Standards")
    TArray<FString> GetCodingStandardsViolations();

protected:
    UPROPERTY()
    TArray<FEng_ModuleDependency> RegisteredModules;

    UPROPERTY()
    FEng_PerformanceTargets PerformanceTargets;

    UPROPERTY()
    TMap<FString, bool> ModuleInitializationStatus;

private:
    void InitializeDefaultModules();
    void ValidateCircularDependencies();
    bool CheckPerformanceThresholds();
    void LogArchitectureStatus();
};

/**
 * World-level architecture subsystem for runtime validation
 */
UCLASS()
class TRANSPERSONALGAME_API UEng_WorldArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void PostInitialize() override;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ValidateWorldState();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool EnforceActorLimits();

protected:
    UPROPERTY()
    TWeakObjectPtr<UEng_TechnicalArchitecture> TechnicalArchitecture;
};