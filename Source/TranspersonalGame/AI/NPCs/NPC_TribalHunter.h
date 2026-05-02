#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "AI/NPCBehaviorTypes.h"
#include "NPC_TribalHunter.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_TribalHunter : public ACharacter
{
    GENERATED_BODY()

public:
    ANPC_TribalHunter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE NPC PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Stats")
    float Stamina;

    // === BEHAVIOR PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_ActivityType CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_EmotionalState EmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FVector HomeLocation;

    // === SOCIAL PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    TArray<AActor*> KnownAllies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    float TrustLevel;

    // === AUDIO PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Audio")
    FString GreetingAudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Audio")
    FString AlertAudioURL;

    // === BEHAVIOR FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetActivity(ENPC_ActivityType NewActivity);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReactToThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void Patrol();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ReturnHome();

    // === SOCIAL FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void AddKnownAlly(AActor* Ally);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void AddKnownThreat(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    bool IsActorKnown(AActor* Actor);

private:
    // Internal state tracking
    float LastPatrolTime;
    float LastNeedsUpdate;
    FVector CurrentPatrolTarget;
    bool bIsPatrolling;
    bool bIsReturningHome;
};