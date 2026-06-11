#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_IntegrationManager.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Validating      UMETA(DisplayName = "Validating"),
    Valid           UMETA(DisplayName = "Valid"),
    Invalid         UMETA(DisplayName = "Invalid"),
    CompileError    UMETA(DisplayName = "Compile Error"),
    RuntimeError    UMETA(DisplayName = "Runtime Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastError;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        Status = EBuild_IntegrationStatus::Unknown;
        ClassCount = 0;
        ActorCount = 0;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsStable;

    FBuild_SystemHealth()
    {
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        TotalActors = 0;
        ActiveComponents = 0;
        bIsStable = false;
    }
};

/**
 * Integration Manager - Orchestrates build validation and system health monitoring
 * Ensures all modules work together correctly and maintains build stability
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_IntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Integration validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllModules();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetModuleStatus(const FString& ModuleName);

    // System health monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemHealth GetSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RunIntegrationTests();

    // Build management
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void CreateBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool RestoreBuildSnapshot(const FString& SnapshotName);

    // Error reporting
    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetIntegrationErrors();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearIntegrationErrors();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_SystemHealth CurrentSystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> IntegrationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bValidationInProgress;

private:
    void ValidateModuleClasses(const FString& ModuleName);
    void ValidateModuleActors(const FString& ModuleName);
    void UpdateSystemHealth();
    void LogIntegrationError(const FString& Error);

    FTimerHandle ValidationTimerHandle;
    FTimerHandle HealthMonitorHandle;
};