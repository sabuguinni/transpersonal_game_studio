#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Eng_SystemIntegrator.generated.h"

UENUM(BlueprintType)
enum class EEng_SystemType : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    Physics         UMETA(DisplayName = "Physics & Collision"),
    Rendering       UMETA(DisplayName = "Rendering & Graphics"),
    Audio           UMETA(DisplayName = "Audio Systems"),
    Input           UMETA(DisplayName = "Input & Controls"),
    Networking      UMETA(DisplayName = "Networking"),
    AI              UMETA(DisplayName = "AI & Behavior"),
    Gameplay        UMETA(DisplayName = "Gameplay Logic"),
    UI              UMETA(DisplayName = "User Interface"),
    Performance     UMETA(DisplayName = "Performance & Optimization")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EEng_SystemType SystemType;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float LastUpdateTime;

    FEng_SystemStatus()
    {
        SystemType = EEng_SystemType::Core;
        bIsInitialized = false;
        bIsActive = false;
        PerformanceScore = 1.0f;
        LastError = TEXT("");
        LastUpdateTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActiveSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallPerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ReportGenerationTime;

    FEng_IntegrationReport()
    {
        TotalSystems = 0;
        ActiveSystems = 0;
        FailedSystems = 0;
        OverallPerformanceScore = 1.0f;
        CriticalErrors.Empty();
        ReportGenerationTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_SystemIntegratorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void InitializeAllSystems();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterSystem(EEng_SystemType SystemType, UObject* SystemObject);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void UnregisterSystem(EEng_SystemType SystemType);

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool IsSystemActive(EEng_SystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    FEng_SystemStatus GetSystemStatus(EEng_SystemType SystemType) const;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    FEng_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void OptimizeSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ForceSystemRestart(EEng_SystemType SystemType);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TMap<EEng_SystemType, FEng_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    TMap<EEng_SystemType, TWeakObjectPtr<UObject>> RegisteredSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ValidationInterval;

private:
    void UpdateSystemStatus(EEng_SystemType SystemType);
    void ValidateSystemDependencies();
    void CheckSystemPerformance();
    bool IsSystemCritical(EEng_SystemType SystemType) const;
    void HandleSystemFailure(EEng_SystemType SystemType, const FString& Error);
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_SystemIntegratorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_SystemIntegratorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterWithIntegrator();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void UnregisterFromIntegrator();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSystemPerformanceScore() const;

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ReportSystemError(const FString& ErrorMessage);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EEng_SystemType ManagedSystemType;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsRegisteredWithIntegrator;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FString> RecentErrors;

private:
    void CalculatePerformanceScore();
    void ValidateComponentIntegration();
    void CleanupOldErrors();
};