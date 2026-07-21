#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Quest_SurvivalTutorialSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_TutorialPhase : uint8
{
    NotStarted,
    BasicMovement,
    ResourceGathering,
    CraftingIntroduction,
    ShelterBuilding,
    WaterLocation,
    DinosaurAwareness,
    SafetyProtocols,
    Completed
};

UENUM(BlueprintType)
enum class EQuest_ResourceType : uint8
{
    Stone,
    Wood,
    Plant,
    Water,
    Meat,
    Hide
};

USTRUCT(BlueprintType)
struct FQuest_TutorialObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString ObjectiveText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    EQuest_TutorialPhase Phase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FVector MarkerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float CompletionRadius;

    FQuest_TutorialObjective()
    {
        ObjectiveText = TEXT("");
        Phase = EQuest_TutorialPhase::NotStarted;
        bCompleted = false;
        MarkerLocation = FVector::ZeroVector;
        CompletionRadius = 200.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_ResourceRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EQuest_ResourceType ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 CurrentAmount;

    FQuest_ResourceRequirement()
    {
        ResourceType = EQuest_ResourceType::Stone;
        RequiredAmount = 1;
        CurrentAmount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_SurvivalTutorialSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_SurvivalTutorialSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorial();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void AdvanceToNextPhase();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void CompleteCurrentObjective();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool CheckObjectiveCompletion(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void AddResource(EQuest_ResourceType ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    bool HasRequiredResources() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    FString GetCurrentObjectiveText() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    EQuest_TutorialPhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    float GetTutorialProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void ShowHint(const FString& HintText);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void TriggerDangerWarning(const FString& DangerType);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    EQuest_TutorialPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FQuest_TutorialObjective> TutorialObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    TArray<FQuest_ResourceRequirement> ResourceRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bTutorialActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    float TutorialStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    int32 CurrentObjectiveIndex;

private:
    void InitializeTutorialObjectives();
    void SetupResourceRequirements();
    void UpdateObjectiveMarkers();
    void PlayTutorialAudio(const FString& AudioCue);
    void LogTutorialProgress();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_TutorialMarker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_TutorialMarker();

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void SetMarkerActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void SetMarkerType(EQuest_TutorialPhase Phase);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void OnPlayerEnterMarker();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    EQuest_TutorialPhase AssociatedPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
    FString MarkerDescription;

private:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};