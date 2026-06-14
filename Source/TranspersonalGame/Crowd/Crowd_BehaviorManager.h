#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Crowd_BehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle = 0,
    Wandering = 1,
    Following = 2,
    Gathering = 3,
    Fleeing = 4,
    Resting = 5,
    Alert = 6
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECrowd_BehaviorState DefaultBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 MaxOccupants;

    FCrowd_BehaviorZone()
    {
        ZoneName = TEXT("DefaultZone");
        DefaultBehavior = ECrowd_BehaviorState::Idle;
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        MaxOccupants = 50;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_BehaviorManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_BehaviorManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FCrowd_BehaviorZone> BehaviorZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance;

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    ECrowd_BehaviorState GetBehaviorForLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void TriggerGlobalAlert(const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddBehaviorZone(const FCrowd_BehaviorZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void RemoveBehaviorZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    TArray<AActor*> GetActorsInZone(const FString& ZoneName);

private:
    UPROPERTY()
    float LastBehaviorUpdate;

    UPROPERTY()
    TArray<AActor*> TrackedActors;

    void UpdateBehaviorStates();
    void ProcessZoneOccupancy();
    FCrowd_BehaviorZone* FindZoneByName(const FString& ZoneName);
};