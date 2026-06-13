#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Build_IntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemHealth : uint8
{
    Healthy     UMETA(DisplayName = "Healthy"),
    Degraded    UMETA(DisplayName = "Degraded"),
    Critical    UMETA(DisplayName = "Critical"),
    Failed      UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct FBuild_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 EnvironmentCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CharacterCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 LightingCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 AudioCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 VFXCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 QACount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    EBuild_SystemHealth OverallHealth = EBuild_SystemHealth::Healthy;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float PerformanceScore = 100.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_IntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    ABuild_IntegrationValidator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_SystemMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bAutoValidateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ValidationInterval = 30.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ValidationErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ValidationWarnings;

public:
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemMetrics GetSystemMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_SystemHealth GetOverallHealth() const { return CurrentMetrics.OverallHealth; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetValidationErrors() const { return ValidationErrors; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetValidationWarnings() const { return ValidationWarnings; }

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearValidationResults();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleCompilation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCrossSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculatePerformanceScore();

protected:
    UFUNCTION()
    void PerformPeriodicValidation();

    void ValidateSystemActorCounts();
    void ValidateEssentialSystems();
    void ValidatePerformanceMetrics();
    void UpdateHealthStatus();

    FTimerHandle ValidationTimerHandle;
};