#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "NPC_SocialSystem.generated.h"

UENUM(BlueprintType)
enum class ENPC_SocialRole : uint8
{
    None = 0,
    PackLeader = 1,
    Hunter = 2,
    Scout = 3,
    Defender = 4,
    Gatherer = 5,
    Elder = 6,
    Outcast = 7
};

UENUM(BlueprintType)
enum class ENPC_SocialStatus : uint8
{
    Unknown = 0,
    Friendly = 1,
    Neutral = 2,
    Suspicious = 3,
    Hostile = 4,
    Feared = 5,
    Respected = 6
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Social")
    TWeakObjectPtr<AActor> TargetActor;

    UPROPERTY(BlueprintReadOnly, Category = "Social")
    ENPC_SocialStatus Status = ENPC_SocialStatus::Unknown;

    UPROPERTY(BlueprintReadOnly, Category = "Social")
    float TrustLevel = 0.0f; // -1.0 to 1.0

    UPROPERTY(BlueprintReadOnly, Category = "Social")
    float LastInteractionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Social")
    int32 InteractionCount = 0;

    FNPC_SocialRelation()
    {
        TargetActor = nullptr;
        Status = ENPC_SocialStatus::Unknown;
        TrustLevel = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

USTRUCT(BlueprintType)
struct FNPC_PackData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TArray<TWeakObjectPtr<AActor>> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    TWeakObjectPtr<AActor> PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    FVector PackCenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    float PackRadius = 2000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    float PackCohesion = 1.0f; // 0.0 to 1.0

    UPROPERTY(BlueprintReadOnly, Category = "Pack")
    bool bIsActive = false;

    FNPC_PackData()
    {
        PackMembers.Empty();
        PackLeader = nullptr;
        PackCenterLocation = FVector::ZeroVector;
        PackRadius = 2000.0f;
        PackCohesion = 1.0f;
        bIsActive = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Social Role Management
    UFUNCTION(BlueprintCallable, Category = "Social System")
    void SetSocialRole(ENPC_SocialRole NewRole);

    UFUNCTION(BlueprintPure, Category = "Social System")
    ENPC_SocialRole GetSocialRole() const { return CurrentRole; }

    // Relationship Management
    UFUNCTION(BlueprintCallable, Category = "Social System")
    void UpdateRelationship(AActor* TargetActor, ENPC_SocialStatus NewStatus, float TrustChange);

    UFUNCTION(BlueprintPure, Category = "Social System")
    FNPC_SocialRelation GetRelationship(AActor* TargetActor) const;

    UFUNCTION(BlueprintPure, Category = "Social System")
    TArray<AActor*> GetActorsWithStatus(ENPC_SocialStatus Status) const;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack System")
    void JoinPack(const TArray<AActor*>& PackMembers, AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack System")
    void LeavePack();

    UFUNCTION(BlueprintPure, Category = "Pack System")
    bool IsInPack() const { return PackData.bIsActive; }

    UFUNCTION(BlueprintPure, Category = "Pack System")
    FNPC_PackData GetPackData() const { return PackData; }

    // Social Behavior
    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    bool ShouldFollowLeader() const;

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    bool ShouldDefendPackMember(AActor* Member) const;

    UFUNCTION(BlueprintCallable, Category = "Social Behavior")
    FVector GetPreferredPosition() const;

    // Communication
    UFUNCTION(BlueprintCallable, Category = "Communication")
    void SendSocialSignal(const FString& SignalType, AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintImplementableEvent, Category = "Communication")
    void OnSocialSignalReceived(const FString& SignalType, AActor* Sender);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Social System")
    ENPC_SocialRole CurrentRole = ENPC_SocialRole::None;

    UPROPERTY(BlueprintReadOnly, Category = "Social System")
    TArray<FNPC_SocialRelation> SocialRelations;

    UPROPERTY(BlueprintReadOnly, Category = "Pack System")
    FNPC_PackData PackData;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social Config")
    float SocialUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social Config")
    float MaxSocialDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social Config")
    int32 MaxTrackedRelations = 20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack Config")
    float PackFormationDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack Config")
    float LeaderFollowDistance = 800.0f;

private:
    float LastSocialUpdate = 0.0f;

    void UpdateSocialRelations();
    void UpdatePackCohesion();
    void CleanupOldRelations();
    FNPC_SocialRelation* FindOrCreateRelation(AActor* TargetActor);
    void BroadcastToPackMembers(const FString& Message);
};