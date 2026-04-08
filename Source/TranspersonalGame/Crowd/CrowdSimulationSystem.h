#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../Core/ConsciousnessSystem.h"
#include "CrowdSimulationSystem.generated.h"

class ACrowdAgent;
class UConsciousnessComponent;

UENUM(BlueprintType)
enum class ECrowdBehaviorState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Dispersing      UMETA(DisplayName = "Dispersing"),
    Following       UMETA(DisplayName = "Following"),
    Panicking       UMETA(DisplayName = "Panicking"),
    Meditating      UMETA(DisplayName = "Meditating"),
    Celebrating     UMETA(DisplayName = "Celebrating"),
    Protesting      UMETA(DisplayName = "Protesting")
};

USTRUCT(BlueprintType)
struct FCrowdFlowData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FlowDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FlowIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Density;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowdBehaviorState BehaviorState;

    FCrowdFlowData()
    {
        FlowDirection = FVector::ZeroVector;
        FlowIntensity = 0.0f;
        Density = 0.0f;
        BehaviorState = ECrowdBehaviorState::Neutral;
    }
};

USTRUCT(BlueprintType)
struct FCollectiveConsciousnessField
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessLevel DominantLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EConsciousnessLevel, float> LevelDistribution;

    FCollectiveConsciousnessField()
    {
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        Intensity = 1.0f;
        DominantLevel = EConsciousnessLevel::Rational;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Crowd Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxCrowdAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float AgentInteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float ConsciousnessInfluenceRadius;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ConsciousnessWeight;

    // Consciousness Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float CollectiveThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float EmergenceRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    bool bEnableCollectiveConsciousness;

    // Runtime Data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<ACrowdAgent*> CrowdAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<FCollectiveConsciousnessField> ConsciousnessFields;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    FCrowdFlowData CurrentFlowData;

public:
    // Crowd Management Functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SpawnCrowdAgents(int32 Count, FVector SpawnCenter);

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void DespawnAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd Management")
    void SetCrowdBehaviorState(ECrowdBehaviorState NewState);

    // Flow Control
    UFUNCTION(BlueprintCallable, Category = "Flow Control")
    void SetFlowDirection(FVector Direction, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Flow Control")
    void CreateFlowField(FVector Start, FVector End, float Width);

    UFUNCTION(BlueprintCallable, Category = "Flow Control")
    FCrowdFlowData GetFlowDataAtLocation(FVector Location);

    // Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void CreateCollectiveConsciousnessField(FVector Center, float Radius, EConsciousnessLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void UpdateCollectiveConsciousness(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    EConsciousnessLevel GetDominantConsciousnessLevel(FVector Location);

    // Event Responses
    UFUNCTION(BlueprintCallable, Category = "Events")
    void OnPlayerConsciousnessChanged(EConsciousnessLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void OnCombatStarted(FVector CombatLocation);

    UFUNCTION(BlueprintCallable, Category = "Events")
    void OnMeditationEvent(FVector MeditationCenter, float Radius);

    // Analytics
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    float GetCrowdDensity(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    FVector GetAverageMovementDirection(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    TMap<EConsciousnessLevel, float> GetConsciousnessDistribution();

private:
    void UpdateCrowdBehavior(float DeltaTime);
    void UpdateFlowFields(float DeltaTime);
    void ProcessConsciousnessInfluence(float DeltaTime);
    void HandleEmergentBehaviors(float DeltaTime);
    
    FVector CalculateSeparation(ACrowdAgent* Agent);
    FVector CalculateAlignment(ACrowdAgent* Agent);
    FVector CalculateCohesion(ACrowdAgent* Agent);
    FVector CalculateConsciousnessInfluence(ACrowdAgent* Agent);

    void SpawnSingleAgent(FVector Location);
    void CleanupInvalidAgents();

    // Spatial partitioning for performance
    TMap<FIntVector, TArray<ACrowdAgent*>> SpatialGrid;
    void UpdateSpatialGrid();
    TArray<ACrowdAgent*> GetNearbyAgents(FVector Location, float Radius);

    float LastUpdateTime;
    int32 CurrentAgentCount;
};