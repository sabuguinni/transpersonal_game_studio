#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_FinalDeliveryIntegrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_DeliveryStatus : uint8
{
    Pending         UMETA(DisplayName = "Pending"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Validated       UMETA(DisplayName = "Validated"),
    Delivered       UMETA(DisplayName = "Delivered"),
    Failed          UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_DeliveryMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 TotalSystemsIntegrated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 VFXSystemsValidated = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 CoreSystemsOperational = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    int32 QAValidationsPassed = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    float IntegrationCompletionPercentage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    float PerformanceScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    FString LastValidationTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Metrics")
    EBuild_DeliveryStatus DeliveryStatus = EBuild_DeliveryStatus::Pending;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemIntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    bool bIsOperational = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    bool bPassedValidation = false;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    float PerformanceImpact = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration Report")
    FString IntegrationNotes;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalDeliveryIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalDeliveryIntegrator();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    void InitializeDeliveryIntegration();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    void GenerateDeliveryReport();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    bool IntegrateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    bool ValidateCoreGameplay();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    bool PerformFinalQAValidation();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    void PrepareForDelivery();

    // Metrics and reporting
    UFUNCTION(BlueprintPure, Category = "Final Delivery Integration")
    FBuild_DeliveryMetrics GetDeliveryMetrics() const { return DeliveryMetrics; }

    UFUNCTION(BlueprintPure, Category = "Final Delivery Integration")
    TArray<FBuild_SystemIntegrationReport> GetSystemReports() const { return SystemReports; }

    UFUNCTION(BlueprintPure, Category = "Final Delivery Integration")
    EBuild_DeliveryStatus GetDeliveryStatus() const { return DeliveryMetrics.DeliveryStatus; }

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    void SetDeliveryStatus(EBuild_DeliveryStatus NewStatus);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    float CalculateOverallPerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    bool CheckSystemCompatibility();

    UFUNCTION(BlueprintCallable, Category = "Final Delivery Integration")
    void OptimizeIntegratedSystems();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery Integration")
    FBuild_DeliveryMetrics DeliveryMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery Integration")
    TArray<FBuild_SystemIntegrationReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery Integration")
    TArray<FString> CriticalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery Integration")
    bool bIntegrationInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Final Delivery Integration")
    bool bDeliveryReady = false;

private:
    void ValidateVFXIntegration();
    void ValidateCoreSystemsIntegration();
    void ValidateQAFramework();
    void UpdateDeliveryMetrics();
    void LogIntegrationStatus();
    FBuild_SystemIntegrationReport CreateSystemReport(const FString& SystemName, bool bOperational, bool bValidated);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_DeliveryValidationActor : public AActor
{
    GENERATED_BODY()

public:
    ABuild_DeliveryValidationActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Delivery Validation")
    void RunDeliveryValidation();

    UFUNCTION(BlueprintCallable, Category = "Delivery Validation")
    bool ValidateGameplayFlow();

    UFUNCTION(BlueprintCallable, Category = "Delivery Validation")
    bool TestSystemInteractions();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Validation")
    bool bValidationActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Validation")
    float ValidationTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Delivery Validation")
    int32 ValidationCycleCount = 0;
};