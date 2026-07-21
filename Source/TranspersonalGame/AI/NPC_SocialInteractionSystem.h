#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "NPC_SocialInteractionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipValue = 0.0f; // -1.0 (hostile) to 1.0 (friendly)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ENPC_SocialRelationType RelationType = ENPC_SocialRelationType::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 InteractionCount = 0;

    FNPC_SocialRelationship()
    {
        TargetActor = nullptr;
        RelationshipValue = 0.0f;
        RelationType = ENPC_SocialRelationType::Neutral;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastSeenLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_NPCBehaviorState LastObservedBehavior = ENPC_NPCBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsAlly = false;

    FNPC_SocialMemory()
    {
        LastSeenLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        LastObservedBehavior = ENPC_NPCBehaviorState::Idle;
        ThreatLevel = 0.0f;
        bIsAlly = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialInteractionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Config")
    float SocialInteractionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Config")
    float MemoryDecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Config")
    float RelationshipDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Config")
    int32 MaxSocialMemories = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Config")
    bool bEnablePackBehavior = true;

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void InitiateSocialInteraction(AActor* TargetActor, ENPC_SocialInteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    FNPC_SocialRelationship GetRelationshipWith(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void UpdateRelationship(AActor* TargetActor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    TArray<AActor*> GetAlliesInRadius(float SearchRadius = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    TArray<AActor*> GetEnemiesInRadius(float SearchRadius = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void AddSocialMemory(AActor* TargetActor, const FNPC_SocialMemory& Memory);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    FNPC_SocialMemory GetSocialMemory(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    bool HasMemoryOf(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void ForgetActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    void BroadcastAlertToAllies(FVector ThreatLocation, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    bool IsInPack();

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    AActor* GetPackLeader();

    UFUNCTION(BlueprintCallable, Category = "Social Interaction")
    TArray<AActor*> GetPackMembers();

protected:
    UPROPERTY()
    TMap<AActor*, FNPC_SocialRelationship> SocialRelationships;

    UPROPERTY()
    TMap<AActor*, FNPC_SocialMemory> SocialMemories;

    UPROPERTY()
    AActor* CurrentPackLeader;

    UPROPERTY()
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Config", meta = (AllowPrivateAccess = "true"))
    float SocialUpdateInterval = 1.0f;

    UPROPERTY()
    float LastSocialUpdate = 0.0f;

private:
    void UpdateSocialRelationships(float DeltaTime);
    void DecayMemories(float DeltaTime);
    void ScanForNearbyActors();
    void UpdatePackStatus();
    ENPC_SocialRelationType DetermineRelationType(float RelationshipValue);
    void ProcessSocialInteraction(AActor* TargetActor, ENPC_SocialInteractionType InteractionType);
    bool IsValidSocialTarget(AActor* TargetActor);
};