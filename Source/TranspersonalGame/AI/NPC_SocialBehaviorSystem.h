#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SharedTypes.h"
#include "NPC_SocialBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_SocialRank : uint8
{
    Child = 1,
    Gatherer = 2,
    Hunter = 3,
    Shaman = 4,
    Leader = 5
};

UENUM(BlueprintType)
enum class ENPC_SocialInteraction : uint8
{
    Greeting,
    Warning,
    Trading,
    Sharing,
    Conflict,
    Cooperation
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<APawn> TargetNPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float TrustLevel = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 InteractionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime = 0.0f;

    FNPC_SocialRelationship()
    {
        TrustLevel = 50.0f;
        FearLevel = 0.0f;
        InteractionCount = 0;
        LastInteractionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_SocialInteraction LastInteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastSeenLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TimeStamp = 0.0f;

    FNPC_SocialMemory()
    {
        LastInteractionType = ENPC_SocialInteraction::Greeting;
        LastSeenLocation = FVector::ZeroVector;
        MemoryStrength = 1.0f;
        TimeStamp = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    ENPC_SocialRank SocialRank = ENPC_SocialRank::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float SocialRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float AggressionLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float CooperationLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    bool bIsTribalMember = true;

    // Social Relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    TArray<FNPC_SocialRelationship> KnownNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    TMap<TWeakObjectPtr<APawn>, FNPC_SocialMemory> SocialMemories;

    // Tribal Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FVector TribalTerritory = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    TWeakObjectPtr<APawn> TribalLeader;

    // Social Interaction Methods
    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void InitiateSocialInteraction(APawn* TargetNPC, ENPC_SocialInteraction InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void UpdateRelationship(APawn* TargetNPC, float TrustChange, float FearChange);

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    FNPC_SocialRelationship* GetRelationship(APawn* TargetNPC);

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    TArray<APawn*> FindNearbyNPCs();

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    bool IsInTribalTerritory(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void SetTribalLeader(APawn* Leader);

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberInteraction(APawn* TargetNPC, ENPC_SocialInteraction InteractionType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_SocialMemory* GetSocialMemory(APawn* TargetNPC);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetOldMemories(float MaxAge = 300.0f);

    // Behavior Decision Making
    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    ENPC_SocialInteraction DecideInteractionType(APawn* TargetNPC);

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldApproach(APawn* TargetNPC);

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldFlee(APawn* TargetNPC);

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    FVector GetSafeLocation();

private:
    // Internal state
    float LastSocialUpdate = 0.0f;
    float SocialUpdateInterval = 1.0f;
    
    // Helper methods
    void UpdateSocialAwareness();
    void ProcessNearbyNPCs();
    void UpdateTribalBehavior();
    float CalculateRelationshipScore(APawn* TargetNPC);
    bool IsHostileTowards(APawn* TargetNPC);
};