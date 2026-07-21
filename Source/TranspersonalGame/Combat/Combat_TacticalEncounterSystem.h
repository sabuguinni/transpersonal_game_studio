#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "Combat_TacticalEncounterSystem.generated.h"

UENUM(BlueprintType)
enum class ECombat_EncounterState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Preparing       UMETA(DisplayName = "Preparing"),
    Active          UMETA(DisplayName = "Active"),
    Escalating      UMETA(DisplayName = "Escalating"),
    Concluding      UMETA(DisplayName = "Concluding"),
    Completed       UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class ECombat_TacticalRole : uint8
{
    PackLeader      UMETA(DisplayName = "Pack Leader"),
    Flanker         UMETA(DisplayName = "Flanker"),
    Ambusher        UMETA(DisplayName = "Ambusher"),
    Scout           UMETA(DisplayName = "Scout"),
    Defender        UMETA(DisplayName = "Defender"),
    Distraction     UMETA(DisplayName = "Distraction")
};

UENUM(BlueprintType)
enum class ECombat_FormationType : uint8
{
    VFormation      UMETA(DisplayName = "V Formation"),
    LineFormation   UMETA(DisplayName = "Line Formation"),
    PincerFormation UMETA(DisplayName = "Pincer Formation"),
    CircleFormation UMETA(DisplayName = "Circle Formation"),
    ScatterFormation UMETA(DisplayName = "Scatter Formation")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    ECombat_TacticalRole AssignedRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Position")
    bool bIsOccupied;

    FCombat_TacticalPosition()
    {
        Position = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        AssignedRole = ECombat_TacticalRole::Scout;
        Priority = 1.0f;
        bIsOccupied = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_EncounterParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 MinParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 MaxParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float EncounterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float DifficultyMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    ECombat_FormationType PreferredFormation;

    FCombat_EncounterParameters()
    {
        MinParticipants = 2;
        MaxParticipants = 5;
        EncounterRadius = 2000.0f;
        TriggerDistance = 1500.0f;
        DifficultyMultiplier = 1.0f;
        PreferredFormation = ECombat_FormationType::VFormation;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_TacticalEncounterSystem : public AActor
{
    GENERATED_BODY()

public:
    ACombat_TacticalEncounterSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* EncounterTrigger;

    // Encounter State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter State")
    ECombat_EncounterState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter State")
    FCombat_EncounterParameters EncounterParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter State")
    TArray<FCombat_TacticalPosition> TacticalPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter State")
    TArray<AActor*> ParticipatingActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter State")
    AActor* PrimaryTarget;

    // Timing and Control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float EncounterDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float StateTransitionTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float TacticalUpdateInterval;

    // AI Coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Coordination")
    bool bUseAdvancedTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Coordination")
    float CoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Coordination")
    float ThreatAssessmentRadius;

public:
    // Core Encounter Management
    UFUNCTION(BlueprintCallable, Category = "Encounter Management")
    void InitializeEncounter();

    UFUNCTION(BlueprintCallable, Category = "Encounter Management")
    void StartEncounter();

    UFUNCTION(BlueprintCallable, Category = "Encounter Management")
    void EndEncounter();

    UFUNCTION(BlueprintCallable, Category = "Encounter Management")
    void UpdateEncounterState();

    // Tactical Position Management
    UFUNCTION(BlueprintCallable, Category = "Tactical Management")
    void GenerateTacticalPositions();

    UFUNCTION(BlueprintCallable, Category = "Tactical Management")
    FCombat_TacticalPosition GetOptimalPosition(ECombat_TacticalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Tactical Management")
    void AssignTacticalRoles();

    UFUNCTION(BlueprintCallable, Category = "Tactical Management")
    void UpdateFormation(ECombat_FormationType NewFormation);

    // AI Coordination
    UFUNCTION(BlueprintCallable, Category = "AI Coordination")
    void CoordinateAIBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI Coordination")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = "AI Coordination")
    void BroadcastTacticalCommand(const FString& Command);

    // Event Handlers
    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsValidParticipant(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float CalculateEncounterDifficulty();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetOptimalTargetPosition();

protected:
    // Internal State Management
    void TransitionToState(ECombat_EncounterState NewState);
    void ProcessStateLogic();
    void UpdateTacticalPositions();
    void ValidateParticipants();
    void CalculateOptimalFormation();

    // Formation Generators
    void GenerateVFormation();
    void GenerateLineFormation();
    void GeneratePincerFormation();
    void GenerateCircleFormation();
    void GenerateScatterFormation();

private:
    float LastTacticalUpdate;
    float LastThreatAssessment;
    bool bEncounterActive;
    int32 ActiveParticipantCount;
};