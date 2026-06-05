#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../Engine/Eng_ArchitecturalGovernance.h"
#include "Core_ArchitecturalCompliance.generated.h"

UENUM(BlueprintType)
enum class ECore_ComplianceStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Compliant       UMETA(DisplayName = "Compliant"),
    NonCompliant    UMETA(DisplayName = "Non-Compliant"),
    Pending         UMETA(DisplayName = "Pending Review"),
    Exempt          UMETA(DisplayName = "Exempt")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_SystemComplianceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    EEng_ArchitecturalLayer AssignedLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    ECore_ComplianceStatus ComplianceStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    float PerformanceScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    TArray<FString> ViolationReasons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compliance")
    FDateTime LastValidation;

    FCore_SystemComplianceData()
    {
        SystemName = TEXT("");
        AssignedLayer = EEng_ArchitecturalLayer::Foundation;
        ComplianceStatus = ECore_ComplianceStatus::Unknown;
        PerformanceScore = 0.0f;
        LastValidation = FDateTime::Now();
    }
};

/**
 * Core Systems Architectural Compliance Manager
 * Ensures all core physics systems comply with architectural governance standards
 * Integrates with Eng_ArchitecturalGovernance for centralized compliance monitoring
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_ArchitecturalCompliance : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_ArchitecturalCompliance();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    bool RegisterCoreSystem(const FString& SystemName, EEng_ArchitecturalLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    bool UnregisterCoreSystem(const FString& SystemName);

    // Compliance Validation
    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    ECore_ComplianceStatus ValidateSystemCompliance(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    bool ValidateAllCoreSystems();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    float GetSystemPerformanceScore(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    void UpdateSystemPerformanceScore(const FString& SystemName, float Score);

    // Governance Integration
    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    bool IntegrateWithGovernanceSystem();

    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    bool CanSystemActivate(const FString& SystemName);

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    TArray<FCore_SystemComplianceData> GetComplianceReport();

    UFUNCTION(BlueprintCallable, Category = "Architectural Compliance")
    FString GenerateComplianceReport();

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Architectural Compliance")
    void OnSystemRegistered(const FString& SystemName, EEng_ArchitecturalLayer Layer);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architectural Compliance")
    void OnComplianceViolation(const FString& SystemName, const FString& Reason);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architectural Compliance")
    void OnPerformanceThresholdExceeded(const FString& SystemName, float Score);

protected:
    // Core system compliance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Compliance Data")
    TMap<FString, FCore_SystemComplianceData> SystemComplianceMap;

    // Governance system reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Governance")
    TObjectPtr<UEng_ArchitecturalGovernance> GovernanceSystem;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinPerformanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPerformanceThreshold;

    // Validation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bAutoValidateOnRegistration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bStrictComplianceMode;

    // Internal methods
    void ValidateSystemArchitecture(const FString& SystemName, FCore_SystemComplianceData& ComplianceData);
    void CheckPerformanceCompliance(const FString& SystemName, FCore_SystemComplianceData& ComplianceData);
    void ReportComplianceViolation(const FString& SystemName, const FString& Reason);
    void InitializeDefaultCoreSystems();

private:
    FTimerHandle ValidationTimerHandle;
    bool bIsInitialized;
    int32 TotalSystemsRegistered;
    int32 CompliantSystemsCount;
};