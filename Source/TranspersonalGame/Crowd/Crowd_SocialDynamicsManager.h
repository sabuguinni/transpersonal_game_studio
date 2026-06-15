#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Crowd_SocialDynamicsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SocialGroup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> Members;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GroupName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_SocialStatus GroupStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GroupMorale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* GroupLeader;

    FCrowd_SocialGroup()
    {
        GroupName = TEXT("Unnamed Group");
        GroupStatus = ECrowd_SocialStatus::Neutral;
        CohesionRadius = 500.0f;
        GroupMorale = 50.0f;
        GroupLeader = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SocialInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Initiator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_SocialInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InteractionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FCrowd_SocialInteraction()
    {
        Initiator = nullptr;
        Target = nullptr;
        InteractionType = ECrowd_SocialInteractionType::Neutral;
        InteractionStrength = 1.0f;
        Duration = 5.0f;
        bIsActive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_SocialDynamicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_SocialDynamicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    TArray<FCrowd_SocialGroup> SocialGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    TArray<FCrowd_SocialInteraction> ActiveInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    float GroupFormationThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    float SocialInfluenceRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    bool bEnableDynamicGrouping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Dynamics")
    bool bEnableConflictResolution;

public:
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void CreateSocialGroup(const TArray<AActor*>& Members, const FString& GroupName);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void DisbandGroup(int32 GroupIndex);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void InitiateSocialInteraction(AActor* Initiator, AActor* Target, ECrowd_SocialInteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void UpdateGroupDynamics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void ProcessSocialInfluence(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    FCrowd_SocialGroup* FindGroupForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    TArray<AActor*> GetNearbyActors(AActor* CenterActor, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void HandleGroupConflict(FCrowd_SocialGroup& Group1, FCrowd_SocialGroup& Group2);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void UpdateGroupMorale(FCrowd_SocialGroup& Group, float MoraleChange);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void AssignGroupLeader(FCrowd_SocialGroup& Group);

private:
    void ProcessDynamicGrouping();
    void UpdateInteractions(float DeltaTime);
    void CleanupExpiredInteractions();
    float CalculateSocialCompatibility(AActor* Actor1, AActor* Actor2);
    void ApplySocialForces(AActor* Actor, const FCrowd_SocialGroup& Group);
};