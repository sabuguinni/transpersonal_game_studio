#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Quest_AdvancedQuestIntegration.generated.h"

UENUM(BlueprintType)
enum class EQuest_IntegrationType : uint8
{
    CrowdObservation,
    SurvivalChallenge,
    TerritoryExploration,
    ResourceManagement,
    DinosaurEncounter,
    EnvironmentalEvent
};

USTRUCT(BlueprintType)
struct FQuest_IntegrationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IntegrationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_IntegrationType IntegrationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ActivationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FQuest_IntegrationData()
    {
        IntegrationName = TEXT("DefaultIntegration");
        IntegrationType = EQuest_IntegrationType::SurvivalChallenge;
        TargetLocation = FVector::ZeroVector;
        ActivationRadius = 500.0f;
        RequiredCrowdSize = 0;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct FQuest_ProgressionMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CrowdInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SurvivalChallengesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSpentInQuest;

    FQuest_ProgressionMetrics()
    {
        CompletedObjectives = 0;
        TotalObjectives = 0;
        CompletionPercentage = 0.0f;
        CrowdInteractions = 0;
        SurvivalChallengesCompleted = 0;
        TimeSpentInQuest = 0.0f;
    }
};

/**
 * Advanced Quest Integration System
 * Integrates quest mechanics with crowd simulation, survival systems, and environmental events
 * Provides comprehensive quest progression tracking and dynamic objective management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_AdvancedQuestIntegration : public AActor
{
    GENERATED_BODY()

public:
    AQuest_AdvancedQuestIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* IntegrationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* ActivationSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Integration")
    TArray<FQuest_IntegrationData> IntegrationPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Integration")
    FQuest_ProgressionMetrics ProgressionMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Integration")
    bool bEnableCrowdIntegration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Integration")
    bool bEnableEnvironmentalEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Integration")
    float IntegrationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Integration")
    int32 MaxActiveIntegrations;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void InitializeQuestIntegration();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void AddIntegrationPoint(const FQuest_IntegrationData& NewIntegration);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void RemoveIntegrationPoint(const FString& IntegrationName);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void UpdateProgressionMetrics();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    bool ActivateIntegration(const FString& IntegrationName);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void DeactivateIntegration(const FString& IntegrationName);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    TArray<FQuest_IntegrationData> GetActiveIntegrations() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    FQuest_ProgressionMetrics GetProgressionMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void ProcessCrowdIntegration(int32 CrowdSize, const FVector& CrowdLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void ProcessEnvironmentalEvent(const FString& EventType, const FVector& EventLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void ValidateQuestObjectives();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void ResetIntegrationSystem();

protected:
    UFUNCTION()
    void OnActivationSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
                                       AActor* OtherActor, 
                                       UPrimitiveComponent* OtherComp, 
                                       int32 OtherBodyIndex, 
                                       bool bFromSweep, 
                                       const FHitResult& SweepResult);

    UFUNCTION()
    void OnActivationSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, 
                                     AActor* OtherActor, 
                                     UPrimitiveComponent* OtherComp, 
                                     int32 OtherBodyIndex);

private:
    float LastUpdateTime;
    TArray<AActor*> OverlappingActors;
    
    void UpdateIntegrationStates();
    void ProcessActiveIntegrations();
    bool ValidateIntegrationRequirements(const FQuest_IntegrationData& Integration);
    void LogIntegrationActivity(const FString& Activity);
};