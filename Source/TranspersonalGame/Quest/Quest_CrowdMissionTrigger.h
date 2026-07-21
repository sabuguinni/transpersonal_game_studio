#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerVolume.h"
#include "Components/BoxComponent.h"
#include "Quest_CrowdMissionTrigger.generated.h"

UENUM(BlueprintType)
enum class EQuest_CrowdMissionType : uint8
{
    NavigateCrowd       UMETA(DisplayName = "Navigate Through Crowd"),
    AvoidCrowdDanger    UMETA(DisplayName = "Avoid Crowd Stampede"),
    LeadCrowd           UMETA(DisplayName = "Lead Crowd to Safety"),
    EscortNPC           UMETA(DisplayName = "Escort NPC Through Crowd"),
    CrowdHunt           UMETA(DisplayName = "Hunt Target in Crowd"),
    CrowdDefense        UMETA(DisplayName = "Defend Against Crowd"),
    CrowdGathering      UMETA(DisplayName = "Gather Resources from Crowd Area")
};

USTRUCT(BlueprintType)
struct FQuest_CrowdMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    EQuest_CrowdMissionType MissionType = EQuest_CrowdMissionType::NavigateCrowd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionTitle = TEXT("Navigate the Crowd");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString MissionDescription = TEXT("Move through the crowd without causing panic");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 RequiredCrowdDensity = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float TimeLimit = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    int32 ExperienceReward = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bRequiresStealthMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    float CrowdPanicThreshold = 0.7f;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AQuest_CrowdMissionTrigger : public ATriggerVolume
{
    GENERATED_BODY()

public:
    AQuest_CrowdMissionTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* TriggerBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FQuest_CrowdMissionData MissionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bMissionActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bMissionCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CrowdDetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 CurrentCrowdCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CurrentCrowdPanicLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Mission")
    float MissionTimer = 0.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void StartMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void FailMission();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UpdateCrowdStatus();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetCrowdCountInRadius();

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    float CalculateCrowdPanicLevel();

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
    void OnMissionStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
    void OnMissionCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
    void OnMissionFailed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd")
    void OnCrowdPanicLevelChanged(float NewPanicLevel);

protected:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    void ProcessMissionLogic();
    void CheckMissionCompletion();
    bool ValidatePlayerBehavior();
};