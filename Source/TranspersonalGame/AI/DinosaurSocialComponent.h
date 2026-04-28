#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
// DISABLED: // DISABLED: #include "DinosaurAIController.h"
#include "DinosaurSocialComponent.generated.h"

UENUM(BlueprintType)
enum class EAI_SocialInteractionType : uint8
{
    Greeting,
    Dominance,
    Submission,
    PlayFighting,
    Grooming,
    Mating,
    Territorial,
    Feeding,
    Warning,
    Comfort
};

UENUM(BlueprintType)
enum class ESocialRelationship : uint8
{
    Unknown,
    Neutral,
    Friendly,
    Hostile,
    Dominant,
    Submissive,
    Mate,
    Offspring,
    Parent,
    PackMember
};

USTRUCT(BlueprintType)
struct FAI_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<AActor> OtherActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ESocialRelationship RelationshipType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Affinity; // -1.0 (hostile) to 1.0 (friendly)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Dominance; // -1.0 (submissive) to 1.0 (dominant)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Trust; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 InteractionCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<EAI_SocialInteractionType> RecentInteractions;

    FAI_SocialRelationship()
    {
        OtherActor = nullptr;
        RelationshipType = ESocialRelationship::Unknown;
        Affinity = 0.0f;
        Dominance = 0.0f;
        Trust = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

USTRUCT(BlueprintType)
struct FAI_SocialInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    EAI_SocialInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<AActor> Initiator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<AActor> Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Intensity; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bIsSuccessful;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FVector InteractionLocation;

    FAI_SocialInteraction()
    {
        InteractionType = EAI_SocialInteractionType::Greeting;
        Initiator = nullptr;
        Target = nullptr;
        Duration = 0.0f;
        Intensity = 0.5f;
        bIsSuccessful = false;
        InteractionLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FHerdBehavior
{
    GENERATED_BODY()

// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    TArray<TWeakObjectPtr<AActor>> HerdMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    TWeakObjectPtr<AActor> HerdLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector HerdCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float HerdRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float CohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float SeparationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    bool bIsActive;

    FHerdBehavior()
    {
        HerdLeader = nullptr;
        HerdCenter = FVector::ZeroVector;
        HerdRadius = 1000.0f;
        CohesionStrength = 0.5f;
        SeparationDistance = 200.0f;
        bIsActive = false;
    }
};

/**
 * Advanced social interaction system for dinosaur NPCs that handles relationships,
 * herd behavior, dominance hierarchies, and complex social dynamics
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurSocialComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurSocialComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FAI_SocialRelationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FAI_SocialInteraction CurrentInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FHerdBehavior HerdInfo;

    // Social Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float InteractionCooldown = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bCanFormHerds = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bCanBeDominant = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    bool bCanMate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 MaxRelationships = 10;

    // Personality Modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Sociability = 0.5f; // 0.0 = solitary, 1.0 = highly social

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Aggressiveness = 0.3f; // 0.0 = passive, 1.0 = highly aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Dominance = 0.5f; // 0.0 = submissive, 1.0 = dominant

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Empathy = 0.4f; // 0.0 = selfish, 1.0 = highly empathetic

public:
    // Relationship Management
    UFUNCTION(BlueprintCallable, Category = "Social")
    void EstablishRelationship(AActor* OtherActor, ESocialRelationship RelationType = ESocialRelationship::Neutral);

    UFUNCTION(BlueprintPure, Category = "Social")
    FAI_SocialRelationship GetRelationshipWith(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateRelationship(AActor* OtherActor, float AffinityChange, float DominanceChange, float TrustChange);

    UFUNCTION(BlueprintPure, Category = "Social")
    bool HasRelationshipWith(AActor* OtherActor) const;

    UFUNCTION(BlueprintPure, Category = "Social")
    TArray<AActor*> GetFriendlyActors() const;

    UFUNCTION(BlueprintPure, Category = "Social")
    TArray<AActor*> GetHostileActors() const;

    // Interaction System
    UFUNCTION(BlueprintCallable, Category = "Social")
    bool BeginInteraction(AActor* TargetActor, EAI_SocialInteractionType InteractionType = EAI_SocialInteractionType::Greeting);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void EndInteraction(bool bWasSuccessful = true);

    UFUNCTION(BlueprintPure, Category = "Social")
    bool IsInteracting() const;

    UFUNCTION(BlueprintPure, Category = "Social")
    bool CanInteractWith(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    void RespondToInteraction(AActor* Initiator, EAI_SocialInteractionType InteractionType);

    // Herd Behavior
    UFUNCTION(BlueprintCallable, Category = "Social")
    void JoinHerd(const TArray<AActor*>& HerdMembers, AActor* Leader = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void LeaveHerd();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateHerdBehavior();

    UFUNCTION(BlueprintPure, Category = "Social")
    bool IsInHerd() const { return HerdInfo.bIsActive && HerdInfo.HerdMembers.Num() > 1; }

    UFUNCTION(BlueprintPure, Category = "Social")
    FVector GetHerdCenter() const { return HerdInfo.HerdCenter; }

    UFUNCTION(BlueprintPure, Category = "Social")
    AActor* GetHerdLeader() const { return HerdInfo.HerdLeader.Get(); }

    // Social Discovery
    UFUNCTION(BlueprintCallable, Category = "Social")
    AActor* FindNearbyDinosaur(float SearchRadius = 0.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> FindNearbyDinosaurs(float SearchRadius = 0.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    AActor* FindBestSocialTarget() const;

    // Social Behaviors
    UFUNCTION(BlueprintCallable, Category = "Social")
    void InitiateDominanceDisplay(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void InitiatePlayBehavior(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void InitiateComfortBehavior(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void InitiateWarningBehavior(AActor* ThreatActor);

    // Social State Queries
    UFUNCTION(BlueprintPure, Category = "Social")
    float GetSocialNeed() const;

    UFUNCTION(BlueprintPure, Category = "Social")
    bool IsLonely() const;

    UFUNCTION(BlueprintPure, Category = "Social")
    bool IsOverstimulated() const;

    UFUNCTION(BlueprintPure, Category = "Social")
    float GetDominanceRank() const;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionStarted, AActor*, OtherActor, EAI_SocialInteractionType, InteractionType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionEnded, AActor*, OtherActor, bool, bWasSuccessful);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelationshipChanged, AActor*, OtherActor);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinedHerd, int32, HerdSize);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftHerd);

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnInteractionStarted OnInteractionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnInteractionEnded OnInteractionEnded;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnRelationshipChanged OnRelationshipChanged;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnJoinedHerd OnJoinedHerd;

    UPROPERTY(BlueprintAssignable, Category = "Social Events")
    FOnLeftHerd OnLeftHerd;

protected:
    // Internal social logic
    void UpdateRelationships(float DeltaTime);
    void ProcessSocialNeeds();
    void DecayRelationships(float DeltaTime);
    void CleanupInvalidRelationships();
    
    EAI_SocialInteractionType DetermineInteractionType(AActor* OtherActor) const;
    float CalculateInteractionSuccess(EAI_SocialInteractionType InteractionType, AActor* OtherActor) const;
    void ApplyInteractionEffects(EAI_SocialInteractionType InteractionType, AActor* OtherActor, bool bWasSuccessful);
    
    // Herd management
    void UpdateHerdCenter();
    void ValidateHerdMembers();
    bool ShouldJoinHerd(const TArray<AActor*>& PotentialHerd) const;
    
    // Timers and state
    float LastInteractionTime = 0.0f;
    float SocialNeedTimer = 0.0f;
    float RelationshipUpdateTimer = 0.0f;
    
    // Cached references
    class UWorld* CachedWorld;
    class ADinosaurAIController* OwnerController;
    class UDinosaurMemoryComponent* MemoryComponent;
};