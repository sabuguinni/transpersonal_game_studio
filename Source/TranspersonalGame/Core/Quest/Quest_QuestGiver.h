#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SharedTypes.h"
#include "Quest_QuestGiver.generated.h"

class UQuest_QuestManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestGiverData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
    FString GiverName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
    FString GiverDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
    TArray<FString> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
    FString GreetingDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
    FString QuestCompleteDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
    float InteractionRadius;

    FQuest_QuestGiverData()
    {
        GiverName = TEXT("Quest Giver");
        GiverDescription = TEXT("A helpful NPC with tasks");
        GreetingDialogue = TEXT("Greetings, traveler. I have work for you.");
        QuestCompleteDialogue = TEXT("Well done! Here is your reward.");
        bIsActive = true;
        InteractionRadius = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_QuestGiver : public AActor
{
    GENERATED_BODY()

public:
    AQuest_QuestGiver();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Giver")
    FQuest_QuestGiverData GiverData;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Giver")
    UQuest_QuestManager* QuestManager;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Giver")
    void InteractWithPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Giver")
    bool CanGiveQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Giver")
    void GiveQuestToPlayer(const FString& QuestID, AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest Giver")
    TArray<FString> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Giver")
    FString GetGreetingDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Giver")
    void SetActive(bool bNewActive);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, 
        AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bPlayerInRange;
    AActor* CurrentPlayer;

    void InitializeQuestGiver();
    void UpdateVisualState();
};