#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bIsValid;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationTime;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("");
        bIsValid = false;
        ErrorMessage = TEXT("");
        ValidationTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemValidationResult> SystemResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ReportTimestamp;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        MemoryUsagePercent = 0.0f;
        OverallStatus = EBuild_IntegrationStatus::Pending;
        ReportTimestamp = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartIntegrationValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GetLastIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegrity(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetActiveActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetMemoryUsagePercent() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalSystems;

private:
    void ValidateCharacterSystem();
    void ValidateWorldGeneration();
    void ValidatePhysicsSystem();
    void ValidateAudioSystem();
    void ValidateLightingSystem();
    void ValidateVFXSystem();
    void ValidateAssetPipeline();
    
    void CompileValidationReport();
    FBuild_SystemValidationResult CreateValidationResult(const FString& SystemName, bool bValid, const FString& Error = TEXT(""));
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationTestActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationTestActor();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestComponentRegistration();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool TestCrossSystemCommunication();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Testing")
    TArray<FString> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Testing")
    bool bTestsCompleted;
};