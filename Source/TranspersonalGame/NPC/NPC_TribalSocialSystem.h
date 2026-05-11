#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "NPCBehaviorTypes.h"
#include "NPC_TribalSocialSystem.generated.h"

// Forward declarations
class ANPC_TribalMember;
class UNPC_TribalBehaviorManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipStrength = 0.0f; // -100 to 100 (hate to love)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Trust = 50.0f; // 0 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float Respect = 50.0f; // 0 to 100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime = 0.0f;

    FNPC_SocialRelationship()
    {
        RelationshipStrength = 0.0f;
        Trust = 50.0f;
        Respect = 50.0f;
        LastInteractionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_TribalRole
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FString RoleName = TEXT("Gatherer");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    int32 RolePriority = 1; // 1-10, higher = more important

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    TArray<FString> ResponsibleTasks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    float Authority = 0.0f; // 0-100, how much others listen to this NPC

    FNPC_TribalRole()
    {
        RoleName = TEXT("Gatherer");
        RolePriority = 1;
        Authority = 0.0f;
        ResponsibleTasks.Add(TEXT("Collect berries"));
        ResponsibleTasks.Add(TEXT("Find water"));
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastSeenLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString LastAction = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bWasThreatening = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bWasHelpful = false;

    FNPC_SocialMemory()
    {
        LastSeenLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        LastAction = TEXT("Unknown");
        bWasThreatening = false;
        bWasHelpful = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalSocialSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalSocialSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social relationship management
    UFUNCTION(BlueprintCallable, Category = "Social")
    void AddRelationship(AActor* TargetActor, float InitialStrength = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyRelationship(AActor* TargetActor, float StrengthDelta, float TrustDelta = 0.0f, float RespectDelta = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_SocialRelationship GetRelationship(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> GetAllKnownActors() const;

    // Tribal role system
    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void SetTribalRole(const FNPC_TribalRole& NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    FNPC_TribalRole GetTribalRole() const { return CurrentRole; }

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    void AddTaskToRole(const FString& TaskName);

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    bool CanPerformTask(const FString& TaskName) const;

    // Social memory system
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RecordSocialInteraction(AActor* TargetActor, const FString& Action, bool bWasThreatening = false, bool bWasHelpful = false);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_SocialMemory GetSocialMemory(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetOldMemories(float MaxAge = 300.0f); // Forget memories older than 5 minutes

    // Tribal hierarchy
    UFUNCTION(BlueprintCallable, Category = "Tribal")
    float GetAuthorityLevel() const { return CurrentRole.Authority; }

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    bool ShouldObeyActor(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal")
    AActor* FindTribalLeader() const;

    // Social decision making
    UFUNCTION(BlueprintCallable, Category = "Social")
    bool ShouldApproachActor(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool ShouldAvoidActor(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    float GetSocialComfort(AActor* TargetActor) const;

    // Group behavior
    UFUNCTION(BlueprintCallable, Category = "Group")
    TArray<AActor*> GetNearbyTribalMembers(float Radius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Group")
    void CallForHelp(const FVector& DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Group")
    void ShareInformation(const FString& Information, float Radius = 500.0f);

protected:
    // Social relationships with other NPCs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    TMap<TWeakObjectPtr<AActor>, FNPC_SocialRelationship> SocialRelationships;

    // Current tribal role
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FNPC_TribalRole CurrentRole;

    // Social memory of interactions
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TMap<TWeakObjectPtr<AActor>, FNPC_SocialMemory> SocialMemories;

    // Tribal group identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    FString TribalGroupName = TEXT("DefaultTribe");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal")
    int32 TribalGroupID = 0;

    // Social behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Sociability = 50.0f; // How much this NPC likes being around others

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Aggression = 20.0f; // How likely to start conflicts

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Empathy = 60.0f; // How much this NPC cares about others

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Leadership = 30.0f; // Natural leadership tendency

private:
    // Helper functions
    void UpdateRelationshipDecay(float DeltaTime);
    void ProcessSocialInteractions();
    bool IsActorInSameTribe(AActor* TargetActor) const;
    float CalculateSocialDistance(AActor* TargetActor) const;
    void BroadcastSocialEvent(const FString& EventType, AActor* TargetActor = nullptr);

    // Timers
    float LastSocialUpdate = 0.0f;
    float SocialUpdateInterval = 2.0f; // Update social state every 2 seconds
};