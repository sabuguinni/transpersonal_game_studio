#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../NPCBehaviorTypes.h"
#include "TribalNPCController.generated.h"

class UNPC_TribalNPC;
class UNPC_SurvivalNeedsComponent;
class UNPC_SocialBehaviorComponent;

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Child       UMETA(DisplayName = "Child"),
    Crafter     UMETA(DisplayName = "Crafter")
};

UENUM(BlueprintType)
enum class ENPC_TribalBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Resting         UMETA(DisplayName = "Resting"),
    Crafting        UMETA(DisplayName = "Crafting"),
    AlertToThreat   UMETA(DisplayName = "Alert to Threat")
};

USTRUCT(BlueprintType)
struct FNPC_TribalPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Courage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    FNPC_TribalPersonality()
    {
        Courage = 0.5f;
        Sociability = 0.5f;
        Curiosity = 0.5f;
        Aggression = 0.3f;
        Intelligence = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Threat")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Threat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Threat")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Threat")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Threat")
    bool bIsActiveThreat = false;

    FNPC_ThreatMemory()
    {
        ThreatActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = 0.0f;
        TimeSinceLastSeen = 0.0f;
        bIsActiveThreat = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_TribalNPCController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_TribalNPCController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIHearingConfig* HearingConfig;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* TribalBehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardComponent* BlackboardComponent;

    // NPC Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    ENPC_TribalRole TribalRole = ENPC_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    FNPC_TribalPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Config")
    float FleeDistance = 2000.0f;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ENPC_TribalBehaviorState CurrentBehaviorState = ENPC_TribalBehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    TArray<FNPC_ThreatMemory> ThreatMemories;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentFearLevel = 0.0f;

    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float StateUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float ThreatMemoryDuration = 30.0f;

    float LastStateUpdateTime = 0.0f;

public:
    // Behavior State Management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_TribalBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "NPC Behavior")
    ENPC_TribalBehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorState();

    // Threat Management
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void UpdateThreatMemories(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Threat Management")
    bool HasActiveThreat() const;

    UFUNCTION(BlueprintPure, Category = "Threat Management")
    AActor* GetNearestThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void ClearThreatMemory(AActor* ThreatActor);

    // Role-Specific Behavior
    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    void ExecuteRoleSpecificBehavior();

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    bool ShouldFleeFromThreat(AActor* ThreatActor) const;

    UFUNCTION(BlueprintCallable, Category = "Role Behavior")
    FVector GetOptimalFleeDirection(AActor* ThreatActor) const;

    // Social Behavior
    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void RespondToDistressCall(AActor* CallingNPC);

    UFUNCTION(BlueprintPure, Category = "Social Behavior")
    TArray<ANPC_TribalNPCController*> GetNearbyTribalNPCs(float SearchRadius = 1000.0f) const;

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Utility")
    float CalculateFearLevel() const;

    UFUNCTION(BlueprintPure, Category = "Utility")
    bool IsInHomeTerritory() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetHomeLocation(const FVector& NewHomeLocation);

protected:
    // AI Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Behavior Logic
    void InitializeAIPerception();
    void InitializeBehaviorTree();
    void UpdateBlackboardValues();
    
    bool IsDinosaur(AActor* Actor) const;
    bool IsPlayer(AActor* Actor) const;
    bool IsHostileNPC(AActor* Actor) const;
    
    float CalculateThreatLevel(AActor* ThreatActor) const;
    void ProcessThreatDetection(AActor* DetectedActor);
    
    // Role-specific behavior implementations
    void ExecuteHunterBehavior();
    void ExecuteGathererBehavior();
    void ExecuteElderBehavior();
    void ExecuteScoutBehavior();
    void ExecuteChildBehavior();
    void ExecuteCrafterBehavior();
};