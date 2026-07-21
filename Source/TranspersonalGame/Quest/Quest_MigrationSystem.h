#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Quest_MigrationSystem.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class UCrowd_MigrationManager;

UENUM(BlueprintType)
enum class EQuest_MigrationPhase : uint8
{
    PreMigration,
    MigrationActive,
    MigrationPeak,
    PostMigration,
    Dormant
};

UENUM(BlueprintType)
enum class EQuest_MigrationQuestType : uint8
{
    FollowHerd,
    ProtectFromPredators,
    HuntDuringMigration,
    ScavengeAfterHunt,
    GuideHerdToSafety,
    AmbushPredators
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MigrationQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_MigrationQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float QuestRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RewardItems;

    FQuest_MigrationQuestData()
    {
        QuestName = TEXT("Migration Quest");
        QuestDescription = TEXT("Interact with the great migration");
        QuestType = EQuest_MigrationQuestType::FollowHerd;
        RequiredHerdSize = 5;
        QuestRadius = 2000.0f;
        RewardExperience = 100;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_MigrationTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_MigrationPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FQuest_MigrationQuestData QuestData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FQuest_MigrationTrigger()
    {
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        RequiredPhase = EQuest_MigrationPhase::MigrationActive;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_MigrationSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MigrationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DetectionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarkerMesh;

    // Migration Quest Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    EQuest_MigrationPhase CurrentMigrationPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    TArray<FQuest_MigrationTrigger> MigrationTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    float MigrationPhaseTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    float PhaseDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    int32 ActiveQuestCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration Quest")
    int32 MaxConcurrentQuests;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    void InitializeMigrationQuests();

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    void UpdateMigrationPhase(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    void CheckQuestTriggers(ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    bool StartMigrationQuest(const FQuest_MigrationQuestData& QuestData, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    void CompleteMigrationQuest(const FString& QuestName, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Migration Quest")
    TArray<FQuest_MigrationQuestData> GetAvailableQuests() const;

    // Migration Integration
    UFUNCTION(BlueprintCallable, Category = "Migration Integration")
    void OnMigrationEvent(const FString& EventType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Migration Integration")
    void SpawnQuestNPCs();

    UFUNCTION(BlueprintCallable, Category = "Migration Integration")
    void UpdateQuestMarkers();

    // Event Handlers
    UFUNCTION()
    void OnPlayerEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
    // Internal quest tracking
    UPROPERTY()
    TMap<FString, FQuest_MigrationQuestData> ActiveQuests;

    UPROPERTY()
    TArray<AActor*> QuestNPCs;

    UPROPERTY()
    UCrowd_MigrationManager* MigrationManager;

    // Helper functions
    void CreateQuestTrigger(const FVector& Location, float Radius, EQuest_MigrationQuestType QuestType);
    void ProcessMigrationPhaseChange();
    FQuest_MigrationQuestData GenerateContextualQuest(const FVector& Location, EQuest_MigrationPhase Phase);
};