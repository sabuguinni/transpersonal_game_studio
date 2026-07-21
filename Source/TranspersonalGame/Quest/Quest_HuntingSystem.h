#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Quest_HuntingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FString DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 CurrentKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float RewardMeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float RewardHide;

    FQuest_HuntTarget()
    {
        DinosaurSpecies = TEXT("Velociraptor");
        RequiredKills = 3;
        CurrentKills = 0;
        RewardMeat = 15.0f;
        RewardHide = 5.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_HuntingSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_HuntingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Quest")
    TArray<FQuest_HuntTarget> ActiveHuntTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Quest")
    bool bQuestActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Quest")
    float QuestRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Quest")
    FVector HuntingGrounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Quest")
    float RemainingTime;

    UFUNCTION(BlueprintCallable, Category = "Hunt Quest")
    void StartHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Hunt Quest")
    void RegisterDinosaurKill(const FString& Species);

    UFUNCTION(BlueprintCallable, Category = "Hunt Quest")
    void CompleteHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Hunt Quest")
    bool CheckQuestCompletion();

    UFUNCTION(BlueprintCallable, Category = "Hunt Quest")
    void SpawnHuntTargets();

    UFUNCTION(BlueprintCallable, Category = "Hunt Quest")
    void UpdateQuestTimer();

    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    FTimerHandle QuestTimerHandle;
    TArray<AActor*> SpawnedTargets;

public:
    virtual void Tick(float DeltaTime) override;
};