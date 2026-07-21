#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Components/ActorComponent.h"
#include "Build_BridgeRecoverySystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_BridgeStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Healthy     UMETA(DisplayName = "Healthy"),
    Degraded    UMETA(DisplayName = "Degraded"),
    Failed      UMETA(DisplayName = "Failed"),
    Recovering  UMETA(DisplayName = "Recovering")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealthMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 TotalActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    int32 EnvironmentActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float LastResponseTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    EBuild_BridgeStatus BridgeStatus = EBuild_BridgeStatus::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FString LastErrorMessage;

    FBuild_SystemHealthMetrics()
    {
        TotalActorCount = 0;
        DinosaurCount = 0;
        EnvironmentActorCount = 0;
        LastResponseTime = 0.0f;
        BridgeStatus = EBuild_BridgeStatus::Unknown;
        LastErrorMessage = TEXT("");
    }
};

/**
 * Bridge Recovery System - Handles UE5 bridge failures and system recovery
 * Monitors bridge health, implements fallback mechanisms, and ensures system stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_BridgeRecoverySystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_BridgeRecoverySystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Bridge Recovery")
    FBuild_SystemHealthMetrics GetSystemHealthMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Bridge Recovery")
    bool AttemptBridgeRecovery();

    UFUNCTION(BlueprintCallable, Category = "Bridge Recovery")
    void ForceMapSave();

    UFUNCTION(BlueprintCallable, Category = "Bridge Recovery")
    bool ValidateActorIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Bridge Recovery")
    void LogSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Bridge Recovery")
    EBuild_BridgeStatus GetBridgeStatus() const { return CurrentHealthMetrics.BridgeStatus; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FBuild_SystemHealthMetrics CurrentHealthMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    float LastHealthCheckTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    int32 RecoveryAttempts = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Recovery")
    bool bAutoRecoveryEnabled = true;

private:
    void UpdateHealthMetrics();
    void CheckBridgeConnectivity();
    bool PerformEmergencyMapSave();
    void ResetRecoveryState();

    FTimerHandle HealthCheckTimer;
    static constexpr float HEALTH_CHECK_INTERVAL = 30.0f;
    static constexpr int32 MAX_RECOVERY_ATTEMPTS = 3;
};