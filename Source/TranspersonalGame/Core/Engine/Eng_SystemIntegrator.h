#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Eng_SystemIntegrator.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemType : uint8
{
    None            UMETA(DisplayName = "None"),
    Physics         UMETA(DisplayName = "Physics System"),
    World           UMETA(DisplayName = "World Generation"),
    Biome           UMETA(DisplayName = "Biome System"),
    Architecture    UMETA(DisplayName = "Architecture System"),
    Character       UMETA(DisplayName = "Character System"),
    Dinosaur        UMETA(DisplayName = "Dinosaur System"),
    Combat          UMETA(DisplayName = "Combat System"),
    Survival        UMETA(DisplayName = "Survival System"),
    Quest           UMETA(DisplayName = "Quest System"),
    Audio           UMETA(DisplayName = "Audio System"),
    VFX             UMETA(DisplayName = "VFX System")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    EEng_SystemType SystemType = EEng_SystemType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bIsInitialized = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float PerformanceScore = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FString LastError;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    float LastUpdateTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    int32 UpdateCount = 0;

    FEng_SystemStatus()
    {
        SystemType = EEng_SystemType::None;
        bIsInitialized = false;
        bIsActive = false;
        PerformanceScore = 100.0f;
        LastError = TEXT("");
        LastUpdateTime = 0.0f;
        UpdateCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_IntegrationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bEnablePerformanceMonitoring = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float PerformanceUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float CriticalPerformanceThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoDisablePoorPerformers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxSystemErrors = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bEnableSystemDependencies = true;

    FEng_IntegrationSettings()
    {
        bEnablePerformanceMonitoring = true;
        PerformanceUpdateInterval = 1.0f;
        CriticalPerformanceThreshold = 30.0f;
        bAutoDisablePoorPerformers = true;
        MaxSystemErrors = 5;
        bEnableSystemDependencies = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_SystemIntegrator();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystem(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterSystem(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemRegistered(EEng_SystemType SystemType) const;

    // System Control
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ActivateSystem(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void DeactivateSystem(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ActivateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void DeactivateAllSystems();

    // System Status
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FEng_SystemStatus GetSystemStatus(EEng_SystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FEng_SystemStatus> GetAllSystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetActiveSystemCount() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UpdateSystemPerformance(EEng_SystemType SystemType, float PerformanceScore);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetAveragePerformance() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<EEng_SystemType> GetPoorPerformingSystems() const;

    // Error Handling
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ReportSystemError(EEng_SystemType SystemType, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearSystemErrors(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetSystemErrorCount(EEng_SystemType SystemType) const;

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SetIntegrationSettings(const FEng_IntegrationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FEng_IntegrationSettings GetIntegrationSettings() const;

    // System Dependencies
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemDependencies(EEng_SystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<EEng_SystemType> GetSystemDependencies(EEng_SystemType SystemType) const;

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void DebugPrintSystemStatus();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void RunSystemIntegrationTests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Integration")
    void ResetAllSystems();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    TMap<EEng_SystemType, FEng_SystemStatus> SystemStatusMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    TMap<EEng_SystemType, int32> SystemErrorCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FEng_IntegrationSettings Settings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    bool bIsInitialized = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Integration")
    float LastPerformanceUpdate = 0.0f;

private:
    void InitializeDefaultSettings();
    void UpdatePerformanceMonitoring();
    void HandlePoorPerformance(EEng_SystemType SystemType);
    TArray<EEng_SystemType> GetRequiredDependencies(EEng_SystemType SystemType) const;
    bool AreAllDependenciesActive(const TArray<EEng_SystemType>& Dependencies) const;
};