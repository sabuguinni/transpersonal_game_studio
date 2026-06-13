#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Success     UMETA(DisplayName = "Success"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FBuild_ValidationReport()
    {
        SystemName = TEXT("");
        Result = EBuild_ValidationResult::Success;
        Message = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bHasCDO;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    TArray<FString> Dependencies;

    FBuild_SystemHealth()
    {
        ModuleName = TEXT("");
        bIsLoaded = false;
        bHasCDO = false;
        ActorCount = 0;
    }
};

/**
 * Integration Validator - Validates system integration and build health
 * Monitors cross-system dependencies and compilation status
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationValidator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_IntegrationValidator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateBuildHealth();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void GenerateIntegrationReport();

    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_ValidationReport> GetValidationReports() const { return ValidationReports; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    TArray<FBuild_SystemHealth> GetSystemHealthReports() const { return SystemHealthReports; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    bool IsSystemHealthy(const FString& SystemName) const;

    UFUNCTION(BlueprintPure, Category = "Integration")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintPure, Category = "Integration")
    float GetLastValidationTime() const { return LastValidationTime; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_ValidationReport> ValidationReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemHealth> SystemHealthReports;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bValidationInProgress;

private:
    void ValidateQASystem();
    void ValidateVFXSystem();
    void ValidateAudioSystem();
    void ValidateNarrativeSystem();
    void ValidateQuestSystem();
    void ValidateCrowdSystem();
    void ValidateCombatSystem();
    void ValidateNPCSystem();
    void ValidateCoreSystem(const FString& ClassName);
    
    void AddValidationReport(const FString& SystemName, EBuild_ValidationResult Result, const FString& Message);
    void AddSystemHealthReport(const FString& ModuleName, bool bLoaded, bool bHasCDO, int32 ActorCount, const TArray<FString>& Dependencies);
    
    FBuild_SystemHealth CreateSystemHealthReport(const FString& ModuleName);
    bool ValidateClassLoading(const FString& ClassName);
    bool ValidateCDOConstruction(UClass* Class);
    int32 CountActorsOfClass(const FString& ClassName);
    TArray<FString> GetSystemDependencies(const FString& SystemName);
};