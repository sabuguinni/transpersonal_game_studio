#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "NPC_AdvancedTribalController.generated.h"

class ANPC_PrimitiveHumanActor;
class UNPC_TribalBehaviorComponent;
class UNPCMemorySystem;

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Chief UMETA(DisplayName = "Chief"),
    Hunter UMETA(DisplayName = "Hunter"),
    Gatherer UMETA(DisplayName = "Gatherer"),
    Crafter UMETA(DisplayName = "Crafter"),
    Scout UMETA(DisplayName = "Scout"),
    Elder UMETA(DisplayName = "Elder"),
    Warrior UMETA(DisplayName = "Warrior"),
    Shaman UMETA(DisplayName = "Shaman")
};

UENUM(BlueprintType)
enum class ENPC_TribalActivity : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Gathering UMETA(DisplayName = "Gathering"),
    Crafting UMETA(DisplayName = "Crafting"),
    SocialInteraction UMETA(DisplayName = "Social Interaction"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Eating UMETA(DisplayName = "Eating"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Fighting UMETA(DisplayName = "Fighting")
};

USTRUCT(BlueprintType)
struct FNPC_TribalDecision
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision")
    ENPC_TribalActivity Activity = ENPC_TribalActivity::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision")
    float Priority = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision")
    float Duration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision")
    bool bRequiresGroup = false;

    FNPC_TribalDecision()
    {
        Activity = ENPC_TribalActivity::Idle;
        Priority = 0.0f;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        Duration = 0.0f;
        bRequiresGroup = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownResourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownDangerZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownTribalMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastMealTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastRestTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation = FVector::ZeroVector;

    FNPC_TribalMemory()
    {
        KnownResourceLocations.Empty();
        KnownDangerZones.Empty();
        KnownTribalMembers.Empty();
        KnownThreats.Empty();
        LastMealTime = 0.0f;
        LastRestTime = 0.0f;
        HomeLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_AdvancedTribalController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_AdvancedTribalController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Tribal Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    ENPC_TribalRole TribalRole = ENPC_TribalRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    ENPC_TribalActivity CurrentActivity = ENPC_TribalActivity::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FNPC_TribalMemory TribalMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    TArray<FNPC_TribalDecision> DecisionQueue;

    // Behavior Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FatigueLevel = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialNeed = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DecisionUpdateInterval = 2.0f;

    // Perception Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float DangerDetectionRange = 1000.0f;

    // Territory Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bStayInTerritory = true;

public:
    // Decision Making
    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    void UpdateDecisionMaking();

    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    FNPC_TribalDecision EvaluateBestAction();

    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    void ExecuteDecision(const FNPC_TribalDecision& Decision);

    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    float CalculateActionPriority(ENPC_TribalActivity Activity);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void UpdateMemory();

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void RememberLocation(FVector Location, bool bIsDangerous = false);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void RememberActor(AActor* Actor, bool bIsThreat = false);

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    FVector FindNearestKnownResource();

    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    bool IsLocationDangerous(FVector Location);

    // Social Behavior
    UFUNCTION(BlueprintCallable, Category = "AI Social")
    void UpdateSocialBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Social")
    TArray<AActor*> FindNearbyTribalMembers(float SearchRadius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "AI Social")
    void InitiateSocialInteraction(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "AI Social")
    bool ShouldFollowLeader();

    // Role-Specific Behavior
    UFUNCTION(BlueprintCallable, Category = "AI Role")
    void UpdateRoleSpecificBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Role")
    void ExecuteHunterBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Role")
    void ExecuteGathererBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Role")
    void ExecuteScoutBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Role")
    void ExecuteChiefBehavior();

    // Survival Needs
    UFUNCTION(BlueprintCallable, Category = "AI Survival")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI Survival")
    bool NeedsFood();

    UFUNCTION(BlueprintCallable, Category = "AI Survival")
    bool NeedsRest();

    UFUNCTION(BlueprintCallable, Category = "AI Survival")
    bool InDanger();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    void SetTerritoryCenter(FVector NewCenter);

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    bool IsInTerritory(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    FVector GetRandomLocationInTerritory();

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal Timers
    float LastDecisionUpdate = 0.0f;
    float LastMemoryUpdate = 0.0f;
    float LastSocialUpdate = 0.0f;

    // References
    ANPC_PrimitiveHumanActor* ControlledTribalMember = nullptr;
    UNPC_TribalBehaviorComponent* TribalBehaviorComponent = nullptr;
};