#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "EngineArchitecture.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Initializing UMETA(DisplayName = "Initializing"),
    Operational UMETA(DisplayName = "Operational"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EEng_SystemStatus Status = EEng_SystemStatus::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString LastError;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float LastValidationTime = 0.0f;

    FEng_SystemValidation()
    {
        SystemName = TEXT("Unknown");
        Status = EEng_SystemStatus::Unknown;
        LastError = TEXT("");
        LastValidationTime = 0.0f;
    }
};

/**
 * EngineArchitecture - Core architecture validation and management system
 * Ensures all systems follow the technical architecture defined by Engine Architect
 * Validates module dependencies, performance constraints, and system integration
 * Critical for maintaining code quality and preventing technical debt
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEngineArchitecture : public UActorComponent
{
    GENERATED_BODY()

public:
    UEngineArchitecture();

protected:
    virtual void BeginPlay() override;

public:
    // System validation registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<FString, FEng_SystemValidation> SystemValidations;

    // Performance constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageGB = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinTargetFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveActors = 10000;

    // Architecture validation functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ValidateCoreArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ReportArchitectureStatus();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentMemoryUsageGB() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinLimits() const;

    // System registration for other agents
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RegisterSystem(const FString& SystemName, EEng_SystemStatus InitialStatus);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateSystemStatus(const FString& SystemName, EEng_SystemStatus NewStatus, const FString& ErrorMessage = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EEng_SystemStatus GetSystemStatus(const FString& SystemName) const;

private:
    // Internal validation functions
    void ValidateModuleSystems();
    void ValidatePerformanceConstraints();
    void ValidateMemoryUsage();
    void ValidateFrameRate();
    
    // Architecture enforcement
    bool EnforceModuleDependencies();
    bool EnforceNamingConventions();
    bool EnforceCodeStandards();
};