#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Quest_QuestTrigger.generated.h"

class UQuest_QuestManager;

UENUM(BlueprintType)
enum class EQuest_TriggerType : uint8
{
    EnterArea           UMETA(DisplayName = "Enter Area"),
    KillTarget          UMETA(DisplayName = "Kill Target"),
    CollectItem         UMETA(DisplayName = "Collect Item"),
    InteractWithNPC     UMETA(DisplayName = "Interact With NPC"),
    SurviveTime         UMETA(DisplayName = "Survive Time"),
    AvoidDetection      UMETA(DisplayName = "Avoid Detection")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_TriggerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    EQuest_TriggerType TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    bool bIsRepeatable;

    FQuest_TriggerData()
    {
        ObjectiveID = TEXT("");
        TriggerType = EQuest_TriggerType::EnterArea;
        RequiredCount = 1;
        TargetTag = TEXT("");
        bIsRepeatable = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API AQuest_QuestTrigger : public AActor
{
    GENERATED_BODY()

public:
    AQuest_QuestTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    FQuest_TriggerData TriggerData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Trigger")
    bool bShowDebugMesh;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Trigger")
    int32 CurrentProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Quest Trigger")
    bool bIsTriggered;

public:
    virtual void Tick(float DeltaTime) override;

    // Trigger Functions
    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void ActivateTrigger();

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void DeactivateTrigger();

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool ProcessTrigger(AActor* TriggeringActor);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    void UpdateProgress(int32 ProgressIncrement = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest Trigger")
    bool IsObjectiveComplete() const;

    // Event Handlers
    UFUNCTION()
    void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Quest Trigger")
    FString GetObjectiveID() const { return TriggerData.ObjectiveID; }

    UFUNCTION(BlueprintPure, Category = "Quest Trigger")
    EQuest_TriggerType GetTriggerType() const { return TriggerData.TriggerType; }

    UFUNCTION(BlueprintPure, Category = "Quest Trigger")
    int32 GetRequiredCount() const { return TriggerData.RequiredCount; }

    UFUNCTION(BlueprintPure, Category = "Quest Trigger")
    int32 GetCurrentProgress() const { return CurrentProgress; }

protected:
    void InitializeVisualMesh();
    bool IsValidTriggeringActor(AActor* Actor) const;
    void NotifyQuestManager();

private:
    UPROPERTY()
    UQuest_QuestManager* QuestManager;

    UPROPERTY()
    TArray<AActor*> OverlappingActors;

    float SurvivalTimer;
    bool bIsSurvivalActive;
};