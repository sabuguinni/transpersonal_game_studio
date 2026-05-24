#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Pawn.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedTypes.h"
#include "NPC_AdvancedAIController.generated.h"

class UNPC_DinosaurBehaviorComponent;
class UNPC_PackBehaviorComponent;
class UNPC_TerritorialBehaviorSystem;

UENUM(BlueprintType)
enum class ENPC_AIState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Territorial UMETA(DisplayName = "Territorial"),
    PackHunting UMETA(DisplayName = "Pack Hunting"),
    Feeding UMETA(DisplayName = "Feeding"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Migrating UMETA(DisplayName = "Migrating"),
    Mating UMETA(DisplayName = "Mating")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None UMETA(DisplayName = "No Threat"),
    Low UMETA(DisplayName = "Low Threat"),
    Medium UMETA(DisplayName = "Medium Threat"),
    High UMETA(DisplayName = "High Threat"),
    Critical UMETA(DisplayName = "Critical Threat")
};

USTRUCT(BlueprintType)
struct FNPC_AIMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    ENPC_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float MemoryStrength;

    FNPC_AIMemoryEntry()
    {
        TargetActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = ENPC_ThreatLevel::None;
        LastSeenTime = 0.0f;
        MemoryStrength = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_AIBehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    float FearThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    bool bIsPackAnimal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    bool bIsTerritorial;

    FNPC_AIBehaviorConfig()
    {
        SightRadius = 2000.0f;
        HearingRadius = 1500.0f;
        PatrolRadius = 1000.0f;
        TerritoryRadius = 2000.0f;
        AggressionLevel = 0.5f;
        FearThreshold = 0.7f;
        bIsPackAnimal = false;
        bIsTerritorial = true;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API ANPC_AdvancedAIController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_AdvancedAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior Tree
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    UBlackboardComponent* BlackboardComponent;

    // AI State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    ENPC_AIState CurrentAIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    ENPC_AIState PreviousAIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Config")
    FNPC_AIBehaviorConfig BehaviorConfig;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    TArray<FNPC_AIMemoryEntry> AIMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Memory")
    float MemoryDecayRate;

    // Behavior Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UNPC_DinosaurBehaviorComponent* DinosaurBehaviorComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UNPC_PackBehaviorComponent* PackBehaviorComponent;

    // Territory Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Territory")
    TArray<AActor*> TerritoryIntruders;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "AI State")
    void SetAIState(ENPC_AIState NewState);

    UFUNCTION(BlueprintPure, Category = "AI State")
    ENPC_AIState GetCurrentAIState() const { return CurrentAIState; }

    UFUNCTION(BlueprintCallable, Category = "AI State")
    bool CanTransitionToState(ENPC_AIState TargetState) const;

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void AddMemoryEntry(AActor* Actor, ENPC_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void UpdateMemoryEntry(AActor* Actor, const FVector& NewLocation);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    FNPC_AIMemoryEntry* FindMemoryEntry(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void DecayMemory(float DeltaTime);

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "AI Territory")
    void SetTerritory(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "AI Territory")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "AI Territory")
    void CheckTerritoryIntrusion();

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "AI Threat")
    ENPC_ThreatLevel AssessThreatLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "AI Threat")
    AActor* GetHighestThreatTarget() const;

    // Behavior Execution
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ExecuteIdleBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ExecutePatrolBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ExecuteHuntingBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ExecuteTerritorialBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ExecuteFleeingBehavior();

protected:
    // Internal state management
    void UpdateAIState(float DeltaTime);
    void ProcessPerceptionData();
    void UpdateBlackboard();
    
    // Behavior state timers
    float StateChangeTimer;
    float LastPerceptionUpdate;
    float TerritoryCheckTimer;
};