#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPCSocialRelationship.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_Relationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    ENPC_RelationshipType RelationshipType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Affinity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Trust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    int32 InteractionCount;

    FNPC_Relationship()
    {
        TargetActor = nullptr;
        RelationshipType = ENPC_RelationshipType::Neutral;
        Affinity = 0.0f;
        Trust = 0.0f;
        Fear = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialRelationship : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialRelationship();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Relationships")
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float RelationshipDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxRelationshipDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxRelationships;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Social")
    void AddRelationship(AActor* TargetActor, ENPC_RelationshipType Type);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void RemoveRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_Relationship GetRelationshipWith(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyAffinity(AActor* TargetActor, float AffinityChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyTrust(AActor* TargetActor, float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyFear(AActor* TargetActor, float FearChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void RecordInteraction(AActor* TargetActor, ENPC_InteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> GetFriends();

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> GetEnemies();

    UFUNCTION(BlueprintCallable, Category = "Social")
    AActor* GetClosestFriend();

    UFUNCTION(BlueprintCallable, Category = "Social")
    AActor* GetBiggestThreat();

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool IsHostileTowards(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool IsFriendlyTowards(AActor* TargetActor);

private:
    FNPC_Relationship* FindRelationship(AActor* TargetActor);
    void UpdateRelationshipTypes();
    void DecayRelationships(float DeltaTime);
};