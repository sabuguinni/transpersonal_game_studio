#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "Narr_StoryCheckpoint.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StoryCheckpoint : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StoryCheckpoint();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMesh;

    UFUNCTION()
    void OnCheckpointEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Story checkpoint properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString CheckpointID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase StoryPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsMainStoryCheckpoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bTriggerOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerRadius;

    // State tracking
    UPROPERTY(BlueprintReadOnly, Category = "Story")
    bool bHasBeenActivated;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    FDateTime ActivationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    int32 ActivationCount;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Story")
    void ActivateCheckpoint();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void ResetCheckpoint();

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool CanActivate() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AddObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsObjectiveCompleted(const FString& ObjectiveID) const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Story")
    void TestCheckpoint();

    UFUNCTION(CallInEditor, Category = "Story")
    void ShowCheckpointInfo();

private:
    void InitializeCheckpoint();
    void BroadcastStoryProgress();
    void UpdateGameState();
    void PlayCheckpointEffects();
};