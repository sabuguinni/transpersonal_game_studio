#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Build_SystemIntegrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Pending         UMETA(DisplayName = "Pending"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Success         UMETA(DisplayName = "Success"),
    Failed          UMETA(DisplayName = "Failed"),
    Timeout         UMETA(DisplayName = "Timeout")
};

UENUM(BlueprintType)
enum class EBuild_SystemType : uint8
{
    Core            UMETA(DisplayName = "Core Systems"),
    World           UMETA(DisplayName = "World Generation"),
    Character       UMETA(DisplayName = "Character Systems"),
    AI              UMETA(DisplayName = "AI Systems"),
    Audio           UMETA(DisplayName = "Audio Systems"),
    VFX             UMETA(DisplayName = "VFX Systems"),
    UI              UMETA(DisplayName = "UI Systems"),
    Network         UMETA(DisplayName = "Network Systems")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EBuild_SystemType SystemType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    EBuild_IntegrationStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    FString Version;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    float LastValidationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
    FString ErrorMessage;

    FBuild_SystemInfo()
    {
        SystemName = TEXT("");
        SystemType = EBuild_SystemType::Core;
        Status = EBuild_IntegrationStatus::Unknown;
        Version = TEXT("1.0.0");
        LastValidationTime = 0.0f;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    TArray<FBuild_SystemInfo> SystemInfos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    int32 TotalSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    int32 SuccessfulSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    int32 FailedSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    float ValidationStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    float ValidationEndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Report")
    FString BuildVersion;

    FBuild_IntegrationReport()
    {
        TotalSystems = 0;
        SuccessfulSystems = 0;
        FailedSystems = 0;
        ValidationStartTime = 0.0f;
        ValidationEndTime = 0.0f;
        BuildVersion = TEXT("DEV");
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Integration))
class TRANSPERSONALGAME_API UBuild_SystemIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_SystemIntegrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === INTEGRATION VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    // === SYSTEM REGISTRATION ===
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RegisterSystem(const FBuild_SystemInfo& SystemInfo);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FBuild_SystemInfo> GetRegisteredSystems() const;

    // === DEPENDENCY VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateDependencies(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetMissingDependencies(const FString& SystemName);

    // === BUILD VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateActorSpawning();

    // === PERFORMANCE VALIDATION ===
    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetSystemPerformanceScore(const FString& SystemName);

    // === ERROR HANDLING ===
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void LogIntegrationError(const FString& SystemName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetSystemErrors(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearSystemErrors(const FString& SystemName);

protected:
    // === PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    TArray<FBuild_SystemInfo> RegisteredSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    bool bLogValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    FBuild_IntegrationReport LastReport;

    // === INTERNAL METHODS ===
    bool ValidateCoreSystem();
    bool ValidateWorldSystem();
    bool ValidateCharacterSystem();
    bool ValidateAISystem();
    bool ValidateAudioSystem();
    bool ValidateVFXSystem();
    bool ValidateUISystem();
    bool ValidateNetworkSystem();

    void UpdateSystemStatus(const FString& SystemName, EBuild_IntegrationStatus Status, const FString& ErrorMessage = TEXT(""));
    FBuild_SystemInfo* FindSystemInfo(const FString& SystemName);
    void InitializeDefaultSystems();
};