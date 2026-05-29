#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Build_EmergencyRecoverySystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_EmergencyType : uint8
{
    None = 0,
    UE5BridgeFailure,
    CompilationError,
    CriticalSystemCrash,
    TimeoutCascade,
    MemoryExhaustion,
    APIRateLimit
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_EmergencyEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Emergency")
    EBuild_EmergencyType EmergencyType = EBuild_EmergencyType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency")
    int32 CycleNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency")
    bool bRecoveryAttempted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency")
    bool bRecoverySuccessful = false;

    FBuild_EmergencyEvent()
    {
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_RecoveryAction
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    FString ActionName;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    bool bIsEmergencyOnly = false;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    float ExecutionTimeSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    bool bRequiresUE5Restart = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_EmergencyRecoverySystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    void ReportEmergency(EBuild_EmergencyType EmergencyType, const FString& AgentName, const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    bool IsEmergencyActive() const;

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    EBuild_EmergencyType GetCurrentEmergencyType() const;

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    TArray<FBuild_EmergencyEvent> GetEmergencyHistory() const;

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    bool AttemptRecovery();

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    void ClearEmergencyState();

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    bool ShouldSkipUE5Commands() const;

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    bool ShouldUseEmergencyWorkflow() const;

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    int32 GetMaxToolCallsForAgent(const FString& AgentName) const;

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    void LogRecoveryAction(const FString& ActionName, const FString& Description, float ExecutionTime);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Emergency State")
    TArray<FBuild_EmergencyEvent> EmergencyHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency State")
    FBuild_EmergencyEvent CurrentEmergency;

    UPROPERTY(BlueprintReadOnly, Category = "Emergency State")
    bool bEmergencyActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    TArray<FBuild_RecoveryAction> RecoveryActions;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    float LastRecoveryAttemptTime = 0.0f;

private:
    void InitializeRecoveryActions();
    bool ExecuteUE5BridgeRecovery();
    bool ExecuteCompilationRecovery();
    bool ExecuteTimeoutRecovery();
    void SaveEmergencyReport();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_EmergencyRecoveryActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_EmergencyRecoveryActor();

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    void TriggerEmergencyRecovery();

    UFUNCTION(BlueprintCallable, Category = "Emergency Recovery")
    void DisplayEmergencyStatus();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* EmergencyIndicator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Settings")
    bool bAutoRecoveryEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emergency Settings")
    float RecoveryCheckInterval = 5.0f;

private:
    FTimerHandle RecoveryCheckTimer;
    void CheckEmergencyStatus();
};