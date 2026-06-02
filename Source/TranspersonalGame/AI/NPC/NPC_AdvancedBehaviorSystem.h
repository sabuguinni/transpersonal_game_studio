#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedTypes.h"
#include "NPC_AdvancedBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorComplexity : uint8
{
    Simple      UMETA(DisplayName = "Simple"),
    Intermediate UMETA(DisplayName = "Intermediate"), 
    Advanced    UMETA(DisplayName = "Advanced"),
    Expert      UMETA(DisplayName = "Expert")
};

UENUM(BlueprintType)
enum class ENPC_SocialRole : uint8
{
    Leader      UMETA(DisplayName = "Leader"),
    Follower    UMETA(DisplayName = "Follower"),
    Scout       UMETA(DisplayName = "Scout"),
    Guardian    UMETA(DisplayName = "Guardian"),
    Loner       UMETA(DisplayName = "Loner")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> ImportantLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownAllies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastThreatTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bPlayerIsKnownThreat;

    FNPC_BehaviorMemory()
    {
        LastThreatTime = 0.0f;
        LastKnownPlayerLocation = FVector::ZeroVector;
        bPlayerIsKnownThreat = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* RelatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsAlly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsRival;

    FNPC_SocialRelationship()
    {
        RelatedActor = nullptr;
        RelationshipStrength = 0.0f;
        LastInteractionTime = 0.0f;
        bIsAlly = false;
        bIsRival = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_AdvancedBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_AdvancedBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorComplexity BehaviorComplexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_SocialRole SocialRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DecisionMakingSpeed;

    // Memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPC_BehaviorMemory BehaviorMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryRetentionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries;

    // Social dynamics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FNPC_SocialRelationship> SocialRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipDecayRate;

    // Behavior tree integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* PrimaryBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* SocialBehaviorTree;

    // Animation integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bUseAdvancedAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float AnimationBlendSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float EmotionalExpressionIntensity;

    // Core behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessEnvironmentalAwareness();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void MakeDecision();

    // Memory functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddThreatToMemory(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddAllyToMemory(AActor* Ally);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdatePlayerMemory(FVector PlayerLocation, bool bIsThreat);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void CleanupOldMemories();

    // Social functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateSocialRelationships();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ProcessSocialInteraction(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    float GetRelationshipStrength(AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyRelationship(AActor* OtherActor, float StrengthChange);

    // AI integration functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardValues();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool ShouldEnterCombatMode();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool ShouldEnterSocialMode();

    // Animation integration functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerEmotionalAnimation(const FString& EmotionType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateAnimationParameters();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void BlendToNewAnimationState(const FString& StateName);

private:
    float LastBehaviorUpdate;
    float LastMemoryCleanup;
    float LastSocialUpdate;
    
    AAIController* CachedAIController;
    UBlackboardComponent* CachedBlackboard;

    void InitializeAIReferences();
    void ProcessComplexBehavior();
    void HandleSocialRoleResponsibilities();
    void UpdateEmotionalState();
};