#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "QuestTypes.h"
#include "Quest_InteractiveQuestMarker.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestMarkerActivated, class AQuest_InteractiveQuestMarker*, QuestMarker);

/**
 * Interactive quest marker that shows quest objectives visually in the world
 * Provides clear visual feedback for survival mission locations
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_InteractiveQuestMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_InteractiveQuestMarker();

protected:
    virtual void BeginPlay() override;

    // Visual components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Marker")
    USphereComponent* InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Marker")
    UStaticMeshComponent* MarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest Marker")
    UTextRenderComponent* QuestText;

    // Quest marker properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_MissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 QuestPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    // Visual feedback
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor ActiveColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor CompletedColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor InactiveColor;

public:
    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestMarkerActivated OnQuestMarkerActivated;

    // Public methods
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateQuestMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void DeactivateQuestMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuestMarker();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetQuestInfo(EQuest_MissionType InMissionType, const FString& InTitle, const FString& InDescription, int32 InPriority);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsPlayerInRange(class APawn* Player) const;

    UFUNCTION(BlueprintPure, Category = "Quest")
    FString GetQuestTypeString() const;

protected:
    // Interaction handling
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Visual updates
    void UpdateVisualState();
    void UpdateQuestText();

private:
    // Internal state
    bool bPlayerInRange;
    TWeakObjectPtr<APawn> CurrentPlayer;
};