#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Quest_CrowdNavigationMission.generated.h"

UENUM(BlueprintType)
enum class EQuest_CrowdNavigationType : uint8
{
    Stealth,        // Navigate without being detected
    Following,      // Follow specific NPCs through crowds
    Infiltration,   // Blend into crowds to reach objectives
    Escape,         // Use crowds to escape pursuit
    Observation     // Observe targets within crowds
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_CrowdNavigationObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    EQuest_CrowdNavigationType NavigationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float CompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    bool bRequiresCrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    int32 MinimumCrowdSize;

    FQuest_CrowdNavigationObjective()
    {
        TargetLocation = FVector::ZeroVector;
        NavigationType = EQuest_CrowdNavigationType::Stealth;
        DetectionRadius = 500.0f;
        CompletionRadius = 100.0f;
        bRequiresCrowdDensity = true;
        MinimumCrowdSize = 10;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_CrowdNavigationMission : public AActor
{
    GENERATED_BODY()

public:
    AQuest_CrowdNavigationMission();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Visual Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MissionMarker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CompletionSphere;

    // Mission Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FQuest_CrowdNavigationObjective CurrentObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    TArray<FQuest_CrowdNavigationObjective> NavigationObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bMissionActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bPlayerDetected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float DetectionCooldown;

    // Crowd Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<AActor*> NearbyCrowdActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 CurrentCrowdDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CrowdDensityCheckRadius;

    // Mission Functions
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void StartCrowdNavigationMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SetNextObjective();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool CheckObjectiveCompletion();

    // Crowd Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdDensity();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    bool IsPlayerInCrowd();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void CheckPlayerDetection();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<AActor*> GetNearbyCrowdActors(float SearchRadius);

    // Navigation Functions
    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void UpdateNavigationHints();

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    FVector GetOptimalCrowdPath();

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    bool ValidateNavigationRoute();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnMissionStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnObjectiveCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnPlayerDetected();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnMissionCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnMissionFailed();

private:
    // Internal tracking
    float DetectionTimer;
    int32 CurrentObjectiveIndex;
    bool bObjectiveCompleted;
    FVector LastPlayerPosition;
    float CrowdDensityUpdateTimer;
};