#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NPCBehaviorComponent.generated.h"

// NPC Memory System - tracks relationships and experiences
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RememberedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Relationship; // -100 to 100 (enemy to friend)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastSeenLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsHostile;

    FNPC_Memory()
    {
        RememberedActor = nullptr;
        Relationship = 0.0f;
        LastSeenLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        bIsHostile = false;
    }
};

// NPC Emotional State - affects behavior decisions
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_EmotionalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float Fear; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float Anger; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float Curiosity; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float Alertness; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float SocialNeed; // 0-100

    FNPC_EmotionalState()
    {
        Fear = 20.0f;
        Anger = 10.0f;
        Curiosity = 30.0f;
        Alertness = 50.0f;
        SocialNeed = 40.0f;
    }
};

// Daily routine activities
UENUM(BlueprintType)
enum class ENPC_Activity : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Foraging UMETA(DisplayName = "Foraging"),
    Socializing UMETA(DisplayName = "Socializing"),
    Resting UMETA(DisplayName = "Resting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Investigating UMETA(DisplayName = "Investigating"),
    Working UMETA(DisplayName = "Working"),
    Hunting UMETA(DisplayName = "Hunting")
};

// NPC social roles in tribe
UENUM(BlueprintType)
enum class ENPC_Role : uint8
{
    Hunter UMETA(DisplayName = "Hunter"),
    Gatherer UMETA(DisplayName = "Gatherer"),
    Guard UMETA(DisplayName = "Guard"),
    Elder UMETA(DisplayName = "Elder"),
    Child UMETA(DisplayName = "Child"),
    Crafter UMETA(DisplayName = "Crafter"),
    Scout UMETA(DisplayName = "Scout"),
    Shaman UMETA(DisplayName = "Shaman")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE NPC PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_Role NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    float Experience; // 0-100

    // === MEMORY SYSTEM ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // === EMOTIONAL STATE ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Emotions")
    FNPC_EmotionalState CurrentEmotions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionDecayRate;

    // === BEHAVIOR STATE ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_Activity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ActivityTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius;

    // === SOCIAL BEHAVIOR ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AActor*> TribeMates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionRange;

    // === SURVIVAL INSTINCTS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float DangerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float FleeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    TArray<TSubclassOf<AActor>> ThreatClasses;

    // === BEHAVIOR FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberActor(AActor* Actor, float RelationshipValue, bool bIsHostile = false);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_Memory* FindMemory(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemory(AActor* Actor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetOldMemories();

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void ModifyEmotion(const FString& EmotionName, float Change);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void UpdateEmotionalState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetActivity(ENPC_Activity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    AActor* FindNearestTribeMate();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void InitiateSocialInteraction(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool IsInSocialRange(AActor* Target);

    // === AI INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardValues();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ReactToPerception(AActor* PerceivedActor, bool bCanSee);

private:
    // Internal state tracking
    float LastEmotionUpdate;
    float LastMemoryCleanup;
    
    // Helper functions
    void DecayEmotions(float DeltaTime);
    void CleanupMemories();
    bool IsActorThreat(AActor* Actor);
    float CalculateRelationshipChange(AActor* Actor);
};