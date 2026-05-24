#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Build_FinalCycleDeliverySystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_FinalDeliveryMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    int32 TotalActorsInLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    int32 VFXSystemsActive = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    int32 DinosaurActorsSpawned = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    int32 EnvironmentActorsPlaced = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    int32 QAValidationsPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    bool bAllCriticalSystemsOperational = false;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    bool bFinalCycleCompleted = false;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    FString CycleCompletionTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    TArray<FString> SystemValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery")
    float TotalIntegrationTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_DeliverySystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Status")
    EBuild_SystemStatus CoreSystemsStatus = EBuild_SystemStatus::Initializing;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Status")
    EBuild_SystemStatus VFXSystemStatus = EBuild_SystemStatus::Initializing;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Status")
    EBuild_SystemStatus DinosaurSystemStatus = EBuild_SystemStatus::Initializing;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Status")
    EBuild_SystemStatus EnvironmentSystemStatus = EBuild_SystemStatus::Initializing;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Status")
    EBuild_SystemStatus QASystemStatus = EBuild_SystemStatus::Initializing;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Status")
    bool bReadyForFinalDelivery = false;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Status")
    FString LastValidationTimestamp;
};

/**
 * Final Cycle Delivery System - Orchestrates the completion and delivery of the final production cycle
 * Ensures all systems are properly integrated, validated, and ready for delivery
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleDeliverySystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleDeliverySystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Final delivery orchestration
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    void InitiateFinalCycleDelivery();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool ValidateAllSystemsForDelivery();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    void CompleteFinalCycleDelivery();

    // System validation
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool ValidateCoreSystemsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool ValidateVFXSystemsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool ValidateDinosaurSystemsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool ValidateEnvironmentSystemsIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool ValidateQASystemsIntegration();

    // Metrics and reporting
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    FBuild_FinalDeliveryMetrics GetFinalDeliveryMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    FBuild_DeliverySystemStatus GetDeliverySystemStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    void GenerateFinalDeliveryReport();

    // Status queries
    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool IsFinalCycleComplete() const { return bFinalCycleCompleted; }

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    bool IsReadyForDelivery() const { return bReadyForFinalDelivery; }

    UFUNCTION(BlueprintCallable, Category = "Final Delivery")
    float GetIntegrationProgress() const { return IntegrationProgress; }

protected:
    // Internal validation methods
    void ValidateWorldState();
    void ValidateActorCounts();
    void ValidateSystemConnections();
    void UpdateDeliveryMetrics();
    void LogDeliveryStatus();

private:
    UPROPERTY()
    FBuild_FinalDeliveryMetrics DeliveryMetrics;

    UPROPERTY()
    FBuild_DeliverySystemStatus SystemStatus;

    UPROPERTY()
    bool bFinalCycleCompleted = false;

    UPROPERTY()
    bool bReadyForFinalDelivery = false;

    UPROPERTY()
    float IntegrationProgress = 0.0f;

    UPROPERTY()
    TArray<FString> ValidationLog;

    UPROPERTY()
    FDateTime DeliveryStartTime;

    UPROPERTY()
    FDateTime DeliveryCompletionTime;
};