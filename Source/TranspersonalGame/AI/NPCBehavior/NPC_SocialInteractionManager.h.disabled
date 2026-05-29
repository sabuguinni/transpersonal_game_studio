#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NPC_SocialInteractionManager.generated.h"

class ANPC_TribalCharacter;

UENUM(BlueprintType)
enum class ENPC_SocialRelationType : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Friendly    UMETA(DisplayName = "Friendly"),
    Hostile     UMETA(DisplayName = "Hostile"),
    Romantic    UMETA(DisplayName = "Romantic"),
    Family      UMETA(DisplayName = "Family"),
    Rival       UMETA(DisplayName = "Rival")
};

UENUM(BlueprintType)
enum class ENPC_SocialAction : uint8
{
    Greet       UMETA(DisplayName = "Greet"),
    Talk        UMETA(DisplayName = "Talk"),
    Trade       UMETA(DisplayName = "Trade"),
    Fight       UMETA(DisplayName = "Fight"),
    Help        UMETA(DisplayName = "Help"),
    Avoid       UMETA(DisplayName = "Avoid"),
    Follow      UMETA(DisplayName = "Follow"),
    Ignore      UMETA(DisplayName = "Ignore")
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ENPC_SocialRelationType RelationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 InteractionCount;

    FNPC_SocialRelation()
    {
        TargetActor = nullptr;
        RelationType = ENPC_SocialRelationType::Neutral;
        RelationshipStrength = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_SocialAction LastAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsVisible;

    FNPC_SocialMemory()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        LastAction = ENPC_SocialAction::Ignore;
        bIsVisible = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialInteractionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialInteractionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social Relations Management
    UFUNCTION(BlueprintCallable, Category = "Social")
    void AddSocialRelation(AActor* TargetActor, ENPC_SocialRelationType RelationType, float InitialStrength = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateRelationship(AActor* TargetActor, float StrengthChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_SocialRelation GetRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool HasRelationship(AActor* TargetActor);

    // Social Actions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void PerformSocialAction(AActor* TargetActor, ENPC_SocialAction Action);

    UFUNCTION(BlueprintCallable, Category = "Social")
    ENPC_SocialAction DecideSocialAction(AActor* TargetActor);

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateSocialMemory(AActor* Actor, const FVector& Location, ENPC_SocialAction Action);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_SocialMemory GetSocialMemory(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetActor(AActor* Actor);

    // Perception
    UFUNCTION(BlueprintCallable, Category = "Perception")
    TArray<AActor*> GetNearbyNPCs(float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    bool CanSeeActor(AActor* TargetActor);

    // Group Behavior
    UFUNCTION(BlueprintCallable, Category = "Group")
    void JoinGroup(const FString& GroupName);

    UFUNCTION(BlueprintCallable, Category = "Group")
    void LeaveGroup();

    UFUNCTION(BlueprintCallable, Category = "Group")
    TArray<AActor*> GetGroupMembers();

protected:
    // Social Relations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_SocialRelation> SocialRelations;

    // Social Memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory", meta = (AllowPrivateAccess = "true"))
    TArray<FNPC_SocialMemory> SocialMemories;

    // Group Information
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group", meta = (AllowPrivateAccess = "true"))
    FString CurrentGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group", meta = (AllowPrivateAccess = "true"))
    bool bIsGroupLeader;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float PerceptionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float SocialUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (AllowPrivateAccess = "true"))
    int32 MaxSocialMemories;

private:
    float LastSocialUpdate;

    // Helper Functions
    void UpdateSocialRelations(float DeltaTime);
    void DecayMemories(float DeltaTime);
    void ProcessNearbyActors();
    float CalculateRelationshipInfluence(ENPC_SocialRelationType RelationType, float Strength);
};