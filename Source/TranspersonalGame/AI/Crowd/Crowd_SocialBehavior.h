#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_SocialBehavior.generated.h"

UENUM(BlueprintType)
enum class ECrowd_SocialState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Dispersing      UMETA(DisplayName = "Dispersing"),
    Following       UMETA(DisplayName = "Following"),
    Avoiding        UMETA(DisplayName = "Avoiding"),
    Panicking       UMETA(DisplayName = "Panicking"),
    Celebrating     UMETA(DisplayName = "Celebrating"),
    Working         UMETA(DisplayName = "Working")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SocialGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AActor*> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* Leader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    ECrowd_SocialState GroupState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FVector GroupCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float GroupCohesion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float GroupRadius;

    FCrowd_SocialGroup()
    {
        Leader = nullptr;
        GroupState = ECrowd_SocialState::Neutral;
        GroupCenter = FVector::ZeroVector;
        GroupCohesion = 1.0f;
        GroupRadius = 500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SocialInfluence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LeadershipInfluence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float PeerPressure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float ConformityTendency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float IndependenceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float EmotionalContagion;

    FCrowd_SocialInfluence()
    {
        LeadershipInfluence = 0.5f;
        PeerPressure = 0.3f;
        ConformityTendency = 0.7f;
        IndependenceLevel = 0.4f;
        EmotionalContagion = 0.6f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_SocialBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_SocialBehavior();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void InitializeSocialBehavior();

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void UpdateSocialState(ECrowd_SocialState NewState);

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void JoinSocialGroup(const FCrowd_SocialGroup& Group);

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void LeaveSocialGroup();

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    FVector CalculateSocialForce();

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void ReactToNearbyActors(const TArray<AActor*>& NearbyActors);

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    bool ShouldFollowLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    void PropagateEmotionalState(ECrowd_SocialState EmotionalState, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    float CalculateGroupCohesion() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    ECrowd_SocialState CurrentSocialState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    FCrowd_SocialInfluence SocialInfluence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    FCrowd_SocialGroup* AssignedGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float SocialInteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float PersonalSpaceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float GroupFollowingStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    float EmotionalSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    bool bIsGroupLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Behavior")
    TArray<AActor*> NearbyActors;

private:
    float StateChangeTimer;
    ECrowd_SocialState PreviousState;
    
    void UpdateNearbyActors();
    FVector CalculateAvoidanceForce(const TArray<AActor*>& ActorsToAvoid);
    FVector CalculateAttractionForce(const TArray<AActor*>& ActorsToFollow);
    FVector CalculateLeaderFollowingForce();
    void HandleEmotionalContagion(float DeltaTime);
    bool ShouldChangeState(ECrowd_SocialState PotentialNewState);
};