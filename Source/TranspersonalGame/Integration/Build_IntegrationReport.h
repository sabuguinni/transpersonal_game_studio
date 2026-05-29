#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Build_IntegrationReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Loading     UMETA(DisplayName = "Loading"),
    Operational UMETA(DisplayName = "Operational"),
    Failed      UMETA(DisplayName = "Failed"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString ClassName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString ErrorMessage;

    FBuild_SystemInfo()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        ClassName = TEXT("");
        bIsLoaded = false;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DinosaurActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VFXActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 StaticMeshActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    FBuild_PerformanceMetrics()
    {
        TotalActors = 0;
        DinosaurActors = 0;
        VFXActors = 0;
        StaticMeshActors = 0;
        FrameTime = 0.0f;
        MemoryUsage = 0.0f;
    }
};

/**
 * Integration Report Subsystem
 * Monitors system health, compilation status, and cross-system compatibility
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBuild_IntegrationReport : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemInfo> GetSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_PerformanceMetrics GetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemOperational(const FString& SystemName);

    // Cross-system validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCharacterVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateDinosaurAIIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateWorldFoliageIntegration();

    // Build validation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool CheckCompilationStatus();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetCompilationErrors();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SaveIntegrationReport(const FString& ReportPath);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemInfo> SystemInfoArray;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CompilationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsOperational;

private:
    void ValidateSystem(const FString& SystemName, const FString& ClassName);
    void UpdatePerformanceMetrics();
    void CheckCrossSystemCompatibility();
    
    // Core systems to monitor
    TArray<TPair<FString, FString>> CoreSystems;
};