#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_SystemIntegrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown,
    Stable,
    NeedsAttention,
    Critical
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Metrics")
    int32 TotalActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Metrics")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Metrics")
    int32 VFXCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Metrics")
    int32 AudioCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Metrics")
    int32 LightingCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Build Metrics")
    float BuildScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Build Metrics")
    EBuild_IntegrationStatus Status = EBuild_IntegrationStatus::Unknown;

    FBuild_SystemMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        VFXCount = 0;
        AudioCount = 0;
        LightingCount = 0;
        BuildScore = 0.0f;
        Status = EBuild_IntegrationStatus::Unknown;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_SystemIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_SystemIntegrator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemMetrics AnalyzeBuildHealth();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void EnforceActorCaps();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetIntegrationStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<AActor*> GetActorsByCategory(const FString& Category);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateBuildReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    FBuild_SystemMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxTotalActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxDinosaurActors = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float IntegrationCheckInterval = 30.0f;

private:
    float LastIntegrationCheck = 0.0f;

    void UpdateMetrics();
    void ValidateModuleDependencies();
    void CheckCrossSystemCompatibility();
    float CalculateBuildScore(const FBuild_SystemMetrics& Metrics);
};